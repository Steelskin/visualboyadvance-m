#ifndef VBAM_GB_GB_EMULATOR_H_
#define VBAM_GB_GB_EMULATOR_H_

#include <array>
#include <cstdint>

#include "../common/sizes.h"
#include "gbCartData.h"
#include "nonstd/optional.hpp"

namespace core {

// One of the possible Game Boy family emulator types.
enum class GbEmulatorType {
    // Detect automatically.
    Automatic = 0,
    // Force DMG mode.
    Dmg,
    // Force SGB mode.
    Sgb,
    // Force SGB2 mode.
    Sgb2,
    // Force CGB mode.
    Cgb,
    // Force AGB mode.
    Agb,
    // Force AGB SP mode.
    AgbSp,
};

// One of the possibles mode for the SGB border display.
enum class SgbBorderMode {
    AlwaysOff = 0,
    AlwaysOn,
    Automatic,
};

// POD structure holding the Game Boy family emulator configuration.
struct gbEmulatorConfig final {
    GbEmulatorType emulator_type = GbEmulatorType::Automatic;
    SgbBorderMode sgb_border_mode = SgbBorderMode::Automatic;
};

// This class will eventually handle all of the GB emulator and will be moved to a public header,
// replacing the `system*()` functions. For now, it holds some data that is used by the GB core.
class gbEmulator final {
public:
    class Client {
    public:
        virtual ~Client() = default;

        // Called when the SGB border display state changes.
        virtual void OnSgbBorderModeChanged(bool sgb_border_on) = 0;
    };

    enum class Status {
        // The emulator is not running.
        kStopped,
        // The emulator is running.
        kRunning,
    };

    // Global getters and setters for the gbEmulator instance. These will eventually be removed.
    static void Set(gbEmulator* instance) { instance_ = instance; }
    static gbEmulator* Get() { return instance_; }

    explicit gbEmulator(Client* client);
    ~gbEmulator() = default;

    // Loads a ROM from a buffer. Returns true on success.
    bool LoadRom(const uint8_t* data, size_t size);

    bool ApplyPatch(const char* patch_name);

    // Resets the emulator.
    void Reset();

    // Shuts down the emulator.
    void Shutdown();

    // Sets the SGB Border Mode.
    void SetSgbBorderMode(SgbBorderMode mode);

    // Accessors.
    const gbEmulatorConfig& config() const { return config_; }
    Status status() const { return status_; }
    bool sgb_border_on() const { return sgb_border_on_; }
    GbEmulatorType running_emulator() const { return running_emulator_; }
    const nonstd::optional<gbCartData>& cart_data() const { return cart_data_; }

    // Returns true if the CGB hardware is present.
    bool HasCgbHw() const;

    // Returns true if the SGB hardware is present.
    bool HasSgbHw() const;

    // Returns true if the AGB hardware is present.
    bool HasAgbHw() const;

    // Returns true if the emulator is running in DMG mode.
    bool IsDmg() const { return running_emulator_ == GbEmulatorType::Dmg; }

    // Returns true if the emulator is running in SGB2 mode.
    bool IsSgb2() const { return running_emulator_ == GbEmulatorType::Sgb2; }

    // Returns true if the emulator is running in CGB mode.
    bool IsCgb() const { return running_emulator_ == GbEmulatorType::Cgb; }

    // Returns true if the emulator is running in AGB mode.
    bool IsAgb() const { return running_emulator_ == GbEmulatorType::Agb; }

    // Returns true if the emulator is running in AGB SP mode.
    bool IsAgbSp() const { return running_emulator_ == GbEmulatorType::AgbSp; }

    // Accessors for the various memory banks. These should eventually be moved to const.
    std::array<uint8_t, k8MiB>& rom() { return rom_; }
    std::array<uint8_t, k128KiB>& ram() { return ram_; }
    std::array<uint8_t, kGBVRamSize>& vram() { return vram_; }
    std::array<uint8_t, kGBWRamSize>& wram() { return wram_; }
    std::array<uint8_t, kGBMemorySize>& memory() { return memory_; }
    std::array<uint16_t, kGBLineBufferSize>& line_buffer() { return line_buffer_; }
    std::array<uint8_t, kTama5RamSize>& tama5_ram() { return tama5_ram_; }

    // Returns the `gbHarware` value for compatibility.
    int gbHardware() const;

private:
    bool ParseRomHeader(size_t size);

    // The configuration.
    gbEmulatorConfig config_;

    Status status_ = Status::kStopped;

    // Set to true if the SGB border should be rendered.
    bool sgb_border_on_ = false;

    // The active emulator type. This is the emulator type that is currently being used while the
    // emulator is running.
    GbEmulatorType running_emulator_ = GbEmulatorType::Automatic;

    // The loaded cartridge data.
    nonstd::optional<gbCartData> cart_data_;

    // Various data banks.
    std::array<uint8_t, k8MiB> rom_;
    std::array<uint8_t, k128KiB> ram_;
    std::array<uint8_t, kGBVRamSize> vram_;
    std::array<uint8_t, kGBWRamSize> wram_;
    std::array<uint8_t, kGBMemorySize> memory_;
    std::array<uint16_t, kGBLineBufferSize> line_buffer_;
    std::array<uint8_t, kTama5RamSize> tama5_ram_;

    // The client object.
    Client *const client_;

    // The global emulator instance. This will eventually go away.
    static gbEmulator* instance_;
};

}  // namespace core

// TODO: Remove these when they are no longer needed.
#define GB_EMULATOR core::gbEmulator::Get()
#define GB_CART_DATA GB_EMULATOR->cart_data()

#endif  // VBAM_GB_GB_EMULATOR_H_
