#ifndef GBGLOBALS_H
#define GBGLOBALS_H

#include "../common/Types.h"

extern uint8_t* bios;

// extern uint8_t* GB_EMULATOR->rom();
// extern uint8_t* GB_EMULATOR->ram();
// extern uint8_t* GB_EMULATOR->vram();
// extern uint8_t* GB_EMULATOR->wram();
// extern uint8_t* GB_EMULATOR->memory();
// extern uint16_t* GB_EMULATOR->line_buffer();
// extern uint8_t* GB_EMULATOR->tama5_ram();

extern uint8_t* gbMemoryMap[16];

extern int gbFrameSkip;
extern uint16_t gbColorFilter[32768];
// extern uint32_t gbEmulatorType;
extern uint32_t gbPaletteOption;
// extern bool GB_EMULATOR->HasCgbHw();
// extern bool GB_EMULATOR->HasSgbHw();
extern int gbWindowLine;
extern int gbSpeed;
extern uint8_t gbBgp[4];
extern uint8_t gbObp0[4];
extern uint8_t gbObp1[4];
extern uint16_t gbPalette[128];
extern bool gbColorOption;
extern bool gbScreenOn;
extern uint8_t gbSCYLine[300];
// gbSCXLine is used for the emulation (bug) of the SX change
// found in the Artic Zone game.
extern uint8_t gbSCXLine[300];
// gbBgpLine is used for the emulation of the
// Prehistorik Man's title screen scroller.
extern uint8_t gbBgpLine[300];
extern uint8_t gbObp0Line[300];
extern uint8_t gbObp1Line[300];
// gbSpritesTicks is used for the emulation of Parodius' Laser Beam.
extern uint8_t gbSpritesTicks[300];

extern uint8_t register_LCDC;
extern uint8_t register_LY;
extern uint8_t register_SCY;
extern uint8_t register_SCX;
extern uint8_t register_WY;
extern uint8_t register_WX;
extern uint8_t register_VBK;
extern uint8_t oldRegister_WY;

extern int emulating;

extern int gbBorderLineSkip;
extern int gbBorderRowSkip;
extern int gbBorderColumnSkip;
extern int gbDmaTicks;

extern void gbRenderLine();
extern void gbDrawSprites(bool);

extern uint8_t (*gbSerialFunction)(uint8_t);

#endif // GBGLOBALS_H
