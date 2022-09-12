#include "wx/main-frame.h"

#include <cassert>
#include <cstdint>

#include <wx/event.h>
#include <wx/frame.h>
#include <wx/generic/numdlgg.h>
#include <wx/longlong.h>
#include <wx/object.h>
#include <wx/progdlg.h>
#include <wx/rtti.h>
#include <wx/time.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>

#include "core/gb/gbGlobals.h"
#include "core/gba/gbaGlobals.h"
#include "core/gba/gbaSound.h"
#include "wx/config/bindings.h"
#include "wx/config/command.h"
#include "wx/config/config-provider.h"
#include "wx/config/file-history.h"
#include "wx/config/handler-id.h"
#include "wx/config/option-proxy.h"
#include "wx/dialogs/log.h"
#include "wx/opts.h"
#include "wx/viewers/base-viewer.h"
#include "wx/viewers/disassemble.h"
#include "wx/viewers/io.h"
#include "wx/viewers/map.h"
#include "wx/viewers/mem.h"
#include "wx/viewers/oam.h"
#include "wx/viewers/palette.h"
#include "wx/viewers/tile.h"
#include "wx/widgets/shortcut-menu-bar.h"
#include "wx/widgets/shortcut-menu-item.h"
#include "wx/widgets/shortcut-menu.h"
#include "wx/wxvbam.h"

// Defined in gbRemote.cpp. TODO: Export this symbol from the core.
extern int remotePort;

#define LoadXRCViewer(t)                                    \
    do {                                                    \
        wxDialog* d = new viewers::t##Viewer;               \
        if (d) {                                            \
            d->SetWindowStyle(wxCAPTION | wxRESIZE_BORDER); \
            d->Show();                                      \
        }                                                   \
    } while (0)

namespace {

void SetStateMenuItem(widgets::ShortcutMenuItem* menu_item, const wxString& label) {
    assert(menu_item);

    const wxString old_label = menu_item->GetItemLabel();
    const size_t tab_index = old_label.find('\t');

    wxString new_label;
    new_label = label;

    if (tab_index != wxString::npos) {
        new_label.append(old_label.substr(tab_index));
    }
    menu_item->SetItemLabel(new_label);
}

void ResetMenuItemAccelerator(widgets::ShortcutMenuItem* menu_item) {
    assert(menu_item);

    if (menu_item->IsSubMenu()) {
        // Handle sub-menu items.
        widgets::ShortcutMenu* menu = menu_item->GetShortcutSubMenu();
        for (size_t i = 0; i < menu->GetMenuItemCount(); i++) {
            ResetMenuItemAccelerator(menu->GetShortcutMenuItem(i));
        }
        return;
    }

    if (config::HandlerIsState(menu_item->handler_id())) {
        // State handlers are handled separately.
        return;
    }

    menu_item->MaybeEnable(wxGetApp().frame->cmd_enable_);
    const wxString old_label = menu_item->GetItemLabel();
    const size_t tab_index = old_label.find('\t');
    wxString new_label;
    new_label = old_label;
    if (tab_index != wxString::npos) {
        new_label.resize(tab_index);
    }
    std::unordered_set<config::UserInput> user_inputs =
        wxGetApp().bindings()->InputsForCommand(config::ShortcutCommand(menu_item->handler_id()));
    if (!user_inputs.empty()) {
        new_label.append('\t');
        new_label.append(user_inputs.begin()->ToLocalizedString());
    }

    if (new_label.empty()) {
        new_label = old_label;
        if (tab_index != wxString::npos) {
            new_label.resize(tab_index);
        }
    }

    if (old_label != new_label) {
        menu_item->SetItemLabel(new_label);
    }
}

wxString DisplayString(size_t index, const wxFileName& file) {
    wxString display_string(file.GetName() + "." + file.GetExt());
    display_string.Replace("&", "&&");

#ifdef __WXMSW__
    // absolute paths always start with Latin characters even in RTL
    // environments and should therefore be rendered as LTR text (possibly with
    // RTL chunks in it). Ensure this on Windows by prepending
    // LEFT-TO-RIGHT EMBEDDING (other platforms detect this automatically)
    display_string.insert(0, wchar_t(0x202a));
#endif

    return wxString::Format("&%zu %s", index + 1, display_string);
}

}  // namespace

wxIMPLEMENT_DYNAMIC_CLASS(MainFrame, wxFrame);

MainFrame::MainFrame()
    : wxFrame(),
      logdlg(std::make_unique<dialogs::LogDialog>()),
      config_provider_(&wxGetApp()),
#ifndef NO_LINK
      gba_link_observer_(config::OptionID::kGBALinkHost,
                         std::bind(&MainFrame::EnableNetworkMenu, this)),
#endif
      status_bar_observer_(config::OptionID::kGenStatusBar,
                           std::bind(&MainFrame::OnStatusBarChanged, this)),
      file_history_observer_(config_provider_->file_history(),
                             std::bind(&MainFrame::OnHistoryChanged, this, std::placeholders::_1)),
      keep_on_top_styler_(this) {
}

MainFrame::~MainFrame() {
#ifndef NO_LINK
    CloseLink();
#endif
}

void MainFrame::OnStateSlotChanged() {
    systemScreenMessage(wxString::Format(_("Current state slot #%d"), state_slot_ + 1));
}

void MainFrame::SetStatusBar(wxStatusBar* menu_bar) {
    wxFrame::SetStatusBar(menu_bar);
    // This will take care of hiding the menu bar at startup, if needed.
    menu_bar->Show(OPTION(kGenStatusBar));
}

void MainFrame::OnHistoryChanged(config::FileHistory* file_history) {
    size_t recent = 0;
    for (const wxFileName& file : file_history->history_files()) {
        const int xrc_id = wxID_FILE1 + recent;
        const config::HandlerID handler_id = config::GetRecentHandlerID(recent);
        const wxString display_string = DisplayString(recent, file);

        widgets::ShortcutMenuItem* item = GetMenuItem(handler_id);
        if (item) {
            item->SetItemLabel(display_string);
        } else {
            item = new widgets::ShortcutMenuItem(widgets::ShortcutMenuItem::EnableCondition::Always,
                                                 true, recent_menu_, xrc_id, display_string);
            recent_menu_->Append(item);
        }
        ResetMenuItemAccelerator(item);

        recent++;
    }

    // Remove extra menu items.
    while (recent < config::FileHistory::kMaxFiles) {
        const int xrc_id = wxID_FILE1 + recent;
        widgets::ShortcutMenuItem* item = recent_menu_->FindShortcutMenuItem(xrc_id);
        if (item) {
            recent_menu_->Remove(item);
        } else {
            break;
        }
        recent++;
    }
}

void MainFrame::OnStatusBarChanged() {
    GetStatusBar()->Show(OPTION(kGenStatusBar));
    SendSizeEvent();
    panel->AdjustSize(false);
    SendSizeEvent();
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_CONTEXT_MENU(MainFrame::OnMenu)
// this is the main window focus?  Or EVT_SET_FOCUS/EVT_KILL_FOCUS?
EVT_ACTIVATE(MainFrame::OnActivate)
// requires DragAcceptFiles(true); even then may not do anything
EVT_DROP_FILES(MainFrame::OnDropFile)

// for window geometry
EVT_MOVE(MainFrame::OnMove)
EVT_MOVE_START(MainFrame::OnMoveStart)
EVT_MOVE_END(MainFrame::OnMoveEnd)
EVT_SIZE(MainFrame::OnSize)

// For tracking menubar state.
EVT_MENU_OPEN(MainFrame::MenuPopped)
EVT_MENU_CLOSE(MainFrame::MenuPopped)
EVT_MENU_HIGHLIGHT_ALL(MainFrame::MenuPopped)

END_EVENT_TABLE()

void MainFrame::OnActivate(wxActivateEvent& event) {
    focused = event.GetActive();

    if (panel && focused)
        panel->SetFocus();

    if (OPTION(kPrefPauseWhenInactive)) {
        if (panel && focused && !paused) {
            panel->Resume();
        } else if (panel && !focused) {
            panel->Pause();
        }
    }
}

void MainFrame::OnDropFile(wxDropFilesEvent& event) {
    wxString* f = event.GetFiles();
    // ignore all but last
    wxGetApp().pending_load = f[event.GetNumberOfFiles() - 1];
}

void MainFrame::OnMenu(wxContextMenuEvent& event) {
    if (IsFullScreen() && ctx_menu) {
        wxPoint p(event.GetPosition());
#if 0  // wx actually recommends ignoring the position

        if (p != wxDefaultPosition)
            p = ScreenToClient(p);

#endif
        PopupMenu(ctx_menu, p);
    }
}

void MainFrame::OnMove(wxMoveEvent&) {
    if (!init_complete_) {
        return;
    }
    if (!IsFullScreen() && !IsMaximized()) {
        const wxPoint window_pos = GetScreenPosition();
        OPTION(kGeomWindowX) = window_pos.x;
        OPTION(kGeomWindowY) = window_pos.y;
    }
}

// On Windows pause sound when moving and resizing the window to prevent dsound
// from looping.
void MainFrame::OnMoveStart(wxMoveEvent&) {
#ifdef __WXMSW__
    soundPause();
#endif
}

void MainFrame::OnMoveEnd(wxMoveEvent&) {
#ifdef __WXMSW__
    soundResume();
#endif
}

void MainFrame::OnSize(wxSizeEvent& event) {
    wxFrame::OnSize(event);
    if (!init_complete_) {
        return;
    }
    const wxRect window_rect = GetRect();
    const wxPoint window_pos = GetScreenPosition();

    if (!IsFullScreen() && !IsMaximized()) {
        if (window_rect.GetHeight() > 0 && window_rect.GetWidth() > 0) {
            OPTION(kGeomWindowHeight) = window_rect.GetHeight();
            OPTION(kGeomWindowWidth) = window_rect.GetWidth();
        }
        OPTION(kGeomWindowX) = window_pos.x;
        OPTION(kGeomWindowY) = window_pos.y;
    }

    OPTION(kGeomIsMaximized) = IsMaximized();
    OPTION(kGeomFullScreen) = IsFullScreen();
}

int MainFrame::FilterEvent(wxEvent& event) {
    static const wxLongLong kCommandDelay(100);
    static wxLongLong last_handled_command = 0;

    wxLongLong now = wxGetLocalTimeMillis();
    if (now - last_handled_command < kCommandDelay) {
        // Ignore events that are too close to the last one.
        return wxEventFilter::Event_Skip;
    }

    if (menus_opened || dialog_opened) {
        return wxEventFilter::Event_Skip;
    }

    if (event.GetEventType() != VBAM_EVT_USER_INPUT_DOWN) {
        // We only treat "VBAM_EVT_USER_INPUT_DOWN" events here.
        return wxEventFilter::Event_Skip;
    }

    const widgets::UserInputEvent& user_input_event = static_cast<widgets::UserInputEvent&>(event);
    nonstd::optional<config::Command> command =
        wxGetApp().bindings()->CommandForInput(user_input_event.input());
    if (command == nonstd::nullopt) {
        // No associated command found.
        return wxEventFilter::Event_Skip;
    }

    if (!command->is_shortcut()) {
        return wxEventFilter::Event_Skip;
    }

    const widgets::ShortcutMenuItem* menu_item = GetMenuItem(command->shortcut().id());
    if (menu_item != nullptr && !menu_item->IsEnabled()) {
        // Command currently inactive.
        return wxEventFilter::Event_Skip;
    }

    // Execute the handler directly.
    last_handled_command = now;
    ExecuteHandler(command->shortcut().id());
    return wxEventFilter::Event_Processed;
}

wxString MainFrame::GetGamePath(wxString path) {
    wxString game_path = path;

    if (game_path.size()) {
        game_path = wxGetApp().GetAbsolutePath(game_path);
    } else {
        game_path = panel->game_dir();
        wxFileName::Mkdir(game_path, 0777, wxPATH_MKDIR_FULL);
    }

    if (!wxFileName::DirExists(game_path))
        game_path = wxFileName::GetCwd();

    if (!wxIsWritable(game_path)) {
        game_path = wxGetApp().GetDataDir();
        wxFileName::Mkdir(game_path, 0777, wxPATH_MKDIR_FULL);
    }

    return game_path;
}

void MainFrame::enable_menus() {
    widgets::ShortcutMenuBar* menubar = wxDynamicCast(GetMenuBar(), widgets::ShortcutMenuBar);
    if (!menubar) {
        return;
    }

    for (const auto& item : menubar->item_map()) {
        ResetMenuItemAccelerator(item.second);
    }
}

void MainFrame::update_state_ts() {
    bool any_states = false;

    for (int i = 0; i < 10; i++) {
        state_timestamps_[i] = wxInvalidDateTime;

        widgets::ShortcutMenuItem* load_menu_item = GetMenuItem(config::GetLoadStateHandlerID(i));
        widgets::ShortcutMenuItem* save_menu_item = GetMenuItem(config::GetSaveStateHandlerID(i));
        assert(load_menu_item);
        assert(save_menu_item);

        if (panel->game_type() == IMAGE_UNKNOWN) {
            // Reset everything.
            const wxString label = wxString::Format("%d", i + 1);
            SetStateMenuItem(load_menu_item, label);
            SetStateMenuItem(save_menu_item, label);
            load_menu_item->Enable(false);
            save_menu_item->Enable(false);
        } else {
            wxString fn;
            fn.Printf(SAVESLOT_FMT, panel->game_name().wc_str(), i + 1);
            wxFileName fp(panel->state_dir(), fn);
            wxDateTime ts;  // = wxInvalidDateTime

            if (fp.IsFileReadable()) {
                ts = fp.GetModificationTime();
                any_states = true;
            }

            // if(ts != state_timestamps_[i] || (!ts.IsValid())) {
            // to prevent assertions (stupid wx):
            if (ts.IsValid() != state_timestamps_[i].IsValid() ||
                (ts.IsValid() && ts != state_timestamps_[i]) || !ts.IsValid()) {
                // wx has no easy way of doing the -- bit independent
                // of locale
                // so use a real date and substitute all digits
                wxDateTime fts = ts.IsValid() ? ts : wxDateTime::Now();
                wxString df = fts.Format(wxT("0&0 %x %X"));

                if (!ts.IsValid())
                    for (size_t j = 0; j < df.size(); j++)
                        if (wxIsdigit(df[j]))
                            df[j] = wxT('-');

                df[0] = i == 9 ? wxT('1') : wxT(' ');
                df[2] = wxT('0') + (i + 1) % 10;

                SetStateMenuItem(load_menu_item, df);
                SetStateMenuItem(save_menu_item, df);
                load_menu_item->Enable(ts.IsValid());
                save_menu_item->Enable(ts.IsValid());
            }

            state_timestamps_[i] = ts;
        }
    }

    uint32_t cmd_flg = any_states ? widgets::CMDEN_SAVST : 0;

    if ((cmd_enable_ & widgets::CMDEN_SAVST) != cmd_flg) {
        cmd_enable_ = (cmd_enable_ & ~widgets::CMDEN_SAVST) | cmd_flg;
        enable_menus();
    }
}

int MainFrame::oldest_state_slot() {
    wxDateTime ot;
    int os = -1;

    for (int i = 0; i < 10; i++) {
        if (!state_timestamps_[i].IsValid())
            return i + 1;

        if (os < 0 || state_timestamps_[i] < ot) {
            os = i;
            ot = state_timestamps_[i];
        }
    }

    return os + 1;
}

int MainFrame::newest_state_slot() {
    wxDateTime nt;
    int ns = -1;

    for (int i = 0; i < 10; i++) {
        if (!state_timestamps_[i].IsValid())
            continue;

        if (ns < 0 || state_timestamps_[i] > nt) {
            ns = i;
            nt = state_timestamps_[i];
        }
    }

    return ns + 1;
}

void MainFrame::ResetMenuAccelerators() {
    widgets::ShortcutMenuBar* menubar = wxDynamicCast(GetMenuBar(), widgets::ShortcutMenuBar);
    for (size_t i = 0; i < menubar->GetMenuCount(); i++) {
        widgets::ShortcutMenu* menu = menubar->GetShortcutMenu(i);
        for (size_t j = 0; j < menu->GetMenuItemCount(); j++) {
            ResetMenuItemAccelerator(menu->GetShortcutMenuItem(j));
        }
    }
}

void MainFrame::MenuPopped(wxMenuEvent& evt) {
    // We consider the menu closed when the main menubar or system menu is closed, not any submenus.
    // On Windows nullptr is the system menu.
    if (evt.GetEventType() == wxEVT_MENU_CLOSE &&
        (evt.GetMenu() == nullptr || evt.GetMenu()->GetMenuBar() == GetMenuBar()))
        SetMenusOpened(false);
    else
        SetMenusOpened(true);

    evt.Skip();
}

// On Windows, opening the menubar will stop the app, but DirectSound will
// loop, so we pause audio here.
void MainFrame::SetMenusOpened(bool state) {
    menus_opened = state;

#ifdef __WXMSW__
    if (menus_opened)
        soundPause();
    else if (!paused)
        soundResume();
#endif
}

// ShowModal that also disables emulator loop
// uses dialog_opened as a nesting counter
int MainFrame::ShowModal(wxDialog* dlg) {
    dlg->SetWindowStyle(dlg->GetWindowStyle() | wxCAPTION | wxRESIZE_BORDER);

    CheckPointer(dlg);
    StartModal();
    int ret = dlg->ShowModal();
    StopModal();
    return ret;
}

void MainFrame::StartModal() {
    // workaround for lack of wxGTK mouse motion events: unblank
    // pointer when dialog popped up
    // it will auto-hide again once game resumes
    panel->ShowPointer();
    // panel->Pause();
    ++dialog_opened;
}

void MainFrame::StopModal() {
    if (!dialog_opened)  // technically an error in the caller
        return;

    --dialog_opened;

    if (!IsPaused())
        panel->Resume();
}

#if defined(VBAM_ENABLE_DEBUGGER)

int MainFrame::GetGDBPort() {
    ModalPause mp(this);
    return wxGetNumberFromUser(
#ifdef __WXMSW__
        wxEmptyString,
#else
        _("Set to 0 for pseudo tty"),
#endif
        _("Port to wait for connection:"), _("GDB Connection"), gopts.gdb_port,
#ifdef __WXMSW__
        1025,
#else
        0,
#endif
        65535, this);
}

void MainFrame::GDBBreak() {
    ModalPause mp(this);

    if (gopts.gdb_port == 0) {
        int port_selected = GetGDBPort();

        if (port_selected != -1) {
            gopts.gdb_port = port_selected;
            update_opts();
        }
    }

    if (gopts.gdb_port > 0) {
        if (!remotePort) {
            wxString msg;
#ifndef __WXMSW__

            if (!gopts.gdb_port) {
                if (!debugOpenPty())
                    return;

                msg.Printf(_("Waiting for connection at %s"), debugGetSlavePty().wc_str());
            } else
#endif
            {
                if (!debugStartListen(gopts.gdb_port))
                    return;

                msg.Printf(_("Waiting for connection on port %d"), gopts.gdb_port);
            }

            wxProgressDialog dlg(_("Waiting for GDB..."), msg, 100, this,
                                 wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_ELAPSED_TIME);
            bool connected = false;

            while (dlg.Pulse()) {
#ifndef __WXMSW__
                if (!gopts.gdb_port)
                    connected = debugWaitPty();
                else
#endif
                    connected = debugWaitSocket();

                if (connected)
                    break;

                // sleep a bit more in case of infinite loop
                wxMilliSleep(10);
            }

            if (connected) {
                remotePort = gopts.gdb_port;
                emulating = 1;
                dbgMain = remoteStubMain;
                dbgSignal = remoteStubSignal;
                dbgOutput = remoteOutput;
                cmd_enable_ |= widgets::CMDEN_GDB;
                enable_menus();
                debugger = true;
            } else {
                remoteCleanUp();
            }
        } else {
            if (armState) {
                armNextPC -= 4;
                reg[15].I -= 4;
            } else {
                armNextPC -= 2;
                reg[15].I -= 2;
            }

            debugger = true;
        }
    }
}
#endif  // defined(VBAM_ENABLE_DEBUGGER)

#ifndef NO_LINK

LinkMode MainFrame::GetConfiguredLinkMode() {
    switch (OPTION(kGBALinkType)) {
        case 0:
            return LINK_DISCONNECTED;
            break;

        case 1:
            if (OPTION(kGBALinkProto))
                return LINK_CABLE_IPC;
            else
                return LINK_CABLE_SOCKET;

            break;

        case 2:
            if (OPTION(kGBALinkProto))
                return LINK_RFU_IPC;
            else
                return LINK_RFU_SOCKET;

            break;

        case 3:
            return LINK_GAMECUBE_DOLPHIN;
            break;

        case 4:
            if (OPTION(kGBALinkProto))
                return LINK_GAMEBOY_IPC;
            else
                return LINK_GAMEBOY_SOCKET;

            break;

        default:
            return LINK_DISCONNECTED;
            break;
    }
}

#endif  // NO_LINK

void MainFrame::IdentifyRom() {
    if (!panel->rom_name.empty())
        return;

    panel->rom_name = panel->loaded_game.GetFullName();
    wxString name;
    wxString scene_rls;
    wxString scene_name;
    wxString rom_crc32_str;
    rom_crc32_str.Printf(wxT("%08X"), panel->rom_crc32);

    if (wxGetApp().rom_database_nointro.FileExists()) {
        wxFileInputStream input(wxGetApp().rom_database_nointro.GetFullPath());
        wxTextInputStream text(input, wxT("\x09"), wxConvUTF8);

        while (input.IsOk() && !input.Eof()) {
            wxString line = text.ReadLine();

            if (line.Contains(wxT("<releaseNumber>"))) {
                scene_rls = line.AfterFirst('>').BeforeLast('<');
            }

            if (line.Contains(wxT("romCRC ")) && line.Contains(rom_crc32_str)) {
                panel->rom_scene_rls = scene_rls;
                break;
            }
        }
    }

    if (wxGetApp().rom_database_scene.FileExists()) {
        wxFileInputStream input(wxGetApp().rom_database_scene.GetFullPath());
        wxTextInputStream text(input, wxT("\x09"), wxConvUTF8);

        while (input.IsOk() && !input.Eof()) {
            wxString line = text.ReadLine();

            if (line.StartsWith(wxT("\tname"))) {
                scene_name = line.AfterLast(' ').BeforeLast('"');
            }

            if (line.StartsWith(wxT("\trom")) && line.Contains(wxT("crc ") + rom_crc32_str)) {
                panel->rom_scene_rls_name = scene_name;
                panel->rom_name = scene_name;
                break;
            }
        }
    }

    if (wxGetApp().rom_database.FileExists()) {
        wxFileInputStream input(wxGetApp().rom_database.GetFullPath());
        wxTextInputStream text(input, wxT("\x09"), wxConvUTF8);

        while (input.IsOk() && !input.Eof()) {
            wxString line = text.ReadLine();

            if (line.StartsWith(wxT("\tname"))) {
                name = line.AfterFirst('"').BeforeLast('"');
            }

            if (line.StartsWith(wxT("\trom")) && line.Contains(wxT("crc ") + rom_crc32_str)) {
                panel->rom_name = name;
                break;
            }
        }
    }
}

widgets::ShortcutMenuBar* MainFrame::GetShortcutMenuBar() {
    return wxDynamicCast(GetMenuBar(), widgets::ShortcutMenuBar);
}

widgets::ShortcutMenuItem* MainFrame::GetMenuItem(config::HandlerID id) {
    return GetShortcutMenuBar()->GetShortcutMenuItem(id);
}

void MainFrame::UpdateViewers() {
    for (auto viewer : popups) {
        if (viewer->auto_update_) {
            viewer->Update();
        }
    }
}

void MainFrame::Disassemble(void) {
    switch (panel->game_type()) {
        case IMAGE_GBA:
            LoadXRCViewer(Disassemble);
            break;

        case IMAGE_GB:
            LoadXRCViewer(GBDisassemble);
            break;

        case IMAGE_UNKNOWN:
            // do nothing
            break;
    }
}
void MainFrame::IOViewer() {
    LoadXRCViewer(IO);
}

void MainFrame::MemViewer() {
    switch (panel->game_type()) {
        case IMAGE_GBA:
            LoadXRCViewer(Mem);
            break;

        case IMAGE_GB:
            LoadXRCViewer(GBMem);
            break;

        default:
            break;
    }
}

void MainFrame::MapViewer() {
    switch (panel->game_type()) {
        case IMAGE_GBA:
            LoadXRCViewer(Map);
            break;

        case IMAGE_GB:
            LoadXRCViewer(GBMap);
            break;

        case IMAGE_UNKNOWN:
            // do nothing
            break;
    }
}

void MainFrame::TileViewer() {
    switch (panel->game_type()) {
        case IMAGE_GBA:
            LoadXRCViewer(Tile);
            break;

        case IMAGE_GB:
            LoadXRCViewer(GBTile);
            break;

        case IMAGE_UNKNOWN:
            // do nothing
            break;
    }
}

void MainFrame::PaletteViewer() {
    switch (panel->game_type()) {
        case IMAGE_GBA:
            LoadXRCViewer(Palette);
            break;

        case IMAGE_GB:
            LoadXRCViewer(GBPalette);
            break;

        case IMAGE_UNKNOWN:
            // do nothing
            break;
    }
}

void MainFrame::OAMViewer() {
    switch (panel->game_type()) {
        case IMAGE_GBA:
            LoadXRCViewer(OAM);
            break;

        case IMAGE_GB:
            LoadXRCViewer(GBOAM);
            break;

        case IMAGE_UNKNOWN:
            // do nothing
            break;
    }
}
