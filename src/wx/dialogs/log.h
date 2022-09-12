#ifndef VBAM_WX_DIALOGS_LOG_H_
#define VBAM_WX_DIALOGS_LOG_H_

#include "wx/dialogs/base-dialog.h"

class wxTextCtrl;

namespace dialogs {

class LogDialog : public dialogs::BaseDialog {
public:
    LogDialog();
    ~LogDialog() override = default;

    void Update() override;

private:
    wxTextCtrl* log;
    void Save(wxCommandEvent& ev);
    void Clear(wxCommandEvent& ev);

    wxDECLARE_EVENT_TABLE();
};

}  // namespace dialogs

#endif  // VBAM_WX_DIALOGS_LOG_H_