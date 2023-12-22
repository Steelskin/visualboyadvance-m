#include "gbEmulator.h"
#include <sys/stat.h>

#include <cassert>
#include <cstring>

#include "../NLS.h"
#include "../System.h"
#include "../common/Patch.h"
#include "gb.h"
#include "gbMemory.h"

namespace core {

gbEmulator* gbEmulator::instance_ = nullptr;

gbEmulator::gbEmulator(Client* const client) : client_(client) {
    assert(client);
}

bool gbEmulator::LoadRom(const uint8_t* data, size_t size) {
    std::memcpy(rom_.data(), data, size);
    if (!ParseRomHeader(size)) {
        return false;
    }

    return true;
}

bool gbEmulator::ApplyPatch(const char* patch_name) {
    if (!cart_data_) {
        return false;
    }

    int size = rom_.size();
    if (!applyPatchNoResize(patch_name, rom_.data(), &size)) {
        return false;
    }

    // We should re-parse the header every time in case a patch has changed the ROM header.
    return ParseRomHeader(size);
}

void gbEmulator::Reset() {
    if (!cart_data_) {
        return;
    }

    // Set the running emulator type.
    if (config_.emulator_type == GbEmulatorType::Automatic) {
        if (cart_data_->SupportsCGB()) {
            running_emulator_ = GbEmulatorType::Cgb;
        } else if (cart_data_->sgb_support()) {
            running_emulator_ = GbEmulatorType::Sgb;
        } else {
            running_emulator_ = GbEmulatorType::Dmg;
        }
    } else {
        running_emulator_ = config_.emulator_type;
    }

    // Set the SGB border mode.
    this->SetSgbBorderMode(config_.sgb_border_mode);
    client_->OnSgbBorderModeChanged(sgb_border_on_);

    // Mark the emulator as running.
    status_ = Status::kRunning;
}

void gbEmulator::Shutdown() {
    running_emulator_ = GbEmulatorType::Automatic;
    status_ = Status::kStopped;
    cart_data_.reset();
}

void gbEmulator::SetSgbBorderMode(SgbBorderMode mode) {
    const bool current_mode = sgb_border_on_;

    config_.sgb_border_mode = mode;
    switch (mode) {
        case SgbBorderMode::AlwaysOff:
            sgb_border_on_ = false;
            break;
        case SgbBorderMode::AlwaysOn:
            sgb_border_on_ = true;
            break;
        case SgbBorderMode::Automatic:
            sgb_border_on_ = HasSgbHw();
            break;
    }

    if (status_ == Status::kRunning && current_mode != sgb_border_on_) {
        client_->OnSgbBorderModeChanged(sgb_border_on_);
    }
}

bool gbEmulator::HasCgbHw() const {
    switch (running_emulator_) {
        case GbEmulatorType::Automatic:
        case GbEmulatorType::Dmg:
        case GbEmulatorType::Cgb:
        case GbEmulatorType::Agb:
        case GbEmulatorType::AgbSp:
            return false;
        case GbEmulatorType::Sgb:
        case GbEmulatorType::Sgb2:
            return true;
    }
}

bool gbEmulator::HasSgbHw() const {
    switch (running_emulator_) {
        case GbEmulatorType::Automatic:
        case GbEmulatorType::Dmg:
        case GbEmulatorType::Cgb:
        case GbEmulatorType::Agb:
        case GbEmulatorType::AgbSp:
            return false;
        case GbEmulatorType::Sgb:
        case GbEmulatorType::Sgb2:
            return true;
    }
}

bool gbEmulator::ParseRomHeader(size_t size) {
    cart_data_ = gbCartData(rom_.data(), size);

    if (!cart_data_) {
        switch (cart_data_->validity()) {
            case gbCartData::Validity::kValid:
            case gbCartData::Validity::kUninitialized:
                // Unreachable.
                assert(false);
                break;
            case gbCartData::Validity::kSizeTooSmall:
                systemMessage(MSG_UNSUPPORTED_ROM_SIZE, N_("Unsupported rom size %02x"), size);
                break;
            case gbCartData::Validity::kUnknownMapperType:
                systemMessage(MSG_UNKNOWN_CARTRIDGE_TYPE, N_("Unknown cartridge type %02x"),
                              cart_data_->mapper_flag());
                break;
            case gbCartData::Validity::kUnknownRomSize:
                systemMessage(MSG_UNSUPPORTED_ROM_SIZE, N_("Unsupported rom size %02x"),
                              cart_data_->rom_flag());
                break;
            case gbCartData::Validity::kUnknownRamSize:
                systemMessage(MSG_UNSUPPORTED_RAM_SIZE, N_("Unsupported ram size %02x"),
                              cart_data_->ram_flag());
                break;
            case gbCartData::Validity::kNoNintendoLogo:
                systemMessage(MSG_INVALID_GAME_BOY_NINTENDO_LOGO, N_("No Nintendo logo in header"));
                break;
            case gbCartData::Validity::kInvalidHeaderChecksum:
                systemMessage(MSG_INVALID_HEADER_CHECKSUM,
                              N_("Invalid header checksum. Found: %02x. Expected: %02x"),
                              cart_data_->header_checksum(), cart_data_->actual_header_checksum());
                break;
        }
        cart_data_ = nonstd::nullopt;
        return false;
    }

    // We ignore romSize > romHeaderSize here, for backwards compatibility. This
    // is necessary for some ROM hacks.
    const size_t rom_header_size = cart_data_->rom_size();
    if (size < rom_header_size) {
        // Not sure if it's 0x00, 0xff or random data.
        std::fill(rom_.data() + size, rom_.data() + rom_header_size, (uint8_t)0);
    }

    // Override for compatibility.
    rom_[0x147] = cart_data_->mapper_flag();

    // The initial RAM byte value.
    uint8_t gbRamFill = 0xff;

    switch (cart_data_->mapper_type()) {
        case gbCartData::MapperType::kNone:
        case gbCartData::MapperType::kMbc1:
            g_mapper = mapperMBC1ROM;
            g_mapperRAM = mapperMBC1RAM;
            g_mapperReadRAM = mapperMBC1ReadRAM;
            break;
        case gbCartData::MapperType::kMbc2:
            g_mapper = mapperMBC2ROM;
            g_mapperRAM = mapperMBC2RAM;
            break;
        case gbCartData::MapperType::kMmm01:
            g_mapper = mapperMMM01ROM;
            g_mapperRAM = mapperMMM01RAM;
            break;
        case gbCartData::MapperType::kMbc3:
        case gbCartData::MapperType::kPocketCamera:
            g_mapper = mapperMBC3ROM;
            g_mapperRAM = mapperMBC3RAM;
            g_mapperReadRAM = mapperMBC3ReadRAM;
            break;
        case gbCartData::MapperType::kMbc5:
            g_mapper = mapperMBC5ROM;
            g_mapperRAM = mapperMBC5RAM;
            g_mapperReadRAM = mapperMBC5ReadRAM;
            break;
        case gbCartData::MapperType::kMbc7:
            g_mapper = mapperMBC7ROM;
            g_mapperRAM = mapperMBC7RAM;
            g_mapperReadRAM = mapperMBC7ReadRAM;
            break;
        case gbCartData::MapperType::kGameGenie:
            // Clean-up Game Genie hardware registers.
            for (size_t i = 0; i < 0x20; i++) {
                rom_[0x4000 + i] = 0;
            }
            g_mapper = mapperGGROM;
            break;
        case gbCartData::MapperType::kGameShark:
            g_mapper = mapperGS3ROM;
            break;
        case gbCartData::MapperType::kTama5:
            gbRamFill = 0x00;
            g_mapperRAM = mapperTAMA5RAM;
            g_mapperReadRAM = mapperTAMA5ReadRAM;
            g_mapperUpdateClock = memoryUpdateTAMA5Clock;
            break;
        case gbCartData::MapperType::kHuC3:
            g_mapper = mapperHuC3ROM;
            g_mapperRAM = mapperHuC3RAM;
            g_mapperReadRAM = mapperHuC3ReadRAM;
            break;
        case gbCartData::MapperType::kHuC1:
            g_mapper = mapperHuC1ROM;
            g_mapperRAM = mapperHuC1RAM;
            break;
        default:
            systemMessage(MSG_UNKNOWN_CARTRIDGE_TYPE, N_("Unknown cartridge type"));
            return false;
    }

    const size_t ramSize = cart_data_->ram_size();
    if (cart_data_->HasRam()) {
        std::memset(ram_.data(), gbRamFill, ramSize);
    }

    gbGenFilter();
    gbSgbInit();
    setColorizerHack(false);

#if !defined(__LIBRETRO__)

    // Populate the IO vectors for battery save/load.
    g_vbamIoVecs.clear();
    if (cart_data_->has_battery()) {
        if (cart_data_->HasRam()) {
            g_vbamIoVecs.push_back({GB_EMULATOR->ram(), ramSize});
        }

        switch (cart_data_->mapper_type()) {
            case gbCartData::MapperType::kMbc3:
                if (cart_data_->has_rtc()) {
                    g_vbamIoVecs.push_back(
                        {&gbDataMBC3.mapperSeconds, MBC3_RTC_DATA_SIZE, -4, &ResetMBC3RTC});
                }
                break;
            case gbCartData::MapperType::kTama5:
                g_vbamIoVecs.push_back({GB_EMULATOR->tama5_ram(), kTama5RamSize});
                g_vbamIoVecs.push_back(
                    {&gbDataTAMA5.mapperSeconds, TAMA5_RTC_DATA_SIZE, -4, &ResetTama5RTC});
                break;
            case gbCartData::MapperType::kHuC3:
                g_vbamIoVecs.push_back(
                    {&gbRTCHuC3.mapperLastTime, HUC3_RTC_DATA_SIZE, -4, &ResetHuc3RTC});
                break;
            case gbCartData::MapperType::kNone:
            case gbCartData::MapperType::kMbc1:
            case gbCartData::MapperType::kMbc2:
            case gbCartData::MapperType::kMbc5:
            case gbCartData::MapperType::kMbc6:
            case gbCartData::MapperType::kMbc7:
            case gbCartData::MapperType::kHuC1:
            case gbCartData::MapperType::kMmm01:
            case gbCartData::MapperType::kPocketCamera:
            case gbCartData::MapperType::kGameGenie:
            case gbCartData::MapperType::kGameShark:
            case gbCartData::MapperType::kUnknown:
                // Do nothing more.
                break;
        }
    }

#endif  // !defined(__LIBRETRO__)

    return true;
}
}  // namespace core
