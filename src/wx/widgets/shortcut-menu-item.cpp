#include "wx/widgets/shortcut-menu-item.h"

#include <cassert>
#include <map>

#include <optional.hpp>

#include "wx/config/handler-id.h"
#include "wx/widgets/shortcut-menu.h"

#if defined(__WXGTK__)
#include "wx/wayland.h"
#endif

namespace widgets {

namespace {

enum class Activate {
    Always,
    Never,
    SupportsBackgroundKeyboardInput,
    FFMpegEnabled,
    LinkEnabled,
    DebuggerEnabled,
    GbaLoggingEnabled,
    OnlineUpdateEnabled,
};

Activate StringToActivate(const wxString& string) {
    static const std::map<wxString, Activate> kStrongToActivate = {
        {"Always", Activate::Always},
        {"Never", Activate::Never},
        {"SupportsBackgroundKeyboardInput", Activate::SupportsBackgroundKeyboardInput},
        {"FFMpegEnabled", Activate::FFMpegEnabled},
        {"LinkEnabled", Activate::LinkEnabled},
        {"DebuggerEnabled", Activate::DebuggerEnabled},
        {"GbaLoggingEnabled", Activate::GbaLoggingEnabled},
        {"OnlineUpdateEnabled", Activate::OnlineUpdateEnabled},
    };

    if (string.empty()) {
        return Activate::Always;
    }

    const auto iter = kStrongToActivate.find(string);
    if (iter == kStrongToActivate.end()) {
        return Activate::Always;
    }

    return iter->second;
}

bool ActivateMenuItem(const wxString& string) {
    switch (StringToActivate(string)) {
        case Activate::Always:
            return true;
        case Activate::Never:
            return false;
        case Activate::SupportsBackgroundKeyboardInput:
#if defined(__WXMSW__)
            return true;
#elif defined(__WXGTK__)
            return !IsWayland();
#elif defined(__WXOSX__)
            return false;
#else
#error "Unsupported platform"
#endif
        case Activate::FFMpegEnabled:
#if defined(NO_FFMPEG)
            return false;
#else
            return true;
#endif
        case Activate::LinkEnabled:
#if defined(NO_LINK)
            return false;
#else
            return true;
#endif
        case Activate::DebuggerEnabled:
#if defined(VBAM_ENABLE_DEBUGGER)
            return true;
#else
            return false;
#endif
        case Activate::GbaLoggingEnabled:
#if defined(GBA_LOGGING)
            return true;
#else
            return false;
#endif
        case Activate::OnlineUpdateEnabled:
#if defined(NO_ONLINEUPDATES)
            return false;
#else
            return true;
#endif
    }

    // Unreachable.
    assert(false);
    return false;
}

}  // namespace

ShortcutMenuItem::ShortcutMenuItem(EnableCondition condition,
                                   bool enable_shortcut,
                                   wxMenu* parent_menu,
                                   int id,
                                   const wxString& name,
                                   const wxString& help,
                                   wxItemKind kind,
                                   wxMenu* sub_menu)
    : wxMenuItem(parent_menu, id, name, help, kind, sub_menu),
      condition_(condition),
      shortcut_(enable_shortcut),
      handler_id_(config::XRCIDToHandlerID(id)) {}

ShortcutMenu* ShortcutMenuItem::GetShortcutMenu() const {
    wxMenu* parent = GetMenu();
    return wxDynamicCast(parent, ShortcutMenu);
}

ShortcutMenu* ShortcutMenuItem::GetShortcutSubMenu() const {
    wxMenu* sub_menu = GetSubMenu();
    return wxDynamicCast(sub_menu, ShortcutMenu);
}

void ShortcutMenuItem::MaybeEnable(uint32_t enable_flags) {
    switch (condition_) {
        case EnableCondition::Always:
            Enable(true);
            break;
        case EnableCondition::Never:
            Enable(false);
            break;
        case EnableCondition::GameRunning:
            Enable((enable_flags & kGameRunning) != 0);
            break;
        case EnableCondition::GBGameRunning:
            Enable((enable_flags & CMDEN_GB) != 0);
            break;
        case EnableCondition::GBAGameRunning:
            Enable((enable_flags & CMDEN_GBA) != 0);
            break;
        case EnableCondition::RewindStateAvailable:
            Enable((enable_flags & CMDEN_REWIND) != 0);
            break;
        case EnableCondition::SoundRecordingOn:
            Enable((enable_flags & CMDEN_SREC) != 0);
            break;
        case EnableCondition::SoundRecordingOff:
            Enable((enable_flags & CMDEN_SREC) == 0);
            break;
        case EnableCondition::VideoRecordingOn:
            Enable((enable_flags & CMDEN_VREC) != 0);
            break;
        case EnableCondition::VideoRecordingOff:
            Enable((enable_flags & CMDEN_VREC) == 0);
            break;
        case EnableCondition::GameRecordingOn:
            Enable((enable_flags & CMDEN_GREC) != 0);
            break;
        case EnableCondition::GameRecordingOff:
            Enable((enable_flags & CMDEN_GREC) == 0);
            break;
        case EnableCondition::GamePlaybackOn:
            Enable((enable_flags & CMDEN_GPLAY) != 0);
            break;
        case EnableCondition::GamePlaybackOff:
            Enable((enable_flags & CMDEN_GPLAY) == 0);
            break;
        case EnableCondition::NoOngoingRecording:
            Enable((enable_flags & kRecordingOn) == 0);
            break;
        case EnableCondition::PlayerAvailable:
            Enable((enable_flags & kPlayerInUse) == 0);
            break;
        case EnableCondition::SaveStateAvailable:
            Enable((enable_flags & CMDEN_SAVST) != 0);
            break;
        case EnableCondition::GdbConnected:
            Enable((enable_flags & CMDEN_GDB) != 0);
            break;
        case EnableCondition::GdbNotConnected:
            Enable((enable_flags & CMDEN_GDB) == 0);
            break;
        case EnableCondition::LinkAvailable:
            Enable((enable_flags & CMDEN_LINK) != 0);
            break;
    }
}

wxIMPLEMENT_DYNAMIC_CLASS(ShortcutMenuItem, wxMenuItem);

ShortcutMenuItemXmlHandler::ShortcutMenuItemXmlHandler() : wxXmlResourceHandler() {
    XRC_ADD_STYLE(wxMENU_TEAROFF);
}

ShortcutMenuItem::EnableCondition ShortcutMenuItemXmlHandler::GetCondition() {
    static const std::map<wxString, ShortcutMenuItem::EnableCondition>
        kStrongToCondition = {
            {"Always", ShortcutMenuItem::EnableCondition::Always},
            {"Never", ShortcutMenuItem::EnableCondition::Never},
            {"GameRunning", ShortcutMenuItem::EnableCondition::GameRunning},
            {"GBGameRunning", ShortcutMenuItem::EnableCondition::GBGameRunning},
            {"GBAGameRunning", ShortcutMenuItem::EnableCondition::GBAGameRunning},
            {"RewindStateAvailable", ShortcutMenuItem::EnableCondition::RewindStateAvailable},
            {"SoundRecordingOn", ShortcutMenuItem::EnableCondition::SoundRecordingOn},
            {"SoundRecordingOff", ShortcutMenuItem::EnableCondition::SoundRecordingOff},
            {"VideoRecordingOn", ShortcutMenuItem::EnableCondition::VideoRecordingOn},
            {"VideoRecordingOff", ShortcutMenuItem::EnableCondition::VideoRecordingOff},
            {"GameRecordingOn", ShortcutMenuItem::EnableCondition::GameRecordingOn},
            {"GameRecordingOff", ShortcutMenuItem::EnableCondition::GameRecordingOff},
            {"GamePlaybackOn", ShortcutMenuItem::EnableCondition::GamePlaybackOn},
            {"GamePlaybackOff", ShortcutMenuItem::EnableCondition::GamePlaybackOff},
            {"NoOngoingRecording", ShortcutMenuItem::EnableCondition::NoOngoingRecording},
            {"PlayerAvailable", ShortcutMenuItem::EnableCondition::PlayerAvailable},
            {"SaveStateAvailable", ShortcutMenuItem::EnableCondition::SaveStateAvailable},
            {"GdbConnected", ShortcutMenuItem::EnableCondition::GdbConnected},
            {"GdbNotConnected", ShortcutMenuItem::EnableCondition::GdbNotConnected},
            {"LinkAvailable", ShortcutMenuItem::EnableCondition::LinkAvailable},
        };

    const wxString condition = GetParamValue("condition");
    if (condition.empty()) {
        return ShortcutMenuItem::EnableCondition::Always;
    }

    const auto iter = kStrongToCondition.find(condition);
    if (iter == kStrongToCondition.end()) {
        ReportParamError("condition",
                         wxString::Format("unknown <condition> value '%s'", condition));

        return ShortcutMenuItem::EnableCondition::Always;
    }

    return iter->second;
}

bool ShortcutMenuItemXmlHandler::IsActive() {
    return ActivateMenuItem(GetParamValue("activate"));
}

wxObject* ShortcutMenuItemXmlHandler::DoCreateResource() {
    ShortcutMenu* menu = wxDynamicCast(GetParent(), ShortcutMenu);

    if (GetClass() == "ShortcutSeparator") {
        menu->AppendShortcutSeparator();
        return nullptr;
    }

    assert(GetClass() == "ShortcutMenuItem");
    if (!IsActive()) {
        // Don't create the menu item if it's not enabled for this configuration.
        return nullptr;
    }

    const int id = GetID();
    const wxString label = GetText("label");
    const wxString help = GetText("help");
    const wxItemKind kind = GetBool("checkable", false) ? wxITEM_CHECK : wxITEM_NORMAL;
    const ShortcutMenuItem::EnableCondition condition = GetCondition();
    const bool shortcut = GetBool("shortcut", true);

    menu->Append(new ShortcutMenuItem(condition, shortcut, menu, id, label, help, kind));
    return nullptr;
}

bool ShortcutMenuItemXmlHandler::CanHandle(wxXmlNode* node) {
    return IsOfClass(node, "ShortcutMenuItem") || IsOfClass(node, "ShortcutSeparator");
}

wxIMPLEMENT_DYNAMIC_CLASS(ShortcutMenuItemXmlHandler, wxXmlResourceHandler);

}  // namespace widgets
