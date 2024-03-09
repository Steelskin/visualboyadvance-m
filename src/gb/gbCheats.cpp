#include "gbCheats.h"

#include "../NLS.h"
#include "../System.h"

#include "gb.h"
#include "gbGlobals.h"

namespace {

bool isValidHex(char c) {
    return (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9');
}

uint8_t hexToInt(char c) {
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= '0' && c <= '9') {
        return c - '0';
    }
    return 0;
}

bool gsVerifyCode(const std::string& code) {
    if (code.size() != 8) {
        return false;
    }

    for (size_t i = 0; i < 8; i++) {
        if (!isValidHex(code[i])) {
            return false;
        }
    }

    return true;
}

uint16_t gsCodeToAddress(const std::string& code) {
    return hexToInt(code[6]) << 12 | hexToInt(code[7]) << 8 | hexToInt(code[4]) << 4 |
           hexToInt(code[5]);
}

uint8_t gsCodeToValue(const std::string& code) {
    return (hexToInt(code[2]) << 4) + hexToInt(code[3]);
}

uint8_t gsCodeToBank(const std::string& code) {
    return hexToInt(code[0]) << 4 | hexToInt(code[1]);
}

bool ggVerifyCode(const std::string& code) {
    const size_t length = code.size();

    // Game Genie codes are either 7 or 11 characters long.
    if (length != 7 && length != 11) {
        return false;
    }

    for (size_t i = 0; i < 3; i++) {
        if (!isValidHex(code[i])) {
            return false;
        }
    }
    if (code[3] != '-') {
        return false;
    }
    for (size_t i = 4; i < 7; i++) {
        if (!isValidHex(code[i])) {
            return false;
        }
    }

    if (length == 7) {
        return true;
    }

    if (code[7] != '-') {
        return false;
    }
    for (size_t i = 8; i < 11; i++) {
        if (!isValidHex(code[i])) {
            return false;
        }
    }

    return true;
}

uint16_t ggCodeToAddress(const std::string& code) {
    return (hexToInt(code[2]) << 8) + (hexToInt(code[4]) << 4) + (hexToInt(code[5])) +
           ((hexToInt(code[6]) ^ 0x0f) << 12);
}

nonstd::optional<uint8_t> ggCodeToCompare(const std::string& code) {
    if (code.size() == 7) {
        return nonstd::nullopt;
    }

    uint8_t compare = (hexToInt(code[8]) << 4) + hexToInt(code[10]);
    compare = compare ^ 0xff;
    compare = (compare >> 2) | ((compare << 6) & 0xc0);
    compare ^= 0x45;
    return nonstd::optional<uint8_t>(compare);
}

uint8_t ggCodeToValue(const std::string& code) {
    return (hexToInt(code[0]) << 4) + hexToInt(code[1]);
}

}  // namespace

// struct gbXxCheat {
//     char cheatDesc[100];
//     char cheatCode[20];
// };

// gbCheat gbCheatList[MAX_CHEATS];
// int gbCheatNumber = 0;
// bool gbCheatMap[0x10000];

// #define GBCHEAT_IS_HEX(a) (((a) >= 'A' && (a) <= 'F') || ((a) >= '0' && (a) <= '9'))
// #define GBCHEAT_HEX_VALUE(a) ((a) >= 'A' ? (a) - 'A' + 10 : (a) - '0')

// void gbCheatUpdateMap()
// {
//     memset(gbCheatMap, 0, 0x10000);

//     for (int i = 0; i < gbCheatNumber; i++) {
//         if (gbCheatList[i].enabled)
//             gbCheatMap[gbCheatList[i].address] = true;
//     }
// }

// #ifndef __LIBRETRO__
// void gbCheatsSaveGame(gzFile gzFile)
// {
//     utilWriteInt(gzFile, gbCheatNumber);
//     if (gbCheatNumber > 0)
//         utilGzWrite(gzFile, &gbCheatList[0], sizeof(gbCheat) * gbCheatNumber);
// }

// void gbCheatsReadGame(gzFile gzFile, int version)
// {
//     if (version <= 8) {
//         int gbGgOn = utilReadInt(gzFile);

//         if (gbGgOn) {
//             int n = utilReadInt(gzFile);
//             gbXxCheat tmpCheat;
//             for (int i = 0; i < n; i++) {
//                 utilGzRead(gzFile, &tmpCheat, sizeof(gbXxCheat));
//                 gbAddGgCheat(tmpCheat.cheatCode, tmpCheat.cheatDesc);
//             }
//         }

//         int gbGsOn = utilReadInt(gzFile);

//         if (gbGsOn) {
//             int n = utilReadInt(gzFile);
//             gbXxCheat tmpCheat;
//             for (int i = 0; i < n; i++) {
//                 utilGzRead(gzFile, &tmpCheat, sizeof(gbXxCheat));
//                 gbAddGsCheat(tmpCheat.cheatCode, tmpCheat.cheatDesc);
//             }
//         }
//     } else {
//         gbCheatNumber = utilReadInt(gzFile);

//         if (gbCheatNumber > 0 && gbCheatNumber <= MAX_CHEATS) {
//             utilGzRead(gzFile, &gbCheatList[0], sizeof(gbCheat) * gbCheatNumber);
//         }
//     }

//     gbCheatUpdateMap();
// }

// void gbCheatsReadGameSkip(gzFile gzFile, int version)
// {
//     if (version <= 8) {
//         int gbGgOn = utilReadInt(gzFile);
//         if (gbGgOn) {
//             int n = utilReadInt(gzFile);
//             if (n > 0) {
//                 utilGzSeek(gzFile, n * sizeof(gbXxCheat), SEEK_CUR);
//             }
//         }

//         int gbGsOn = utilReadInt(gzFile);
//         if (gbGsOn) {
//             int n = utilReadInt(gzFile);
//             if (n > 0) {
//                 utilGzSeek(gzFile, n * sizeof(gbXxCheat), SEEK_CUR);
//             }
//         }
//     } else {
//         int n = utilReadInt(gzFile);

//         if (n > 0) {
//             utilGzSeek(gzFile, n * sizeof(gbCheat), SEEK_CUR);
//         }
//     }
// }
// #endif /* __LIBRETRO__ */

// void gbCheatsSaveCheatList(const char* file)
// {
//     if (gbCheatNumber == 0)
//         return;
//     FILE* f = utilOpenFile(file, "wb");
//     if (f == NULL)
//         return;
//     int version = 1;
//     fwrite(&version, 1, sizeof(version), f);
//     int type = 1;
//     fwrite(&type, 1, sizeof(type), f);
//     fwrite(&gbCheatNumber, 1, sizeof(gbCheatNumber), f);
//     fwrite(gbCheatList, 1, sizeof(gbCheat) * gbCheatNumber, f);
//     fclose(f);
// }

// bool gbCheatsLoadCheatList(const char* file)
// {
//     gbCheatNumber = 0;

//     gbCheatUpdateMap();

//     int count = 0;

//     FILE* f = utilOpenFile(file, "rb");

//     if (f == NULL)
//         return false;

//     int version = 0;

//     if (fread(&version, 1, sizeof(version), f) != sizeof(version)) {
//         fclose(f);
//         return false;
//     }

//     if (version != 1) {
//         systemMessage(MSG_UNSUPPORTED_CHEAT_LIST_VERSION,
//             N_("Unsupported cheat list version %d"), version);
//         fclose(f);
//         return false;
//     }

//     int type = 0;
//     if (fread(&type, 1, sizeof(type), f) != sizeof(type)) {
//         fclose(f);
//         return false;
//     }

//     if (type != 1) {
//         systemMessage(MSG_UNSUPPORTED_CHEAT_LIST_TYPE,
//             N_("Unsupported cheat list type %d"), type);
//         fclose(f);
//         return false;
//     }

//     if (fread(&count, 1, sizeof(count), f) != sizeof(count)) {
//         fclose(f);
//         return false;
//     }

//     if (fread(gbCheatList, 1, sizeof(gbCheatList), f) > sizeof(gbCheatList)) {
//         fclose(f);
//         return false;
//     }

//     gbCheatNumber = count;
//     gbCheatUpdateMap();

//     return true;
// }

// bool gbVerifyGsCode(const char* code)
// {
//     size_t len = strlen(code);

//     if (len == 0)
//         return true;

//     if (len != 8)
//         return false;

//     for (int i = 0; i < 8; i++)
//         if (!GBCHEAT_IS_HEX(code[i]))
//             return false;

//     /*  int address = GBCHEAT_HEX_VALUE(code[6]) << 12 |
//     GBCHEAT_HEX_VALUE(code[7]) << 8 |
//     GBCHEAT_HEX_VALUE(code[4]) << 4 |
//     GBCHEAT_HEX_VALUE(code[5]);*/

//     return true;
// }

// bool gbAddGsCheat(const char* code, const char* desc)
// {
//     if (gbCheatNumber > (MAX_CHEATS - 1)) {
//         systemMessage(MSG_MAXIMUM_NUMBER_OF_CHEATS,
//             N_("Maximum number of cheats reached."));
//         return false;
//     }

//     if (!gbVerifyGsCode(code)) {
//         systemMessage(MSG_INVALID_GAMESHARK_CODE,
//             N_("Invalid GameShark code: %s"), code);
//         return false;
//     }

//     int i = gbCheatNumber;

//     strcpy(gbCheatList[i].cheatCode, code);
//     strcpy(gbCheatList[i].cheatDesc, desc);

//     gbCheatList[i].sram_bank = GBCHEAT_HEX_VALUE(code[0]) << 4 | GBCHEAT_HEX_VALUE(code[1]);

//     gbCheatList[i].value = GBCHEAT_HEX_VALUE(code[2]) << 4 | GBCHEAT_HEX_VALUE(code[3]);

//     gbCheatList[i].address = GBCHEAT_HEX_VALUE(code[6]) << 12 | GBCHEAT_HEX_VALUE(code[7]) << 8 |
//     GBCHEAT_HEX_VALUE(code[4]) << 4 | GBCHEAT_HEX_VALUE(code[5]);

//     gbCheatList[i].compare = 0;

//     gbCheatList[i].enabled = true;

//     int gsSramBank = gbCheatList[i].sram_bank;

//     if ((gsSramBank != 1) && ((gsSramBank & 0xF0) != 0x80) && ((gsSramBank & 0xF0) != 0x90) &&
//     ((gsSramBank & 0xF0) != 0xA0) && ((gsSramBank) != 0xF0) && ((gsSramBank) != 0xF1))
//         systemMessage(MSG_WRONG_GAMESHARK_CODE,
//             N_("Wrong GameShark code type : %s"), code);
//     else if (((gsSramBank & 0xF0) == 0xA0) || ((gsSramBank) == 0xF0) || ((gsSramBank) == 0xF1))
//         systemMessage(MSG_UNSUPPORTED_GAMESHARK_CODE,
//             N_("Unsupported GameShark code type : %s"), code);

//     gbCheatNumber++;

//     return true;
// }

// bool gbVerifyGgCode(const char* code)
// {
//     size_t len = strlen(code);

//     if (len != 11 && len != 7 && len != 6 && len != 0)
//         return false;

//     if (len == 0)
//         return true;

//     if (!GBCHEAT_IS_HEX(code[0]))
//         return false;
//     if (!GBCHEAT_IS_HEX(code[1]))
//         return false;
//     if (!GBCHEAT_IS_HEX(code[2]))
//         return false;
//     if (code[3] != '-')
//         return false;
//     if (!GBCHEAT_IS_HEX(code[4]))
//         return false;
//     if (!GBCHEAT_IS_HEX(code[5]))
//         return false;
//     if (!GBCHEAT_IS_HEX(code[6]))
//         return false;
//     if (code[7] != 0) {
//         if (code[7] != '-')
//             return false;
//         if (code[8] != 0) {
//             if (!GBCHEAT_IS_HEX(code[8]))
//                 return false;
//             if (!GBCHEAT_IS_HEX(code[9]))
//                 return false;
//             if (!GBCHEAT_IS_HEX(code[10]))
//                 return false;
//         }
//     }

//     //  int replace = (GBCHEAT_HEX_VALUE(code[0]) << 4) +
//     //    GBCHEAT_HEX_VALUE(code[1]);

//     int address = (GBCHEAT_HEX_VALUE(code[2]) << 8) + (GBCHEAT_HEX_VALUE(code[4]) << 4) +
//     (GBCHEAT_HEX_VALUE(code[5])) + ((GBCHEAT_HEX_VALUE(code[6]) ^ 0x0f) << 12);

//     if (address >= 0x8000 && address <= 0x9fff)
//         return false;

//     if (address >= 0xc000)
//         return false;

//     if (code[7] == 0 || code[8] == '0')
//         return true;

//     int compare = (GBCHEAT_HEX_VALUE(code[8]) << 4) + (GBCHEAT_HEX_VALUE(code[10]));
//     compare = compare ^ 0xff;
//     compare = (compare >> 2) | ((compare << 6) & 0xc0);
//     compare ^= 0x45;

//     int cloak = (GBCHEAT_HEX_VALUE(code[8])) ^ (GBCHEAT_HEX_VALUE(code[9]));

//     if (cloak >= 1 && cloak <= 7)
//         return false;

//     return true;
// }

// bool gbAddGgCheat(const char* code, const char* desc)
// {
//     if (gbCheatNumber > (MAX_CHEATS - 1)) {
//         systemMessage(MSG_MAXIMUM_NUMBER_OF_CHEATS,
//             N_("Maximum number of cheats reached."));
//         return false;
//     }

//     if (!gbVerifyGgCode(code)) {
//         systemMessage(MSG_INVALID_GAMEGENIE_CODE,
//             N_("Invalid GameGenie code: %s"), code);
//         return false;
//     }

//     int i = gbCheatNumber;

//     size_t len = strlen(code);

//     strcpy(gbCheatList[i].cheatCode, code);
//     strcpy(gbCheatList[i].cheatDesc, desc);

//     gbCheatList[i].sram_bank = 0x101;
//     gbCheatList[i].value = (GBCHEAT_HEX_VALUE(code[0]) << 4) + GBCHEAT_HEX_VALUE(code[1]);

//     gbCheatList[i].address = (GBCHEAT_HEX_VALUE(code[2]) << 8) + (GBCHEAT_HEX_VALUE(code[4]) <<
//     4) + (GBCHEAT_HEX_VALUE(code[5])) + ((GBCHEAT_HEX_VALUE(code[6]) ^ 0x0f) << 12);

//     gbCheatList[i].compare = 0;

//     if (len != 7 && len != 8) {

//         int compare = (GBCHEAT_HEX_VALUE(code[8]) << 4) + (GBCHEAT_HEX_VALUE(code[10]));
//         compare = compare ^ 0xff;
//         compare = (compare >> 2) | ((compare << 6) & 0xc0);
//         compare ^= 0x45;

//         gbCheatList[i].compare = compare;
//         //gbCheatList[i].code = 0;
//         gbCheatList[i].sram_bank = 0x100;  // fix for compare value
//     }

//     gbCheatList[i].enabled = true;

//     gbCheatMap[gbCheatList[i].address] = true;

//     gbCheatNumber++;

//     return true;
// }

// void gbCheatRemove(int i)
// {
//     if (i < 0 || i >= gbCheatNumber) {
//         systemMessage(MSG_INVALID_CHEAT_TO_REMOVE,
//             N_("Invalid cheat to remove %d"), i);
//         return;
//     }

//     if ((i + 1) < gbCheatNumber) {
//         memcpy(&gbCheatList[i], &gbCheatList[i + 1], sizeof(gbCheat) * (gbCheatNumber - i - 1));
//     }

//     gbCheatNumber--;

//     gbCheatUpdateMap();
// }

// void gbCheatRemoveAll()
// {
//     gbCheatNumber = 0;
//     gbCheatUpdateMap();
// }

// void gbCheatEnable(int i)
// {
//     if (i >= 0 && i < gbCheatNumber) {
//         if (!gbCheatList[i].enabled) {
//             gbCheatList[i].enabled = true;
//             gbCheatUpdateMap();
//         }
//     }
// }

// void gbCheatDisable(int i)
// {
//     if (i >= 0 && i < gbCheatNumber) {
//         if (gbCheatList[i].enabled) {
//             gbCheatList[i].enabled = false;
//             gbCheatUpdateMap();
//         }
//     }
// }

// bool gbCheatReadGSCodeFile(const char* fileName)
// {
//     FILE* file = utilOpenFile(fileName, "rb");

//     if (!file) {
//         systemMessage(MSG_CANNOT_OPEN_FILE, N_("Cannot open file %s"), fileName);
//         return false;
//     }

//     fseek(file, 0x18, SEEK_SET);
//     int count = 0;
//     if(fread(&count, 1, 2, file) == 0 ) {
//         fclose(file);
//         return false;
//     }

//     int dummy = 0;
//     gbCheatRemoveAll();
//     char desc[13];
//     char code[9];
//     int i;
//     for (i = 0; i < count; i++) {
//         FREAD_UNCHECKED(&dummy, 1, 2, file);
//         FREAD_UNCHECKED(desc, 1, 12, file);
//         desc[12] = 0;
//         FREAD_UNCHECKED(code, 1, 8, file);
//         code[8] = 0;
//         gbAddGsCheat(code, desc);
//     }

//     for (i = 0; i < gbCheatNumber; i++)
//         gbCheatDisable(i);

//     fclose(file);
//     return true;
// }

// // Used to emulated GG codes
// uint8_t gbCheatRead(uint16_t address)
// {
//     if (!coreOptions.cheatsEnabled)
//         return gbMemoryMap[address >> 12][address & 0xFFF];

//     for (int i = 0; i < gbCheatNumber; i++) {
//         if (gbCheatList[i].enabled && gbCheatList[i].address == address) {
//             switch (gbCheatList[i].sram_bank) {
//                 case 0x100:  // GameGenie support
//                     if (gbMemoryMap[address >> 12][address & 0xFFF] == gbCheatList[i].compare)
//                         return gbCheatList[i].value;
//                     break;
//                 case 0x101:  // GameGenie 6 digits code support
//                     return gbCheatList[i].value;
//                     break;
//             }
//         }
//     }
//     return gbMemoryMap[address >> 12][address & 0xFFF];
// }

// // Used to emulate GS codes.
// void gbCheatWrite(bool reboot)
// {
//     static int gbNextCheat = 0;

//     if (coreOptions.cheatsEnabled) {
//         uint16_t address = 0;

//         if (gbNextCheat >= gbCheatNumber)
//             gbNextCheat = 0;

//         for (int i = gbNextCheat; i < gbCheatNumber; i++) {
//             if (gbCheatList[i].enabled) {
//                 address = gbCheatList[i].address;
//                 if ((!reboot) && (address >= 0x8000) && !((address >= 0xA000) && (address <
//                 0xC000))) { // These codes are executed one per one, at each Vblank
//                     // address is >= 0x8000, but not within 0xA000-0xBFFF.
//                     // i.e. address is inVRAM or WRAM or OAM or IO Reg.
//                     // okay so the way this works is that on vblank interrupt, the gameshark
//                     // rewrites the RAM value. Though I don't get the `gbNextCheat` part or the
//                     // `reboot` part.
//                     switch (gbCheatList[i].sram_bank) {
//                         case 0x01:
//                             gbWriteMemory(address, gbCheatList[i].value);
//                             gbNextCheat = i + 1;
//                             return;
//                         case 0x90:
//                         case 0x91:
//                         case 0x92:
//                         case 0x93:
//                         case 0x94:
//                         case 0x95:
//                         case 0x96:
//                         case 0x97:
//                         case 0x98:
//                         case 0x99:
//                         case 0x9A:
//                         case 0x9B:
//                         case 0x9C:
//                         case 0x9D:
//                         case 0x9E:
//                         case 0x9F:
//                             // `sram_bank` is 0x90-0x9F
//                             int oldbank = gbMemory[0xff70];

//                             // Write `value` to `address` in bank `sram_bank & 0xf`.
//                             gbWriteMemory(0xff70, gbCheatList[i].sram_bank & 0xf);
//                             gbWriteMemory(address, gbCheatList[i].value);

//                             // Restore old bank.
//                             gbWriteMemory(0xff70, oldbank);

//                             // Apply next cheat next time? Why aren't they all applied at once?
//                             gbNextCheat = i + 1;
//                             return;
//                     }
//                 } else // These codes are only executed when the game is booted
//                 {
//                     switch (gbCheatList[i].sram_bank & 0xF0) {
//                         case 0x80:
//                             // `sram_bank` is 0x80-0x8F
//                             gbWriteMemory(0x0000, 0x0A);
//                             gbWriteMemory(0x4000, gbCheatList[i].value & 0xF);
//                             gbWriteMemory(address, gbCheatList[i].value);
//                             gbNextCheat = i + 1;
//                             return;
//                     }
//                 }
//             }
//         }
//     }
// }

// static
gbCheatManager& gbCheatManager::instance() {
    static gbCheatManager g_instance;
    return g_instance;
}

void gbCheatManager::resetCheats() {
    cheat_address_map_.clear();
    cheats_for_interrupt_.clear();
    cheats_for_reset_.clear();
}

bool gbCheatManager::addCheat(const std::string& code, gbCheatType type) {
    const auto cheat = parseCheat(code, type);
    if (!cheat) {
        systemMessage(MSG_INVALID_GAMESHARK_CODE, N_("Invalid code: %s"), code.c_str());
        return false;
    }

    switch (type) {
        case gbCheatType::GameShark: {
            // bank must be in range 0x80-0xAF or 0xF0-0xF1
            const uint8_t sram_bank = cheat->sram_bank.value();
            if ((sram_bank < 0x80 || sram_bank > 0xAF) && (sram_bank < 0xF0 || sram_bank > 0xF1)) {
                systemMessage(MSG_WRONG_GAMESHARK_CODE, N_("Wrong GameShark code type: %s"),
                              code.c_str());
                return false;
            }

            if (sram_bank >= 0xA0 && sram_bank <= 0xAF) {
                systemMessage(MSG_UNSUPPORTED_GAMESHARK_CODE,
                              N_("Unsupported GameShark code type: %s"), code.c_str());
                return false;
            }

            if (sram_bank >= 0xF0) {
                cheats_for_reset_.insert(cheat.value());
            } else {
                cheats_for_interrupt_.insert(cheat.value());
            }

            return true;
        }
        case gbCheatType::GameGenie: {
            cheat_address_map_[cheat->address].insert(cheat.value());

            return true;
        }
    }

    assert(false);
    return false;
}

bool gbCheatManager::removeCheat(const std::string& code, gbCheatType type) {
    const auto cheat = parseCheat(code, type);
    if (!cheat) {
        return false;
    }

    switch (type) {
        case gbCheatType::GameShark:
            return cheats_for_interrupt_.erase(cheat.value()) == 1;
        case gbCheatType::GameGenie:
            return cheat_address_map_.erase(cheat->address) == 1;
    }

    assert(false);
    return false;
}

nonstd::optional<uint8_t> gbCheatManager::readAddressWithCheats(const uint8_t* const memory_map[16],
                                                                uint16_t address) const {
    if (!coreOptions.cheatsEnabled) {
        return nonstd::nullopt;
    }

    const size_t address_map = address >> 12;
    const size_t address_offset = address & 0xFFF;
    const uint8_t* memory = memory_map[address_map];
    const uint8_t original_value = memory[address_offset];

    const auto iter = cheat_address_map_.find(address);
    if (iter != cheat_address_map_.end()) {
        for (const gbCheat& cheat : iter->second) {
            if (cheat.compare.has_value()) {
                // Compare with value.
                if (cheat.compare.value() == original_value) {
                    return cheat.value;
                }
            } else {
                // No comparison required.
                return cheat.value;
            }
        }
    }

    return original_value;
}

void gbCheatManager::applyCheatsForReset() const {
    if (!coreOptions.cheatsEnabled) {
        return;
    }

    for (const auto& cheat : cheats_for_reset_) {
        // Enable the cartridge RAM.
        gbWriteMemory(0x0000, 0x0A);
        // Switch to the appropriate bank.
        gbWriteMemory(0x4000, cheat.value & 0xF);
        // Write `value` to `address`.
        gbWriteMemory(cheat.address, cheat.value);
        // Disable cartridge RAM.
        gbWriteMemory(0x0000, 0x00);
    }
}

void gbCheatManager::applyCheatsForInterrupt() const {
    if (!coreOptions.cheatsEnabled) {
        return;
    }

    // Save the old bank.
    const uint8_t old_bank = gbMemory[0xff70];

    for (const auto& cheat : cheats_for_interrupt_) {
        // Write `value` to `address` in bank `sram_bank`.
        gbWriteMemory(0xff70, cheat.sram_bank.value());
        gbWriteMemory(cheat.address, cheat.value);
    }

    // Restore old bank.
    gbWriteMemory(0xff70, old_bank);
}

// static
nonstd::optional<gbCheatManager::gbCheat> gbCheatManager::parseCheat(const std::string& code,
                                                                     gbCheatType type) {
    switch (type) {
        case gbCheatType::GameShark:
            if (!gsVerifyCode(code)) {
                return nonstd::nullopt;
            }

            return gbCheatManager::gbCheat{gsCodeToAddress(code), nonstd::nullopt,
                                           gsCodeToBank(code), gsCodeToValue(code)};
        case gbCheatType::GameGenie:

            if (!ggVerifyCode(code)) {
                return nonstd::nullopt;
            }

            const uint16_t address = ggCodeToAddress(code);
            if (address >= 0x8000 && address <= 0x9fff) {
                return nonstd::nullopt;
            }
            if (address >= 0xc000) {
                return nonstd::nullopt;
            }

            return gbCheatManager::gbCheat{address, ggCodeToCompare(code), nonstd::nullopt,
                                           ggCodeToValue(code)};
    }

    assert(false);
    return nonstd::nullopt;
}
