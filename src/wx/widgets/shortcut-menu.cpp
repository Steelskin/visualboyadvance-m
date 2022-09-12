#include "wx/widgets/shortcut-menu.h"

#include <cassert>

#include "wx/widgets/shortcut-menu-bar.h"
#include "wx/widgets/shortcut-menu-item.h"

namespace widgets {

ShortcutMenuBar* ShortcutMenu::GetShortcutMenuBar() const {
    wxMenuBar* menu_bar = GetMenuBar();
    return wxDynamicCast(menu_bar, ShortcutMenuBar);
}

ShortcutMenuItem* ShortcutMenu::GetShortcutMenuItem(size_t pos) const {
    wxMenuItem* item = FindItemByPosition(pos);
    return wxDynamicCast(item, ShortcutMenuItem);
}

ShortcutMenuItem* ShortcutMenu::FindShortcutMenuItem(int xrc_id) const {
    wxMenuItem* item = FindItem(xrc_id);
    return wxDynamicCast(item, ShortcutMenuItem);
}

void ShortcutMenu::AppendShortcutSeparator() {
    DoAppend(new ShortcutMenuItem(ShortcutMenuItem::EnableCondition::Always, false, this,
                                  wxID_SEPARATOR));
}

void ShortcutMenu::AppendShortcutMenu(int xrc_id,
                                      const wxString& title,
                                      ShortcutMenu* menu,
                                      const wxString& help) {
    DoAppend(new ShortcutMenuItem(ShortcutMenuItem::EnableCondition::Always, false, this, xrc_id,
                                  title, help, wxITEM_NORMAL, menu));
}

wxMenuItem* ShortcutMenu::DoAppend(wxMenuItem* item) {
    ShortcutMenuItem* shortcut_item = wxDynamicCast(item, ShortcutMenuItem);
    assert(shortcut_item);

    // Do the append operation.
    shortcut_item = wxDynamicCast(wxMenu::DoAppend(item), ShortcutMenuItem);
    assert(shortcut_item);

    // Register the item with the menu bar.
    ShortcutMenuBar* menu_bar = GetShortcutMenuBar();
    if (menu_bar) {
        menu_bar->RegisterMenuItem(shortcut_item);
    }

    return shortcut_item;
}

wxMenuItem* ShortcutMenu::DoInsert(size_t /*pos*/, wxMenuItem* /*item*/) {
    // Unsupported operation.
    assert(false);
    return nullptr;
}

wxMenuItem* ShortcutMenu::DoRemove(wxMenuItem* item) {
    ShortcutMenuItem* shortcut_item = wxDynamicCast(item, ShortcutMenuItem);
    assert(shortcut_item);

    shortcut_item = wxDynamicCast(wxMenu::DoRemove(item), ShortcutMenuItem);

    // Unregister the old item with the menu bar.
    ShortcutMenuBar* menu_bar = GetShortcutMenuBar();
    if (menu_bar) {
        menu_bar->UnregisterMenuItem(shortcut_item);
    }

    return shortcut_item;
}

bool ShortcutMenu::DoDelete(wxMenuItem* item) {
    ShortcutMenuItem* shortcut_item = wxDynamicCast(item, ShortcutMenuItem);
    assert(shortcut_item);

    // This will call DoRemove.
    return wxMenu::DoDelete(item);
}

bool ShortcutMenu::DoDestroy(wxMenuItem* item) {
    ShortcutMenuItem* shortcut_item = wxDynamicCast(item, ShortcutMenuItem);
    assert(shortcut_item);

    // This will call DoRemove.
    return wxMenu::DoDestroy(item);
}

wxIMPLEMENT_DYNAMIC_CLASS(ShortcutMenu, wxMenu);

ShortcutMenuXmlHandler::ShortcutMenuXmlHandler() {
    XRC_ADD_STYLE(wxMENU_TEAROFF);
}

wxObject* ShortcutMenuXmlHandler::DoCreateResource() {
    ShortcutMenu* menu =
        GetInstance() ? wxDynamicCast(GetInstance(), ShortcutMenu) : new ShortcutMenu();

    // Create the children now.
    CreateChildren(menu);

    if (menu->GetMenuItemCount() == 0) {
        // Skip empty menus.
        if (!GetInstance()) {
            delete menu;
        }
        return nullptr;
    }

    const wxString title = GetText("label");
    const wxString help = GetText("help");

    ShortcutMenuBar* menu_bar = wxDynamicCast(GetParent(), ShortcutMenuBar);
    if (menu_bar) {
        // Append to the MenuBar.
        menu_bar->Append(menu, title);
        return menu;
    }

    ShortcutMenu* parent_menu = wxDynamicCast(GetParent(), ShortcutMenu);
    if (parent_menu) {
        parent_menu->AppendShortcutMenu(GetID(), title, menu, help);
        return menu;
    }

    // Unknown element node.
    assert(false);
    return nullptr;
}

bool ShortcutMenuXmlHandler::CanHandle(wxXmlNode* node) {
    return IsOfClass(node, "ShortcutMenu");
}

wxIMPLEMENT_DYNAMIC_CLASS(ShortcutMenuXmlHandler, wxXmlResourceHandler);

}  // namespace widgets