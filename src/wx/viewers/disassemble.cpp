#include "wx/viewers/disassemble.h"

#include <wx/log.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valtext.h>

#include "core/gb/gb.h"
#include "core/gb/gbDis.h"
#include "core/gb/gbGlobals.h"
#include "core/gba/gba.h"
#include "core/gba/gbaCpuArmDis.h"
#include "core/gba/gbaGlobals.h"
#include "wx/main-frame.h"
#include "wx/widgets/wxmisc.h"
#include "wx/wxhead.h"
#include "wx/wxvbam.h"

namespace viewers {

#define baddialog()                                                              \
    do {                                                                         \
        wxLogError(_("Unable to load dialog %s from resources"), dname.c_str()); \
        wxGetApp().frame->Close(true);                                           \
        return;                                                                  \
    } while (0)
#define gethex(v, n, l)                                 \
    do {                                                \
        v = XRCCTRL(*this, n, wxTextCtrl);              \
        if (!v)                                         \
            baddialog();                                \
        wxTextValidator hv(wxFILTER_INCLUDE_CHAR_LIST); \
        hv.SetIncludes(val_hexdigits);                  \
        v->SetValidator(hv);                            \
        v->SetMaxLength(l);                             \
    } while (0)

// a list box with no horizontal scrollbar and a funky vertical scrollbar:
// range = 1 - 500
//   but/pagesz = # of lines shown/# of lines shown - 1
// 1-100 = normal
// 101-200 = 10x semi-stationary
// 201-300 = stationary @ center
// 301-400 = 10x semi-stationary
// 401-500 = normal

// note that since listboxes' size is impossible to control correctly (at
// least with wxGTK), this uses a textctrl to display the items.

class DisList : public wxPanel {
public:
    DisList();
    ~DisList() override = default;

    // called after init to create subcontrols and size panel
    void Refit(int cols);
    void MoveSB();
    void MoveView(wxScrollEvent& ev);

    // called by parent's refill handler or any other time strings have
    // changed
    void Refill();

    // make addr visible and then select it
    void SetSel(uint32_t addr);
    void UnSel() { issel = false; }

    // currently visible lines
    int nlines;
    // at least nlines strings to display
    wxArrayString strings;
    // and their starting addrs (mostly for scrollbar)
    wxArrayInt addrs;
    // how far back to scroll for single line
    int back_size;
    // address of top line
    uint32_t topaddr;
    // max address for scrollbar
    uint32_t maxaddr;

    wxDECLARE_DYNAMIC_CLASS(DisList);
    wxDECLARE_EVENT_TABLE();

protected:
    // assigned to textctrl to avoid mouse input
    void MouseEvent(wxMouseEvent& ev) {
        (void)ev;  // unused param
    }
    // the subwidgets
    wxTextCtrl tc;
    wxScrollBar sb;
    // cached computed line tc size
    int lineheight, extraheight;
    // need to know if tc/sb have been Create()d yet
    bool didinit;
    // selection info
    uint32_t seladdr;
    bool issel;

private:
    void RefillNeeded();
    void Resize(wxSizeEvent& ev);
    void SetSel();
};

wxIMPLEMENT_DYNAMIC_CLASS(DisList, wxPanel);

DisList::DisList() : wxPanel(), nlines(0), topaddr(0), tc(), sb(), didinit(false), issel(false) {}

void DisList::Refit(int cols) {
    if (!didinit) {
        tc.Create(this, wxID_ANY, wxEmptyString, wxPoint(0, 0), wxDefaultSize,
                  wxTE_READONLY | wxTE_MULTILINE | wxTE_DONTWRAP | wxTE_NOHIDESEL);
        tc.Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(DisList::MouseEvent), NULL, this);
        tc.Connect(wxEVT_LEFT_UP, wxMouseEventHandler(DisList::MouseEvent), NULL, this);
        tc.Connect(wxEVT_MOTION, wxMouseEventHandler(DisList::MouseEvent), NULL, this);
        // FIXME: take tc out of tab order
        sb.Create(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
        sb.SetScrollbar(0, 15, 500, 15);
        didinit = true;
    }

    wxString line(wxT('M'), cols);
    // GetBestSize() in wxGTK is pretty damn worthless.  It's surprising
    // autosizing works at all.  wxGTK does not give good values
    // for wxTextCtrl (always 80xht_of_1liner)
    // Given this, we'll fake it using wxStaticText for lineheight
    // and add an extra line for extraheight
    // Just using GetTextExtent for lineheight might not work if extra
    // spacing is usually added between lines.
    wxStaticText st(this, wxID_ANY, line);
    wxSize winsz = st.GetBestSize();
    st.SetLabel(line + wxT('\n') + line);
    wxSize winsz2 = st.GetBestSize();
    lineheight = winsz2.GetHeight() - winsz.GetHeight();
    extraheight = winsz.GetHeight() /* - lineheight */;
    winsz.SetHeight(extraheight + 15 * lineheight);
    winsz.SetWidth(winsz.GetWidth() + sb.GetBestSize().GetWidth());
    SetMinSize(winsz);
    SetSize(winsz);
}

void DisList::MoveSB() {
    int pos;

    if (topaddr <= 100)
        pos = topaddr;
    else if (topaddr >= maxaddr - 100)
        pos = topaddr - maxaddr + 500;
    else if (topaddr < 1100)
        pos = (topaddr - 100) / 10 + 100;
    else if (topaddr >= maxaddr - 1100)
        pos = (topaddr - maxaddr + 1100) / 10 + 300;
    else  // FIXME this pos is very likely wrong... but I cannot trigger it
        pos = 250;

    sb.SetScrollbar(pos, 20, 500, 20);
}

void DisList::MoveView(wxScrollEvent& ev) {
    int pos = ev.GetPosition();

    if (pos < 100)
        topaddr = pos;
    else if (pos >= 400)
        topaddr = maxaddr + pos - 500;
    else if (ev.GetEventType() == wxEVT_SCROLL_LINEUP) {
        topaddr -= back_size;
        MoveSB();
    } else if (ev.GetEventType() == wxEVT_SCROLL_LINEDOWN) {
        topaddr = addrs[1];
        MoveSB();
    } else if (ev.GetEventType() == wxEVT_SCROLL_PAGEUP) {
        topaddr -= (nlines - 2) * back_size;
        MoveSB();
    } else if (ev.GetEventType() == wxEVT_SCROLL_PAGEDOWN) {
        topaddr = addrs[nlines - 2];
        MoveSB();
    } else if (ev.GetEventType() == wxEVT_SCROLL_THUMBRELEASE) {
        if (pos <= 200)
            topaddr = (pos - 100) * 10 + 100;
        else if (pos >= 300)
            topaddr = (pos - 300) * 10 + maxaddr - 1100;
        else
            // 200 .. 300 -> 1100 .. maxaddr - 1100
            topaddr = (pos - 200) * ((maxaddr - 2200) / 100) + 1100;

        MoveSB();
    }  // ignore THUMBTRACK and CHANGED

    RefillNeeded();
}

// generate event; due to ProcessEvent()'s nature, it will return only
// when refill complete
void DisList::RefillNeeded() {
    wxCommandEvent ev(EVT_REFILL_NEEDED, GetId());
    ev.SetEventObject(this);
    GetEventHandler()->ProcessEvent(ev);
}

// called by parent's refill handler or any other time strings have changed
void DisList::Refill() {
    MoveSB();
    wxString val;

    for (size_t i = 0; i < (size_t)nlines && i < strings.size(); i++) {
        val += strings[i];
        val += wxT('\n');
    }

    tc.SetValue(val);
    SetSel();
}

// on resize, recompute shown lines and refill if necessary
void DisList::Resize(wxSizeEvent& ev) {
    (void)ev;      // unused params
    if (!didinit)  // prevent crash on win32
        return;

    wxSize sz = GetSize();
    int sbw = sb.GetSize().GetWidth();
    sz.SetWidth(sz.GetWidth() - sbw);
    sb.Move(sz.GetWidth(), 0);
    sb.SetSize(sbw, sz.GetHeight());
    nlines = (sz.GetHeight() + lineheight - 1) / lineheight;
    wxString val;
    tc.SetSize(sz.GetWidth(), (nlines + 1) * lineheight + extraheight);

    if ((size_t)nlines > strings.size())
        RefillNeeded();
    else {
        for (size_t i = 0; i < (size_t)nlines && i < strings.size(); i++) {
            val += strings[i];
            val += wxT('\n');
        }

        tc.SetValue(val);
        SetSel();
    }
}

// highlight selected line, if visible
void DisList::SetSel() {
    tc.SetSelection(0, 0);

    if (!issel)
        return;

    if ((size_t)nlines > addrs.size() || (uint32_t)addrs[0] > seladdr ||
        (uint32_t)addrs[nlines - 1] <= seladdr)
        return;

    for (int i = 0, start = 0; i < nlines; i++) {
        if ((uint32_t)addrs[i + 1] > seladdr) {
            int end = start + strings[i].size() + 1;
// on win32, wx helpfully inserts a CR before every LF
// it also doesn't highlight the whole line, but that's
// not critical
#ifdef __WXMSW__
            start += i;
            end += i + 1;
#endif
            tc.SetSelection(start, end);
            return;
        }

        start += strings[i].size() + 1;
    }
}

void DisList::SetSel(uint32_t addr) {
    seladdr = addr;
    issel = true;

    if (addrs.size() < 4 || addrs.size() < (size_t)nlines || topaddr > addr ||
        (uint32_t)addrs[addrs.size() - 4] < addr) {
        topaddr = addr;
        strings.clear();
        addrs.clear();
        RefillNeeded();
    } else
        SetSel();
}

BEGIN_EVENT_TABLE(DisList, wxPanel)
EVT_SIZE(DisList::Resize)
EVT_SCROLL(DisList::MoveView)
END_EVENT_TABLE()

DisassembleViewer::DisassembleViewer() : BaseViewer(wxT("Disassemble")) {
    gethex(goto_addr, "GotoAddress", 8);
    goto_addr->SetFocus();

    for (int i = 0; i < 17; i++) {
        // it is unfortunately impossible to reliably assign
        // number ranges to XRC IDs, so the string name has to be
        // used every time
        wxString n;
        n.Printf(wxT("R%d"), i);
        regv[i] = XRCCTRL_D(*this, n, wxControl);

        if (!regv[i])
            baddialog();
    }

#define flagctrl(n)                         \
    do {                                    \
        n = XRCCTRL(*this, #n, wxCheckBox); \
        if (!n)                             \
            baddialog();                    \
    } while (0)
#define regctrl(n)                            \
    do {                                      \
        n##v = XRCCTRL(*this, #n, wxControl); \
        if (!n##v)                            \
            baddialog();                      \
    } while (0)
    flagctrl(N);
    flagctrl(Z);
    flagctrl(C);
    flagctrl(V);
    flagctrl(I);
    flagctrl(F);
    flagctrl(T);
    regctrl(Mode);
    dis = XRCCTRL(*this, "Disassembly", DisList);

    if (!dis)
        baddialog();

    // refit listing for longest line
    dis->Refit(70);
    Fit();
    SetMinSize(GetSize());
    dis->maxaddr = (uint32_t)~0;
    disassembly_mode_ = DisassemblyMode::Automatic;
    GotoPC();
}
void DisassembleViewer::Update() {
    GotoPC();
}
void DisassembleViewer::Next(wxCommandEvent& ev) {
    (void)ev;  // unused params
    CPULoop(1);
    GotoPC();
}
void DisassembleViewer::Goto(wxCommandEvent& ev) {
    (void)ev;  // unused params
    wxString as = goto_addr->GetValue();

    if (!as.size())
        return;

    long a;
    as.ToLong(&a, 16);
    dis->SetSel(a);
    UpdateDis();
}
// wx-2.8.4 or MacOSX compiler can't resolve overloads in evt table
void DisassembleViewer::GotoPCEv(wxCommandEvent&) {
    GotoPC();
}
void DisassembleViewer::GotoPC() {
#if 0

        // this is what the win32 interface used
        if (armState)
            dis->SetSel(armNextPC - 16);
        else
            dis->SetSel(armNextPC - 8);

        // doesn't make sense, though.  Maybe it's just trying to keep the
        // sel 4 instructions below top...
#endif
    dis->SetSel(armNextPC);
    UpdateDis();
}
void DisassembleViewer::RefreshCmd(wxCommandEvent& ev) {
    (void)ev;  // unused params
    UpdateDis();
}
void DisassembleViewer::UpdateDis() {
    N->SetValue(reg[16].I & 0x80000000);
    Z->SetValue(reg[16].I & 0x40000000);
    C->SetValue(reg[16].I & 0x20000000);
    V->SetValue(reg[16].I & 0x10000000);
    I->SetValue(reg[16].I & 0x00000080);
    F->SetValue(reg[16].I & 0x00000040);
    T->SetValue(reg[16].I & 0x00000020);
    wxString s;
    s.Printf(wxT("%02X"), reg[16].I & 0x1f);
    Modev->SetLabel(s);

    for (int i = 0; i < 17; i++) {
        s.Printf(wxT("%08X"), reg[i].I);
        regv[i]->SetLabel(s);
    }
}

void DisassembleViewer::RefillListEv(wxCommandEvent& ev) {
    (void)ev;  // unused params
    // what an unsafe calling convention
    // examination of disArm shows that max len is 69 chars
    // (e.g. 0x081cb6db), and I assume disThumb is shorter
    char buf[4096];
    dis->strings.clear();
    dis->addrs.clear();
    uint32_t addr = dis->topaddr;
    const bool arm_mode = disassembly_mode_ == DisassemblyMode::Arm ||
                          (armState && disassembly_mode_ == DisassemblyMode::Automatic);
    dis->back_size = arm_mode ? 4 : 2;

    for (int i = 0; i < dis->nlines; i++) {
        dis->addrs.push_back(addr);

        if (arm_mode)
            addr += disArm(addr, buf, 4096, DIS_VIEW_CODE | DIS_VIEW_ADDRESS);
        else
            addr += disThumb(addr, buf, 4096, DIS_VIEW_CODE | DIS_VIEW_ADDRESS);

        dis->strings.push_back(wxString(buf, wxConvLibc));
    }

    dis->Refill();
}

void DisassembleViewer::AutomaticMode(wxCommandEvent& ev) {
    disassembly_mode_ = DisassemblyMode::Automatic;
    RefillListEv(ev);
}

void DisassembleViewer::ArmMode(wxCommandEvent& ev) {
    disassembly_mode_ = DisassemblyMode::Arm;
    RefillListEv(ev);
}

void DisassembleViewer::ThumbMode(wxCommandEvent& ev) {
    disassembly_mode_ = DisassemblyMode::Thumb;
    RefillListEv(ev);
}

BEGIN_EVENT_TABLE(DisassembleViewer, BaseViewer)
EVT_COMMAND(wxID_ANY, EVT_REFILL_NEEDED, DisassembleViewer::RefillListEv)
EVT_BUTTON(XRCID("Goto"), DisassembleViewer::Goto)
EVT_TEXT_ENTER(XRCID("GotoAddress"), DisassembleViewer::Goto)
EVT_BUTTON(XRCID("GotoPC"), DisassembleViewer::GotoPCEv)
EVT_BUTTON(XRCID("Next"), DisassembleViewer::Next)
EVT_BUTTON(XRCID("Refresh"), DisassembleViewer::RefreshCmd)
EVT_BUTTON(XRCID("InsAuto"), DisassembleViewer::AutomaticMode)
EVT_BUTTON(XRCID("InsARM"), DisassembleViewer::ArmMode)
EVT_BUTTON(XRCID("InsThumb"), DisassembleViewer::ThumbMode)
END_EVENT_TABLE()

GBDisassembleViewer::GBDisassembleViewer() : BaseViewer(wxT("GBDisassemble")) {
    gethex(goto_addr, "GotoAddress", 4);
    goto_addr->SetFocus();
    regctrl(AF);
    regctrl(BC);
    regctrl(DE);
    regctrl(HL);
    regctrl(SP);
    regctrl(PC);
    regctrl(LY);
    regctrl(IFF);
    flagctrl(Z);
    flagctrl(N);
    flagctrl(H);
    flagctrl(C);
    dis = XRCCTRL(*this, "Disassembly", DisList);

    if (!dis)
        baddialog();

    // refit listing for longest line
    dis->Refit(26);
    Fit();
    SetMinSize(GetSize());
    dis->maxaddr = (uint32_t)~0;
    GotoPC();
}

void GBDisassembleViewer::Update() {
    GotoPC();
}

void GBDisassembleViewer::Next(wxCommandEvent& ev) {
    (void)ev;  // unused params
    gbEmulate(1);
    GotoPC();
}
void GBDisassembleViewer::Goto(wxCommandEvent& ev) {
    (void)ev;  // unused params
    wxString as = goto_addr->GetValue();

    if (!as.size())
        return;

    long a;
    as.ToLong(&a, 16);
    dis->SetSel(a);
    UpdateDis();
}
// wx-2.8.4 or MacOSX compiler can't resolve overloads in evt table
void GBDisassembleViewer::GotoPCEv(wxCommandEvent& ev) {
    (void)ev;  // unused params
    GotoPC();
}
void GBDisassembleViewer::GotoPC() {
    dis->SetSel(PC.W);
    UpdateDis();
}
void GBDisassembleViewer::RefreshCmd(wxCommandEvent& ev) {
    (void)ev;  // unused params
    UpdateDis();
}
void GBDisassembleViewer::UpdateDis() {
    Z->SetValue(AF.B.B0 & GB_Z_FLAG);
    N->SetValue(AF.B.B0 & GB_N_FLAG);
    H->SetValue(AF.B.B0 & GB_H_FLAG);
    C->SetValue(AF.B.B0 & GB_C_FLAG);
#define grv16(n, val)                    \
    do {                                 \
        wxString s;                      \
        s.Printf(wxT("%04X"), (int)val); \
        n##v->SetLabel(s);               \
    } while (0)
#define rv16(n) grv16(n, n.W)
#define srv16(n) grv16(n, n)
    rv16(AF);
    rv16(BC);
    rv16(DE);
    rv16(HL);
    rv16(SP);
    rv16(PC);
    srv16(IFF);
    wxString s;
    s.Printf(wxT("%02X"), register_LY);
    LYv->SetLabel(s);
}

void GBDisassembleViewer::RefillListEv(wxCommandEvent& ev) {
    (void)ev;  // unused params
    // what an unsafe calling convention
    // examination of gbDis shows that max len is 26 chars
    // (e.g. 0xe2)
    char buf[30];
    uint16_t addr = dis->topaddr;
    dis->strings.clear();
    dis->addrs.clear();
    dis->back_size = 1;

    for (int i = 0; i < dis->nlines; i++) {
        dis->addrs.push_back(addr);
        addr += gbDis(buf, addr);
        dis->strings.push_back(wxString(buf, wxConvLibc));
    }

    dis->Refill();
}

BEGIN_EVENT_TABLE(GBDisassembleViewer, BaseViewer)
EVT_COMMAND(wxID_ANY, EVT_REFILL_NEEDED, GBDisassembleViewer::RefillListEv)
EVT_BUTTON(XRCID("Goto"), GBDisassembleViewer::Goto)
EVT_TEXT_ENTER(XRCID("GotoAddress"), GBDisassembleViewer::Goto)
EVT_BUTTON(XRCID("GotoPC"), GBDisassembleViewer::GotoPCEv)
EVT_BUTTON(XRCID("Next"), GBDisassembleViewer::Next)
EVT_BUTTON(XRCID("Refresh"), GBDisassembleViewer::RefreshCmd)
END_EVENT_TABLE()

}  // namespace viewers