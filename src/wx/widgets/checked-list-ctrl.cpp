#include "widgets/checked-list-ctrl.h"

#include <wx/checkbox.h>
#include <wx/dcmemory.h>
#include <wx/renderer.h>
#include <memory>

wxDEFINE_EVENT(CHECKED_LIST_ITEM_CHECKED, wxListEvent);
wxDEFINE_EVENT(CHECKED_LIST_ITEM_UNCHECKED, wxListEvent);

namespace widgets {

namespace {

int GetItemImageFromAdditionalState(int addstate) {
    bool checked = (addstate & wxLIST_STATE_CHECKED) != 0;

    if (checked) {
        return wxCLC_CHECKED_IMGIDX;
    } else {
        return wxCLC_UNCHECKED_IMGIDX;
    }
}

int GetAndRemoveAdditionalState(long* state, int statemask) {
    int additionalstate = 0;

    if (!state) {
        return -1;
    }

    // extract the bits we are interested in
    bool checked = (*state & wxLIST_STATE_CHECKED) != 0;

    // and set them in a different variable if they are included in the statemask
    if (checked && (statemask & wxLIST_STATE_CHECKED)) {
        additionalstate |= wxLIST_STATE_CHECKED;
    }

    // remove them from the original state var...
    *state &= ~wxLIST_STATE_CHECKED;
    return additionalstate;
}

}  // namespace

CheckedListCtrl::CheckedListCtrl() : wxListCtrl(), image_list_() {}

CheckedListCtrl::CheckedListCtrl(wxWindow* parent,
                                 wxWindowID id,
                                 const wxPoint& pt,
                                 const wxSize& sz,
                                 long style,
                                 const wxValidator& validator,
                                 const wxString& name)
    : wxListCtrl() {
    Create(parent, id, pt, sz, style, validator, name);
}

bool CheckedListCtrl::Create(wxWindow* parent,
                             wxWindowID id,
                             const wxPoint& pt,
                             const wxSize& sz,
                             long style,
                             const wxValidator& validator,
                             const wxString& name) {
    if (!wxListCtrl::Create(parent, id, pt, sz, style, validator, name))
        return FALSE;

    // support xrc by making init separate
    return Init();
}

bool CheckedListCtrl::Init() {
    // use native size images instead of 16x16
    std::unique_ptr<wxCheckBox> check_box = std::make_unique<wxCheckBox>(nullptr, wxID_ANY, wxEmptyString);
    const wxSize check_box_size = check_box->GetBestSize();

    image_list_.Create(check_box_size.GetWidth(), check_box_size.GetHeight(), TRUE);
    SetImageList(&image_list_, wxIMAGE_LIST_SMALL);

    // pasted from wxWiki
    // but with native size instead of 16x16
    // constructor only takes wxSize in 2.9+, apparently
    wxBitmap unchecked_bmp(check_box_size.GetWidth(), check_box_size.GetHeight());
    wxBitmap checked_bmp(check_box_size.GetWidth(), check_box_size.GetHeight());

    // Bitmaps must not be selected by a DC for addition to the image list but I don't see
    // a way of diselecting them in wxMemoryDC so let's just use a code block to end the scope
    {
        wxMemoryDC renderer_dc;
        // Unchecked
        renderer_dc.SelectObject(unchecked_bmp);
        renderer_dc.SetBackground(
            *wxTheBrushList->FindOrCreateBrush(GetBackgroundColour(), wxBRUSHSTYLE_SOLID));
        renderer_dc.Clear();
        wxRendererNative::Get().DrawCheckBox(this, renderer_dc, wxRect(0, 0, 16, 16), 0);

        // Checked
        renderer_dc.SelectObject(checked_bmp);
        renderer_dc.SetBackground(
            *wxTheBrushList->FindOrCreateBrush(GetBackgroundColour(), wxBRUSHSTYLE_SOLID));
        renderer_dc.Clear();
        wxRendererNative::Get().DrawCheckBox(this, renderer_dc, wxRect(0, 0, 16, 16),
                                             wxCONTROL_CHECKED);
    }

    // the add order must respect the wxCLC_XXX_IMGIDX defines in the headers !
    image_list_.Add(unchecked_bmp);
    image_list_.Add(checked_bmp);

    return true;
}

wxColour CheckedListCtrl::GetBgColourFromAdditionalState() {
    if (this->IsEnabled())
        return *wxWHITE;

#ifdef __WXMSW__
    return wxColour(212, 208, 200);
#else
    return wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
#endif
}

bool CheckedListCtrl::GetItem(wxListItem& info) const {
    // wx internal wxListCtrl::GetItem remove from the state mask the
    // wxLIST_STATE_CHECKED bit since is is not part of wx standard flags...
    // so we need to check those flags against the original wxListItem's
    // statemask...
    wxListItem original(info);

#ifdef __WXDEBUG__
    // we always want to retrieve also the image state for checking purposes...
    info.m_mask |= wxLIST_MASK_IMAGE;
#endif

    if (!wxListCtrl::GetItem(info)) {
        return false;
    }

    // these are our additional supported states: read them from state_list_
    bool checked = (state_list_[info.m_itemId] & wxLIST_STATE_CHECKED) != 0;

    // now intercept state requests about enable or check mode
    if ((original.m_mask & wxLIST_MASK_STATE) && (original.m_stateMask & wxLIST_STATE_CHECKED)) {
        info.m_state |= (state_list_[info.m_itemId] & wxLIST_STATE_CHECKED);
        info.m_stateMask |= wxLIST_STATE_CHECKED;
        info.m_mask |= wxLIST_MASK_STATE;  // contains valid info !
    }

    // check that state & image are synch
    if (info.GetColumn()) {
        return true;
    }

#ifdef __WXDEBUG__
    wxASSERT_MSG((int)state_list_.GetCount() == (int)GetItemCount(),
                 "Something went wrong! See InsertItem()");

    // read info by image index
    bool imagecheck = (info.m_image == wxCLC_CHECKED_IMGIDX);
    wxASSERT_MSG((checked && imagecheck) || (!checked && !imagecheck),
                 "This is item has checked state but it's shown as unchecked (or viceversa)");
#endif

    return true;
}

bool CheckedListCtrl::SetItem(wxListItem& info) {
    // only col 0 gets a checkbox
    if (info.GetColumn()) {
        return wxListCtrl::SetItem(info);
    }

    // remove the checked & enabled states from the state flag:
    // we'll store them in our separate array
    int additionalstate = GetAndRemoveAdditionalState(&info.m_state, info.m_stateMask);

    // set image index
    // we will ignore the info.m_image field since we need
    // to overwrite it...
    if (info.m_mask & wxLIST_MASK_STATE) {
        // if some state is not included in the state mask, then get the state info
        // from our internal state array
        if (!(info.m_stateMask & wxLIST_STATE_CHECKED)) {
            additionalstate |= (state_list_[info.m_itemId] & wxLIST_STATE_CHECKED);
        }

        // state is valid: use it to determine the image to set...
        info.m_mask |= wxLIST_MASK_IMAGE;
        info.m_image = GetItemImageFromAdditionalState(additionalstate);
        // since when changing the background color, also the foreground color
        // and the font of the item are changed, we try to respect the user
        // choices of such attributes
        info.SetTextColour(this->GetItemTextColour(info.GetId()));
        // before wx 2.6.2 the wxListCtrl::SetItemFont function is missing
        info.SetFont(this->GetItemFont(info.GetId()));
        // change the background color to respect the enabled/disabled status...
        info.SetBackgroundColour(GetBgColourFromAdditionalState());
        state_list_[info.m_itemId] = additionalstate;
    } else {
        // state is invalid; don't change image
        info.m_mask &= ~wxLIST_MASK_IMAGE;
    }

    // save the changes
    return wxListCtrl::SetItem(info);
}

long CheckedListCtrl::InsertItem(wxListItem& info) {
    int additionalstate = GetAndRemoveAdditionalState(&info.m_state, info.m_stateMask);

    // we always want to insert items with images...
    info.m_mask |= wxLIST_MASK_IMAGE;
    info.m_image = GetItemImageFromAdditionalState(additionalstate);
    info.SetBackgroundColour(GetBgColourFromAdditionalState());
    int itemcount = GetItemCount();
    wxASSERT_MSG(info.m_itemId <= itemcount, "Invalid index!");
    wxASSERT_MSG((int)state_list_.GetCount() == (int)GetItemCount(), "Something went wrong!");

    if (info.m_itemId == itemcount) {
        // we are adding a new item at the end of the list
        state_list_.Add(additionalstate);
    } else {
        // we must shift all following items
        state_list_.Add(state_list_[itemcount - 1]);

        for (int i = itemcount - 1; i > info.m_itemId; i--) {
            state_list_[i] = state_list_[i - 1];
        }

        state_list_[info.m_itemId] = additionalstate;
    }

    return wxListCtrl::InsertItem(info);
}

bool CheckedListCtrl::SetItemState(long item, long state, long stateMask) {
    wxListItem li;
    li.SetId(item);
    li.SetMask(wxLIST_MASK_STATE);
    li.SetState(state);
    li.SetStateMask(stateMask);
    // so we are sure to use CheckedListCtrl::SetItem
    // (and not wxListCtrl::SetItem)
    return SetItem(li);
}

int CheckedListCtrl::GetItemState(long item, long stateMask) const {
    wxListItem li;
    li.SetId(item);
    li.SetMask(wxLIST_MASK_STATE);
    li.SetStateMask(stateMask);

    // so we are sure to use CheckedListCtrl::GetItem
    // (and not wxListCtrl::GetItem)
    if (!GetItem(li))
        return -1;

    return li.GetState();
}

long CheckedListCtrl::SetItem(long index, int col, const wxString& label, int WXUNUSED(imageId)) {
    wxListItem li;
    li.SetId(index);
    li.SetColumn(col);
    li.SetText(label);
    li.SetMask(wxLIST_MASK_TEXT);
    // so we are sure to use CheckedListCtrl::SetItem
    // (and not wxListCtrl::SetItem)
    return SetItem(li);
}

long CheckedListCtrl::InsertItem(long index, const wxString& label, int WXUNUSED(imageIndex)) {
    wxListItem info;
    info.m_text = label;
    info.m_mask = wxLIST_MASK_TEXT;
    info.m_itemId = index;
    return InsertItem(info);
}

void CheckedListCtrl::Check(long item, bool checked) {
    // NB: the "statemask" is not the "mask" of a list item;
    //     in the "mask" you use the wxLIST_MASK_XXXX defines;
    //     in the "statemask" you use the wxLIST_STATE_XXX defines
    //     to set a specific bit of the wxListInfo::m_state var
    if (checked) {
        // the 2nd parameter says: activate the STATE bit relative to CHECK feature
        // the 3rd parameter says: set only *that* bit
        SetItemState(item, wxLIST_STATE_CHECKED, wxLIST_STATE_CHECKED);
    } else {
        SetItemState(item, 0, wxLIST_STATE_CHECKED);
    }
}

void CheckedListCtrl::CheckAll(bool check) {
    for (int i = 0; i < GetItemCount(); i++) {
        Check(i, check);
    }
}

bool CheckedListCtrl::DeleteItem(long item) {
    // shift our additional state array
    // for (int i=item,max=GetItemCount(); i < max-1; i++)
    //  state_list_[i] = state_list_[i+1];
    state_list_.RemoveAt(item, 1);
    return wxListCtrl::DeleteItem(item);
}

// event handlers

void CheckedListCtrl::OnMouseEvent(wxMouseEvent& event) {
    if (!event.LeftDown()) {
        event.Skip();
        return;
    }

    int flags;
    long item = HitTest(event.GetPosition(), flags);

    if (item == wxNOT_FOUND) {
        // skip this item
        event.Skip();
        return;
    }

    // user clicked exactly on the checkbox or on the item row ?
    bool processcheck = (flags & wxLIST_HITTEST_ONITEMICON) || (flags & wxLIST_HITTEST_ONITEM);

    if (processcheck) {
        wxListEvent ev(wxEVT_NULL, GetId());
        ev.m_itemIndex = item;

        // send the check event
        if (IsChecked(item)) {
            ev.SetEventType(CHECKED_LIST_ITEM_UNCHECKED);
            Check(item, false);
            AddPendingEvent(ev);
        } else {
            ev.SetEventType(CHECKED_LIST_ITEM_CHECKED);
            Check(item, true);
            AddPendingEvent(ev);
        }
    }

    event.Skip();
}

wxIMPLEMENT_DYNAMIC_CLASS(CheckedListCtrl, wxListCtrl);

// clang-format off
wxBEGIN_EVENT_TABLE(CheckedListCtrl, wxListCtrl)
    EVT_LEFT_DOWN(CheckedListCtrl::OnMouseEvent)
wxEND_EVENT_TABLE();
// clang-format on

}  // namespace widgets