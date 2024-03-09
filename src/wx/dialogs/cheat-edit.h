#ifndef VBAM_WX_DIALOGS_CHEAT_EDIT_H_
#define VBAM_WX_DIALOGS_CHEAT_EDIT_H_

#include <utility>
#include <vector>

#include <wx/clrpicker.h>
#include <wx/dialog.h>

#include "../common/system.h"
#include "config/cheat-manager.h"
#include "widgets/keep-on-top-styler.h"

namespace dialogs {

// Manages the Cheat Edition dialog.
class CheatEdit : public wxDialog {
public:
    static CheatEdit* NewInstance(wxWindow* parent);
    ~CheatEdit() override = default;

    void SetCheat(const config::Cheat& cheat);
    void Reset(VbamSystem system);

    // Accessor.
    config::Cheat cheat() const { return cheat_; }

private:
    // The constructor is private so initialization has to be done via the
    // static method. This is because this class is destroyed when its
    // owner, `parent` is destroyed. This prevents accidental deletion.
    CheatEdit(wxWindow* parent);

    void ResetChoices();

    config::Cheat cheat_;

    const std::vector<std::pair<wxString, config::CheatType>> gb_cheats_;
    const std::vector<std::pair<wxString, config::CheatType>> gba_cheats_;

    const widgets::KeepOnTopStyler keep_on_top_styler_;
};

}  // namespace dialogs

#endif  // VBAM_WX_DIALOGS_CHEAT_EDIT_H_
