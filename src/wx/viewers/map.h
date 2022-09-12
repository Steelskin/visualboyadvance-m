#ifndef VBAM_WX_VIEWERS_MAP_H_
#define VBAM_WX_VIEWERS_MAP_H_

#include "wx/viewers/base-viewer.h"

class wxRadioButton;

namespace viewers {

class MapViewer : public GfxViewer {
public:
    MapViewer();
    ~MapViewer() override = default;

    void Update() override;

    void UpdateMouseInfoEv(wxMouseEvent& ev);
    uint32_t AddressFromSel();

    void UpdateMouseInfo();

protected:
    uint16_t control, mode;
    int frame, bg;
    wxRadioButton *fr0, *fr1, *bg0, *bg1, *bg2, *bg3;
    wxControl *modelab, *mapbase, *charbase, *size, *colors, *prio, *mosaic,
        *overflow;
    wxControl *coords_, *addr_, *tile_, *flip_, *palette_;
    int selx, sely;

    // following routines were copied from win32/MapView.cpp with little
    // attempt to read & validate, except:
    //    stride = 1024, rgb instead of bgr
    // FIXME: probably needs changing for big-endian

    void renderTextScreen();
    void renderRotScreen();

    void renderMode3();
    void renderMode4();
    void renderMode5();

    wxDECLARE_EVENT_TABLE();
};

class GBMapViewer : public GfxViewer {
public:
    GBMapViewer();
    ~GBMapViewer() override = default;

    void Update() override;
    void UpdateMouseInfoEv(wxMouseEvent& ev);
    void UpdateMouseInfo();

protected:
    int charbase, mapbase;
    wxControl *coords_, *addr_, *tile_, *flip_, *palette_, *prio_;
    int selx, sely;

    DECLARE_EVENT_TABLE()
};

}  // namespace viewers

#endif  // VBAM_WX_VIEWERS_MAP_H_
