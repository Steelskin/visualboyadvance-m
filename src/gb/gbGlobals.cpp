#include "gbGlobals.h"

#include <cstddef>
#include "../common/Types.h"

uint8_t* gbMemoryMap[16];

// uint8_t* GB_EMULATOR->memory() = nullptr;
// uint8_t* GB_EMULATOR->vram() = nullptr;
// uint8_t* GB_EMULATOR->rom() = nullptr;
// uint8_t* GB_EMULATOR->ram() = nullptr;
// uint8_t* GB_EMULATOR->wram() = nullptr;
// uint16_t* GB_EMULATOR->line_buffer() = nullptr;
// uint8_t* GB_EMULATOR->tama5_ram() = nullptr;

uint16_t gbPalette[128];
uint8_t gbBgp[4] = { 0, 1, 2, 3 };
uint8_t gbObp0[4] = { 0, 1, 2, 3 };
uint8_t gbObp1[4] = { 0, 1, 2, 3 };
int gbWindowLine = -1;

uint16_t gbColorFilter[32768];
uint32_t gbEmulatorType = 0;
uint32_t gbPaletteOption = 0;
int gbBorderLineSkip = 160;
int gbBorderRowSkip = 0;
int gbBorderColumnSkip = 0;
int gbDmaTicks = 0;
// bool GB_EMULATOR->HasCgbHw() = false;
// bool GB_EMULATOR->HasSgbHw() = false;
bool gbColorOption = false;

uint8_t (*gbSerialFunction)(uint8_t) = NULL;
