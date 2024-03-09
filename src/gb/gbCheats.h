#ifndef VBAM_GB_GBCHEATS_H_
#define VBAM_GB_GBCHEATS_H_

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <zlib.h>

#include "nonstd/optional.hpp"

enum class gbCheatType {
    GameShark,
    GameGenie,
};

class gbCheatManager final {
public:
    static gbCheatManager& instance();

    static bool isCodeValid(const std::string& code, gbCheatType type);

    // Not copyable or movable.
    gbCheatManager(const gbCheatManager&) = delete;
    gbCheatManager& operator=(const gbCheatManager&) = delete;
    gbCheatManager(gbCheatManager&&) = delete;
    gbCheatManager& operator=(gbCheatManager&&) = delete;

    void resetCheats();

    bool addCheat(const std::string& code, gbCheatType type);
    bool removeCheat(const std::string& code, gbCheatType type);

    // Should only be accessible by the core emulator.
    nonstd::optional<uint8_t> readAddressWithCheats(const uint8_t* const memory_map[16],
                                                    uint16_t address) const;
    void applyCheatsForReset() const;
    void applyCheatsForInterrupt() const;

private:
    // Private constructor and destructor to prevent instantiation.
    gbCheatManager() = default;
    ~gbCheatManager() = default;

    // The internal representation of a Game Boy series cheat.
    struct gbCheat {
        // The adress to read from or write to.
        uint16_t address;
        // The value to compare to for GameGenie codes.
        nonstd::optional<uint8_t> compare;
        // The SRAM bank to use for GameShark codes.
        nonstd::optional<uint8_t> sram_bank;
        // The value to read from or write to `address`.
        uint8_t value;

        bool operator==(const gbCheat& other) const {
            return address == other.address && compare == other.compare &&
                   sram_bank == other.sram_bank && value == other.value;
        }
    };

    struct gbCheatHasher {
        std::size_t operator()(const gbCheat& cheat) const {
            size_t hash = std::hash<uint16_t>{}(cheat.address) ^ std::hash<uint8_t>{}(cheat.value);
            if (cheat.compare.has_value()) {
                hash ^= std::hash<uint8_t>{}(cheat.compare.value());
            }
            if (cheat.sram_bank.has_value()) {
                hash ^= std::hash<uint8_t>{}(cheat.sram_bank.value());
            }
            return hash;
        }
    };

    static nonstd::optional<gbCheat> parseCheat(const std::string& code, gbCheatType type);

    std::unordered_map<uint16_t, std::unordered_set<gbCheat, gbCheatHasher>> cheat_address_map_;
    std::unordered_set<gbCheat, gbCheatHasher> cheats_for_interrupt_;
    std::unordered_set<gbCheat, gbCheatHasher> cheats_for_reset_;
};

#define GB_CHEAT_MANAGER gbCheatManager::instance()

// Helper delegate to save and load cheats to/from a file for save states.
class gbCheatSaveStateDelegate {
public:
    virtual ~gbCheatSaveStateDelegate() = default;
    virtual void SaveCheatsToFile(gzFile file) = 0;
    virtual bool LoadCheatsFromFile(gzFile file, bool activate) = 0;
};
extern gbCheatSaveStateDelegate* g_gbSaveStateDelegate;

// // Internal emulator read/write address.
// uint8_t gbCheatRead(uint16_t);
// void gbCheatWrite(bool);

// struct gbCheat {
//     char cheatCode[20];
//     char cheatDesc[32];
//     uint16_t address;
//     int sram_bank;
//     uint8_t compare;
//     uint8_t value;
//     bool enabled;
// };

// #ifndef __LIBRETRO__
// void gbCheatsSaveGame(gzFile);
// void gbCheatsReadGame(gzFile, int);
// void gbCheatsReadGameSkip(gzFile, int);
// #endif
// void gbCheatsSaveCheatList(const char*);
// bool gbCheatsLoadCheatList(const char*);
// bool gbCheatReadGSCodeFile(const char*);

// bool gbAddGsCheat(const char*, const char*);
// bool gbAddGgCheat(const char*, const char*);
// void gbCheatRemove(int);
// void gbCheatRemoveAll();
// void gbCheatEnable(int);
// void gbCheatDisable(int);

// bool gbVerifyGsCode(const char* code);
// bool gbVerifyGgCode(const char* code);

// extern int gbCheatNumber;
// extern gbCheat gbCheatList[MAX_CHEATS];
// extern bool gbCheatMap[0x10000];

#endif  // VBAM_GB_GBCHEATS_H_
