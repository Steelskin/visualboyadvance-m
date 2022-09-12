#include "wx/viewers/base-viewer.h"

#include <wx/checkbox.h>
#include <wx/filedlg.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/xrc/xmlres.h>

#include "wx/config/option-proxy.h"
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

void BaseViewer::CloseDlg(wxCloseEvent& ev) {
    (void)ev;  // unused params
    // stop tracking dialog
    MainFrame* f = wxGetApp().frame;

    for (auto iter = f->popups.begin(); iter != f->popups.end(); ++iter) {
        if (*iter == this) {
            f->popups.erase(iter);
            break;
        }
    }

    // just forwarding this event does not cause system to close window
    // ev.Skip();
    // so do it manually
    Destroy();
}

BaseViewer::BaseViewer(const wxString& name) : wxDialog(), auto_update_(false) {
    dname = name;
    MainFrame* f = wxGetApp().frame;

    // using LoadDialog precludes non-wxDialog nodes, which is good
    // since BaseViewer is strictly wxDialog-derived.
    if (!wxXmlResource::Get()->LoadDialog(this, f, name))
        baddialog();

    f->popups.push_back(this);
    SetEscapeId(wxID_CLOSE);
    Fit();
}

void BaseViewer::ActiveCtrl(wxCommandEvent& ev) {
    wxWindow* ctrl = wxStaticCast(ev.GetEventObject(), wxWindow);
    // TransferDataFromWindow() only operates on children, not self
    // so just simulate
    wxValidator* v = ctrl->GetValidator();

    if (v) {
        v->TransferFromWindow();
        Update();
    }
}

BEGIN_EVENT_TABLE(BaseViewer, wxDialog)
EVT_CLOSE(BaseViewer::CloseDlg)
EVT_CHECKBOX(XRCID("AutoUpdate"), BaseViewer::SetAutoUpdate)
EVT_RADIOBUTTON(wxID_ANY, BaseViewer::ActiveCtrl)
EVT_SPINCTRL(wxID_ANY, BaseViewer::ActiveCtrlSpin)
// this does not interfere with/intercept disassemble scrolls
EVT_SCROLL(BaseViewer::ActiveCtrlScr)
END_EVENT_TABLE()

DEFINE_EVENT_TYPE(EVT_REFILL_NEEDED)

ColorView::ColorView(wxWindow* parent, wxWindowID id)
    // default for MSW appears to be BORDER_SUNKEN
    : wxControl(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE), r_(0), g_(0), b_(0) {
    wxBoxSizer* sz = new wxBoxSizer(wxHORIZONTAL);
    cp = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(75, 75), wxBORDER_SUNKEN);
    sz->Add(cp);
    wxGridSizer* gs = new wxGridSizer(2);
    wxStaticText* lab = new wxStaticText(this, wxID_ANY, _("Red:"));
    gs->Add(lab, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    rt = new wxStaticText(this, wxID_ANY, wxT("255"), wxDefaultPosition,
#if !defined(__WXGTK__)
                          wxDefaultSize, wxST_NO_AUTORESIZE);
#else
                          wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
#endif  // !defined(__WXGTK__)
    gs->Add(rt, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    lab = new wxStaticText(this, wxID_ANY, _("Green:"));
    gs->Add(lab, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    gt = new wxStaticText(this, wxID_ANY, wxT("255"), wxDefaultPosition,
#if !defined(__WXGTK__)
                          wxDefaultSize, wxST_NO_AUTORESIZE);
#else
                          wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
#endif  // !defined(__WXGTK__)
    gs->Add(gt, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    lab = new wxStaticText(this, wxID_ANY, _("Blue:"));
    gs->Add(lab, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    bt = new wxStaticText(this, wxID_ANY, wxT("255"), wxDefaultPosition,
#if !defined(__WXGTK__)
                          wxDefaultSize, wxST_NO_AUTORESIZE);
#else
                          wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
#endif  // !defined(__WXGTK__)
    gs->Add(bt, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    sz->Add(gs);
    sz->Layout();
    SetSizerAndFit(sz);
    SetRGB(-1, -1, -1);
}

void ColorView::SetRGB(int r, int g, int b) {
    if (r == -1 || g == -1 || b == -1) {
        cp->SetBackgroundColour(wxNullColour);
        rt->SetLabel(wxT(""));
        gt->SetLabel(wxT(""));
        bt->SetLabel(wxT(""));
        return;
    }

    cp->SetBackgroundColour(wxColour(r, g, b));
    wxString s;
    // FIXME: make shift an option; currently hard-coded to rgb555
    s.Printf(wxT("%d"), r >> 3);
    rt->SetLabel(s);
    s.Printf(wxT("%d"), g >> 3);
    gt->SetLabel(s);
    s.Printf(wxT("%d"), b >> 3);
    bt->SetLabel(s);
}

IMPLEMENT_DYNAMIC_CLASS(PixView, wxPanel);

BEGIN_EVENT_TABLE(PixView, wxPanel)
EVT_PAINT(PixView::Redraw)
EVT_LEFT_UP(PixView::SelPoint)
END_EVENT_TABLE()

bool PixView::InitBMP(int w, int h, ColorView* cv) {
    im = wxImage(w, h);
    bm = new wxBitmap(im);

    if (!bm)
        return false;

    selx = sely = -1;
    cview = cv;
    return true;
}

void PixView::SetData(const unsigned char* data, int stride, int x, int y) {
    if (!bm)
        return;

    ox = x;
    oy = y;

    if (!data) {
        im.SetRGB(wxRect(0, 0, im.GetWidth(), im.GetHeight()), 0, 0, 0);
        selx = sely = -1;
    } else {
        data += (y * stride + x) * 3;

        for (y = 0; y < im.GetHeight(); y++) {
            for (x = 0; x < im.GetWidth(); x++) {
                im.SetRGB(x, y, *data, data[1], data[2]);
                data += 3;
            }

            data += 3 * (stride - x);
        }
    }

    delete bm;
    bm = new wxBitmap(im);

    if (selx >= 0 && cview)
        cview->SetRGB(im.GetRed(selx, sely), im.GetGreen(selx, sely), im.GetBlue(selx, sely));
    else if (cview)
        cview->SetRGB(-1, -1, -1);

    Refresh();
}

void PixView::SetSel(int x, int y, bool desel_cview_update) {
    if (x >= ox && y >= oy && x - ox < im.GetWidth() && y - oy < im.GetHeight()) {
        int oselx = selx, osely = sely;
        selx = x - ox;
        sely = y - oy;

        if (selx != oselx || sely != osely) {
            if (cview)
                cview->SetRGB(im.GetRed(selx, sely), im.GetGreen(selx, sely),
                              im.GetBlue(selx, sely));

            Refresh();
        }
    } else {
        bool r = selx >= 0 && sely >= 0;
        selx = sely = -1;

        if (r) {
            if (cview && desel_cview_update)
                cview->SetRGB(-1, -1, -1);

            Refresh();
        }
    }
}

void PixView::Redraw(wxPaintEvent& ev) {
    (void)ev;  // unused params
    if (!bm)
        return;

    wxPaintDC dc(this);
    double sx, sy;
    int w, h;
    GetClientSize(&w, &h);
    sx = (double)w / im.GetWidth();
    sy = (double)h / im.GetHeight();
    dc.SetUserScale(sx, sy);
    dc.DrawBitmap(*bm, 0, 0, false);
    dc.SetUserScale(sx / 8, sy / 8);
    // grid color is hard-coded to gray
    dc.SetPen(*wxGREY_PEN);

    // grid is only on top/left, just like win32 port
    for (int y = 0; y < im.GetHeight(); y++)
        dc.DrawLine(0, y * 8, im.GetWidth() * 8, y * 8);

    for (int x = 0; x < im.GetWidth(); x++)
        dc.DrawLine(x * 8, 0, x * 8, im.GetHeight() * 8);

    if (selx >= 0) {
        // sel color is hard-coded to red
        dc.SetPen(*wxRED_PEN);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRectangle(selx * 8, sely * 8, 8, 8);
    }
}

void PixView::SelPoint(wxMouseEvent& ev) {
    if (!bm)
        return;

    int w, h;
    GetClientSize(&w, &h);
    wxPoint p = ev.GetPosition();

    if (p.x < 0 || p.x >= w || p.y < 0 || p.y >= h) {
        bool r = selx >= 0 && sely >= 0;
        selx = sely = -1;

        if (r) {
            Refresh();

            if (cview)
                cview->SetRGB(-1, -1, -1);
        }

        return;
    }

    int oselx = selx, osely = sely;
    selx = p.x * im.GetWidth() / w;
    sely = p.y * im.GetHeight() / h;

    if (selx != oselx || sely != osely) {
        Refresh();

        if (cview)
            cview->SetRGB(im.GetRed(selx, sely), im.GetGreen(selx, sely), im.GetBlue(selx, sely));
    }
}

wxIMPLEMENT_DYNAMIC_CLASS(PixViewEvt, PixView);

void PixViewEvt::SetData(const unsigned char* data, int stride, int x, int y) {
    PixView::SetData(data, stride, x, y);

    if (selx >= 0 && sely >= 0)
        click();
}

void PixViewEvt::SelPoint(wxMouseEvent& ev) {
    PixView::SelPoint(ev);
    click();
}

void PixViewEvt::click() {
    wxMouseEvent ev;
    ev.SetEventType(EVT_COMMAND_GFX_CLICK);
    ev.ResumePropagation(wxEVENT_PROPAGATE_MAX);
    ev.SetEventObject(this);
    ev.SetId(GetId());
    // ev.SetX(selx);
    ev.m_x = selx;
    // ev.SetY(sely);
    ev.m_y = sely;
    GetEventHandler()->ProcessEvent(ev);
}

wxIMPLEMENT_DYNAMIC_CLASS(DispCheckBox, wxCheckBox);

BEGIN_EVENT_TABLE(DispCheckBox, wxCheckBox)
EVT_LEFT_DOWN(DispCheckBox::MouseEvent)
EVT_LEFT_UP(DispCheckBox::MouseEvent)
END_EVENT_TABLE()

wxIMPLEMENT_DYNAMIC_CLASS(GfxPanel, wxPanel);

void GfxPanel::DrawBitmap(wxPaintEvent& ev) {
    (void)ev;  // unused params
    if (!bm)
        return;

    wxPaintDC dc(this);
    wxSize sz = GetClientSize();
    double scalex = (double)sz.GetWidth() / bmw;
    double scaley = (double)sz.GetHeight() / bmh;
    dc.SetUserScale(scalex, scaley);
    dc.DrawBitmap(*bm, 0, 0);

    if (selx >= 0) {
        if (selx > bmw - 4 || sely > bmh - 4)
            pv->SetData(NULL, 0, 0, 0);
        else
            pv->SetData(im->GetData(), im->GetWidth(), selx - 4, sely - 4);
    }
}

void GfxPanel::Click(wxMouseEvent& ev) {
    DoSel(ev, ev.GetEventType() == wxEVT_LEFT_DOWN);
}

void GfxPanel::MouseMove(wxMouseEvent& ev) {
    if (!ev.LeftIsDown()) {
        ev.Skip();
        return;
    }

    DoSel(ev);
}

void GfxPanel::DoSel(wxMouseEvent& ev, bool force) {
    if (!bm)
        return;

    int x = ev.GetX(), y = ev.GetY();

    if (x < 0 || y < 0)
        return;

    wxSize sz = GetClientSize();

    if (x > sz.GetWidth() || y > sz.GetHeight())
        return;

    x = x * bmw / sz.GetWidth();
    y = y * bmh / sz.GetHeight();
    wxMouseEvent ev2(ev);
    ev2.SetEventType(EVT_COMMAND_GFX_CLICK);
    ev2.ResumePropagation(wxEVENT_PROPAGATE_MAX);
    ev2.SetEventObject(this);
    ev2.SetId(GetId());
    // ev2.SetX(x);
    ev2.m_x = x;
    // ev2.SetY(y);
    ev2.m_y = y;
    GetEventHandler()->ProcessEvent(ev2);

    if (x < 4)
        x = 4;
    else if (x > bmw - 4)
        x = bmw - 4;

    if (y < 4)
        y = 4;
    else if (y > bmh - 4)
        y = bmh - 4;

    if (force || selx != x || sely != y) {
        selx = x;
        sely = y;
        pv->SetData(im->GetData(), im->GetWidth(), selx - 4, sely - 4);
    }
}

DEFINE_EVENT_TYPE(EVT_COMMAND_GFX_CLICK)

BEGIN_EVENT_TABLE(GfxPanel, wxPanel)
EVT_PAINT(GfxPanel::DrawBitmap)
EVT_LEFT_DOWN(GfxPanel::Click)
EVT_LEFT_UP(GfxPanel::Click)
EVT_MOTION(GfxPanel::MouseMove)
END_EVENT_TABLE()

GfxViewer::GfxViewer(const wxString& dname, int maxw, int maxh)
    : BaseViewer(dname), image(maxw, maxh) {
    gv = XRCCTRL(*this, "GfxView", GfxPanel);
    if (!gv) {
        baddialog();
    }

    gvs_ = dynamic_cast<wxScrolledWindow*>(gv->GetParent());
    if (!gvs_) {
        baddialog();
    }

    gvs_->SetMinSize(gvs_->GetSize());
    gvs_->SetScrollRate(1, 1);
    gv->SetSize(maxw, maxh);
    gvs_->SetVirtualSize(maxw, maxh);
    gv->im = &image;
    gv->bmw = maxw;
    gv->bmh = maxh;
    ColorView* cv;
    colorctrl(cv, "Color");
    pixview(gv->pv, "Zoom", 8, 8, cv);
    str_ = XRCCTRL(*this, "Stretch", wxCheckBox);

    if (!str_)
        baddialog();
}

void GfxViewer::ChangeBMP() {
    delete gv->bm;
    gv->bm = new wxBitmap(image);
    gv->Refresh();
}

void GfxViewer::BMPSize(int w, int h) {
    if (gv->bmw != w || gv->bmh != h) {
        gv->bmw = w;
        gv->bmh = h;

        if (!str_->GetValue()) {
            gv->SetSize(w, h);
            gvs_->SetVirtualSize(gv->GetSize());
        }
    }
}

void GfxViewer::StretchTog(wxCommandEvent& ev) {
    (void)ev;  // unused params
    wxSize sz;

    if (str_->GetValue()) {
        // first time to remove scrollbars
        gvs_->SetVirtualSize(gvs_->GetClientSize());
        // second time to expand to outer edges
        sz = gvs_->GetClientSize();
    } else
        sz = wxSize(gv->bmw, gv->bmh);

    gv->SetSize(sz);
    gvs_->SetVirtualSize(sz);
}

void GfxViewer::SaveBMP(wxCommandEvent& ev) {
    (void)ev;  // unused params
    GameArea* panel = wxGetApp().frame->GetPanel();
    bmp_save_dir_ = wxGetApp().frame->GetGamePath(OPTION(kGenScreenshotDir));
    // no attempt is made here to translate the dialog type name
    // it's just a suggested name, anyway
    wxString def_name = panel->game_name() + wxT('-') + dname;
    def_name.resize(def_name.size() - 6);  // strlen("BaseViewer")

    const int capture_format = OPTION(kPrefCaptureFormat);
    if (capture_format == 0)
        def_name.append(".png");
    else
        def_name.append(".bmp");

    wxFileDialog dlg(GetGrandParent(), _("Select output file"), bmp_save_dir_, def_name,
                     _("PNG images|*.png|BMP images|*.bmp"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    dlg.SetFilterIndex(capture_format);
    int ret = dlg.ShowModal();
    bmp_save_dir_ = dlg.GetDirectory();

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

void GfxViewer::RefreshEv(wxCommandEvent& ev) {
    (void)ev;  // unused params
    Update();
}

wxString GfxViewer::bmp_save_dir_ = wxEmptyString;

BEGIN_EVENT_TABLE(GfxViewer, BaseViewer)
EVT_CHECKBOX(XRCID("Stretch"), GfxViewer::StretchTog)
EVT_BUTTON(XRCID("Refresh"), GfxViewer::RefreshEv)
EVT_BUTTON(XRCID("Save"), GfxViewer::SaveBMP)
EVT_BUTTON(XRCID("SaveGBOAM"), GfxViewer::SaveBMP)
EVT_BUTTON(XRCID("SaveGBAOAM"), GfxViewer::SaveBMP)
END_EVENT_TABLE()

}  // namespace viewers
