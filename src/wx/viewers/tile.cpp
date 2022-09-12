#include "wx/viewers/tile.h"

#include <wx/filedlg.h>
#include <wx/log.h>
#include <wx/radiobut.h>
#include <wx/slider.h>
#include <wx/xrc/xmlres.h>

#include "core/gb/gbGlobals.h"
#include "core/gba/gbaGlobals.h"
#include "wx/config/option-proxy.h"
#include "wx/main-frame.h"
#include "wx/widgets/wxmisc.h"
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
#define getradio(sv, n, var, val) getvfld(sv, n, wxRadioButton, wxBoolIntValidator(&var, val))
#define getslider(sv, n, var) getvfld(sv, n, wxSlider, wxGenericValidator(&var))

#define getlab(v, n, mv)                  \
    do {                                  \
        v = XRCCTRL(*this, n, wxControl); \
        if (!v)                           \
            baddialog();                  \
        v->SetLabel(wxT(mv));             \
    } while (0)

TileViewer::TileViewer() : GfxViewer("TileViewer", 32 * 8, 32 * 8) {
    is256_ = charbase_ = 0;
    getradio(, "Color16", is256_, 0);
    getradio(, "Color256", is256_, 1);
    getradio(, "CharBase0", charbase_, 0);
    do {
        wxRadioButton* _w = (dynamic_cast<wxRadioButton*>(
            (*this).FindWindow(wxXmlResource ::DoGetXRCID("CharBase1"))));
        if (!_w)
            do {
                for (bool wxdoif62 = false;
                     !wxdoif62 &&
                     wxLog ::IsLevelEnabled(wxLOG_Error, wxString ::FromAscii(wxLOG_COMPONENT));
                     wxdoif62 = true)
                    wxLogger(wxLOG_Error,
                             "c:\\Users\\steel\\src\\visualboyadvance-"
                             "m\\src\\wx\\viewers\\gfxviewers.cpp",
                             1519, __FUNCTION__, wxLOG_COMPONENT)
                        .Log(wxGetTranslation(("Unable to load dialog %s from resources")),
                             dname.c_str());
                wxGetApp().frame->Close(true);
                return;
            } while (0);
        _w->SetValidator(wxBoolIntValidator(&charbase_, 0x4000));
    } while (0);
    getradio(, "CharBase2", charbase_, 0x8000);
    getradio(, "CharBase3", charbase_, 0xc000);
    getradio(, "CharBase4", charbase_, 0x10000);
    getslider(, "Palette", palette_);
    getlab(tileno_, "Tile", "1WWW");
    getlab(addr_, "Address", "06WWWWWW");
    selx_ = sely_ = -1;
    Fit();
    Update();
}
void TileViewer::Update() {
    // Following copied almost verbatim from TileView.cpp
    uint16_t* palette = (uint16_t*)g_paletteRAM;
    uint8_t* charBase = &g_vram[charbase_];
    int maxY;

    if (is256_) {
        int tile = 0;
        maxY = 16;

        for (int y = 0; y < maxY; y++) {
            for (int x = 0; x < 32; x++) {
                if (charbase_ == 4 * 0x4000)
                    render256(tile, x, y, charBase, &palette[256]);
                else
                    render256(tile, x, y, charBase, palette);

                tile++;
            }
        }

        BMPSize(32 * 8, maxY * 8);
    } else {
        int tile = 0;
        maxY = 32;

        if (charbase_ == 3 * 0x4000)
            maxY = 16;

        for (int y = 0; y < maxY; y++) {
            for (int x = 0; x < 32; x++) {
                render16(tile, x, y, charBase, palette);
                tile++;
            }
        }

        BMPSize(32 * 8, maxY * 8);
    }

    ChangeBMP();
    UpdateMouseInfo();
}
void TileViewer::UpdateMouseInfoEv(wxMouseEvent& ev) {
    selx_ = ev.GetX();
    sely_ = ev.GetY();
    UpdateMouseInfo();
}

void TileViewer::UpdateMouseInfo() {
    if (selx_ > gv->bmw || sely_ > gv->bmh)
        selx_ = sely_ = -1;

    if (selx_ < 0) {
        addr_->SetLabel(wxEmptyString);
        tileno_->SetLabel(wxEmptyString);
    } else {
        int x = selx_ / 8;
        int y = sely_ / 8;
        int t = 32 * y + x;

        if (is256_)
            t *= 2;

        wxString s;
        s.Printf(wxT("%d"), t);
        tileno_->SetLabel(s);
        s.Printf(wxT("%08X"), 0x6000000 + charbase_ + 32 * t);
        addr_->SetLabel(s);
    }
}
// following 2 functions copied almost verbatim from TileView.cpp
void TileViewer::render256(int tile, int x, int y, uint8_t* charBase, uint16_t* palette) {
    uint8_t* bmp = &image.GetData()[24 * x + 8 * 32 * 24 * y];

    for (int j = 0; j < 8; j++) {
        for (int i = 0; i < 8; i++) {
            uint8_t c = charBase[tile * 64 + j * 8 + i];
            uint16_t color = palette[c];
            *bmp++ = (color & 0x1f) << 3;
            *bmp++ = ((color >> 5) & 0x1f) << 3;
            *bmp++ = ((color >> 10) & 0x1f) << 3;
        }

        bmp += 31 * 24;  // advance line
    }
}

void TileViewer::render16(int tile, int x, int y, uint8_t* charBase, uint16_t* palette) {
    uint8_t* bmp = &image.GetData()[24 * x + 8 * 32 * 24 * y];
    int pal = this->palette_;

    if (this->charbase_ == 4 * 0x4000)
        pal += 16;

    for (int j = 0; j < 8; j++) {
        for (int i = 0; i < 8; i++) {
            uint8_t c = charBase[tile * 32 + j * 4 + (i >> 1)];

            if (i & 1)
                c = c >> 4;
            else
                c = c & 15;

            uint16_t color = palette[pal * 16 + c];
            *bmp++ = (color & 0x1f) << 3;
            *bmp++ = ((color >> 5) & 0x1f) << 3;
            *bmp++ = ((color >> 10) & 0x1f) << 3;
        }

        bmp += 31 * 24;  // advance line
    }
}

void TileViewer::SaveGBATile(wxCommandEvent& ev) {
    (void)ev;  // unused params
    GameArea* panel = wxGetApp().frame->GetPanel();
    wxString bmp_save_dir = wxGetApp().frame->GetGamePath(OPTION(kGenScreenshotDir));
    // no attempt is made here to translate the dialog type name
    // it's just a suggested name, anyway
    wxString def_name = panel->game_name() + wxT('-') + dname;
    def_name.resize(def_name.size() - 6);  // strlen("Viewer")

    const int capture_format = OPTION(kPrefCaptureFormat);
    if (capture_format == 0)
        def_name.append(".png");
    else
        def_name.append(".bmp");

    wxFileDialog dlg(GetGrandParent(), _("Select output file"), bmp_save_dir, def_name,
                     _("PNG images|*.png|BMP images|*.bmp"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    dlg.SetFilterIndex(capture_format);
    int ret = dlg.ShowModal();
    bmp_save_dir = dlg.GetDirectory();

    if (ret != wxID_OK)
        return;

    wxBitmap obmp = gv->bm->GetSubBitmap(wxRect(0, 0, gv->bmw, gv->bmh));
    wxString fn = dlg.GetPath();
    wxBitmapType fmt = dlg.GetFilterIndex() ? wxBITMAP_TYPE_BMP : wxBITMAP_TYPE_PNG;

    if (fn.size() > 4) {
        if (wxString(fn.substr(fn.size() - 4)).IsSameAs(wxT(".bmp"), false))
            fmt = wxBITMAP_TYPE_BMP;
        else if (wxString(fn.substr(fn.size() - 4)).IsSameAs(wxT(".png"), false))
            fmt = wxBITMAP_TYPE_PNG;
    }

    obmp.SaveFile(fn, fmt);
}

BEGIN_EVENT_TABLE(TileViewer, GfxViewer)
EVT_BUTTON(XRCID("SaveGBATile"), TileViewer::SaveGBATile)
EVT_GFX_CLICK(wxID_ANY, TileViewer::UpdateMouseInfoEv)
END_EVENT_TABLE()

GBTileViewer::GBTileViewer() : GfxViewer(wxT("GBTileViewer"), 16 * 8, 16 * 8) {
    bank = charbase = 0;
    getradio(, "Bank0", bank, 0);
    getradio(, "Bank1", bank, 0x2000);
    getradio(, "CharBase0", charbase, 0);
    getradio(, "CharBase1", charbase, 0x800);
    getslider(, "Palette", palette);
    getlab(tileno, "Tile", "2WW");
    getlab(addr, "Address", "WWWW");
    selx = sely = -1;
    Fit();
    Update();
}
void GBTileViewer::Update() {
    // following copied almost verbatim from GBTileView.cpp
    uint8_t* charBase = (gbVram != NULL) ? &gbVram[bank + charbase] : &gbMemory[0x8000 + charbase];
    int tile = 0;

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            render(tile, x, y, charBase);
            tile++;
        }
    }

    ChangeBMP();
    UpdateMouseInfo();
}
void GBTileViewer::UpdateMouseInfoEv(wxMouseEvent& ev) {
    selx = ev.GetX();
    sely = ev.GetY();
    UpdateMouseInfo();
}

void GBTileViewer::UpdateMouseInfo() {
    if (selx > gv->bmw || sely > gv->bmh)
        selx = sely = -1;

    if (selx < 0) {
        addr->SetLabel(wxEmptyString);
        tileno->SetLabel(wxEmptyString);
    } else {
        int x = selx / 8;
        int y = sely / 8;
        int t = 16 * y + x;
        wxString s;
        s.Printf(wxT("%d"), t);
        tileno->SetLabel(s);
        s.Printf(wxT("%04X"), 0x8000 + charbase + 16 * t);
        addr->SetLabel(s);
    }
}

// following function copied almost verbatim from GBTileView.cpp
void GBTileViewer::render(int tile, int x, int y, uint8_t* charBase) {
    uint8_t* bmp = &image.GetData()[24 * x + 8 * 16 * 24 * y];

    for (int j = 0; j < 8; j++) {
        uint8_t mask = 0x80;
        uint8_t tile_a = charBase[tile * 16 + j * 2];
        uint8_t tile_b = charBase[tile * 16 + j * 2 + 1];

        for (int i = 0; i < 8; i++) {
            uint8_t c = (tile_a & mask) ? 1 : 0;
            c += ((tile_b & mask) ? 2 : 0);

            if (gbCgbMode) {
                c = c + palette * 4;
            } else {
                c = gbBgp[c];
            }

            uint16_t color = gbPalette[c];
            *bmp++ = (color & 0x1f) << 3;
            *bmp++ = ((color >> 5) & 0x1f) << 3;
            *bmp++ = ((color >> 10) & 0x1f) << 3;
            mask >>= 1;
        }

        bmp += 15 * 24;  // advance line
    }
}

void GBTileViewer::SaveGBTile(wxCommandEvent& ev) {
    (void)ev;  // unused params
    GameArea* panel = wxGetApp().frame->GetPanel();
    wxString bmp_save_dir = wxGetApp().frame->GetGamePath(OPTION(kGenScreenshotDir));
    // no attempt is made here to translate the dialog type name
    // it's just a suggested name, anyway
    wxString def_name = panel->game_name() + wxT('-') + dname;
    def_name.resize(def_name.size() - 6);  // strlen("Viewer")

    const int capture_format = OPTION(kPrefCaptureFormat);
    if (capture_format == 0)
        def_name.append(".png");
    else
        def_name.append(".bmp");

    wxFileDialog dlg(GetGrandParent(), _("Select output file"), bmp_save_dir, def_name,
                     _("PNG images|*.png|BMP images|*.bmp"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    dlg.SetFilterIndex(capture_format);
    int ret = dlg.ShowModal();
    bmp_save_dir = dlg.GetDirectory();

    if (ret != wxID_OK)
        return;

    wxBitmap obmp = gv->bm->GetSubBitmap(wxRect(0, 0, gv->bmw, gv->bmh));
    wxString fn = dlg.GetPath();
    wxBitmapType fmt = dlg.GetFilterIndex() ? wxBITMAP_TYPE_BMP : wxBITMAP_TYPE_PNG;

    if (fn.size() > 4) {
        if (wxString(fn.substr(fn.size() - 4)).IsSameAs(wxT(".bmp"), false))
            fmt = wxBITMAP_TYPE_BMP;
        else if (wxString(fn.substr(fn.size() - 4)).IsSameAs(wxT(".png"), false))
            fmt = wxBITMAP_TYPE_PNG;
    }

    obmp.SaveFile(fn, fmt);
}

BEGIN_EVENT_TABLE(GBTileViewer, GfxViewer)
EVT_BUTTON(XRCID("SaveGBTile"), GBTileViewer::SaveGBTile)
EVT_GFX_CLICK(wxID_ANY, GBTileViewer::UpdateMouseInfoEv)
END_EVENT_TABLE()

}  // namespace viewers