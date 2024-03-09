#ifndef VBAM_GBA_CHEATS_H_
#define VBAM_GBA_CHEATS_H_

#include <cstdint>
#include <string>

#include "../System.h"

#include "nonstd/optional.hpp"
#include <unordered_map>
#include <unordered_set>

enum class gbaCheatType {
    Generic,
    GameShark,
    ActionReplayV3,
    CodeBreaker,
    Flashcart,
};

class gbaCheatManager final {
public:
    static gbaCheatManager& instance();

    static bool isCodeValid(const std::vector<std::string>& codes, gbaCheatType type);

    // Not copyable or movable.
    gbaCheatManager(const gbaCheatManager&) = delete;
    gbaCheatManager& operator=(const gbaCheatManager&) = delete;
    gbaCheatManager(gbaCheatManager&&) = delete;
    gbaCheatManager& operator=(gbaCheatManager&&) = delete;

    void resetCheats();

    bool addCheat(const std::vector<std::string>& codes, gbaCheatType type);
    bool removeCheat(const std::vector<std::string>& codes, gbaCheatType type);

private:
    // Private constructor and destructor to prevent instantiation.
    gbaCheatManager() = default;
    ~gbaCheatManager() = default;

    // The internal representation of a Game Boy series cheat.
    struct Cheat {
        // The adress to read from or write to.
        uint16_t address;
        // The value to compare to for GameGenie codes.
        nonstd::optional<uint8_t> compare;
        // The SRAM bank to use for GameShark codes.
        nonstd::optional<uint8_t> sram_bank;
        // The value to read from or write to `address`.
        uint8_t value;

        bool operator==(const Cheat& other) const {
            return address == other.address && compare == other.compare &&
                   sram_bank == other.sram_bank && value == other.value;
        }
    };

    struct CheatHasher {
        std::size_t operator()(const Cheat& cheat) const {
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

    static nonstd::optional<Cheat> parseCheat(const std::string& code, gbaCheatType type);

    std::unordered_map<uint16_t, std::unordered_set<Cheat, CheatHasher>> cheat_address_map_;
    std::unordered_set<Cheat, CheatHasher> cheats_for_interrupt_;
    std::unordered_set<Cheat, CheatHasher> cheats_for_reset_;

    Cheat deadface_v2_;
    Cheat deadface_v3_;
};

#define GBA_CHEAT_MANAGER gbaCheatManager::instance()

struct CheatsData {
    int code;
    int size;
    int status;
    bool enabled;
    uint32_t rawaddress;
    uint32_t address;
    uint32_t value;
    uint32_t oldValue;
    char codestring[20];
    char desc[32];
};

void cheatsAddCheatCode(const char* code, const char* desc);
void cheatsAddGSACode(const char* code, const char* desc, bool v3);
void cheatsAddCBACode(const char* code, const char* desc);
bool cheatsImportGSACodeFile(const char* name, int game, bool v3);
void cheatsDelete(int number, bool restore);
void cheatsDeleteAll(bool restore);
void cheatsEnable(int number);
void cheatsDisable(int number);
#ifndef __LIBRETRO__
void cheatsSaveGame(gzFile file);
void cheatsReadGame(gzFile file, int version);
void cheatsReadGameSkip(gzFile file, int version);
void cheatsSaveCheatList(const char* file);
bool cheatsLoadCheatList(const char* file);
#endif
#ifdef BKPT_SUPPORT
void cheatsWriteMemory(uint32_t address, uint32_t value);
void cheatsWriteHalfWord(uint32_t address, uint16_t value);
void cheatsWriteByte(uint32_t address, uint8_t value);
#endif
int cheatsCheckKeys(uint32_t keys, uint32_t extended);

extern int cheatsNumber;
extern CheatsData cheatsList[MAX_CHEATS];

#endif // VBAM_GBA_CHEATS_H_
