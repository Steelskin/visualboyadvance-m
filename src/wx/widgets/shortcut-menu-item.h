#ifndef VBAM_SHORTCUT_MENU_ITEM_H_
#define VBAM_SHORTCUT_MENU_ITEM_H_

#include <cstdint>

#include <wx/menuitem.h>
#include <wx/xrc/xmlres.h>

#include "wx/config/handler-id.h"

// The X11 headers define a macro called Always.
#if defined(Always)
#undef Always
#endif  // defined(Always)

namespace widgets {

// Forward declarations.
class ShortcutMenuBar;
class ShortcutMenu;

// The `EnableCondition` value is used to determine if a menu item should be
// enabled or disabled based on the current state of the emulator. The
// `ShortcutMenuItem` class will use this value to determine if the menu item
// should be enabled or disabled.
// In `MainFrame`, the `cmd_enable_` value is used to determine the current
// state of the emulator. The `EnableFlag` enum is used to determine the
// possible values of `cmd_enable_`.
// TODO: These should be removed in favor of keeping these individual states
// as part of another class, either `MainFrame` or `wxvbamApp`.
enum EnableFlag : uint32_t {
    CMDEN_GB = (1u << 0), // GB ROM loaded
    CMDEN_GBA = (1u << 1), // GBA ROM loaded
    CMDEN_REWIND = (1u << 2), // rewind states available
    CMDEN_SREC = (1u << 3), // sound recording in progress
    CMDEN_VREC = (1u << 4), // video recording in progress
    CMDEN_GREC = (1u << 5), // game recording in progress
    CMDEN_GPLAY = (1u << 6), // game playback in progress
    CMDEN_SAVST = (1u << 7), // any save states available
    CMDEN_GDB = (1u << 8), // gdb connected
    CMDEN_LINK = (1u << 9), // link enabled
};

static constexpr uint32_t kGameRunning = CMDEN_GB | CMDEN_GBA;
static constexpr uint32_t kRecordingOn = CMDEN_SREC | CMDEN_VREC | CMDEN_GREC;
static constexpr uint32_t kUnloadEnableFlagsKeep = CMDEN_GDB | CMDEN_LINK;
static constexpr uint32_t kPlayerInUse = kRecordingOn | CMDEN_GPLAY;

class ShortcutMenuItem : public wxMenuItem {
public:
    enum class EnableCondition {
        Always,
        Never,
        GameRunning,
        GBGameRunning,
        GBAGameRunning,
        RewindStateAvailable,
        SoundRecordingOn,
        SoundRecordingOff,
        VideoRecordingOn,
        VideoRecordingOff,
        GameRecordingOn,
        GameRecordingOff,
        GamePlaybackOn,
        GamePlaybackOff,
        NoOngoingRecording,
        PlayerAvailable,
        SaveStateAvailable,
        GdbConnected,
        GdbNotConnected,
        LinkAvailable,
    };

    ShortcutMenuItem(EnableCondition condition = EnableCondition::Never,
                              bool enable_shortcut = false,
                              wxMenu* parent_menu = nullptr,
                              int id = wxID_SEPARATOR,
                              const wxString& name = wxEmptyString,
                              const wxString& help = wxEmptyString,
                              wxItemKind kind = wxITEM_NORMAL,
                              wxMenu* sub_menu = nullptr);
    ~ShortcutMenuItem() override = default;

    // Helper accessor for the owner ShortcutMenu, if any.
    ShortcutMenu* GetShortcutMenu() const;

    // Helper accessor for the sub menu, if any.
    ShortcutMenu* GetShortcutSubMenu() const;

    // Enable or disable the menu item based on the current state.
    void MaybeEnable(uint32_t enable_flags);

    // Returns true if shortcuts should be enabled for this menu item.
    bool shortcut() const { return shortcut_; }

    // Returns the handler ID for this menu item.
    config::HandlerID handler_id() const { return handler_id_; }

    wxDECLARE_DYNAMIC_CLASS(ShortcutMenuItem);

private:
    EnableCondition condition_;
    bool shortcut_;
    config::HandlerID handler_id_;
};

class ShortcutMenuItemXmlHandler : public wxXmlResourceHandler {
public:
    ShortcutMenuItemXmlHandler();
    ~ShortcutMenuItemXmlHandler() override = default;

    wxDECLARE_DYNAMIC_CLASS(ShortcutMenuItemXmlHandler);

private:
    ShortcutMenuItem::EnableCondition GetCondition();
    bool IsActive();

    // wxXmlResourceHandler implementation.
    wxObject* DoCreateResource() override;
    bool CanHandle(wxXmlNode* node) override;
};

}  // namespace widgets

#endif  // VBAM_SHORTCUT_MENU_ITEM_H_
