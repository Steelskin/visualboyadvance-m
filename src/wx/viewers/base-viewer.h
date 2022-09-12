#ifndef VBAM_WX_VIEWERS_BASE_VIEWER_H_
#define VBAM_WX_VIEWERS_BASE_VIEWER_H_

#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/image.h>
#include <wx/panel.h>
#include <wx/spinbutt.h>
#include <wx/stattext.h>

class wxCheckBox;

namespace viewers {

// Display a color in a square, with the RGB value to its right.

// this is too hard to integrate into xrc (impossible to initialize
// correctly) so no accomodations are made for this

class ColorView : public wxControl {
public:
    ColorView(wxWindow* parent, wxWindowID id);
    void SetRGB(int r, int g, int b);
    void GetRGB(int& _r, int& _g, int& _b) {
        _r = r_;
        _g = g_;
        _b = b_;
    }

protected:
    int r_, g_, b_;
    wxPanel* cp;
    wxStaticText *rt, *gt, *bt;
};

// Display a small bitmap in jumbopixel style.  If a pixel is selected, it
// is highlighted with a border.  For wxvbam, no event is generated.
// Instead, a ColorView can be assigned to it, and on selection, that
// widget will be updated to the selected color.
// The whole class can also be derived to add more functionality to the
// button click.

// It must be intialized in 3 phases: 2-phase xrc-style (new + Create()),
// and then InitBMP()

class PixView : public wxPanel {
public:
    PixView() : wxPanel(), bm(0) {}
    bool InitBMP(int w = 8, int h = 8, ColorView* cv = NULL);
    // stride is in pixels
    // format is rgb24 (aka wxImage format)
    // x/y is added to data and returned coords
    // if data == NULL, bitmap will be reset to default (all-black)
    virtual void SetData(const unsigned char* data, int stride, int x = 0, int y = 0);
    // desel if out of displayed range
    void SetSel(int x, int y, bool dsel_cview_update = true);
    // -1, -1 = no sel
    void GetSel(int& x, int& y) {
        x = selx < 0 ? -1 : ox + selx;
        y = sely < 0 ? -1 : oy + sely;
    }
    ColorView* cview;

protected:
    wxImage im;
    wxBitmap* bm;
    void Redraw(wxPaintEvent& ev);
    virtual void SelPoint(wxMouseEvent& ev);
    int ox, oy, selx, sely;

    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS(PixView)
};

// a graphics viewer panel; expected to be inside of a wxScrollWindow
class GfxPanel : public wxPanel {
public:
    GfxPanel() : wxPanel(), bm(0), selx(-1), sely(-1) {}
    int bmw, bmh;
    wxBitmap* bm;
    wxImage* im;
    PixView* pv;

protected:
    void DrawBitmap(wxPaintEvent& ev);
    void Click(wxMouseEvent& ev);
    void MouseMove(wxMouseEvent& ev);
    void DoSel(wxMouseEvent& ev, bool force = false);

private:
    int selx, sely;

    DECLARE_DYNAMIC_CLASS(GfxPanel)
    DECLARE_EVENT_TABLE()
};

// if the jumbopixel view is all there is, maybe send a GFX_CLICK..
class PixViewEvt : public PixView {
    // generates a GFX_CLICK if a point is selected
    void SetData(const unsigned char* data, int stride, int x = 0, int y = 0);

protected:
    // always generates a GFX_CLICK
    void SelPoint(wxMouseEvent& ev);
    void click();
    wxDECLARE_DYNAMIC_CLASS(PixViewEvt);
};

// a display-only checkbox which does not look like it's disabled
class DispCheckBox : public wxCheckBox {
public:
    bool AcceptsFocus() const { return false; }
    void MouseEvent(wxMouseEvent& ev) {
        (void)ev;  // unused param
    }
    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS(DispCheckBox);
};

BEGIN_DECLARE_EVENT_TYPES()
// event generated on mouse click
// generates wxMouseEvent with coords adjusted to original bitmap
// size regardless of scaling
DECLARE_LOCAL_EVENT_TYPE(EVT_COMMAND_GFX_CLICK, 0)
END_DECLARE_EVENT_TYPES()
#define EVT_GFX_CLICK(id, fun)                                                                     \
    DECLARE_EVENT_TABLE_ENTRY(EVT_COMMAND_GFX_CLICK, id, wxID_ANY, wxMouseEventHandler(fun), NULL) \
    ,

class BaseViewer : public wxDialog {
public:
    BaseViewer(const wxString& name);
    virtual ~BaseViewer() override = default;

    void CloseDlg(wxCloseEvent& ev);

    void Update() override = 0;

    bool auto_update_;

    // A lot of viewers have GUI elements to set parameters.  Almost all
    // of them just read back the value and update the display.  This
    // event handler does that with validators.
    void ActiveCtrl(wxCommandEvent& ev);
    void ActiveCtrlScr(wxScrollEvent& ev) { ActiveCtrl(ev); }
    void ActiveCtrlSpin(wxSpinEvent& ev) { ActiveCtrl(ev); }

protected:
    wxString dname;
    void SetAutoUpdate(wxCommandEvent& ev) { auto_update_ = ev.IsChecked(); }

    wxDECLARE_EVENT_TABLE();
};

BEGIN_DECLARE_EVENT_TYPES()
// event generated when fewer lines available than needed
DECLARE_LOCAL_EVENT_TYPE(EVT_REFILL_NEEDED, 0)
END_DECLARE_EVENT_TYPES()

class GfxViewer : public BaseViewer {
public:
    GfxViewer(const wxString& dname, int maxw, int maxh);
    void ChangeBMP();
    void BMPSize(int w, int h);

protected:
    void StretchTog(wxCommandEvent& ev);
    void RefreshEv(wxCommandEvent& ev);
    void SaveBMP(wxCommandEvent& ev);
    wxImage image;
    GfxPanel* gv;

private:
    static wxString bmp_save_dir_;
    wxScrolledWindow* gvs_;
    wxCheckBox* str_;

    DECLARE_EVENT_TABLE()
};

}  // namespace viewers

#endif  // VBAM_WX_VIEWERS_BASE_VIEWER_H_
