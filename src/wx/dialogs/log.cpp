#include "wx/dialogs/log.h"

#include <wx/checkbox.h>
#include <wx/ffile.h>
#include <wx/filedlg.h>
#include <wx/textctrl.h>

#include "wx/main-frame.h"
#include "wx/widgets/wxmisc.h"
#include "wx/wxhead.h"

#include "core/gba/gbaGlobals.h"

#define baddialog()                                                              \
    do {                                                                         \
        wxLogError(_("Unable to load dialog %s from resources"), dname.c_str()); \
        wxGetApp().frame->Close(true);                                           \
        return;                                                                  \
    } while (0)

#define getlogf(n, val)                                                 \
    do {                                                                \
        wxCheckBox* cb = XRCCTRL(*this, n, wxCheckBox);                 \
        if (!cb)                                                        \
            baddialog();                                                \
        cb->SetValidator(wxBoolIntValidator(&systemVerbose, val, val)); \
    } while (0)

namespace dialogs {

LogDialog::LogDialog() : dialogs::BaseDialog(nullptr, "Logging") {
    const wxString dname = wxT("Logging");

    SetEscapeId(wxID_OK);
    getlogf("SWI", VERBOSE_SWI);
    getlogf("UnalignedMemory", VERBOSE_UNALIGNED_MEMORY);
    getlogf("IllWrite", VERBOSE_ILLEGAL_WRITE);
    getlogf("IllRead", VERBOSE_ILLEGAL_READ);
    getlogf("DMA0", VERBOSE_DMA0);
    getlogf("DMA1", VERBOSE_DMA1);
    getlogf("DMA2", VERBOSE_DMA2);
    getlogf("DMA3", VERBOSE_DMA3);
    getlogf("UndefInstruction", VERBOSE_UNDEFINED);
    getlogf("AGBPrint", VERBOSE_AGBPRINT);
    getlogf("SoundOut", VERBOSE_SOUNDOUTPUT);
    log = XRCCTRL(*this, "Log", wxTextCtrl);

    if (!log)
        baddialog();

    Fit();
}

void LogDialog::Update() {
    wxString l = wxGetApp().log;
    log->SetValue(l);
    log->ShowPosition(l.size() > 2 ? l.size() - 2 : 0);
}

void LogDialog::Save(wxCommandEvent& ev) {
    (void)ev;  // unused params
    static wxString logdir = wxEmptyString, def_name = wxEmptyString;

    if (def_name.empty())
        def_name = wxGetApp().frame->GetPanel()->game_name() + wxT(".log");

    wxString pats = _("Text files (*.txt;*.log)|*.txt;*.log|");
    pats.append(wxALL_FILES);
    wxFileDialog dlg(this, _("Select output file"), logdir, def_name, pats,
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    int ret = dlg.ShowModal();
    def_name = dlg.GetPath();
    logdir = dlg.GetDirectory();

    if (ret != wxID_OK)
        return;

    wxFFile f(def_name, wxT("w"));

    if (f.IsOpened()) {
        f.Write(wxGetApp().log);
        f.Close();
    }
}

void LogDialog::Clear(wxCommandEvent& ev) {
    (void)ev;  // unused params
    wxGetApp().log.clear();
    Update();
}

BEGIN_EVENT_TABLE(LogDialog, wxDialog)
EVT_BUTTON(wxID_SAVE, LogDialog::Save)
EVT_BUTTON(XRCID("Clear"), LogDialog::Clear)
END_EVENT_TABLE()

}  // namespace dialogs
