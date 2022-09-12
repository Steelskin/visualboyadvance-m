#include "wx/widgets/shortcut-menu-bar.h"

#include <cassert>

#include <wx/accel.h>
#include <wx/frame.h>
#include <wx/object.h>

#include "wx/config/handler-id.h"
#include "wx/widgets/shortcut-menu-item.h"
#include "wx/widgets/shortcut-menu.h"

namespace widgets {

extern const char ShortcutMenuBarNameStr[] = "shortcutmenubar";

ShortcutMenu* ShortcutMenuBar::GetShortcutMenu(size_t pos) const {
    wxMenu* menu = GetMenu(pos);
    return wxDynamicCast(menu, ShortcutMenu);
}

ShortcutMenuItem* ShortcutMenuBar::GetShortcutMenuItem(config::HandlerID id) const {
    const auto it = item_map_.find(id);
    if (it == item_map_.end()) {
        return nullptr;
    }

    return it->second;
}

ShortcutMenuItem* ShortcutMenuBar::GetShortcutMenuItem(int xrc_id) const {
    return GetShortcutMenuItem(config::XRCIDToHandlerID(xrc_id));
}

ShortcutMenu* ShortcutMenuBar::GetRecentMenu() const {
    wxMenuItem* menu_item = this->FindItem(XRCID("RecentMenu"));
    assert(menu_item);
    return wxDynamicCast(menu_item, ShortcutMenuItem)->GetShortcutSubMenu();
}

void ShortcutMenuBar::SetAcceleratorTable(const wxAcceleratorTable& /*accel*/) {
    // Do nothing. We don't want to set up accelerators on the menu bar.
    wxMenuBar::SetAcceleratorTable(wxNullAcceleratorTable);
}

bool ShortcutMenuBar::Append(wxMenu* menu, const wxString& title) {
    ShortcutMenu* shortcut_menu = wxDynamicCast(menu, ShortcutMenu);
    assert(shortcut_menu);

    const bool success = wxMenuBar::Append(menu, title);
    if (!success) {
        return false;
    }

    // Append the menu items to the item map.
    RegisterMenuItems(shortcut_menu);

    return true;
}

bool ShortcutMenuBar::Insert(size_t /*pos*/, wxMenu* /*menu*/, const wxString& /*title*/) {
    // Unsupported operation.
    assert(false);
    return false;
}

wxMenu* ShortcutMenuBar::Replace(size_t /*pos*/, wxMenu* /*menu*/, const wxString& /*title*/) {
    // Unsupported operation.
    assert(false);
    return nullptr;
}

wxMenu* ShortcutMenuBar::Remove(size_t pos) {
    ShortcutMenu* shortcut_menu = GetShortcutMenu(pos);
    assert(shortcut_menu);

    shortcut_menu = wxDynamicCast(wxMenuBar::Remove(pos), ShortcutMenu);
    if (shortcut_menu == nullptr) {
        return nullptr;
    }

    // Remove the menu items from the item map.
    UnregisterMenuItems(shortcut_menu);

    return shortcut_menu;
}

void ShortcutMenuBar::RegisterMenuItem(ShortcutMenuItem* item) {
    assert(item);

    if (item->IsSubMenu()) {
        RegisterMenuItems(wxDynamicCast(item->GetSubMenu(), ShortcutMenu));
        return;
    }

    const config::HandlerID id = item->handler_id();
    if (id == config::HandlerID::Noop) {
        // Skip.
        return;
    }

    assert(item_map_.find(id) == item_map_.end());
    item_map_[id] = item;
}

void ShortcutMenuBar::UnregisterMenuItem(ShortcutMenuItem* item) {
    assert(item);

    if (item->IsSubMenu()) {
        UnregisterMenuItems(wxDynamicCast(item->GetSubMenu(), ShortcutMenu));
        return;
    }

    const config::HandlerID id = item->handler_id();
    if (id == config::HandlerID::Noop) {
        // Skip.
        return;
    }
    const auto it = item_map_.find(id);
    assert(it != item_map_.end());
    assert(it->second == item);

    item_map_.erase(it);
}

void ShortcutMenuBar::RegisterMenuItems(ShortcutMenu* menu) {
    assert(menu);

    const size_t count = menu->GetMenuItemCount();
    for (size_t i = 0; i < count; ++i) {
        RegisterMenuItem(menu->GetShortcutMenuItem(i));
    }
}

void ShortcutMenuBar::UnregisterMenuItems(ShortcutMenu* menu) {
    assert(menu);

    const size_t count = menu->GetMenuItemCount();
    for (size_t i = 0; i < count; ++i) {
        UnregisterMenuItem(menu->GetShortcutMenuItem(i));
    }
}

wxIMPLEMENT_DYNAMIC_CLASS(ShortcutMenuBar, wxMenuBar);

ShortcutMenuBarXmlHandler::ShortcutMenuBarXmlHandler() : wxXmlResourceHandler() {
    XRC_ADD_STYLE(wxMB_DOCKABLE);
}

wxObject* ShortcutMenuBarXmlHandler::DoCreateResource() {
    XRC_MAKE_INSTANCE(control, ShortcutMenuBar)

    CreateChildren(control);

    if (GetParentAsWindow()) {
        wxFrame* parent_frame = wxDynamicCast(GetParent(), wxFrame);
        if (parent_frame) {
            parent_frame->SetMenuBar(control);
        }
    }

    return control;
}

bool ShortcutMenuBarXmlHandler::CanHandle(wxXmlNode* node) {
    return IsOfClass(node, "ShortcutMenuBar");
}

wxIMPLEMENT_DYNAMIC_CLASS(ShortcutMenuBarXmlHandler, wxXmlResourceHandler);

}  // namespace widgets
