#include "dialogs/gb-rom-info.h"

#include <wx/control.h>
#include <wx/xrc/xmlres.h>

#include "../common/sizes.h"
#include "../gb/gb.h"
#include "dialogs/game-maker.h"
#include "dialogs/validated-child.h"

namespace dialogs {

namespace {

// Returns a localized string indicating the cartridge type (mapper) for the loaded GB/GBC
// cartridge.
wxString GetCartType() {
    wxString mapper_type;
    switch (GB_CART_DATA->mapper_type()) {
        case gbCartData::MapperType::kNone:
            mapper_type = _("No mapper");
            break;
        case gbCartData::MapperType::kMbc1:
            mapper_type = "MBC1";
            break;
        case gbCartData::MapperType::kMbc2:
            mapper_type = "MBC2";
            break;
        case gbCartData::MapperType::kMbc3:
            mapper_type = "MBC3";
            break;
        case gbCartData::MapperType::kMbc5:
            mapper_type = "MBC5";
            break;
        case gbCartData::MapperType::kMbc6:
            mapper_type = "MBC6";
            break;
        case gbCartData::MapperType::kMbc7:
            mapper_type = "MBC7";
            break;
        case gbCartData::MapperType::kPocketCamera:
            mapper_type = _("Pocket Camera");
            break;
        case gbCartData::MapperType::kMmm01:
            mapper_type = "MMM01";
            break;
        case gbCartData::MapperType::kHuC1:
            mapper_type = "HuC-1";
            break;
        case gbCartData::MapperType::kHuC3:
            mapper_type = "HuC-3";
            break;
        case gbCartData::MapperType::kTama5:
            mapper_type = "Bandai TAMA5";
            break;
        case gbCartData::MapperType::kGameGenie:
            mapper_type = "Game Genie";
            break;
        case gbCartData::MapperType::kGameShark:
            mapper_type = "Game Shark";
            break;
        case gbCartData::MapperType::kUnknown:
            mapper_type = _("Unknown");
            break;
    }

    const wxString has_ram = GB_CART_DATA->HasRam() ? _(" + RAM") : "";
    const wxString has_rtc = GB_CART_DATA->has_rtc() ? _(" + RTC") : "";
    const wxString has_battery = GB_CART_DATA->has_battery() ? _(" + Battery") : "";
    const wxString has_rumble = GB_CART_DATA->has_rumble() ? _(" + Rumble") : "";
    const wxString has_motion = GB_CART_DATA->has_sensor() ? _(" + Motion Sensor") : "";

    return wxString::Format(_("%02X (%s%s%s%s%s%s)"), GB_CART_DATA->mapper_flag(), mapper_type,
                            has_ram, has_rtc, has_battery, has_rumble, has_motion);
}

// Returns a localized string indicating SGB support for the loaded GB/GBC cartridge.
wxString GetCartSGBFlag() {
    if (GB_CART_DATA->sgb_support()) {
        return wxString::Format(_("%02X (Supported)"), GB_CART_DATA->sgb_flag());
    } else {
        return wxString::Format(_("%02X (Not supported)"), GB_CART_DATA->sgb_flag());
    }
}

// Returns a localized string indicating CGB support for the loaded GB/GBC cartridge.
wxString GetCartCGBFlag() {
    switch (GB_CART_DATA->cgb_support()) {
        case gbCartData::CGBSupport::kNone:
            return wxString::Format(_("%02X (Not supported)"), GB_CART_DATA->cgb_flag());
        case gbCartData::CGBSupport::kSupported:
            return wxString::Format(_("%02X (Supported)"), GB_CART_DATA->cgb_flag());
        case gbCartData::CGBSupport::kRequired:
            return wxString::Format(_("%02X (Required)"), GB_CART_DATA->cgb_flag());
        default:
            // Unreachable.
            assert(false);
            return "";
    }
}

// Returns a localized string indicating the ROM size of the loaded GB/GBC cartridge.
wxString GetCartRomSize() {
    switch (GB_CART_DATA->rom_size()) {
        case k32KiB:
            return wxString::Format(_("%02X (32 KiB)"), GB_CART_DATA->rom_flag());
        case k64KiB:
            return wxString::Format(_("%02X (64 KiB)"), GB_CART_DATA->rom_flag());
        case k128KiB:
            return wxString::Format(_("%02X (128 KiB)"), GB_CART_DATA->rom_flag());
        case k256KiB:
            return wxString::Format(_("%02X (256 KiB)"), GB_CART_DATA->rom_flag());
        case k512KiB:
            return wxString::Format(_("%02X (512 KiB)"), GB_CART_DATA->rom_flag());
        case k1MiB:
            return wxString::Format(_("%02X (1 MiB)"), GB_CART_DATA->rom_flag());
        case k2MiB:
            return wxString::Format(_("%02X (2 MiB)"), GB_CART_DATA->rom_flag());
        case k4MiB:
            return wxString::Format(_("%02X (4 MiB)"), GB_CART_DATA->rom_flag());
        default:
            return wxString::Format(_("%02X (Unknown)"), GB_CART_DATA->rom_flag());
    }
}

// Returns a localized string indicating the ROM size of the loaded GB/GBC cartridge.
wxString GetCartRamSize() {
    switch (GB_CART_DATA->ram_size()) {
        case 0:
            return wxString::Format(_("%02X (None)"), GB_CART_DATA->ram_flag());
        case k256B:
            return wxString::Format(_("%02X (256 B)"), GB_CART_DATA->ram_flag());
        case k512B:
            return wxString::Format(_("%02X (512 B)"), GB_CART_DATA->ram_flag());
        case k2KiB:
            return wxString::Format(_("%02X (2 KiB)"), GB_CART_DATA->ram_flag());
        case k8KiB:
            return wxString::Format(_("%02X (8 KiB)"), GB_CART_DATA->ram_flag());
        case k32KiB:
            return wxString::Format(_("%02X (32 KiB)"), GB_CART_DATA->ram_flag());
        case k128KiB:
            return wxString::Format(_("%02X (128 KiB)"), GB_CART_DATA->ram_flag());
        case k64KiB:
            return wxString::Format(_("%02X (64 KiB)"), GB_CART_DATA->ram_flag());
        default:
            return wxString::Format(_("%02X (Unknown)"), GB_CART_DATA->ram_flag());
    }
}

// Returns a localized string indicating the destination code of the loaded GB/GBC cartridge.
wxString GetCartDestinationCode() {
    switch (GB_CART_DATA->destination_code()) {
        case gbCartData::DestinationCode::kJapanese:
            return wxString::Format(_("%02X (Japan)"), GB_CART_DATA->destination_code_flag());
        case gbCartData::DestinationCode::kWorldwide:
            return wxString::Format(_("%02X (World)"), GB_CART_DATA->destination_code_flag());
        case gbCartData::DestinationCode::kUnknown:
            return wxString::Format(_("%02X (Unknown)"), GB_CART_DATA->destination_code_flag());
        default:
            // Unreachable.
            assert(false);
            return "";
    }
}

}  // namespace

// static
GbRomInfo* GbRomInfo::NewInstance(wxWindow* parent) {
    assert(parent);
    return new GbRomInfo(parent);
}

GbRomInfo::GbRomInfo(wxWindow* parent) : wxDialog(), keep_on_top_styler_(this) {
#if !wxCHECK_VERSION(3, 1, 0)
    // This needs to be set before loading any element on the window. This also
    // has no effect since wx 3.1.0, where it became the default.
    this->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
#endif
    wxXmlResource::Get()->LoadDialog(this, parent, "GBROMInfo");

    Bind(wxEVT_SHOW, &GbRomInfo::OnDialogShowEvent, this);
}

void GbRomInfo::OnDialogShowEvent(wxShowEvent& event) {
    // Let the event propagate.
    event.Skip();

    if (!event.IsShown()) {
        return;
    }

    // Populate the dialog.
    GetValidatedChild(this, "Title")->SetLabel(GB_CART_DATA->title());
    GetValidatedChild(this, "MakerCode")->SetLabel(GB_CART_DATA->maker_code());
    GetValidatedChild(this, "MakerName")->SetLabel(GetGameMakerName(GB_CART_DATA->maker_code()));
    GetValidatedChild(this, "CartridgeType")->SetLabel(GetCartType());
    GetValidatedChild(this, "SGBCode")->SetLabel(GetCartSGBFlag());
    GetValidatedChild(this, "CGBCode")->SetLabel(GetCartCGBFlag());
    GetValidatedChild(this, "ROMSize")->SetLabel(GetCartRomSize());
    GetValidatedChild(this, "RAMSize")->SetLabel(GetCartRamSize());
    GetValidatedChild(this, "DestCode")->SetLabel(GetCartDestinationCode());
    GetValidatedChild(this, "LicCode")
        ->SetLabel(wxString::Format("%02X", GB_CART_DATA->old_licensee_code()));
    GetValidatedChild(this, "Version")
        ->SetLabel(wxString::Format("%02X", GB_CART_DATA->version_flag()));
    GetValidatedChild(this, "HeaderChecksum")
        ->SetLabel(wxString::Format(_("%02X (Actual: %02X)"), GB_CART_DATA->header_checksum(),
                                    GB_CART_DATA->actual_header_checksum()));
    GetValidatedChild(this, "CartridgeChecksum")
        ->SetLabel(wxString::Format(_("%04X (Actual: %04X)"), GB_CART_DATA->global_checksum(),
                                    GB_CART_DATA->actual_global_checksum()));

    // Re-fit everything.
    Fit();
}

}  // namespace dialogs
