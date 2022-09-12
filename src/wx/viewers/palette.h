#ifndef VBAM_WX_VIEWERS_PALETTE_H_
#define VBAM_WX_VIEWERS_PALETTE_H_

#include "wx/viewers/base-viewer.h"

namespace viewers {

class PaletteViewer : public BaseViewer {
public:
    PaletteViewer();
    ~PaletteViewer() override = default;


    void Update() override;
    void SelBG(wxMouseEvent& ev);
    void SelSprite(wxMouseEvent& ev);
    void ShowSel();
    void SaveBG(wxCommandEvent& ev);
    void SaveOBJ(wxCommandEvent& ev);
    void ChangeBackdrop(wxCommandEvent& ev);

protected:
    ColorView* cv;
    PixView *bpv, *spv;
    uint8_t colbmp[16 * 16 * 3 * 2];
    wxControl *addr, *val;

    wxDECLARE_EVENT_TABLE();
};


class GBPaletteViewer : public BaseViewer {
public:
    GBPaletteViewer();
    ~GBPaletteViewer() override = default;

    void Update() override;
    void SelBG(wxMouseEvent& ev);
    void SelSprite(wxMouseEvent& ev);
    void ShowSel();
    void SaveBG(wxCommandEvent& ev);
    void SaveOBJ(wxCommandEvent& ev);

protected:
    ColorView* cv;
    PixView *bpv, *spv;
    uint8_t colbmp[4 * 8 * 3 * 2];
    wxControl *idx, *val;
    DECLARE_EVENT_TABLE()
};

}  // namespace viewers

#endif  // VBAM_WX_VIEWERS_PALETTE_H_