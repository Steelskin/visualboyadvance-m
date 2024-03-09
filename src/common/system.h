#ifndef VBAM_COMMON_SYSTEM_H_
#define VBAM_COMMON_SYSTEM_H_

// One of the emulated systems.
enum class VbamSystem {
    // The Game Boy family (DMG/SGB/CGB).
    GameBoy = 0,
    // The Game Boy Advance family (AGB).
    GameBoyAdvance,
};

// Helper object to provide a system.
class VbamSystemProvider {
public:
    virtual ~VbamSystemProvider() = default;
    virtual VbamSystem GetSystem() = 0;
};

#endif  // VBAM_COMMON_SYSTEM_H_
