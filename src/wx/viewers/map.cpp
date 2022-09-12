#include "wx/viewers/map.h"

#include <wx/radiobut.h>

#include "core/gb/gbGlobals.h"
#include "core/gba/gbaGlobals.h"
#include "wx/main-frame.h"
#include "wx/widgets/wxmisc.h"
#include "wx/wxhead.h"
#include "wx/wxvbam.h"

// TODO: this should be in a header
extern uint8_t gbInvertTab[256];

namespace viewers {

#define baddialog()                                                              \
    do {                                                                         \
        wxLogError(_("Unable to load dialog %s from resources"), dname.c_str()); \
        wxGetApp().frame->Close(true);                                           \
        return;                                                                  \
    } while (0)

#define getvfld(sv, n, t, v)             \
    do {                                 \
        t* _w = sv XRCCTRL(*this, n, t); \
        if (!_w)                         \
            baddialog();                 \
        _w->SetValidator(v);             \
    } while (0)
#define getradio(sv, n, var, val) getvfld(sv, n, wxRadioButton, wxBoolIntValidator(&var, val))

#define getlab(v, n, mv)                  \
    do {                                  \
        v = XRCCTRL(*this, n, wxControl); \
        if (!v)                           \
            baddialog();                  \
        v->SetLabel(wxT(mv));             \
    } while (0)

MapViewer::MapViewer() : GfxViewer("MapViewer", 1024, 1024) {
    frame = bg = 0;
    getradio(fr0 =, "Frame0", frame, 0);
    getradio(fr1 =, "Frame1", frame, 0xa000);
    getradio(bg0 =, "BG0", bg, 0);
    getradio(bg1 =, "BG1", bg, 1);
    getradio(bg2 =, "BG2", bg, 2);
    getradio(bg3 =, "BG3", bg, 3);
    getlab(modelab, "Mode", "8");
    getlab(mapbase, "MapBase", "0xWWWWWWWW");
    getlab(charbase, "CharBase", "0xWWWWWWWW");
    getlab(size, "Size", "1024x1024");
    getlab(colors, "Colors", "2WW");
    getlab(prio, "Priority", "3");
    getlab(mosaic, "Mosaic", "0");
    getlab(overflow, "Overflow", "0");
    getlab(coords_, "Coords", "(1023,1023)");
    getlab(addr_, "Address", "0xWWWWWWWW");
    getlab(tile_, "Tile", "1023");
    getlab(flip_, "Flip", "HV");
    getlab(palette_, "Palette", "---");
    Fit();
    selx = sely = -1;
    Update();
}
void MapViewer::Update() {
    mode = DISPCNT & 7;

    switch (bg) {
        case 0:
            control = BG0CNT;
            break;

        case 1:
            control = BG1CNT;
            break;

        case 2:
            control = BG2CNT;
            break;

        case 3:
            control = BG3CNT;
            break;
    }

    bool fr0en = true, fr1en = true, bg0en = true, bg1en = true, bg2en = true, bg3en = true;

    switch (mode) {
        case 0:
            fr0en = fr1en = false;
            renderTextScreen();
            break;

        case 1:
            fr0en = fr1en = false;
            bg3en = false;

            if (bg == 3) {
                bg = 0;
                control = BG0CNT;
                bg0->SetValue(true);
            }

            if (bg < 2)
                renderTextScreen();
            else
                renderRotScreen();

            break;

        case 2:
            fr0en = fr1en = false;
            bg0en = bg1en = false;

            if (bg < 2) {
                bg = 2;
                control = BG2CNT;
                bg2->SetValue(true);
            }

            renderRotScreen();
            break;

        case 3:
            fr0en = fr1en = false;
            bg0en = bg1en = bg2en = bg3en = false;
            bg = 2;
            bg2->SetValue(true);
            renderMode3();
            break;

        case 4:
            bg0en = bg1en = bg2en = bg3en = false;
            bg = 2;
            bg2->SetValue(true);
            renderMode4();
            break;

        case 5:
        case 6:
        case 7:
            bg = 2;
            bg2->SetValue(true);
            renderMode5();
            break;
    }

    ChangeBMP();
    fr0->Enable(fr0en);
    fr1->Enable(fr1en);
    bg0->Enable(bg0en);
    bg1->Enable(bg1en);
    bg2->Enable(bg2en);
    bg3->Enable(bg3en);
    wxString s;
    s.Printf(wxT("%d"), (int)mode);
    modelab->SetLabel(s);

    if (mode >= 3) {
        mapbase->SetLabel(wxEmptyString);
        charbase->SetLabel(wxEmptyString);
    } else {
        s.Printf(wxT("0x%08X"), ((control >> 8) & 0x1f) * 0x800 + 0x6000000);
        mapbase->SetLabel(s);
        s.Printf(wxT("0x%08X"), ((control >> 2) & 0x03) * 0x4000 + 0x6000000);
        charbase->SetLabel(s);
    }

    s.Printf(wxT("%dx%d"), gv->bmw, gv->bmh);
    size->SetLabel(s);
    colors->SetLabel(control & 0x80 ? wxT("256") : wxT("16"));
    s.Printf(wxT("%d"), control & 3);
    prio->SetLabel(s);
    mosaic->SetLabel(control & 0x40 ? wxT("1") : wxT("0"));
    overflow->SetLabel(bg <= 1 ? wxEmptyString : control & 0x2000 ? wxT("1") : wxT("0"));
    UpdateMouseInfo();
}

void MapViewer::UpdateMouseInfoEv(wxMouseEvent& ev) {
    selx = ev.GetX();
    sely = ev.GetY();
    UpdateMouseInfo();  // note that this will be inaccurate if game
    // not paused since last refresh
}

uint32_t MapViewer::AddressFromSel() {
    uint32_t base = ((control >> 8) & 0x1f) * 0x800 + 0x6000000;

    // all text bgs (16 bits)
    if (mode == 0 || (mode < 3 && bg < 2) || mode == 6 || mode == 7) {
        if (sely > 255) {
            base += 0x800;

            if (gv->bmw > 256)
                base += 0x800;
        }

        if (selx >= 256)
            base += 0x800;

        return base + ((selx & 0xff) >> 3) * 2 + 64 * ((sely & 0xff) >> 3);
    }

    // rot bgs (8 bits)
    if (mode < 3)
        return base + (selx >> 3) + (gv->bmw >> 3) * (sely >> 3);

    // mode 3/5 (16 bits)
    if (mode != 4)
        return 0x6000000 + 0xa000 * frame + (selx + gv->bmw * sely) * 2;

    // mode 4 (8 bits)
    return 0x6000000 + 0xa000 * frame + selx + gv->bmw * sely;
}

void MapViewer::UpdateMouseInfo() {
    if (selx > gv->bmw || sely > gv->bmh)
        selx = sely = -1;

    if (selx < 0) {
        coords_->SetLabel(wxEmptyString);
        addr_->SetLabel(wxEmptyString);
        tile_->SetLabel(wxEmptyString);
        flip_->SetLabel(wxEmptyString);
        palette_->SetLabel(wxEmptyString);
    } else {
        wxString s;
        s.Printf(wxT("(%d,%d)"), selx, sely);
        coords_->SetLabel(s);
        uint32_t address = AddressFromSel();
        s.Printf(wxT("0x%08X"), address);
        addr_->SetLabel(s);

        if ((!mode || (mode < 3 || mode > 5)) && bg < 2) {
            uint16_t value = *((uint16_t*)&g_vram[address - 0x6000000]);
            s.Printf(wxT("%d"), value & 1023);
            tile_->SetLabel(s);
            s = value & 1024 ? wxT('H') : wxT('-');
            s += value & 2048 ? wxT('V') : wxT('-');
            flip_->SetLabel(s);

            if (control & 0x80)
                palette_->SetLabel(wxT("---"));
            else {
                s.Printf(wxT("%d"), (value >> 12) & 15);
                palette_->SetLabel(s);
            }
        } else {
            tile_->SetLabel(wxT("---"));
            flip_->SetLabel(wxT("--"));
            palette_->SetLabel(wxT("---"));
        }
    }
}

void MapViewer::renderTextScreen() {
    uint16_t* palette = (uint16_t*)g_paletteRAM;
    uint8_t* charBase = &g_vram[((control >> 2) & 0x03) * 0x4000];
    uint16_t* screenBase = (uint16_t*)&g_vram[((control >> 8) & 0x1f) * 0x800];
    uint8_t* bmp = image.GetData();
    int sizeX = 256;
    int sizeY = 256;

    switch ((control >> 14) & 3) {
        case 0:
            break;

        case 1:
            sizeX = 512;
            break;

        case 2:
            sizeY = 512;
            break;

        case 3:
            sizeX = 512;
            sizeY = 512;
            break;
    }

    BMPSize(sizeX, sizeY);

    if (control & 0x80) {
        for (int y = 0; y < sizeY; y++) {
            int yy = y & 255;

            if (y == 256 && sizeY > 256) {
                screenBase += 0x400;

                if (sizeX > 256)
                    screenBase += 0x400;
            }

            uint16_t* screenSource = screenBase + ((yy >> 3) * 32);

            for (int x = 0; x < sizeX; x++) {
                uint16_t data = *screenSource;
                int tile = data & 0x3FF;
                int tileX = (x & 7);
                int tileY = y & 7;

                if (data & 0x0400)
                    tileX = 7 - tileX;

                if (data & 0x0800)
                    tileY = 7 - tileY;

                uint8_t c = charBase[tile * 64 + tileY * 8 + tileX];
                uint16_t color = palette[c];
                *bmp++ = (color & 0x1f) << 3;
                *bmp++ = ((color >> 5) & 0x1f) << 3;
                *bmp++ = ((color >> 10) & 0x1f) << 3;

                if (data & 0x0400) {
                    if (tileX == 0)
                        screenSource++;
                } else if (tileX == 7)
                    screenSource++;

                if (x == 255 && sizeX > 256) {
                    screenSource = screenBase + 0x400 + ((yy >> 3) * 32);
                }
            }

            bmp += 3 * (1024 - sizeX);
        }
    } else {
        for (int y = 0; y < sizeY; y++) {
            int yy = y & 255;

            if (y == 256 && sizeY > 256) {
                screenBase += 0x400;

                if (sizeX > 256)
                    screenBase += 0x400;
            }

            uint16_t* screenSource = screenBase + ((yy >> 3) * 32);

            for (int x = 0; x < sizeX; x++) {
                uint16_t data = *screenSource;
                int tile = data & 0x3FF;
                int tileX = (x & 7);
                int tileY = y & 7;

                if (data & 0x0400)
                    tileX = 7 - tileX;

                if (data & 0x0800)
                    tileY = 7 - tileY;

                uint8_t color = charBase[tile * 32 + tileY * 4 + (tileX >> 1)];

                if (tileX & 1) {
                    color = (color >> 4);
                } else {
                    color &= 0x0F;
                }

                int pal = (*screenSource >> 8) & 0xF0;
                uint16_t color2 = palette[pal + color];
                *bmp++ = (color2 & 0x1f) << 3;
                *bmp++ = ((color2 >> 5) & 0x1f) << 3;
                *bmp++ = ((color2 >> 10) & 0x1f) << 3;

                if (data & 0x0400) {
                    if (tileX == 0)
                        screenSource++;
                } else if (tileX == 7)
                    screenSource++;

                if (x == 255 && sizeX > 256) {
                    screenSource = screenBase + 0x400 + ((yy >> 3) * 32);
                }
            }

            bmp += 3 * (1024 - sizeX);
        }
    }

#if 0

		switch (bg)
		{
		case 0:
			renderView(BG0HOFS << 8, BG0VOFS << 8,
			           0x100, 0x000,
			           0x000, 0x100,
			           (sizeX - 1) << 8,
			           (sizeY - 1) << 8,
			           true);
			break;

		case 1:
			renderView(BG1HOFS << 8, BG1VOFS << 8,
			           0x100, 0x000,
			           0x000, 0x100,
			           (sizeX - 1) << 8,
			           (sizeY - 1) << 8,
			           true);
			break;

		case 2:
			renderView(BG2HOFS << 8, BG2VOFS << 8,
			           0x100, 0x000,
			           0x000, 0x100,
			           (sizeX - 1) << 8,
			           (sizeY - 1) << 8,
			           true);
			break;

		case 3:
			renderView(BG3HOFS << 8, BG3VOFS << 8,
			           0x100, 0x000,
			           0x000, 0x100,
			           (sizeX - 1) << 8,
			           (sizeY - 1) << 8,
			           true);
			break;
		}

#endif
}

void MapViewer::renderRotScreen() {
    uint16_t* palette = (uint16_t*)g_paletteRAM;
    uint8_t* charBase = &g_vram[((control >> 2) & 0x03) * 0x4000];
    uint8_t* screenBase = (uint8_t*)&g_vram[((control >> 8) & 0x1f) * 0x800];
    uint8_t* bmp = image.GetData();
    int sizeX = 128;
    int sizeY = 128;

    switch ((control >> 14) & 3) {
        case 0:
            break;

        case 1:
            sizeX = sizeY = 256;
            break;

        case 2:
            sizeX = sizeY = 512;
            break;

        case 3:
            sizeX = sizeY = 1024;
            break;
    }

    BMPSize(sizeX, sizeY);

    if (control & 0x80) {
        for (int y = 0; y < sizeY; y++) {
            for (int x = 0; x < sizeX; x++) {
                int tile = screenBase[(x >> 3) + (y >> 3) * (sizeX >> 3)];
                int tileX = (x & 7);
                int tileY = y & 7;
                uint8_t color = charBase[tile * 64 + tileY * 8 + tileX];
                uint16_t color2 = palette[color];
                *bmp++ = (color2 & 0x1f) << 3;
                *bmp++ = ((color2 >> 5) & 0x1f) << 3;
                *bmp++ = ((color2 >> 10) & 0x1f) << 3;
            }
        }

        bmp += 3 * (1024 - sizeX);
    } else {
        for (int y = 0; y < sizeY; y++) {
            for (int x = 0; x < sizeX; x++) {
                int tile = screenBase[(x >> 3) + (y >> 3) * (sizeX >> 3)];
                int tileX = (x & 7);
                int tileY = y & 7;
                uint8_t color = charBase[tile * 64 + tileY * 8 + tileX];
                uint16_t color2 = palette[color];
                *bmp++ = (color2 & 0x1f) << 3;
                *bmp++ = ((color2 >> 5) & 0x1f) << 3;
                *bmp++ = ((color2 >> 10) & 0x1f) << 3;
            }
        }

        bmp += 3 * (1024 - sizeX);
    }
}

void MapViewer::renderMode3() {
    uint8_t* bmp = image.GetData();
    uint16_t* src = (uint16_t*)&g_vram[0];
    BMPSize(240, 160);

    for (int y = 0; y < 160; y++) {
        for (int x = 0; x < 240; x++) {
            uint16_t data = *src++;
            *bmp++ = (data & 0x1f) << 3;
            *bmp++ = ((data >> 5) & 0x1f) << 3;
            *bmp++ = ((data >> 10) & 0x1f) << 3;
        }

        bmp += 3 * (1024 - 240);
    }
}

void MapViewer::renderMode4() {
    uint8_t* bmp = image.GetData();
    uint8_t* src = frame ? &g_vram[0xa000] : &g_vram[0];
    uint16_t* pal = (uint16_t*)&g_paletteRAM[0];
    BMPSize(240, 160);

    for (int y = 0; y < 160; y++) {
        for (int x = 0; x < 240; x++) {
            uint8_t c = *src++;
            uint16_t data = pal[c];
            *bmp++ = (data & 0x1f) << 3;
            *bmp++ = ((data >> 5) & 0x1f) << 3;
            *bmp++ = ((data >> 10) & 0x1f) << 3;
        }

        bmp += 3 * (1024 - 240);
    }
}

void MapViewer::renderMode5() {
    uint8_t* bmp = image.GetData();
    uint16_t* src = (uint16_t*)(frame ? &g_vram[0xa000] : &g_vram[0]);
    BMPSize(160, 128);

    for (int y = 0; y < 128; y++) {
        for (int x = 0; x < 160; x++) {
            uint16_t data = *src++;
            *bmp++ = (data & 0x1f) << 3;
            *bmp++ = ((data >> 5) & 0x1f) << 3;
            *bmp++ = ((data >> 10) & 0x1f) << 3;
        }

        bmp += 3 * (1024 - 160);
    }
}

BEGIN_EVENT_TABLE(MapViewer, GfxViewer)
EVT_GFX_CLICK(wxID_ANY, MapViewer::UpdateMouseInfoEv)
END_EVENT_TABLE()

GBMapViewer::GBMapViewer() : GfxViewer(wxT("GBMapViewer"), 256, 256) {
    charbase = 0x0000;
    mapbase = 0x1800;
    getradio(, "CharBase0", charbase, 0x0000);
    getradio(, "CharBase1", charbase, 0x0800);
    getradio(, "MapBase0", mapbase, 0x1800);
    getradio(, "MapBase1", mapbase, 0x1c00);
    getlab(coords_, "Coords", "(2WW,2WW)");
    getlab(addr_, "Address", "0xWWWW");
    getlab(tile_, "Tile", "2WW");
    getlab(flip_, "Flip", "HV");
    getlab(palette_, "Palette", "---");
    getlab(prio_, "Priority", "P");
    Fit();
    selx = sely = -1;
    Update();
}
void GBMapViewer::Update() {
    uint8_t *bank0, *bank1;

    if (gbCgbMode) {
        bank0 = &gbVram[0x0000];
        bank1 = &gbVram[0x2000];
    } else {
        bank0 = &gbMemory[0x8000];
        bank1 = NULL;
    }

    int tile_map_address = mapbase;
    // following copied almost verbatim from win32/GBMapView.cpp

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            uint8_t* bmp = &image.GetData()[y * 8 * 32 * 24 + x * 24];
            uint8_t attrs = 0;

            if (bank1 != NULL)
                attrs = bank1[tile_map_address];

            uint8_t tile = bank0[tile_map_address];
            tile_map_address++;

            if (charbase) {
                if (tile < 128)
                    tile += 128;
                else
                    tile -= 128;
            }

            for (int j = 0; j < 8; j++) {
                int charbase_address = attrs & 0x40 ? charbase + tile * 16 + (7 - j) * 2
                                                    : charbase + tile * 16 + j * 2;
                uint8_t tile_a = 0;
                uint8_t tile_b = 0;

                if (attrs & 0x08) {
                    tile_a = bank1[charbase_address++];
                    tile_b = bank1[charbase_address];
                } else {
                    tile_a = bank0[charbase_address++];
                    tile_b = bank0[charbase_address];
                }

                if (attrs & 0x20) {
                    tile_a = gbInvertTab[tile_a];
                    tile_b = gbInvertTab[tile_b];
                }

                uint8_t mask = 0x80;

                while (mask > 0) {
                    uint8_t c = (tile_a & mask) ? 1 : 0;
                    c += (tile_b & mask) ? 2 : 0;

                    if (gbCgbMode)
                        c = c + (attrs & 7) * 4;

                    uint16_t color = gbPalette[c];
                    *bmp++ = (color & 0x1f) << 3;
                    *bmp++ = ((color >> 5) & 0x1f) << 3;
                    *bmp++ = ((color >> 10) & 0x1f) << 3;
                    mask >>= 1;
                }

                bmp += 31 * 24;
            }
        }
    }

    ChangeBMP();
    UpdateMouseInfo();
}

void GBMapViewer::UpdateMouseInfoEv(wxMouseEvent& ev) {
    selx = ev.GetX();
    sely = ev.GetY();
    UpdateMouseInfo();  // note that this will be inaccurate if game
    // not paused since last refresh
}

void GBMapViewer::UpdateMouseInfo() {
    if (selx > gv->bmw || sely > gv->bmh)
        selx = sely = -1;

    if (selx < 0) {
        coords_->SetLabel(wxEmptyString);
        addr_->SetLabel(wxEmptyString);
        tile_->SetLabel(wxEmptyString);
        flip_->SetLabel(wxEmptyString);
        palette_->SetLabel(wxEmptyString);
        prio_->SetLabel(wxEmptyString);
    } else {
        wxString s;
        s.Printf(wxT("(%d,%d)"), selx, sely);
        coords_->SetLabel(s);
        uint16_t address = mapbase + 0x8000 + (sely >> 3) * 32 + (selx >> 3);
        s.Printf(wxT("0x%04X"), address);
        addr_->SetLabel(s);
        uint8_t attrs = 0;
        uint8_t tilev = gbMemoryMap[9][address & 0xfff];

        if (gbCgbMode) {
            attrs = gbVram[0x2000 + address - 0x8000];
            tilev = gbVram[address & 0x1fff];
        }

        if (charbase) {
            if (tilev >= 128)
                tilev -= 128;
            else
                tilev += 128;
        }

        s.Printf(wxT("%d"), (int)tilev);
        tile_->SetLabel(s);
        s = attrs & 0x20 ? wxT('H') : wxT('-');
        s += attrs & 0x40 ? wxT('V') : wxT('-');
        flip_->SetLabel(s);

        if (gbCgbMode) {
            s.Printf(wxT("%d"), attrs & 7);
            palette_->SetLabel(s);
        } else
            palette_->SetLabel(wxT("---"));

        prio_->SetLabel(wxString(attrs & 0x80 ? wxT('P') : wxT('-')));
    }
}

BEGIN_EVENT_TABLE(GBMapViewer, GfxViewer)
EVT_GFX_CLICK(wxID_ANY, GBMapViewer::UpdateMouseInfoEv)
END_EVENT_TABLE()

}  // namespace viewers
