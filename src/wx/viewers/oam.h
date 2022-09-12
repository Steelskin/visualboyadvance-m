#ifndef VBAM_WX_VIEWERS_OAM_H_
#define VBAM_WX_VIEWERS_OAM_H_

#include "wx/viewers/base-viewer.h"

namespace viewers {

class OAMViewer : public GfxViewer {
public:
    OAMViewer();
    ~OAMViewer() override = default;

    void Update() override;

protected:
    int sprite;
    wxControl *pos, *mode, *colors, *pallab, *tile, *prio, *size, *rot, *flg;
};

class GBOAMViewer : public GfxViewer {
public:
    GBOAMViewer();
    ~GBOAMViewer() override = default;

    void Update() override;

protected:
    int sprite;
    wxControl *pos, *tilelab, *prio, *oap, *pallab, *flg, *banklab;
};

}  // namespace viewers

#endif  // VBAM_WX_VIEWERS_OAM_H_
