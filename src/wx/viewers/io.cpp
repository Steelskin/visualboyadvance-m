#include "wx/viewers/io.h"

#include <wx/checkbox.h>
#include <wx/choice.h>

#include "core/gba/gbaGlobals.h"
#include "core/gba/gbaInline.h"
#include "wx/main-frame.h"
#include "wx/wxhead.h"
#include "wx/wxvbam.h"

namespace viewers {

#define baddialog()                                                              \
    do {                                                                         \
        wxLogError(_("Unable to load dialog %s from resources"), dname.c_str()); \
        wxGetApp().frame->Close(true);                                           \
        return;                                                                  \
    } while (0)

namespace {
#include "wx/viewers/ioregs.h"
}

// static
wxString IOViewer::lline;

IOViewer::IOViewer() : BaseViewer(wxT("IOViewer")) {
    for (int i = 0; i < 16; i++) {
        wxString s;
        s.Printf(wxT("B%d"), i);
        bit[i] = XRCCTRL_D(*this, s, wxCheckBox);
        s.append(wxT("lab"));
        bitlab[i] = XRCCTRL_D(*this, s, wxControl);

        if (!bit[i] || !bitlab[i])
            baddialog();
    }

    addr_ = XRCCTRL(*this, "Address", wxChoice);
    val = XRCCTRL(*this, "Value", wxControl);

    if (!addr_ || !val)
        baddialog();

    addr_->Clear();
    wxString longline = lline;
    int lwidth = 0;

    for (long unsigned int i = 0; i < NUM_IOREGS; i++) {
        addr_->Append(wxGetTranslation(ioregs[i].name));

        // find longest label
        // this is probably horribly expensive
        // cache for entire app...
        // and while at it, translate all the strings
        if (!lline) {
            for (int j = 0; j < 16; j++) {
                if (!ioregs[i].bits[j].IsEmpty()) {
                    ioregs[i].bits[j] = wxGetTranslation(ioregs[i].bits[j]);
                    int w, h;
                    bitlab[0]->GetTextExtent(ioregs[i].bits[j], &w, &h);

                    if (w > lwidth) {
                        lwidth = w;
                        longline = ioregs[i].bits[j];
                    }
                }
            }
        }
    }

    if (!lline)
        lline = longline;

    bitlab[0]->SetLabel(lline);
    Fit();
    addr_->SetSelection(0);
    Select(0);
}

void IOViewer::SelectEv(wxCommandEvent& ev) {
    (void)ev;  // unused params
    Select(addr_->GetSelection());
}

void IOViewer::Select(int sel) {
    int i;
    uint16_t mask;

    for (mask = 1, i = 0; mask; mask <<= 1, i++) {
        bit[i]->Enable(mask & ioregs[sel].write);
        bitlab[i]->SetLabel(ioregs[sel].bits[i]);
    }

    Update(sel);
}

void IOViewer::Update() {
    Update(addr_->GetSelection());
}

void IOViewer::Update(int sel) {
    uint16_t* addr =
        ioregs[sel].address ? ioregs[sel].address : (uint16_t*)&g_ioMem[ioregs[sel].offset];
    uint16_t mask, reg = *addr;
    int i;

    for (mask = 1, i = 0; mask; mask <<= 1, i++)
        bit[i]->SetValue(mask & reg);

    wxString s;
    s.Printf(wxT("%04X"), reg);
    val->SetLabel(s);
}

void IOViewer::CheckBit(wxCommandEvent& ev) {
    for (int i = 0; i < 16; i++)
        if (ev.GetEventObject() == bit[i]) {
            // it'd be faster to store the value and just flip
            // the bit, but it's easier this way
            uint16_t mask, reg = 0;
            int j;

            for (mask = 1, j = 0; mask; mask <<= 1, j++)
                if (bit[j]->GetValue())
                    reg |= mask;

            wxString s;
            s.Printf(wxT("%04X"), reg);
            val->SetLabel(s);
            return;
        }

    ev.Skip();
}

void IOViewer::RefreshEv(wxCommandEvent& ev) {
    (void)ev;  // unused params
    Update();
}

void IOViewer::Apply(wxCommandEvent& ev) {
    (void)ev;  // unused params
    int sel = addr_->GetSelection();
    uint16_t* addr =
        ioregs[sel].address ? ioregs[sel].address : (uint16_t*)&g_ioMem[ioregs[sel].offset];
    uint16_t mask, reg = *addr;
    reg &= ~ioregs[sel].write;
    int i;

    for (mask = 1, i = 0; mask; mask <<= 1, i++) {
        if ((mask & ioregs[sel].write) && bit[i]->GetValue())
            reg |= mask;
    }

    CPUWriteHalfWord(0x4000000 + ioregs[sel].offset, reg);
    Update(sel);
}

BEGIN_EVENT_TABLE(IOViewer, BaseViewer)
EVT_BUTTON(XRCID("Refresh"), IOViewer::RefreshEv)
EVT_BUTTON(wxID_APPLY, IOViewer::Apply)
EVT_CHOICE(XRCID("Address"), IOViewer::SelectEv)
EVT_CHECKBOX(wxID_ANY, IOViewer::CheckBit)
END_EVENT_TABLE()

}  // namespace viewers
