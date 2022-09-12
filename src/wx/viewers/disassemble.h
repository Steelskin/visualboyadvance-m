#ifndef VBAM_WX_VIEWERS_DISASSEMBLE_H_
#define VBAM_WX_VIEWERS_DISASSEMBLE_H_

#include <wx/checkbox.h>
#include <wx/scrolbar.h>
#include <wx/textctrl.h>

#include "wx/viewers/base-viewer.h"

namespace viewers {

class DisList;

class DisassembleViewer : public BaseViewer {
public:
    DisassembleViewer();
    ~DisassembleViewer() override = default;

    void Update() override;
    void Next(wxCommandEvent& ev);
    void Goto(wxCommandEvent& ev);

    // wx-2.8.4 or MacOSX compiler can't resolve overloads in evt table
    void GotoPCEv(wxCommandEvent&);
    void GotoPC();
    void RefreshCmd(wxCommandEvent& ev);
    void UpdateDis();

    void RefillListEv(wxCommandEvent& ev);

private:
    enum class DisassemblyMode {
        Automatic,
        Arm,
        Thumb,
    };

    void AutomaticMode(wxCommandEvent& ev);
    void ArmMode(wxCommandEvent& ev);
    void ThumbMode(wxCommandEvent& ev);

    DisList* dis;
    wxTextCtrl* goto_addr;
    wxCheckBox *N, *Z, *C, *V, *I, *F, *T;
    DisassemblyMode disassembly_mode_;
    wxControl *regv[17], *Modev;

    wxDECLARE_EVENT_TABLE();
};

class GBDisassembleViewer : public BaseViewer {
public:
    GBDisassembleViewer();
    ~GBDisassembleViewer() override = default;

    void Update() override;
    void Next(wxCommandEvent& ev);
    void Goto(wxCommandEvent& ev);
    // wx-2.8.4 or MacOSX compiler can't resolve overloads in evt table
    void GotoPCEv(wxCommandEvent& ev);
    void GotoPC();
    void RefreshCmd(wxCommandEvent& ev);
    void UpdateDis();

    void RefillListEv(wxCommandEvent& ev);

    DisList* dis;
    wxTextCtrl* goto_addr;
    wxControl *AFv, *BCv, *DEv, *HLv, *SPv, *PCv, *LYv, *IFFv;
    wxCheckBox *Z, *N, *H, *C;
    wxDECLARE_EVENT_TABLE();
};

}  // namespace viewers

#endif  // VBAM_WX_VIEWERS_DISASSEMBLE_H_