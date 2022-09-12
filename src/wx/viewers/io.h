#ifndef VBAM_WX_VIEWERS_IO_H_
#define VBAM_WX_VIEWERS_IO_H_

#include "wx/viewers/base-viewer.h"

class wxChoice;
class wxCheckBox;

namespace viewers {

class IOViewer : public BaseViewer {
public:
    IOViewer();
    ~IOViewer() override = default;

    void SelectEv(wxCommandEvent& ev);
    void Select(int sel);
    void Update() override;

    void Update(int sel);
    void CheckBit(wxCommandEvent& ev);

    void RefreshEv(wxCommandEvent& ev);

    void Apply(wxCommandEvent& ev);

    wxChoice* addr_;
    wxControl* val;
    wxCheckBox* bit[16];
    wxControl* bitlab[16];

    static wxString lline;

    wxDECLARE_EVENT_TABLE();
};

}  // namespace viewers

#endif  // VBAM_WX_VIEWERS_IO_H_