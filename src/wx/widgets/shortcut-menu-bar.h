#ifndef VBAM_WIDGETS_SHORTCUT_MENU_BAR_H_
#define VBAM_WIDGETS_SHORTCUT_MENU_BAR_H_

#include <unordered_map>

#include <wx/menu.h>
#include <wx/xrc/xmlres.h>

#include "wx/config/handler-id.h"

namespace widgets {

class ShortcutMenu;
class ShortcutMenuItem;

class ShortcutMenuBar : public wxMenuBar {
public:
    // Internal map of handler ID to manu items. The ShortcutMenuItems own all
    // of the relevant data.
    using ItemMap = std::unordered_map<config::HandlerID, ShortcutMenuItem*>;

    ShortcutMenuBar() = default;
    ~ShortcutMenuBar() override = default;

    // Helper accessors for the derived classes.
    ShortcutMenu* GetShortcutMenu(size_t pos) const;
    ShortcutMenuItem* GetShortcutMenuItem(config::HandlerID id) const;
    ShortcutMenuItem* GetShortcutMenuItem(int xrc_id) const;
    ShortcutMenu* GetRecentMenu() const;

    const ItemMap& item_map() const { return item_map_; }

    // wxMenuBar implementation.
    void SetAcceleratorTable(const wxAcceleratorTable& accel) override;
    bool Append( wxMenu *menu, const wxString &title ) override;
    bool Insert(size_t pos, wxMenu *menu, const wxString& title) override;
    wxMenu* Replace(size_t pos, wxMenu *menu, const wxString& title) override;
    wxMenu* Remove(size_t pos) override;

    wxDECLARE_DYNAMIC_CLASS(ShortcutMenuBar);

private:
    // Methods to register and unregister menu items. Will assert on failure.
    void RegisterMenuItem(ShortcutMenuItem* item);
    void UnregisterMenuItem(ShortcutMenuItem* item);
    void RegisterMenuItems(ShortcutMenu* menu);
    void UnregisterMenuItems(ShortcutMenu* menu);

    ItemMap item_map_;

    // For acces to RegisterMenuItem/UnregisterMenuItem.
    friend class ShortcutMenu;
};

class ShortcutMenuBarXmlHandler : public wxXmlResourceHandler {
public:
    ShortcutMenuBarXmlHandler();
    ~ShortcutMenuBarXmlHandler() override = default;

    wxDECLARE_DYNAMIC_CLASS(ShortcutMenuBarXmlHandler);

private:
    // wxXmlResourceHandler implementation.
    wxObject* DoCreateResource() override;
    bool CanHandle(wxXmlNode* node) override;
};

}  // namespace widgets

#endif  // VBAM_WIDGETS_SHORTCUT_MENU_BAR_H_