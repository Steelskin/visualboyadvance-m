#include "wx/viewers/palette.h"

#include <wx/colordlg.h>
#include <wx/colourdata.h>
#include <wx/ffile.h>
#include <wx/filedlg.h>
#include <wx/log.h>
#include <wx/xrc/xmlres.h>

#include "core/gb/gbGlobals.h"
#include "core/gba/gbaGlobals.h"
#include "wx/main-frame.h"
#include "wx/wxvbam.h"

namespace viewers {

#define baddialog()                                                              \
    do {                                                                         \
        wxLogError(_("Unable to load dialog %s from resources"), dname.c_str()); \
        wxGetApp().frame->Close(true);                                           \
        return;                                                                  \
    } while (0)

#define unkctrl(n, v)                                                     \
    do {                                                                  \
        if (!wxXmlResource::Get()->AttachUnknownControl(wxT(n), v, this)) \
            baddialog();                                                  \
    } while (0)
#define colorctrl(v, n)                    \
    do {                                   \
        v = new ColorView(this, XRCID(n)); \
        if (!v)                            \
            baddialog();                   \
        unkctrl(n, v);                     \
    } while (0)
#define pixview(v, n, w, h, cv)         \
    do {                                \
        v = XRCCTRL(*this, n, PixView); \
        if (!v)                         \
            baddialog();                \
        v->InitBMP(w, h, cv);           \
    } while (0)
#define getlab(v, n, mv)                  \
    do {                                  \
        v = XRCCTRL(*this, n, wxControl); \
        if (!v)                           \
            baddialog();                  \
        v->SetLabel(wxT(mv));             \
    } while (0)

namespace {
static int ptype = 0;
static wxString pdir;
void savepal(wxWindow* parent, const uint8_t* data, int ncols, const wxString type) {
    // no attempt is made here to translate the palette type name
    // it's just a suggested name, anyway
    wxString def_name = wxGetApp().frame->GetPanel()->game_name() + wxT('-') + type;

    if (ptype == 2)
        def_name += wxT(".act");
    else
        def_name += wxT(".pal");

    wxFileDialog dlg(parent, _("Select output file and type"), pdir, def_name,
                     _("Windows Palette (*.pal)|*.pal|PaintShop Palette (*.pal)|*.pal|Adobe Color "
                       "Table (*.act)|*.act"),
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    dlg.SetFilterIndex(ptype);
    int ret = dlg.ShowModal();
    ptype = dlg.GetFilterIndex();
    pdir = dlg.GetDirectory();

    if (ret != wxID_OK)
        return;

    wxFFile f(dlg.GetPath(), wxT("wb"));

    // FIXME: check for errors
    switch (ptype) {
        case 0:  // Windows palette
        {
            f.Write("RIFF", 4);
            uint32_t d = wxUINT32_SWAP_ON_BE(256 * 4 + 16);
            f.Write(&d, 4);
            f.Write("PAL data", 8);
            d = wxUINT32_SWAP_ON_BE(256 * 4 + 4);
            f.Write(&d, 4);
            uint16_t w = wxUINT16_SWAP_ON_BE(0x0300);
            f.Write(&w, 2);
            w = wxUINT16_SWAP_ON_BE(256);  // cuases problems if not 16 or 256
            f.Write(&w, 2);

            for (int i = 0; i < ncols; i++, data += 3) {
                f.Write(data, 3);
                uint8_t z = 0;
                f.Write(&z, 1);
            }

            for (int i = ncols; i < 256; i++) {
                d = 0;
                f.Write(&d, 4);
            }
        } break;

        case 1:  // PaintShop palette
        {
#define jasc_head "JASC-PAL\r\n0100\r\n256\r\n"
            f.Write(jasc_head, sizeof(jasc_head) - 1);

            for (int i = 0; i < ncols; i++, data += 3) {
                char buf[14];
                int l = sprintf(buf, "%d %d %d\r\n", data[0], data[1], data[2]);
                f.Write(buf, l);
            }

            for (int i = ncols; i < 256; i++)
                f.Write("0 0 0\r\n", 7);

            break;
        }

        case 2:  // Adobe color table
        {
            f.Write(data, ncols * 3);
            uint32_t d = 0;

            for (int i = ncols; i < 256; i++)
                f.Write(&d, 3);
        } break;
    }

    f.Close();  // FIXME: check for errors
}

}  // namespace

PaletteViewer::PaletteViewer() : BaseViewer("PaletteViewer") {
    colorctrl(cv, "Color");
    pixview(bpv, "Background", 16, 16, cv);
    pixview(spv, "Sprite", 16, 16, cv);
    getlab(addr, "Address", "0x5000WWW");
    getlab(val, "Value", "0xWWWW");
    Fit();
    Update();
}
void PaletteViewer::Update() {
    if (g_paletteRAM) {
        uint16_t* pp = (uint16_t*)g_paletteRAM;
        uint8_t* bmp = colbmp;

        for (int i = 0; i < 512; i++, pp++) {
            *bmp++ = (*pp & 0x1f) << 3;
            *bmp++ = (*pp & 0x3e0) >> 2;
            *bmp++ = (*pp & 0x7c00) >> 7;
        }
    } else
        memset(colbmp, 0, sizeof(colbmp));

    bpv->SetData(colbmp, 16, 0, 0);
    spv->SetData(colbmp + 16 * 16 * 3, 16, 0, 0);
    ShowSel();
}
void PaletteViewer::SelBG(wxMouseEvent& ev) {
    (void)ev;  // unused params
    spv->SetSel(-1, -1, false);
    ShowSel();
}
void PaletteViewer::SelSprite(wxMouseEvent& ev) {
    (void)ev;  // unused params
    bpv->SetSel(-1, -1, false);
    ShowSel();
}
void PaletteViewer::ShowSel() {
    int x, y;
    bool isbg = true;
    bpv->GetSel(x, y);

    if (x < 0) {
        isbg = false;
        spv->GetSel(x, y);

        if (x < 0) {
            addr->SetLabel(wxEmptyString);
            val->SetLabel(wxEmptyString);
            return;
        }
    }

    int off = x + y * 16;

    if (!isbg)
        off += 16 * 16;

    uint8_t* pix = &colbmp[off * 3];
    uint16_t v = (pix[0] >> 3) + ((pix[1] >> 3) << 5) + ((pix[2] >> 3) << 10);
    wxString s;
    s.Printf(wxT("0x%04X"), (int)v);
    val->SetLabel(s);
    s.Printf(wxT("0x%08X"), 0x5000000 + 2 * off);
    addr->SetLabel(s);
}
void PaletteViewer::SaveBG(wxCommandEvent& ev) {
    (void)ev;  // unused params
    savepal(this, colbmp, 16 * 16, wxT("bg"));
}
void PaletteViewer::SaveOBJ(wxCommandEvent& ev) {
    (void)ev;  // unused params
    savepal(this, colbmp + 16 * 16 * 3, 16 * 16, wxT("obj"));
}
void PaletteViewer::ChangeBackdrop(wxCommandEvent& ev) {
    (void)ev;  // unused params
    // FIXME: this should really be a preference
    // should also have some way of indicating selection
    // perhaps replace w/ checkbox + colorpickerctrl
    static wxColourData* cd = NULL;
    wxColourDialog dlg(this, cd);

    if (dlg.ShowModal() == wxID_OK) {
        if (!cd)
            cd = new wxColourData();

        *cd = dlg.GetColourData();
        wxColour c = cd->GetColour();
        // Binary or the upper 5 bits of each color choice
        customBackdropColor = ((c.Red() >> 3) != 0) || (((c.Green() >> 3) << 5) != 0) ||
                              (((c.Blue() >> 3) << 10) != 0);
    } else
        // kind of an unintuitive way to turn it off...
        customBackdropColor = -1;
}

BEGIN_EVENT_TABLE(PaletteViewer, BaseViewer)
EVT_BUTTON(XRCID("SaveBG"), PaletteViewer::SaveBG)
EVT_BUTTON(XRCID("SaveOBJ"), PaletteViewer::SaveOBJ)
EVT_BUTTON(XRCID("ChangeBackdrop"), PaletteViewer::ChangeBackdrop)
EVT_GFX_CLICK(XRCID("Background"), PaletteViewer::SelBG)
EVT_GFX_CLICK(XRCID("Sprite"), PaletteViewer::SelSprite)
END_EVENT_TABLE()

GBPaletteViewer::GBPaletteViewer() : BaseViewer("GBPaletteViewer") {
    colorctrl(cv, "Color");
    pixview(bpv, "Background", 4, 8, cv);
    pixview(spv, "Sprite", 4, 8, cv);
    getlab(idx, "Index", "3W");
    getlab(val, "Value", "0xWWWW");
    Fit();
    Update();
}
void GBPaletteViewer::Update() {
    uint16_t* pp = gbPalette;
    uint8_t* bmp = colbmp;

    for (int i = 0; i < 64; i++, pp++) {
        *bmp++ = (*pp & 0x1f) << 3;
        *bmp++ = (*pp & 0x3e0) >> 2;
        *bmp++ = (*pp & 0x7c00) >> 7;
    }

    bpv->SetData(colbmp, 4, 0, 0);
    spv->SetData(colbmp + 4 * 8 * 3, 4, 0, 0);
    ShowSel();
}
void GBPaletteViewer::SelBG(wxMouseEvent& ev) {
    (void)ev;  // unused params
    spv->SetSel(-1, -1, false);
    ShowSel();
}
void GBPaletteViewer::SelSprite(wxMouseEvent& ev) {
    (void)ev;  // unused params
    bpv->SetSel(-1, -1, false);
    ShowSel();
}
void GBPaletteViewer::ShowSel() {
    int x, y;
    bool isbg = true;
    bpv->GetSel(x, y);

    if (x < 0) {
        isbg = false;
        spv->GetSel(x, y);

        if (x < 0) {
            idx->SetLabel(wxEmptyString);
            val->SetLabel(wxEmptyString);
            return;
        }
    }

    uint8_t* pix = &colbmp[(x + y * 4) * 3];

    if (isbg)
        pix += 4 * 8 * 3;

    uint16_t v = (pix[0] >> 3) + ((pix[1] >> 3) << 5) + ((pix[2] >> 3) << 10);
    wxString s;
    s.Printf(wxT("0x%04X"), (int)v);
    val->SetLabel(s);
    s.Printf(wxT("%d"), x + y * 4);
    idx->SetLabel(s);
}
void GBPaletteViewer::SaveBG(wxCommandEvent& ev) {
    (void)ev;  // unused params
    savepal(this, colbmp, 4 * 8, wxT("bg"));
}
void GBPaletteViewer::SaveOBJ(wxCommandEvent& ev) {
    (void)ev;  // unused params
    savepal(this, colbmp + 4 * 8 * 3, 4 * 8, wxT("obj"));
}

BEGIN_EVENT_TABLE(GBPaletteViewer, BaseViewer)
EVT_BUTTON(XRCID("SaveBG"), GBPaletteViewer::SaveBG)
EVT_BUTTON(XRCID("SaveOBJ"), GBPaletteViewer::SaveOBJ)
EVT_GFX_CLICK(XRCID("Background"), GBPaletteViewer::SelBG)
EVT_GFX_CLICK(XRCID("Sprite"), GBPaletteViewer::SelSprite)
END_EVENT_TABLE()

}  // namespace viewers
