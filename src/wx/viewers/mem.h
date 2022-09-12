#ifndef VBAM_WX_VIEWERS_MEM_H_
#define VBAM_WX_VIEWERS_MEM_H_

#include "wx/viewers/base-viewer.h"

class wxChoice;
class wxTextCtrl;

namespace viewers {

class MemView;

class MemViewerBase : public BaseViewer {
public:
    MemViewerBase(uint32_t max);
    ~MemViewerBase() override = default;

    void BlockStart(wxCommandEvent& ev);
    void GotoEv(wxCommandEvent& ev);
    void Goto(uint32_t addr);
    void RefreshCmd(wxCommandEvent& ev);

    void Load(wxCommandEvent& ev);
    void Save(wxCommandEvent& ev);

    void Update() override;

    virtual void MemLoad(wxString& name, uint32_t addr, uint32_t len) = 0;
    virtual void MemSave(wxString& name, uint32_t addr, uint32_t len) = 0;

protected:
    int addrlen;
    wxChoice* bs;
    wxTextCtrl* goto_addr;
    MemView* mv;

    wxDialog* selregion;
    wxTextCtrl *selreg_addr, *selreg_len;
    wxControl* selreg_lenlab;
    wxString memsave_fn;

    wxDECLARE_EVENT_TABLE();
};

class MemViewer : public MemViewerBase {
public:
    MemViewer();
    ~MemViewer() override = default;

    // wx-2.8.4 or MacOSX compiler can't resolve overloads in evt table
    void RefillListEv(wxCommandEvent& ev);

    void WriteVal(wxCommandEvent& ev);

    void Update() override;

    void MemLoad(wxString& name, uint32_t addr, uint32_t len) override;
    void MemSave(wxString& name, uint32_t addr, uint32_t len) override;

    wxDECLARE_EVENT_TABLE();
};

class GBMemViewer : public MemViewerBase {
public:
    GBMemViewer();
    ~GBMemViewer() override = default;

    // wx-2.8.4 or MacOSX compiler can't resolve overloads in evt table
    void RefillListEv(wxCommandEvent& ev);

    void Update() override;

    void WriteVal(wxCommandEvent& ev);

    void MemLoad(wxString& name, uint32_t addr, uint32_t len) override;

    void MemSave(wxString& name, uint32_t addr, uint32_t len) override;
    wxDECLARE_EVENT_TABLE();
};

}  // namespace viewers

#endif  // VBAM_WX_VIEWERS_MEM_H_
