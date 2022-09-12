#include "wx/viewers/oam.h"

#include <wx/log.h>
#include <wx/spinctrl.h>
#include <wx/valgen.h>

#include "core/gb/gbGlobals.h"
#include "core/gba/gbaGlobals.h"
#include "wx/main-frame.h"
#include "wx/wxhead.h"
#include "wx/wxvbam.h"

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
#define getspin(sv, n, var) getvfld(sv, n, wxSpinCtrl, wxGenericValidator(&var))

#define getlab(v, n, mv)                  \
    do {                                  \
        v = XRCCTRL(*this, n, wxControl); \
        if (!v)                           \
            baddialog();                  \
        v->SetLabel(wxT(mv));             \
    } while (0)

namespace {

void utilReadScreenPixels(uint8_t* dest, int w, int h) {
    uint8_t* b = dest;
    int sizeX = w;
    int sizeY = h;
    switch (systemColorDepth) {
        case 16: {
            uint16_t* p = (uint16_t*)(g_pix + (w + 2) * 2);  // skip first black line
            for (int y = 0; y < sizeY; y++) {
                for (int x = 0; x < sizeX; x++) {
                    uint16_t v = *p++;

                    *b++ = ((v >> systemRedShift) & 0x001f) << 3;    // R
                    *b++ = ((v >> systemGreenShift) & 0x001f) << 3;  // G
                    *b++ = ((v >> systemBlueShift) & 0x01f) << 3;    // B
                }
                p++;  // skip black pixel for filters
                p++;  // skip black pixel for filters
            }
        } break;
        case 24: {
            uint8_t* pixU8 = (uint8_t*)g_pix;
            for (int y = 0; y < sizeY; y++) {
                for (int x = 0; x < sizeX; x++) {
                    if (systemRedShift < systemBlueShift) {
                        *b++ = *pixU8++;  // R
                        *b++ = *pixU8++;  // G
                        *b++ = *pixU8++;  // B
                    } else {
                        uint8_t blue = *pixU8++;
                        uint8_t green = *pixU8++;
                        uint8_t red = *pixU8++;

                        *b++ = red;
                        *b++ = green;
                        *b++ = blue;
                    }
                }
            }
        } break;
        case 32: {
            uint32_t* pixU32 = (uint32_t*)(g_pix + 4 * (w + 1));
            for (int y = 0; y < sizeY; y++) {
                for (int x = 0; x < sizeX; x++) {
                    uint32_t v = *pixU32++;
                    *b++ = ((v >> systemBlueShift) & 0x001f) << 3;   // B
                    *b++ = ((v >> systemGreenShift) & 0x001f) << 3;  // G
                    *b++ = ((v >> systemRedShift) & 0x001f) << 3;    // R
                }
                pixU32++;
            }
        } break;
    }
}

}  // namespace

OAMViewer::OAMViewer() : GfxViewer("OAMViewer", 544, 496) {
    sprite = 0;
    getspin(, "Sprite", sprite);
    getlab(pos, "Pos", "5WW,2WW");
    getlab(mode, "Mode", "3");
    getlab(colors, "Colors", "256");
    getlab(pallab, "Palette", "1W");
    getlab(tile, "Tile", "1WWW");
    getlab(prio, "Priority", "3");
    getlab(size, "Size", "64x64");
    getlab(rot, "Rotation", "3W");
    getlab(flg, "Flags", "RHVMD");
    Fit();
    Update();
}
void OAMViewer::Update() {
    BMPSize(544, 496);
    wxImage screen(240, 160);
    systemRedShift = 19;
    systemGreenShift = 11;
    systemBlueShift = 3;
    utilReadScreenPixels(screen.GetData(), 240, 160);
    systemRedShift = 3;
    systemGreenShift = 11;
    systemBlueShift = 19;

    for (int sprite_no = 0; sprite_no < 128; sprite_no++) {
        uint16_t* sparms = &((uint16_t*)g_oam)[4 * sprite_no];
        uint16_t a0 = sparms[0], a1 = sparms[1], a2 = sparms[2];
        uint16_t* pal = &((uint16_t*)g_paletteRAM)[0x100];
        int sizeX = 8, sizeY = 8;

        // following is almost verbatim from OamView.cpp
        // shape = (a0 >> 14) & 3;
        // size = (a1 >> 14) & 3;
        switch (((a0 >> 12) & 0xc) | (a1 >> 14)) {
            case 0:
                break;

            case 1:
                sizeX = sizeY = 16;
                break;

            case 2:
                sizeX = sizeY = 32;
                break;

            case 3:
                sizeX = sizeY = 64;
                break;

            case 4:
                sizeX = 16;
                break;

            case 5:
                sizeX = 32;
                break;

            case 6:
                sizeX = 32;
                sizeY = 16;
                break;

            case 7:
                sizeX = 64;
                sizeY = 32;
                break;

            case 8:
                sizeY = 16;
                break;

            case 9:
                sizeY = 32;
                break;

            case 10:
                sizeX = 16;
                sizeY = 32;
                break;

            case 11:
                sizeX = 32;
                sizeY = 64;
                break;

            default:
                pos->SetLabel(wxEmptyString);
                mode->SetLabel(wxEmptyString);
                colors->SetLabel(wxEmptyString);
                pallab->SetLabel(wxEmptyString);
                tile->SetLabel(wxEmptyString);
                prio->SetLabel(wxEmptyString);
                size->SetLabel(wxEmptyString);
                rot->SetLabel(wxEmptyString);
                flg->SetLabel(wxEmptyString);
                continue;
        }

        wxImage spriteData(64, 64);
        uint8_t* bmp = spriteData.GetData();

        if (a0 & 0x2000) {
            int c = (a2 & 0x3FF);
            // if((DISPCNT & 7) > 2 && (c < 512))
            //     return;
            int inc = 32;

            if (DISPCNT & 0x40)
                inc = sizeX >> 2;
            else
                c &= 0x3FE;

            for (int y = 0; y < sizeY; y++) {
                for (int x = 0; x < sizeX; x++) {
                    uint32_t color = g_vram[0x10000 + (((c + (y >> 3) * inc) * 32 + (y & 7) * 8 +
                                                        (x >> 3) * 64 + (x & 7)) &
                                                       0x7FFF)];
                    color = pal[color];
                    *bmp++ = (color & 0x1f) << 3;
                    *bmp++ = ((color >> 5) & 0x1f) << 3;
                    *bmp++ = ((color >> 10) & 0x1f) << 3;
                }

                bmp += (64 - sizeX) * 3;
            }
        } else {
            int c = (a2 & 0x3FF);
            // if((DISPCNT & 7) > 2 && (c < 512))
            //     return;
            int inc = 32;

            if (DISPCNT & 0x40)
                inc = sizeX >> 3;

            int palette = (a2 >> 8) & 0xF0;

            for (int y = 0; y < sizeY; y++) {
                for (int x = 0; x < sizeX; x++) {
                    uint32_t color = g_vram[0x10000 + (((c + (y >> 3) * inc) * 32 + (y & 7) * 4 +
                                                        (x >> 3) * 32 + ((x & 7) >> 1)) &
                                                       0x7FFF)];

                    if (x & 1)
                        color >>= 4;
                    else
                        color &= 0x0F;

                    color = pal[palette + color];
                    *bmp++ = (color & 0x1f) << 3;
                    *bmp++ = ((color >> 5) & 0x1f) << 3;
                    *bmp++ = ((color >> 10) & 0x1f) << 3;
                }

                bmp += (64 - sizeX) * 3;
            }
        }

        if (sprite == sprite_no) {
            wxString s;
            s.Printf(wxT("%d,%d"), a1 & 511, a0 & 255);
            pos->SetLabel(s);
            s.Printf(wxT("%d"), (a0 >> 10) & 3);
            mode->SetLabel(s);
            colors->SetLabel(a0 & 8192 ? wxT("256") : wxT("16"));
            s.Printf(wxT("%d"), (a2 >> 12) & 15);
            pallab->SetLabel(s);
            s.Printf(wxT("%d"), a2 & 1023);
            tile->SetLabel(s);
            s.Printf(wxT("%d"), (a2 >> 10) & 3);
            prio->SetLabel(s);
            s.Printf(wxT("%dx%d"), sizeX, sizeY);
            s.Printf(wxT("%dx%d"), 0, 0);
            size->SetLabel(s);

            if (a0 & 512) {
                s.Printf(wxT("%d"), (a1 >> 9) & 31);
                rot->SetLabel(s);
            } else
                rot->SetLabel(wxEmptyString);

            s = wxEmptyString;

            if (a0 & 512)
                s.append(wxT("R--"));
            else {
                s.append(wxT('-'));
                s.append(a1 & 4096 ? wxT('H') : wxT('-'));
                s.append(a1 & 8192 ? wxT('V') : wxT('-'));
            }

            s.append(a0 & 4096 ? wxT('M') : wxT('-'));
            s.append(a0 & 1024 ? wxT('D') : wxT('-'));
            flg->SetLabel(s);
            uint8_t* box = spriteData.GetData();
            int sprite_posx = a1 & 511;
            int sprite_posy = a0 & 255;
            uint8_t* screen_box = screen.GetData();

            if (sprite_posx >= 0 && sprite_posx <= (239 - sizeY) && sprite_posy >= 0 &&
                sprite_posy <= (159 - sizeX))
                screen_box += (sprite_posx * 3) + (sprite_posy * screen.GetWidth() * 3);

            for (int y = 0; y < sizeY; y++) {
                for (int x = 0; x < sizeX; x++) {
                    uint32_t color = 0;

                    if (y == 0 || y == sizeY - 1 || x == 0 || x == sizeX - 1) {
                        color = 255;
                        *box++ = (color & 0x1f) << 3;
                        *box++ = ((color >> 5) & 0x1f) << 3;
                        *box++ = ((color >> 10) & 0x1f) << 3;

                        if (sprite_posx >= 0 && sprite_posx <= (239 - sizeY) && sprite_posy >= 0 &&
                            sprite_posy <= (159 - sizeX)) {
                            *screen_box++ = (color & 0x1f) << 3;
                            *screen_box++ = ((color >> 5) & 0x1f) << 3;
                            *screen_box++ = ((color >> 10) & 0x1f) << 3;
                        }
                    } else {
                        box += 3;

                        if (sprite_posx >= 0 && sprite_posx <= (239 - sizeY) && sprite_posy >= 0 &&
                            sprite_posy <= (159 - sizeX))
                            screen_box += 3;
                    }
                }

                box += (spriteData.GetWidth() - sizeX) * 3;

                if (sprite_posx >= 0 && sprite_posx <= (239 - sizeY) && sprite_posy >= 0 &&
                    sprite_posy <= (159 - sizeX))
                    screen_box += (screen.GetWidth() - sizeX) * 3;
            }
        }

        image.Paste(spriteData, (sprite_no % 16) * 34, (sprite_no / 16) * 34);
    }

    image.Paste(screen, 0, 304);
    ChangeBMP();
}

GBOAMViewer::GBOAMViewer() : GfxViewer("GBOAMViewer", 8, 16) {
    sprite = 0;
    getspin(, "Sprite", sprite);
    getlab(pos, "Pos", "2WW,2WW");
    getlab(tilelab, "Tile", "2WW");
    getlab(prio, "Priority", "W");
    getlab(oap, "OAP", "W");
    getlab(pallab, "Palette", "W");
    getlab(flg, "Flags", "HV");
    getlab(banklab, "Bank", "W");
    Fit();
    Update();
}
void GBOAMViewer::Update() {
    uint8_t* bmp = image.GetData();
    // following is almost verbatim from GBOamView.cpp
    uint16_t addr = sprite * 4 + 0xfe00;
    int size = register_LCDC & 4;
    uint8_t y = gbMemory[addr++];
    uint8_t x = gbMemory[addr++];
    uint8_t tile = gbMemory[addr++];

    if (size)
        tile &= 254;

    uint8_t flags = gbMemory[addr++];
    int w = 8;
    int h = size ? 16 : 8;
    BMPSize(w, h);
    uint8_t* bank0;
    uint8_t* bank1;

    if (gbCgbMode) {
        if (register_VBK & 1) {
            bank0 = &gbVram[0x0000];
            bank1 = &gbVram[0x2000];
        } else {
            bank0 = &gbVram[0x0000];
            bank1 = &gbVram[0x2000];
        }
    } else {
        bank0 = &gbMemory[0x8000];
        bank1 = NULL;
    }

    int init = 0x0000;
    uint8_t* pal = gbObp0;

    if ((flags & 0x10))
        pal = gbObp1;

    for (int yy = 0; yy < h; yy++) {
        int address = init + tile * 16 + 2 * yy;
        int a = 0;
        int b = 0;

        if (gbCgbMode && flags & 0x08) {
            a = bank1[address++];
            b = bank1[address++];
        } else {
            a = bank0[address++];
            b = bank0[address++];
        }

        for (int xx = 0; xx < 8; xx++) {
            uint8_t mask = 1 << (7 - xx);
            uint8_t c = 0;

            if ((a & mask))
                c++;

            if ((b & mask))
                c += 2;

            // make sure that sprites will work even in CGB mode
            if (gbCgbMode) {
                c = c + (flags & 0x07) * 4 + 32;
            } else {
                c = pal[c];
            }

            uint16_t color = gbPalette[c];
            *bmp++ = (color & 0x1f) << 3;
            *bmp++ = ((color >> 5) & 0x1f) << 3;
            *bmp++ = ((color >> 10) & 0x1f) << 3;
        }
    }

    ChangeBMP();
    wxString s;
    s.Printf(wxT("%d,%d"), x, y);
    pos->SetLabel(s);
    s.Printf(wxT("%d"), tile);
    tilelab->SetLabel(s);
    prio->SetLabel(flags & 0x80 ? wxT("1") : wxT("0"));
    oap->SetLabel(flags & 0x08 ? wxT("1") : wxT("0"));
    s.Printf(wxT("%d"), flags & 7);
    pallab->SetLabel(s);
    s = flags & 0x20 ? wxT('H') : wxT('-');
    s.append(flags & 0x40 ? wxT('V') : wxT('-'));
    flg->SetLabel(s);
    banklab->SetLabel(flags & 0x10 ? wxT("1") : wxT("0"));
}

}  // namespace viewers