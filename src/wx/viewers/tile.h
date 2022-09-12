#ifndef VBAM_WX_VIEWERS_TILE_H_
#define VBAM_WX_VIEWERS_TILE_H_

#include "wx/viewers/base-viewer.h"

namespace viewers {

class TileViewer : public GfxViewer {
public:
    TileViewer();
    ~TileViewer() override = default;

    void Update() override;
    void UpdateMouseInfoEv(wxMouseEvent& ev);
    void UpdateMouseInfo();
    // following 2 functions copied almost verbatim from TileView.cpp
    void render256(int tile, int x, int y, uint8_t* charBase, uint16_t* palette);
    void render16(int tile, int x, int y, uint8_t* charBase, uint16_t* palette);
    void SaveGBATile(wxCommandEvent& ev);

protected:
    int charbase_ = 0;
    int is256_ = 0;
    int palette_ = 0;
    wxControl *tileno_, *addr_;
    int selx_, sely_;

    wxDECLARE_EVENT_TABLE();
};

class GBTileViewer : public GfxViewer {
public:
    GBTileViewer();
    ~GBTileViewer() override = default;

    void Update() override;
    void UpdateMouseInfoEv(wxMouseEvent& ev);
    void UpdateMouseInfo();

    // following function copied almost verbatim from GBTileView.cpp
    void render(int tile, int x, int y, uint8_t* charBase);
    void SaveGBTile(wxCommandEvent& ev);

protected:
    int bank, charbase, palette;
    wxControl *addr, *tileno;
    int selx, sely;

    wxDECLARE_EVENT_TABLE();
};

}  // namespace viewers

#endif  // VBAM_WX_VIEWERS_TILE_H_