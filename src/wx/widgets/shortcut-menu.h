#ifndef VBAM_WX_WIDGETS_SHORTCUT_MENU_H_
#define VBAM_WX_WIDGETS_SHORTCUT_MENU_H_

#include <wx/menu.h>
#include <wx/xrc/xmlres.h>

// Forward declaration.
class wxMenuItem;

namespace widgets {

class ShortcutMenuBar;
class ShortcutMenuItem;

class ShortcutMenu : public wxMenu {
public:
    ShortcutMenu() = default;
    ~ShortcutMenu() override = default;

    ShortcutMenuBar* GetShortcutMenuBar() const;
    ShortcutMenuItem* GetShortcutMenuItem(size_t pos) const;
    ShortcutMenuItem* FindShortcutMenuItem(int xrc_id) const;

    void AppendShortcutSeparator();
    void AppendShortcutMenu(int xrc_ir,
                            const wxString& title,
                            ShortcutMenu* menu,
                            const wxString& help);

    wxDECLARE_DYNAMIC_CLASS(ShortcutMenu);

private:
    // wxMenu implementation.
    wxMenuItem* DoAppend(wxMenuItem* item) override;
    wxMenuItem* DoInsert(size_t pos, wxMenuItem* item) override;
    wxMenuItem* DoRemove(wxMenuItem* item) override;
    bool DoDelete(wxMenuItem* item) override;
    bool DoDestroy(wxMenuItem* item) override;
};

class ShortcutMenuXmlHandler : public wxXmlResourceHandler {
public:
    ShortcutMenuXmlHandler();
    ~ShortcutMenuXmlHandler() override = default;

    wxDECLARE_DYNAMIC_CLASS(ShortcutMenuXmlHandler);

private:
    // wxXmlResourceHandler implementation.
    wxObject* DoCreateResource() override;
    bool CanHandle(wxXmlNode* node) override;
};

}  // namespace widgets

#endif  // VBAM_WX_WIDGETS_SHORTCUT_MENU_H_