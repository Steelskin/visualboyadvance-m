#include "wx/viewers/mem.h"

#include <wx/caret.h>
#include <wx/choice.h>
#include <wx/dcclient.h>
#include <wx/ffile.h>
#include <wx/filedlg.h>
#include <wx/radiobut.h>
#include <wx/settings.h>
#include <wx/scrolbar.h>
#include <wx/textctrl.h>
#include <wx/valtext.h>

#include "core/gb/gbGlobals.h"
#include "core/gba/gbaInline.h"
#include "wx/config/option-proxy.h"
#include "wx/main-frame.h"
#include "wx/widgets/wxmisc.h"
#include "wx/wxhead.h"
#include "wx/wxvbam.h"

namespace viewers {

// These are what mfc interface used.  Maybe it would be safer
// to avoid the Quick() routines in favor of the long ones...
#define CPUWriteByteQuick(addr, b) \
    ::map[(addr) >> 24].address[(addr) & ::map[(addr) >> 24].mask] = (b)
#define CPUWriteHalfWordQuick(addr, b) \
    WRITE16LE((uint16_t*)&::map[(addr) >> 24].address[(addr) & ::map[(addr) >> 24].mask], b)
#define CPUWriteMemoryQuick(addr, b) \
    WRITE32LE((uint32_t*)&::map[(addr) >> 24].address[(addr) & ::map[(addr) >> 24].mask], b)
#define GBWriteByteQuick(addr, b) *((uint8_t*)&gbMemoryMap[(addr) >> 12][(addr) & 0xfff]) = (b)
#define GBWriteHalfWordQuick(addr, b) \
    WRITE16LE((uint16_t*)&gbMemoryMap[(addr) >> 12][(addr) & 0xfff], b)
#define GBWriteMemoryQuick(addr, b) \
    WRITE32LE((uint32_t*)&gbMemoryMap[(addr) >> 12][(addr) & 0xfff], b)
#define GBReadMemoryQuick(addr) READ32LE((uint32_t*)&gbMemoryMap[(addr) >> 12][(addr) & 0xfff])

#define getvfld(sv, n, t, v)             \
    do {                                 \
        t* _w = sv XRCCTRL(*this, n, t); \
        if (!_w)                         \
            baddialog();                 \
        _w->SetValidator(v);             \
    } while (0)
#define getradio(sv, n, var, val) getvfld(sv, n, wxRadioButton, wxBoolIntValidator(&var, val))

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

namespace {

wxString memsave_dir = wxEmptyString;

int getKeyboardKeyCode(const wxKeyEvent& event) {
    const int key_code = event.GetKeyCode();
    if (key_code > WXK_START) {
        return key_code;
    }
    int uc = event.GetUnicodeKey();
    if (uc != WXK_NONE) {
        if (uc < 32) {  // not all control chars
            switch (uc) {
                case WXK_BACK:
                case WXK_TAB:
                case WXK_RETURN:
                case WXK_ESCAPE:
                    return uc;
                default:
                    return WXK_NONE;
            }
        }
        return uc;
    } else {
        return event.GetKeyCode();
    }
}

}  // namespace

// a hex editor with a funky scrollbar like above

// since it's impossible to exercise this much control over a text
// control, it uses a panel into which text is drawn.
// Maybe some day the above will be changed to do that as well, since
// it allows better mouse control as well.

class MemView : public wxPanel {
public:
    MemView();
    // called after init to create subcontrols and size panel
    void Refit();
    void MoveSB();
    void MoveView(wxScrollEvent& ev);

private:
    void RefillNeeded();

public:
    // called by parent's refill handler or any other time strings have
    // changed
    void Refill();

private:
    void Refill(wxDC& dc);
    void RepaintEv(wxPaintEvent& ev);
    void Repaint();
    void Resize(wxSizeEvent& ev);

public:
    // make addr visible
    void ShowAddr(uint32_t addr, bool force_update = false);

    // current selection, or topaddr if none
    uint32_t GetAddr();
    // currently visible lines
    int nlines;
    // at least nlines * 4 words to display
    wxArrayInt words;
    // address of top line
    uint32_t topaddr;
    // max address for scrollbar
    uint32_t maxaddr;
    // bytes per word == (1 << fmt)
    int fmt;
    // after write, these contain write addr and val
    uint32_t writeaddr, writeval;
    // when selection is made, this widget is updated w/ addr
    wxControl* addrlab;

protected:
    // easier than checking maxaddr
    int addrlen;

    void MouseEvent(wxMouseEvent& ev);
    void KeyEvent(wxKeyEvent& ev);
    // the subwidgets
    wxPanel disp;
    wxScrollBar sb;
    wxCaret* caret;
    // cached text size
    int charheight, charwidth;
    // need to know if tc/sb have been Create()d yet
    bool didinit;
    // selection info
    int selnib, seladdr;
    bool isasc;
    void ShowCaret();

    DECLARE_DYNAMIC_CLASS(MemView)  // for xrc
    DECLARE_EVENT_TABLE()
};

BEGIN_DECLARE_EVENT_TYPES()
// event generated when write occurs
// check writeaddr/writeval/fmt
DECLARE_LOCAL_EVENT_TYPE(EVT_WRITEVAL, 0)
END_DECLARE_EVENT_TYPES()

IMPLEMENT_DYNAMIC_CLASS(MemView, wxPanel)

MemView::MemView()
    : wxPanel(), nlines(0), topaddr(0), addrlab(0), disp(), sb(), didinit(false), selnib(-1) {}

void MemView::Refit() {
    addrlen = maxaddr > 0xffff ? 8 : 4;

    if (!didinit) {
        disp.Create(this, wxID_ANY, wxPoint(0, 0), wxDefaultSize,
                    wxBORDER_NONE | wxWANTS_CHARS | wxTAB_TRAVERSAL);
        disp.Connect(wxEVT_PAINT, wxPaintEventHandler(MemView::RepaintEv), NULL, this);
        disp.Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(MemView::MouseEvent), NULL, this);
        disp.Connect(wxEVT_MOTION, wxMouseEventHandler(MemView::MouseEvent), NULL, this);
        disp.Connect(wxEVT_LEFT_UP, wxMouseEventHandler(MemView::MouseEvent), NULL, this);
        disp.Connect(wxEVT_CHAR, wxKeyEventHandler(MemView::KeyEvent), NULL, this);
        disp.SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        sb.Create(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
        sb.SetScrollbar(0, 15, 500, 15);
    }

    wxClientDC dc(&disp);
    // doesn't seem to inherit font properly
    dc.SetFont(GetFont());
    dc.GetTextExtent(wxT('M'), &charwidth, &charheight);

    if (!didinit) {
        // using 2-phase init doesn't seem to work
        caret = new wxCaret(&disp, charwidth, charheight);
        caret->Hide();
        disp.SetCaret(caret);
        didinit = true;
    }

    wxSize sz(charwidth * (69 + addrlen) + sb.GetBestSize().GetWidth(), charheight * 15);
    sz = sz + GetSize() - GetClientSize();
    SetMinSize(sz);
    SetSize(sz);
}

void MemView::MouseEvent(wxMouseEvent& ev) {
    if (ev.GetEventType() == wxEVT_MOTION && !ev.LeftIsDown())
        return;

    int x = ev.GetX() / charwidth, y = ev.GetY() / charheight;
    x -= addrlen + 3;

    if (x < 0 || y < 0 || y > nlines)
        return;

    int word, nib;
    int nnib = 2 << fmt, nword = 16 >> fmt;
    int preasc = (nnib + 1) * nword + 2;
    isasc = x >= preasc;

    if (isasc) {
        word = (x - preasc) * 2 / nnib;
        nib = (x - preasc) * 2 % nnib;
    } else {
        word = x / (nnib + 1);
        nib = x % (nnib + 1);
        nib = nnib - nib - 1;
    }

    if (nib < 0 || word >= nword)
        return;

    seladdr = topaddr + y * 16;
    selnib = word * nnib + nib;
    Show(seladdr);
}

void MemView::ShowCaret() {
    if (seladdr < (int)topaddr || seladdr >= (int)topaddr + nlines * 16)
        selnib = -1;

    if (selnib < 0) {
        while (caret->IsVisible())
            caret->Hide();

        if (addrlab)
            addrlab->SetLabel(wxEmptyString);

        return;
    }

    if (addrlab) {
        wxString lab;
        uint32_t addr = seladdr + selnib / 2;

        if (!isasc)
            addr &= ~((1 << fmt) - 1);

        lab.Printf(addrlen == 8 ? wxT("0x%08X") : wxT("0x%04X"), addr);
        addrlab->SetLabel(lab);
    }

    int y = (seladdr - topaddr) / 16;
    int x = addrlen + 3;
    int nnib = 2 << fmt, nword = 16 >> fmt;

    if (isasc)
        x += (nnib + 1) * nword + 2 + selnib / 2;
    else
        x += (nnib + 1) * (selnib / nnib) + nnib - selnib % nnib - 1;

    caret->Move(x * charwidth, y * charheight);

    while (!caret->IsVisible())
        caret->Show();

    disp.SetFocus();
}

void MemView::KeyEvent(wxKeyEvent& ev) {
    uint32_t k = getKeyboardKeyCode(ev);
    int nnib = 2 << fmt;

    switch (k) {
        case WXK_RIGHT:
        case WXK_NUMPAD_RIGHT:
            if (isasc)
                selnib += 2;
            else if (ev.GetModifiers() == wxMOD_SHIFT)
                selnib += 2 << fmt;
            else if (!(selnib % nnib))
                selnib += nnib + nnib - 1;
            else
                selnib--;

            if (selnib >= 32) {
                if (seladdr == (int)maxaddr - 16)
                    selnib = 32 - nnib;
                else {
                    selnib -= 32;
                    seladdr += 16;
                }
            }

            break;

        case WXK_LEFT:
        case WXK_NUMPAD_LEFT:
            if (isasc)
                selnib -= 2;
            else if (ev.GetModifiers() == wxMOD_SHIFT)
                selnib -= 2 << fmt;
            else if (!(++selnib % nnib))
                selnib -= nnib * 2;

            if (selnib < 0) {
                if (!seladdr)
                    selnib = nnib - 1;
                else {
                    selnib += 32;
                    seladdr -= 16;
                }
            }

            break;

        case WXK_DOWN:
        case WXK_NUMPAD_DOWN:
            if (seladdr < (int)maxaddr - 16)
                seladdr += 16;

            break;

        case WXK_UP:
        case WXK_NUMPAD_UP:
            if (seladdr > 0)
                seladdr -= 16;

            break;

        default:
            if (k > 0x7f || (isasc && !isprint(k)) || (!isasc && !isxdigit(k))) {
                ev.Skip();
                return;
            }

            // location in data array
            int wno = (seladdr - topaddr) / 4 + selnib / 8;
            int bno = (selnib % 8) / 2;
            int nibno = selnib % 2;

            // now that selnib/seladdr isn't needed any more, advance pointer
            if (isasc)
                selnib += 2;
            else if (!(selnib % nnib))
                selnib += nnib + nnib - 1;
            else
                selnib--;

            if (selnib >= 32) {
                if (seladdr == (int)maxaddr - 16)
                    selnib = 32 - nnib;
                else {
                    selnib -= 32;
                    seladdr += 16;
                }
            }

            uint32_t mask, val;

            if (isasc) {
                mask = 0xff << bno * 8;
                val = k << bno * 8;
            } else {
                mask = 8 * (0xf << bno) + 4 * nibno;
                val = isdigit(k) ? k - '0' : tolower(k) + 10 - 'a';
                val <<= bno * 8 + nibno * 4;
            }

            if ((words[wno] & mask) == val)
                break;

            words[wno] = ((words[wno] & ~mask) | val);
            writeaddr = topaddr + 4 * wno;
            val = words[wno];

            switch (fmt) {
                case 0:
                    writeval = (val >> bno * 8) & 0xff;
                    writeaddr += bno;
                    break;

                case 1:
                    writeval = (val >> (bno / 2) * 16) & 0xffff;
                    writeaddr += bno & ~1;
                    break;

                case 2:
                    writeval = val;
                    break;
            }

            // write value; this will not return until value has been written
            wxCommandEvent new_event(EVT_WRITEVAL, GetId());
            new_event.SetEventObject(this);
            GetEventHandler()->ProcessEvent(new_event);
            // now refresh whole screen.  Really need to make this more
            // efficient some day
            Repaint();
    }

    Show(seladdr);
}

void MemView::MoveSB() {
    int pos;

    if (topaddr / 16 <= 100)  // <= 100
        pos = topaddr / 16;
    else if (topaddr / 16 >= maxaddr / 16 - 100)  // >= 400
        pos = topaddr / 16 - maxaddr / 16 + 500;
    else if (topaddr / 16 < 1100)  // <= 200
        pos = (topaddr / 16 - 100) / 10 + 100;
    else if (topaddr / 16 >= maxaddr / 16 - 1100)  // >= 300
        pos = (topaddr / 16 - maxaddr / 16 + 1100) / 10 + 300;
    else  // > 200 && < 300
        pos = ((topaddr / 16) - 1100) / (((maxaddr / 16) - 2200) / 100) + 200;

    sb.SetScrollbar(pos, 20, 500, 20);
}

void MemView::MoveView(wxScrollEvent& ev) {
    int pos = ev.GetPosition();

    if (pos < 100)
        topaddr = pos * 16;
    else if (pos >= 400)
        topaddr = maxaddr + (pos - 500) * 16;
    else if (ev.GetEventType() == wxEVT_SCROLL_LINEUP) {
        topaddr -= 16;
        MoveSB();
    } else if (ev.GetEventType() == wxEVT_SCROLL_LINEDOWN) {
        topaddr += 16;
        MoveSB();
    } else if (ev.GetEventType() == wxEVT_SCROLL_PAGEUP) {
        topaddr -= (nlines - 2) * 16;
        MoveSB();
    } else if (ev.GetEventType() == wxEVT_SCROLL_PAGEDOWN) {
        topaddr += (nlines - 2) * 16;
        MoveSB();
    } else if (ev.GetEventType() == wxEVT_SCROLL_THUMBRELEASE) {
        if (pos <= 200)
            topaddr = ((pos - 100) * 10 + 100) * 16;
        else if (pos >= 300)
            topaddr = ((pos - 300) * 10 - 1100) * 16 + maxaddr;
        else
            topaddr = ((pos - 200) * ((maxaddr / 16 - 2200) / 100) + 1100) * 16;

        MoveSB();
    }  // ignore THUMBTRACK and CHANGED
    // do not interrupt scroll because no event was triggered
    else if (pos <= 200)
        topaddr = ((pos - 100) * 10 + 100) * 16;
    else if (pos >= 300)
        topaddr = ((pos - 300) * 10 - 1100) * 16 + maxaddr;
    else if (pos > 200 && pos < 300)
        topaddr = ((pos - 200) * ((maxaddr / 16 - 2200) / 100) + 1100) * 16;

    RefillNeeded();
}

// generate event; due to ProcessEvent()'s nature, it will return only
// when refill complete
void MemView::RefillNeeded() {
    wxCommandEvent ev(EVT_REFILL_NEEDED, GetId());
    ev.SetEventObject(this);
    GetEventHandler()->ProcessEvent(ev);
}

// FIXME:  following 3 repainters do not work as intended: there is
// more flickering than necessary and yet still garbage on screen sometimes
// (probably in part from wxCaret)

// called by parent's refill handler or any other time strings have changed
void MemView::Refill() {
    MoveSB();
    wxClientDC dc(&disp);
    dc.Clear();
    Refill(dc);
    ShowCaret();
}

void MemView::Repaint() {
    wxClientDC dc(&disp);
    dc.SetBackgroundMode(wxSOLID);
    Refill(dc);
}

void MemView::RepaintEv(wxPaintEvent& ev) {
    (void)ev;  // unused params
    wxPaintDC dc(&disp);
    dc.SetBackgroundMode(wxSOLID);
    Refill(dc);
    ShowCaret();
}

void MemView::Refill(wxDC& dc) {
    // don't want caret drawing at same time due to timer event
    wxCaretSuspend cs(&disp);
    // doesn't seem to inherit font properly
    dc.SetFont(GetFont());

    for (size_t i = 0; i < (size_t)nlines && i < words.size() / 4; i++) {
        wxString line, word;
        line.Printf(maxaddr > 0xffff ? wxT("%08X   ") : wxT("%04X   "), topaddr + (int)i * 16);

        for (int j = 0; j < 4; j++) {
            uint32_t v = words[i * 4 + j];

            switch (fmt) {
                case 0:
                    word.Printf(wxT("%02X %02X %02X %02X "), v & 0xff, (v >> 8) & 0xff,
                                (v >> 16) & 0xff, (v >> 24) & 0xff);
                    break;

                case 1:
                    word.Printf(wxT("%04X %04X "), v & 0xffff, (v >> 16) & 0xffff);
                    break;

                case 2:
                    word.Printf(wxT("%08X "), v);
                    break;
            }

            line.append(word);
        }

        line.append(wxT("  "));

        for (int j = 0; j < 4; j++) {
            uint32_t v = words[i * 4 + j];
#define appendc(c) \
    line.append(isascii((c) & 0xff) && isprint((c) & 0xff) ? (wxChar)((c) & 0xff) : wxT('.'))
            appendc(v);
            appendc(v >> 8);
            appendc(v >> 16);
            appendc(v >> 24);
        }

        dc.DrawText(line, 0, i * charheight);
    }

    int lloc = charwidth * ((addrlen + 1) * 2 + 1) / 2;
    dc.DrawLine(lloc, 0, lloc, nlines * charheight);
    lloc = charwidth * (2 * (addrlen + 3 + 32 + 4 + (fmt == 0 ? 3 * 4 : fmt == 1 ? 4 : 0)) + 1) / 2;
    dc.DrawLine(lloc, 0, lloc, nlines * charheight);
}

// on resize, recompute shown lines and refill if necessary
void MemView::Resize(wxSizeEvent& ev) {
    (void)ev;      // unused params
    if (!didinit)  // prevent crash on win32
        return;

    wxSize sz = GetClientSize();
    int sbw = sb.GetSize().GetWidth();
    sz.SetWidth(sz.GetWidth() - sbw);
    sb.Move(sz.GetWidth(), 0);
    sb.SetSize(sbw, sz.GetHeight());
    nlines = (sz.GetHeight() + charheight - 1) / charheight;
    wxString val;
    disp.SetSize(sz.GetWidth(), (nlines + 1) * charheight);

    if ((size_t)nlines > words.size() / 4) {
        if (topaddr + nlines * 16 > maxaddr)
            topaddr = maxaddr - nlines * 16 + 1;

        RefillNeeded();
    } else
        Refill();
}

void MemView::ShowAddr(uint32_t addr, bool force_update) {
    if (addr < topaddr || addr >= topaddr + (nlines - 1) * 16) {
        // align to nearest 16-byte block
        // note that mfc interface only aligns to nearest (1<<fmt)-byte
        uint32_t newtopaddr = addr & ~0xf;

        if (newtopaddr + nlines * 16 > maxaddr)
            newtopaddr = maxaddr - nlines * 16 + 1;

        force_update = newtopaddr != topaddr;
        topaddr = newtopaddr;
    }

    if (force_update) {
        words.clear();
        RefillNeeded();
    } else
        ShowCaret();
}

uint32_t MemView::GetAddr() {
    if (selnib < 0)
        return topaddr;
    else
        return seladdr + (selnib / 2 & ~((1 << fmt) - 1));
}

BEGIN_EVENT_TABLE(MemView, wxPanel)
EVT_SIZE(MemView::Resize)
EVT_SCROLL(MemView::MoveView)
END_EVENT_TABLE()

DEFINE_EVENT_TYPE(EVT_WRITEVAL)

MemViewerBase::MemViewerBase(uint32_t max) : BaseViewer("MemViewer") {
    mv = XRCCTRL(*this, "MemView", MemView);
    if (!mv) {
        baddialog();
    }

    bs = XRCCTRL(*this, "BlockStart", wxChoice);
    if (!bs) {
        baddialog();
    }

    bs->Append(wxT(""));
    bs->SetFocus();
    mv->fmt = max > 0xffff ? 2 : 1;
    getradio(, "Fmt8", mv->fmt, 0);
    getradio(, "Fmt16", mv->fmt, 1);
    getradio(, "Fmt32", mv->fmt, 2);
    mv->maxaddr = max;
    addrlen = max > 0xffff ? 8 : 4;
    gethex(goto_addr, "GotoAddress", addrlen);
    wxControl* addr = XRCCTRL(*this, "CurAddress", wxControl);

    if (!addr)
        baddialog();

    addr->SetLabel(addrlen == 8 ? wxT("0xWWWWWWWW") : wxT("0xWWWW"));
    // refit listing for longest line
    mv->Refit();
    Fit();
    // don't let address display resize when window size changes
    addr->SetMinSize(addr->GetSize());
    mv->addrlab = addr;
    SetMinSize(GetSize());
    Goto(0);
    // initialize load/save support dialog already
    {
        const wxString dname = wxT("MemSelRegion");
        selregion = wxXmlResource::Get()->LoadDialog(this, dname);

        if (!selregion)
            baddialog();

#define this selregion  // for gethex()
        gethex(selreg_addr, "Address", addrlen);
        gethex(selreg_len, "Size", addrlen);
#undef this
        selreg_lenlab = XRCCTRL(*selregion, "SizeLab", wxControl);
        selregion->Fit();
    }
}
void MemViewerBase::BlockStart(wxCommandEvent& ev) {
    (void)ev;  // unused params
    unsigned long l;
    bs->GetStringSelection().ToULong(&l, 0);
    Goto(l);
}
void MemViewerBase::GotoEv(wxCommandEvent& ev) {
    (void)ev;  // unused params
    unsigned long l;
    wxString v = goto_addr->GetValue();

    if (v.empty())
        return;

    v.ToULong(&l, 16);
    Goto(l);
}
void MemViewerBase::Goto(uint32_t addr) {
    mv->ShowAddr(addr, true);
}
void MemViewerBase::RefreshCmd(wxCommandEvent& ev) {
    (void)ev;  // unused params
    Update();
}

void MemViewerBase::Update() {}

void MemViewerBase::Load(wxCommandEvent& ev) {
    (void)ev;  // unused params
    if (memsave_fn.empty())
        memsave_fn = wxGetApp().frame->GetPanel()->game_name() + wxT(".dmp");

    wxString pats = _("Memory dumps (*.dmp;*.bin)|*.dmp;*.bin|");
    pats.append(wxALL_FILES);
    wxFileDialog dlg(this, _("Select memory dump file"), memsave_dir, memsave_fn, pats,
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    int ret = dlg.ShowModal();
    memsave_fn = dlg.GetPath();
    memsave_dir = dlg.GetDirectory();

    if (ret != wxID_OK)
        return;

    wxFileName fn(memsave_fn);

    if (!fn.IsFileReadable()) {
        wxLogError(wxT("Can't open file %s"), memsave_fn.c_str());
        return;
    }

    unsigned long addr, len = fn.GetSize().ToULong();

    if (!len)
        return;

    wxString s;
    s.Printf(addrlen == 4 ? wxT("%04X") : wxT("%08X"), mv->GetAddr());
    selreg_addr->SetValue(s);
    selreg_len->Disable();
    selreg_lenlab->Disable();
    s.Printf(addrlen == 4 ? wxT("%04X") : wxT("%08X"), len);
    selreg_len->SetValue(s);
    selregion->SetWindowStyle(wxCAPTION | wxRESIZE_BORDER);

    if (OPTION(kDispKeepOnTop))
        selregion->SetWindowStyle(selregion->GetWindowStyle() | wxSTAY_ON_TOP);
    else
        selregion->SetWindowStyle(selregion->GetWindowStyle() & ~wxSTAY_ON_TOP);

    if (selregion->ShowModal() != wxID_OK)
        return;

    selreg_addr->GetValue().ToULong(&addr, 16);
    MemLoad(memsave_fn, addr, len);
}

void MemViewerBase::Save(wxCommandEvent& ev) {
    (void)ev;  // unused params
    wxString s;
    s.Printf(addrlen == 4 ? wxT("%04X") : wxT("%08X"), mv->GetAddr());
    selreg_addr->SetValue(s);
    selreg_len->Enable();
    selreg_lenlab->Enable();
    selreg_len->SetValue(wxEmptyString);
    selregion->SetWindowStyle(wxCAPTION | wxRESIZE_BORDER);

    if (OPTION(kDispKeepOnTop))
        selregion->SetWindowStyle(selregion->GetWindowStyle() | wxSTAY_ON_TOP);
    else
        selregion->SetWindowStyle(selregion->GetWindowStyle() & ~wxSTAY_ON_TOP);

    if (selregion->ShowModal() != wxID_OK)
        return;

    unsigned long addr, len;
    selreg_addr->GetValue().ToULong(&addr, 16);
    selreg_len->GetValue().ToULong(&len, 16);

    if (memsave_fn.empty())
        memsave_fn = wxGetApp().frame->GetPanel()->game_name() + wxT(".dmp");

    wxString pats = _("Memory dumps (*.dmp;*.bin)|*.dmp;*.bin|");
    pats.append(wxALL_FILES);
    wxFileDialog dlg(this, _("Select output file"), memsave_dir, memsave_fn, pats,
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    int ret = dlg.ShowModal();
    memsave_dir = dlg.GetDirectory();
    memsave_fn = dlg.GetPath();

    if (ret != wxID_OK)
        return;

    MemSave(memsave_fn, addr, len);
}

BEGIN_EVENT_TABLE(MemViewerBase, BaseViewer)
EVT_CHOICE(XRCID("BlockStart"), MemViewerBase::BlockStart)
EVT_BUTTON(XRCID("Goto"), MemViewerBase::GotoEv)
EVT_TEXT_ENTER(XRCID("GotoAddress"), MemViewerBase::GotoEv)
EVT_BUTTON(XRCID("Refresh"), MemViewerBase::RefreshCmd)
EVT_BUTTON(wxID_SAVE, MemViewerBase::Save)
EVT_BUTTON(wxID_OPEN, MemViewerBase::Load)
END_EVENT_TABLE()

MemViewer::MemViewer() : MemViewerBase(std::numeric_limits<uint32_t>::max()) {
    bs->Append("0x00000000 - BIOS");
    bs->Append("0x02000000 - WRAM");
    bs->Append("0x03000000 - IRAM");
    bs->Append("0x04000000 - I / O");
    bs->Append("0x05000000 - PALETTE");
    bs->Append("0x06000000 - VRAM");
    bs->Append("0x07000000 - OAM");
    bs->Append("0x08000000 - ROM");
    bs->SetSelection(1);
    Fit();
}

// wx-2.8.4 or MacOSX compiler can't resolve overloads in evt table
void MemViewer::RefillListEv(wxCommandEvent& ev) {
    (void)ev;  // unused params
    Update();
}

void MemViewer::Update() {
    uint32_t addr = mv->topaddr;
    mv->words.resize(mv->nlines * 4);

    for (int i = 0; i < mv->nlines; i++) {
        if (i && !addr)
            break;

        for (int j = 0; j < 4; j++, addr += 4)
            mv->words[i * 4 + j] = CPUReadMemoryQuick(addr);
    }

    mv->Refill();
}

void MemViewer::WriteVal(wxCommandEvent& ev) {
    (void)ev;  // unused params
    switch (mv->fmt) {
        case 0:
            CPUWriteByteQuick(mv->writeaddr, mv->writeval);
            break;

        case 1:
            CPUWriteHalfWordQuick(mv->writeaddr, mv->writeval);
            break;

        case 2:
            CPUWriteMemoryQuick(mv->writeaddr, mv->writeval);
            break;
    }
}

void MemViewer::MemLoad(wxString& name, uint32_t addr, uint32_t len) {
    wxFFile f(name, wxT("rb"));

    if (!f.IsOpened())
        return;

    // this does the equivalent of the CPUWriteMemoryQuick()
    while (len > 0) {
        memoryMap m = map[addr >> 24];
        uint32_t off = addr & m.mask;
        int wlen = (off + len) > m.mask ? m.mask + 1 - off : len;
        wlen = f.Read(m.address + off, wlen);

        if (wlen < 0)
            return;  // FIXME: give error

        len -= wlen;
        addr += wlen;
    }
}

void MemViewer::MemSave(wxString& name, uint32_t addr, uint32_t len) {
    wxFFile f(name, wxT("wb"));

    if (!f.IsOpened())
        return;

    // this does the equivalent of the CPUReadMemoryQuick()
    while (len > 0) {
        memoryMap m = map[addr >> 24];
        uint32_t off = addr & m.mask;
        int wlen = (off + len) > m.mask ? m.mask + 1 - off : len;
        wlen = f.Write(m.address + off, wlen);

        if (wlen < 0)
            return;  // FIXME: give error

        len -= wlen;
        addr += wlen;
    }
}

BEGIN_EVENT_TABLE(MemViewer, MemViewerBase)
EVT_COMMAND(wxID_ANY, EVT_REFILL_NEEDED, MemViewer::RefillListEv)
EVT_COMMAND(wxID_ANY, EVT_WRITEVAL, MemViewer::WriteVal)
END_EVENT_TABLE()

GBMemViewer::GBMemViewer() : MemViewerBase((uint16_t)~0) {
    bs->Append("0x0000 - ROM");
    bs->Append("0x4000 - ROM");
    bs->Append("0x8000 - VRAM");
    bs->Append("0xA000 - SRAM");
    bs->Append("0xC000 - RAM");
    bs->Append("0xD000 - WRAM");
    bs->Append("0xFF00 - I / O");
    bs->Append("0xFF80 - RAM");
    bs->SetSelection(1);
    Fit();
}

// wx-2.8.4 or MacOSX compiler can't resolve overloads in evt table
void GBMemViewer::RefillListEv(wxCommandEvent& ev) {
    (void)ev;  // unused params
    Update();
}

void GBMemViewer::Update() {
    uint32_t addr = mv->topaddr;
    mv->words.resize(mv->nlines * 4);

    for (int i = 0; i < mv->nlines; i++) {
        if (i && !(uint16_t)addr)
            break;

        for (int j = 0; j < 4; j++, addr += 4)
            mv->words[i * 4 + j] = GBReadMemoryQuick(addr);
    }

    mv->Refill();
}

void GBMemViewer::WriteVal(wxCommandEvent& ev) {
    (void)ev;  // unused params
    switch (mv->fmt) {
        case 0:
            GBWriteByteQuick(mv->writeaddr, mv->writeval);
            break;

        case 1:
            GBWriteHalfWordQuick(mv->writeaddr, mv->writeval);
            break;

        case 2:
            GBWriteMemoryQuick(mv->writeaddr, mv->writeval);
            break;
    }
}

void GBMemViewer::MemLoad(wxString& name, uint32_t addr, uint32_t len) {
    wxFFile f(name, wxT("rb"));

    if (!f.IsOpened())
        return;

    // this does the equivalent of the GBWriteMemoryQuick()
    while (len > 0) {
        uint8_t* maddr = gbMemoryMap[addr >> 12];
        uint32_t off = addr & 0xfff;
        int wlen = (off + len) > 0xfff ? 0x1000 - off : len;
        wlen = f.Read(maddr + off, wlen);

        if (wlen < 0)
            return;  // FIXME: give error

        len -= wlen;
        addr += wlen;
    }
}

void GBMemViewer::MemSave(wxString& name, uint32_t addr, uint32_t len) {
    wxFFile f(name, wxT("wb"));

    if (!f.IsOpened())
        return;

    // this does the equivalent of the GBReadMemoryQuick()
    while (len > 0) {
        uint8_t* maddr = gbMemoryMap[addr >> 12];
        uint32_t off = addr & 0xfff;
        int wlen = (off + len) > 0xfff ? 0x1000 - off : len;
        wlen = f.Write(maddr + off, wlen);

        if (wlen < 0)
            return;  // FIXME: give error

        len -= wlen;
        addr += wlen;
    }
}

BEGIN_EVENT_TABLE(GBMemViewer, MemViewerBase)
EVT_COMMAND(wxID_ANY, EVT_REFILL_NEEDED, GBMemViewer::RefillListEv)
EVT_COMMAND(wxID_ANY, EVT_WRITEVAL, GBMemViewer::WriteVal)
END_EVENT_TABLE()

}  // namespace viewers