#ifndef VBAM_WX_WIDGETS_CHECKED_LIST_CTRL_H_
#define VBAM_WX_WIDGETS_CHECKED_LIST_CTRL_H_

#include <wx/imaglist.h>
#include <wx/listctrl.h>
#include "wx/event.h"

wxDECLARE_EVENT(CHECKED_LIST_ITEM_CHECKED, wxListEvent);
wxDECLARE_EVENT(CHECKED_LIST_ITEM_UNCHECKED, wxListEvent);

namespace widgets {

// image indexes (used internally by wxCheckedListCtrl)
#define wxCLC_UNCHECKED_IMGIDX 0 // unchecked
#define wxCLC_CHECKED_IMGIDX 1 // checked

// additional state flags (wx's defines should end at 0x0100; see listbase.h)
#define wxLIST_STATE_CHECKED 0x010000

extern const char CheckedListCtrlNameStr[];

// Adapted from unknown author at http://wiki.wxwidgets.org/wiki.pl?WxListCtrl
// Modified by Francesco Montorsi (2005) and Fabrice 'Steelskin' de Gans for VBA-m.
class CheckedListCtrl : public wxListCtrl {
public:
    CheckedListCtrl();
    ~CheckedListCtrl() override = default;

    CheckedListCtrl(wxWindow* parent,
                    wxWindowID id = wxID_ANY,
                    const wxPoint& pt = wxDefaultPosition,
                    const wxSize& sz = wxDefaultSize,
                    long style = wxLC_REPORT,
                    const wxValidator& validator = wxDefaultValidator,
                    const wxString& name = wxString::FromAscii(CheckedListCtrlNameStr));

    bool Create(wxWindow* parent,
                wxWindowID id = -1,
                const wxPoint& pt = wxDefaultPosition,
                const wxSize& sz = wxDefaultSize,
                long style = wxLC_REPORT,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxString::FromAscii(CheckedListCtrlNameStr));

    // for xrc usage, a separate init function is needed.
    bool Init();

    // utilities
    // core overloads (i.e. the most generic overloads)
    bool GetItem(wxListItem& info) const;
    bool SetItem(wxListItem& info);
    long InsertItem(wxListItem& info);
    bool DeleteItem(long item);
    bool DeleteAllItems() {
        state_list_.Clear();
        return wxListCtrl::DeleteAllItems();
    }

    // shortcuts to the SetItemState function
    void Check(long item, bool checked);
    void CheckAll(bool checked = true);

    // shortcuts to the GetItemState function
    bool IsChecked(long item) const { return GetItemState(item, wxLIST_STATE_CHECKED) != 0; }

    // we overload these so we are sure they will use our
    // #GetItem and #SetItem functions...
    bool SetItemState(long item, long state, long stateMask);
    int GetItemState(long item, long stateMask) const;
    long InsertItem(long index, const wxString& label, int imageIndex = -1);
    long SetItem(long index, int col, const wxString& label, int imageId = -1);

    // the image associated with an element is already in used by wxCheckedListCtrl
    // itself to show the checkbox and it cannot be handled by the user !
    bool SetItemImage(long, int) {
        wxASSERT_MSG(0, "This function cannot be used with wxCheckedListCtrl !");
        return FALSE;
    }

private:
    // event handlers
    void OnMouseEvent(wxMouseEvent& event);

    // internal utilities
    wxColour GetBgColourFromAdditionalState();

    // we have to keep a different array to keep track of the additional
    // states we support....
    wxArrayInt state_list_;

    // our set of checkbox images...
    wxImageList image_list_;

    wxDECLARE_DYNAMIC_CLASS(CheckedListCtrl);
    wxDECLARE_EVENT_TABLE();
};

}  // namespace widgets

#endif  // VBAM_WX_WIDGETS_CHECKED_LIST_CTRL_H_
