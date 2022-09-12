#ifndef VBAM_WX_MAIN_FRAME_H_
#define VBAM_WX_MAIN_FRAME_H_

#include <array>
#include <list>
#include <memory>

#include <wx/dialog.h>
#include <wx/frame.h>
#include <wx/rtti.h>
#include <wx/xrc/xmlres.h>

#include "wx/config/file-history.h"
#include "wx/config/handler-id.h"
#include "wx/config/option-id.h"
#include "wx/config/option-observer.h"
#include "wx/widgets/keep-on-top-styler.h"
#include "wx/wxvbam.h"

#ifndef NO_LINK
#include "core/gba/gbaLink.h"
#endif

// Forward declarations.
class GameArea;

namespace config {
class ConfigProvider;
}  // namespace config

namespace dialogs {
class LogDialog;
}  // namespace dialogs 

namespace viewers {
class BaseViewer;
}  // namespace viewers

namespace widgets {
class ShortcutMenu;
class ShortcutMenuBar;
class ShortcutMenuItem;
}  // namespace widgets

struct checkable_mi_t {
    int cmd;
    wxMenuItem* mi;
    int mask, val;
    bool initialized = false;
};
typedef std::unordered_map<int, checkable_mi_t> checkable_mi_map_t;

class MainFrame : public wxFrame {
public:
    MainFrame();
    ~MainFrame() override;

    bool BindControls();
    void MenuOptionIntMask(const wxString& menuName, int field, int mask);
    void MenuOptionIntRadioValue(const wxString& menuName, int field, int mask);
    void MenuOptionBool(const wxString& menuName, bool field);
    void GetMenuOptionConfig(const wxString& menu_name, const config::OptionID& option_id);
    void GetMenuOptionInt(const wxString& menuName, int* field, int mask);
    void GetMenuOptionBool(const wxString& menuName, bool* field);
    void SetMenuOption(const wxString& menuName, bool value);

    int FilterEvent(wxEvent& event);

    GameArea* GetPanel() { return panel; }

    // Gets the XRC dialog with the given name.
    wxDialog* GetXRCDialog(const wxString& name);

    wxString GetGamePath(wxString path);

    // wxMSW pauses the game for menu popups and modal dialogs, but wxGTK
    // does not.  It's probably desirable to pause the game.  To do this for
    // dialogs, use this function instead of dlg->ShowModal()
    int ShowModal(wxDialog* dlg);

    // and here are the wrapper functions for use when ShowModal() isn't
    // possible
    void StartModal();
    void StopModal();

    // however, adding a handler for open/close menu to do the same is unsafe.
    // there is no guarantee every show will be matched by a hide.
    void MenuPopped(wxMenuEvent& evt);

    // adjust menus based on current cmd_enable_
    void enable_menus();
#ifndef NO_LINK
    void EnableNetworkMenu();
#endif

    // adjust menus based on available save game states
    void update_state_ts();

    // retrieve oldest/newest slot
    // returns lowest-numbered slot on ties
    int oldest_state_slot();  // or empty slot if available
    int newest_state_slot();  // or 0 if none

    // Resets all menu accelerators.
    void ResetMenuAccelerators();

    // 2.8 has no HasFocus(), and FindFocus() doesn't work right
    bool HasFocus() const override { return focused; }

    // the cheat search dialog isn't destroyed or tracked, but it needs
    // to be cleared between games
    void ResetCheatSearch();

    void IdentifyRom();

#ifndef NO_LINK
    // Returns the link mode to set according to the options
    LinkMode GetConfiguredLinkMode();
#endif

#if defined(VBAM_ENABLE_DEBUGGER)

    // Queries the user for the GDB port to use.
    int GetGDBPort();

    // Start GDB listener
    void GDBBreak();

#endif  // defined(VBAM_ENABLE_DEBUGGER)

    // call this to update the viewers once a frame:
    void UpdateViewers();

    bool MenusOpened() { return menus_opened; }
    void SetMenusOpened(bool state);

    bool DialogOpened() { return dialog_opened != 0; }

    bool IsPaused(bool incendental = false) {
        return (paused && !pause_next && !incendental) || dialog_opened;
    }

    // flags for enabling commands
    uint32_t cmd_enable_;

    // since they will all have to be destroyed on game unload:
    std::list<viewers::BaseViewer*> popups;

    // this won't actually be destroyed, but it needs to be tracked so only
    // one is ever up and it needs to be pinged when new messages arrive
    std::unique_ptr<dialogs::LogDialog> logdlg;

    wxDECLARE_DYNAMIC_CLASS(MainFrame);
    wxDECLARE_EVENT_TABLE();

private:
    // Static list of handlers for the menu items.
    using Handler = std::function<void(MainFrame*)>;
    using HandlerMap = std::array<Handler, config::kNbHandlers>;
    static const HandlerMap& GetHandlerMap();
    static Handler GetHandler(config::HandlerID id);

    // Execute the handler for the given ID.
    void ExecuteHandler(config::HandlerID id);

    GameArea* panel;

    int dialog_opened = 0;

    bool autoLoadMostRecent;
    bool paused = false;
    bool menus_opened = false;

    // copy of top-level menu bar as a context menu
    wxMenu* ctx_menu;

    // load/save states timestamps.
    std::array<wxDateTime, 10> state_timestamps_ = {};

    checkable_mi_map_t checkable_mi;

    // The current state directory.
    wxString state_dir_;

    // The current state slot.
    int state_slot_ = 0;

    // The current battery directory.
    wxString battery_dir_;

    // The current gameshark file directory.
    wxString gameshark_dir_;

    // quicker & more accurate than FindFocus() != NULL
    bool focused = false;

    // One-time toggle to indicate that this object is fully initialized. This
    // used to filter events that are sent during initialization.
    bool init_complete_ = false;

    widgets::ShortcutMenu* recent_menu_ = nullptr;

    config::ConfigProvider* const config_provider_;
#ifndef NO_LINK
    const config::OptionsObserver gba_link_observer_;
#endif
    const config::OptionsObserver status_bar_observer_;
    const config::FileHistoryObserver file_history_observer_;
    const widgets::KeepOnTopStyler keep_on_top_styler_;

    // Load a named wxDialog from the XRC file
    wxDialog* LoadXRCDialog(const wxString& name);

    // Bind the application icon.
    void BindAppIcon();

    widgets::ShortcutMenuBar* GetShortcutMenuBar();
    widgets::ShortcutMenuItem* GetMenuItem(config::HandlerID id);

    // The various viewer popups; these can be popped up as often as desired.
    void Disassemble();
    void IOViewer();
    void MapViewer();
    void MemViewer();
    void OAMViewer();
    void PaletteViewer();
    void TileViewer();

    // Called when the save state slot changes.
    void OnStateSlotChanged();

    // wxWidgets callbacks.

    // For enabling / disabling the status bar.
    void OnStatusBarChanged();
    // for detecting window focus
    void OnActivate(wxActivateEvent&);
    // may work, may not...  if so, load dropped file
    void OnDropFile(wxDropFilesEvent&);
    // pop up menu in fullscreen mode
    void OnMenu(wxContextMenuEvent&);

    // window geometry
    void OnMove(wxMoveEvent& event);
    void OnMoveStart(wxMoveEvent& event);
    void OnMoveEnd(wxMoveEvent& event);
    void OnSize(wxSizeEvent& event);

    // Called when the history changes.
    void OnHistoryChanged(config::FileHistory* file_history);

    // wxFrame override.
    void SetStatusBar(wxStatusBar* menuBar) override;

    // Menu handler callbacks. For the sake of sanity, keep them in the same
    // order as the config::HandlerID enum. The implementations are in
    // separate files, called main-frame-handlers-*.cpp. There is one file for
    // each top-level menu.

    // File
    void OnOpenGba();
    void OnOpenGB();
    void OnOpenGBC();

    // File - Recent
    void OnRecentReset();
    void OnRecentFreeze();
    void OnRecentFile1();
    void OnRecentFile2();
    void OnRecentFile3();
    void OnRecentFile4();
    void OnRecentFile5();
    void OnRecentFile6();
    void OnRecentFile7();
    void OnRecentFile8();
    void OnRecentFile9();
    void OnRecentFile10();

    // File
    void OnRomInformation();

    // File - e-Reader
    void OnResetLoadingDotCodeFile();
    void OnSetLoadingDotCodeFile();
    void OnResetSavingDotCodeFile();
    void OnSetSavingDotCodeFile();

    // File - Load State
    void OnLoadStateRecent();
    void OnLoadStateSlot();
    void OnLoadStateAutoLoad();
    void OnLoadState01();
    void OnLoadState02();
    void OnLoadState03();
    void OnLoadState04();
    void OnLoadState05();
    void OnLoadState06();
    void OnLoadState07();
    void OnLoadState08();
    void OnLoadState09();
    void OnLoadState10();
    void OnLoadStateFile();
    void OnLoadStateKeepSaves();
    void OnLoadStateKeepCheats();

    // File - Save State
    void OnSaveStateOldest();
    void OnSaveStateSlot();
    void OnSaveStateIncreaseSlot();
    void OnSaveState01();
    void OnSaveState02();
    void OnSaveState03();
    void OnSaveState04();
    void OnSaveState05();
    void OnSaveState06();
    void OnSaveState07();
    void OnSaveState08();
    void OnSaveState09();
    void OnSaveState10();
    void OnSaveStateFile();

    // File
    void OnStateIncreaseSlot();
    void OnStateDecreaseSlot();

    // File - Import
    void OnImportBatteryFile();
    void OnImportGamesharkCodeFile();
    void OnImportGamesharkActionReplaySnapshot();

    // File - Export
    void OnExportBatteryFile();
    void OnExportGamesharkSnapshot();

    // File
    void OnScreenCapture();

    // File - Record
    void OnRecordSoundStartRecording();
    void OnRecordSoundStopRecording();
    void OnRecordAVIStartRecording();
    void OnRecordAVIStopRecording();
    void OnRecordMovieStartRecording();
    void OnRecordMovieStopRecording();

    // File - Playback
    void OnPlayMovieStartPlaying();
    void OnPlayMovieStopPlaying();

    // File
    void OnClose();
    void OnExit();

    // Emulation
    void OnPause();
    void OnNextFrame();
    void OnRewind();
    void OnToggleFullscreen();
    void OnEmulatorSpeedupToggle();
    void OnVSync();
    void OnFrameSkipAuto();
    void OnSkipIntro();
    void OnApplyPatches();
    void OnPauseWhenInactive();
    void OnReset();

    // Options - Link
    void OnLinkStart();

    // Options - Link - Type
    void OnLinkTypeNothing();
    void OnLinkTypeCable();
    void OnLinkTypeWireless();
    void OnLinkTypeGameCube();
    void OnLinkTypeGameboy();

    // Options - Link
    void OnLinkLocal();
    void OnLinkAuto();
    void OnLinkSpeedOn();
    void OnLinkConfigure();

    // Options - Video
    void OnDisplayConfigure();
    void OnFullscreen();
    void OnSetSize1x();
    void OnSetSize2x();
    void OnSetSize3x();
    void OnSetSize4x();
    void OnSetSize5x();
    void OnSetSize6x();
    void OnChangeFilter();
    void OnChangeIFB();
    void OnRetainAspect();
    void OnBilinear();
    void OnKeepOnTop();
    void OnNoStatusMsg();
    void OnTransparent();

    // Options - Audio
    void OnSoundConfigure();
    void OnIncreaseVolume();
    void OnDecreaseVolume();
    void OnToggleSound();
    void OnGBASoundInterpolation();
    void OnGBEnhanceSound();
    void OnGBSurround();
    void OnGBDeclicking();

    // Options - Input
    void OnJoypadConfigure();
    void OnAllowKeyboardBackgroundInput();
    void OnAllowJoystickBackgroundInput();

    // Options - Input - Autofire
    void OnJoypadAutofireA();
    void OnJoypadAutofireB();
    void OnJoypadAutofireL();
    void OnJoypadAutofireR();

    // Options - Input - Autohold
    void OnJoypadAutoholdUp();
    void OnJoypadAutoholdDown();
    void OnJoypadAutoholdLeft();
    void OnJoypadAutoholdRight();
    void OnJoypadAutoholdA();
    void OnJoypadAutoholdB();
    void OnJoypadAutoholdL();
    void OnJoypadAutoholdR();
    void OnJoypadAutoholdSelect();
    void OnJoypadAutoholdStart();

    // Options - Game Boy Advance
    void OnGameBoyAdvanceConfigure();
    void OnRTC();
    void OnBootRomGba();
    void OnAGBPrinter();
    void OnGBALcdFilter();

    // Options - Game Boy
    void OnGameBoyConfigure();
    void OnGBColorOption();
    void OnGBLcdFilter();
    void OnColorizerHack();
    void OnPrinter();
    void OnPrintGather();
    void OnPrintSnap();
    void OnBootRomGB();
    void OnBootRomGBC();

    // Options
    void OnGeneralConfigure();
    void OnSpeedupConfigure();
    void OnEmulatorDirectories();
    void OnCustomize();

    // Options - UI Settings
    void OnStatusBar();
    void OnHideMenuBar();
    void OnSuspendScreenSaver();

    // Tools - Cheats
    void OnCheatsList();
    void OnCheatsSearch();
    void OnCheatsAutoSaveLoad();
    void OnCheatsEnable();

    // Tools - GDB
    void OnDebugGDBBreak();
    void OnDebugGDBPort();
    void OnDebugGDBBreakOnLoad();
    void OnDebugGDBDisconnect();

    // Tools - Viewers
    void OnDisassemble();
    void OnLogging();
    void OnIOViewer();
    void OnMapViewer();
    void OnMemoryViewer();
    void OnOAMViewer();
    void OnPaletteViewer();
    void OnTileViewer();

    // Tools - Video Layers
    void OnVideoLayersReset();
    void OnVideoLayersBG0();
    void OnVideoLayersBG1();
    void OnVideoLayersBG2();
    void OnVideoLayersBG3();
    void OnVideoLayersOBJ();
    void OnVideoLayersWIN0();
    void OnVideoLayersWIN1();
    void OnVideoLayersOBJWIN();

    // Tools - Sound Channels
    void OnSoundChannel1();
    void OnSoundChannel2();
    void OnSoundChannel3();
    void OnSoundChannel4();
    void OnDirectSoundA();
    void OnDirectSoundB();

    // Help
    void OnBugReport();
    void OnFAQ();
    void OnTranslate();
    void OnUpdateEmu();
    void OnFactoryReset();
    void OnAbout();
};

// a helper class to avoid forgetting StopModal()
class ModalPause {
public:
    ModalPause(MainFrame* frame) : frame_(frame) { frame_->StartModal(); }
    ~ModalPause() { frame_->StopModal(); }

private:
    MainFrame* frame_;
};

#endif  // VBAM_WX_DIALOGS_MAIN_FRAME_H_
