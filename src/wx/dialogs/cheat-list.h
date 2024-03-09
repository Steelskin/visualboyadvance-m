#ifndef VBAM_WX_DIALOGS_CHEAT_LIST_H_
#define VBAM_WX_DIALOGS_CHEAT_LIST_H_

#include <wx/dialog.h>
#include <wx/listctrl.h>

#include "../common/system.h"
#include "config/cheat-manager.h"
#include "widgets/keep-on-top-styler.h"

// Forward declaration.
namespace widgets {
class CheckedListCtrl;
}

namespace dialogs {

// Forward declaration.
class CheatEdit;

// Manages the Cheat List dialog.
class CheatList : public wxDialog {
public:
    class Delegate : public VbamSystemProvider, public config::CheatManagerProvider {
    public:
        virtual ~Delegate() = default;
    };

    static CheatList* NewInstance(wxWindow* parent,
                                  Delegate* const delegate);
    ~CheatList() override = default;

private:
    // The constructor is private so initialization has to be done via the
    // static method. This is because this class is destroyed when its
    // owner, `parent` is destroyed. This prevents accidental deletion.
    CheatList(wxWindow* parent,
              Delegate* const delegate);

    void RepopulateCheats();
    void AddCheat(int index);
    void ReloadCheat(int index);

    void OnShow(wxShowEvent& show_event);
    void OnCheatChecked(wxListEvent& event);
    void OnCheatUnchecked(wxListEvent& event);
    void OnCheatEdition(wxListEvent& event);
    void OnTool(wxCommandEvent& event);

    // The cheat list control.
    widgets::CheckedListCtrl* cheat_list_;

    // List items used to modify the cheat list data.
    wxListItem code_item_;
    wxListItem desc_item_;

    wxString cheat_dir_name_;
    wxString cheat_file_name_;

    // The cheat edition dialog.
    CheatEdit* const cheat_edit_;

    Delegate* const delegate_;
    const widgets::KeepOnTopStyler keep_on_top_styler_;
};

}  // namespace dialogs

#endif  // VBAM_WX_DIALOGS_CHEAT_LIST_H_
