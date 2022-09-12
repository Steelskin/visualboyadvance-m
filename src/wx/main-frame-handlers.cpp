#include "wx/main-frame.h"

#include <wx/generic/numdlgg.h>
#include <wx/progdlg.h>

#include "wx/config/handler-id.h"
#include "wx/config/option-proxy.h"
#include "wx/opts.h"
#include "wx/widgets/shortcut-menu-item.h"
#include "wx/wxvbam.h"

// static
const MainFrame::HandlerMap& MainFrame::GetHandlerMap() {
    static const HandlerMap kHandlerMap = []() {
        // clang-format off
        const HandlerMap handler_map = {
            // File
            /*OpenGba*/ &MainFrame::OnOpenGba,
            /*OpenGb*/ &MainFrame::OnOpenGB,
            /*OpenGbc*/ &MainFrame::OnOpenGBC,

            // File - Recent
            /*RecentReset*/ &MainFrame::OnRecentReset,
            /*RecentFreeze*/ &MainFrame::OnRecentFreeze,
            /*RecentFile1*/ &MainFrame::OnRecentFile1,
            /*RecentFile2*/ &MainFrame::OnRecentFile2,
            /*RecentFile3*/ &MainFrame::OnRecentFile3,
            /*RecentFile4*/ &MainFrame::OnRecentFile4,
            /*RecentFile5*/ &MainFrame::OnRecentFile5,
            /*RecentFile6*/ &MainFrame::OnRecentFile6,
            /*RecentFile7*/ &MainFrame::OnRecentFile7,
            /*RecentFile8*/ &MainFrame::OnRecentFile8,
            /*RecentFile9*/ &MainFrame::OnRecentFile9,
            /*RecentFile10*/ &MainFrame::OnRecentFile10,

            // File
            /*RomInformation*/ &MainFrame::OnRomInformation,

            // File - e-Reader
            /*ResetLoadingDotCodeFile*/ &MainFrame::OnResetLoadingDotCodeFile,
            /*SetLoadingDotCodeFile*/ &MainFrame::OnSetLoadingDotCodeFile,
            /*ResetSavingDotCodeFile*/ &MainFrame::OnResetSavingDotCodeFile,
            /*SetSavingDotCodeFile*/ &MainFrame::OnSetSavingDotCodeFile,

            // File - Load State
            /*LoadStateRecent*/ &MainFrame::OnLoadStateRecent,
            /*LoadStateSlot*/ &MainFrame::OnLoadStateSlot,
            /*LoadStateAutoLoad*/ &MainFrame::OnLoadStateAutoLoad,
            /*LoadState01*/ &MainFrame::OnLoadState01,
            /*LoadState02*/ &MainFrame::OnLoadState02,
            /*LoadState03*/ &MainFrame::OnLoadState03,
            /*LoadState04*/ &MainFrame::OnLoadState04,
            /*LoadState05*/ &MainFrame::OnLoadState05,
            /*LoadState06*/ &MainFrame::OnLoadState06,
            /*LoadState07*/ &MainFrame::OnLoadState07,
            /*LoadState08*/ &MainFrame::OnLoadState08,
            /*LoadState09*/ &MainFrame::OnLoadState09,
            /*LoadState10*/ &MainFrame::OnLoadState10,
            /*LoadStateFile*/ &MainFrame::OnLoadStateFile,
            /*LoadStateKeepSaves*/ &MainFrame::OnLoadStateKeepSaves,
            /*LoadStateKeepCheats*/ &MainFrame::OnLoadStateKeepCheats,

            // File - Save State
            /*SaveStateOldest*/ &MainFrame::OnSaveStateOldest,
            /*SaveStateSlot*/ &MainFrame::OnSaveStateSlot,
            /*SaveStateIncreaseSlot*/ &MainFrame::OnSaveStateIncreaseSlot,
            /*SaveState01*/ &MainFrame::OnSaveState01,
            /*SaveState02*/ &MainFrame::OnSaveState02,
            /*SaveState03*/ &MainFrame::OnSaveState03,
            /*SaveState04*/ &MainFrame::OnSaveState04,
            /*SaveState05*/ &MainFrame::OnSaveState05,
            /*SaveState06*/ &MainFrame::OnSaveState06,
            /*SaveState07*/ &MainFrame::OnSaveState07,
            /*SaveState08*/ &MainFrame::OnSaveState08,
            /*SaveState09*/ &MainFrame::OnSaveState09,
            /*SaveState10*/ &MainFrame::OnSaveState10,
            /*SaveStateFile*/ &MainFrame::OnSaveStateFile,

            // File
            /*StateIncreaseSlot*/ &MainFrame::OnStateIncreaseSlot,
            /*StateDecreaseSlot*/ &MainFrame::OnStateDecreaseSlot,

            // File - Import
            /*ImportBatteryFile*/ &MainFrame::OnImportBatteryFile,
            /*ImportGamesharkCodeFile*/ &MainFrame::OnImportGamesharkCodeFile,
            /*ImportGamesharkActionReplaySnapshot*/ &MainFrame::OnImportGamesharkActionReplaySnapshot,

            // File - Export
            /*ExportBatteryFile*/ &MainFrame::OnExportBatteryFile,
            /*ExportGamesharkSnapshot*/ &MainFrame::OnExportGamesharkSnapshot,

            // File
            /*ScreenCapture*/ &MainFrame::OnScreenCapture,

            // File - Record
            /*RecordSoundStartRecording*/ &MainFrame::OnRecordSoundStartRecording,
            /*RecordSoundStopRecording*/ &MainFrame::OnRecordSoundStopRecording,
            /*RecordAVIStartRecording*/ &MainFrame::OnRecordAVIStartRecording,
            /*RecordAVIStopRecording*/ &MainFrame::OnRecordAVIStopRecording,
            /*RecordMovieStartRecording*/ &MainFrame::OnRecordMovieStartRecording,
            /*RecordMovieStopRecording*/ &MainFrame::OnRecordMovieStopRecording,

            // File - Playback
            /*PlayMovieStartPlaying*/ &MainFrame::OnPlayMovieStartPlaying,
            /*PlayMovieStopPlaying*/ &MainFrame::OnPlayMovieStopPlaying,

            // File
            /*Close*/ &MainFrame::OnClose,
            /*Exit*/ &MainFrame::OnExit,

            // Emulation
            /*Pause*/ &MainFrame::OnPause,
            /*NextFrame*/ &MainFrame::OnNextFrame,
            /*Rewind*/ &MainFrame::OnRewind,
            /*ToggleFullscreen*/ &MainFrame::OnToggleFullscreen,
            /*EmulatorSpeedupToggle*/ &MainFrame::OnEmulatorSpeedupToggle,
            /*VSync*/ &MainFrame::OnVSync,
            /*FrameSkipAuto*/ &MainFrame::OnFrameSkipAuto,
            /*SkipIntro*/ &MainFrame::OnSkipIntro,
            /*ApplyPatches*/ &MainFrame::OnApplyPatches,
            /*PauseWhenInactive*/ &MainFrame::OnPauseWhenInactive,
            /*Reset*/ &MainFrame::OnReset,

            // Options - Link
            /*LinkStart*/ &MainFrame::OnLinkStart,

            // Options - Link - Type
            /*LinkNothing*/ &MainFrame::OnLinkTypeNothing,
            /*LinkCable*/ &MainFrame::OnLinkTypeCable,
            /*LinkWireless*/ &MainFrame::OnLinkTypeWireless,
            /*LinkGameCube*/ &MainFrame::OnLinkTypeGameCube,
            /*LinkGameboy*/ &MainFrame::OnLinkTypeGameboy,

            // Options - Link
            /*LinkLocal*/ &MainFrame::OnLinkLocal,
            /*LinkAuto*/ &MainFrame::OnLinkAuto,
            /*LinkSpeedOn*/ &MainFrame::OnLinkSpeedOn,
            /*LinkConfigure*/ &MainFrame::OnLinkConfigure,

            // Options - Video
            /*DisplayConfigure*/ &MainFrame::OnDisplayConfigure,
            /*Fullscreen*/ &MainFrame::OnFullscreen,
            /*SetSize1x*/ &MainFrame::OnSetSize1x,
            /*SetSize2x*/ &MainFrame::OnSetSize2x,
            /*SetSize3x*/ &MainFrame::OnSetSize3x,
            /*SetSize4x*/ &MainFrame::OnSetSize4x,
            /*SetSize5x*/ &MainFrame::OnSetSize5x,
            /*SetSize6x*/ &MainFrame::OnSetSize6x,
            /*ChangeFilter*/ &MainFrame::OnChangeFilter,
            /*ChangeIFB*/ &MainFrame::OnChangeIFB,
            /*RetainAspect*/ &MainFrame::OnRetainAspect,
            /*Bilinear*/ &MainFrame::OnBilinear,
            /*KeepOnTop*/ &MainFrame::OnKeepOnTop,
            /*NoStatusMsg*/ &MainFrame::OnNoStatusMsg,
            /*Transparent*/ &MainFrame::OnTransparent,

            // Options - Audio
            /*SoundConfigure*/ &MainFrame::OnSoundConfigure,
            /*IncreaseVolume*/ &MainFrame::OnIncreaseVolume,
            /*DecreaseVolume*/ &MainFrame::OnDecreaseVolume,
            /*ToggleSound*/ &MainFrame::OnToggleSound,
            /*GBASoundInterpolation*/ &MainFrame::OnGBASoundInterpolation,
            /*GBEnhanceSound*/ &MainFrame::OnGBEnhanceSound,
            /*GBSurround*/ &MainFrame::OnGBSurround,
            /*GBDeclicking*/ &MainFrame::OnGBDeclicking,

            // Options - Input
            /*JoypadConfigure*/ &MainFrame::OnJoypadConfigure,
            /*AllowKeyboardBackgroundInput*/ &MainFrame::OnAllowKeyboardBackgroundInput,
            /*AllowJoystickBackgroundInput*/ &MainFrame::OnAllowJoystickBackgroundInput,

            // Options - Input - Autofire
            /*JoypadAutofireA*/ &MainFrame::OnJoypadAutofireA,
            /*JoypadAutofireB*/ &MainFrame::OnJoypadAutofireB,
            /*JoypadAutofireL*/ &MainFrame::OnJoypadAutofireL,
            /*JoypadAutofireR*/ &MainFrame::OnJoypadAutofireR,

            // Options - Input - Autohold
            /*JoypadAutoholdUp*/ &MainFrame::OnJoypadAutoholdUp,
            /*JoypadAutoholdDown*/ &MainFrame::OnJoypadAutoholdDown,
            /*JoypadAutoholdLeft*/ &MainFrame::OnJoypadAutoholdLeft,
            /*JoypadAutoholdRight*/ &MainFrame::OnJoypadAutoholdRight,
            /*JoypadAutoholdA*/ &MainFrame::OnJoypadAutoholdA,
            /*JoypadAutoholdB*/ &MainFrame::OnJoypadAutoholdB,
            /*JoypadAutoholdL*/ &MainFrame::OnJoypadAutoholdL,
            /*JoypadAutoholdR*/ &MainFrame::OnJoypadAutoholdR,
            /*JoypadAutoholdSelect*/ &MainFrame::OnJoypadAutoholdSelect,
            /*JoypadAutoholdStart*/ &MainFrame::OnJoypadAutoholdStart,

            // Options - Game Boy Advance
            /*GameBoyAdvanceConfigure*/ &MainFrame::OnGameBoyAdvanceConfigure,
            /*RTC*/ &MainFrame::OnRTC,
            /*BootRomGba*/ &MainFrame::OnBootRomGba,
            /*AGBPrinter*/ &MainFrame::OnAGBPrinter,
            /*GBALcdFilter*/ &MainFrame::OnGBALcdFilter,

            // Options - Game Boy
            /*GameBoyConfigure*/ &MainFrame::OnGameBoyConfigure,
            /*GBColorOption*/ &MainFrame::OnGBColorOption,
            /*GBLcdFilter*/ &MainFrame::OnGBLcdFilter,
            /*ColorizerHack*/ &MainFrame::OnColorizerHack,
            /*Printer*/ &MainFrame::OnPrinter,
            /*PrintGather*/ &MainFrame::OnPrintGather,
            /*PrintSnap*/ &MainFrame::OnPrintSnap,
            /*BootRomGB*/ &MainFrame::OnBootRomGB,
            /*BootRomGBC*/ &MainFrame::OnBootRomGBC,

            // Options
            /*GeneralConfigure*/ &MainFrame::OnGeneralConfigure,
            /*SpeedupConfigure*/ &MainFrame::OnSpeedupConfigure,
            /*EmulatorDirectories*/ &MainFrame::OnEmulatorDirectories,
            /*Customize*/ &MainFrame::OnCustomize,

            // Options - UI Settings
            /*StatusBar*/ &MainFrame::OnStatusBar,
            /*HideMenuBar*/ &MainFrame::OnHideMenuBar,
            /*SuspendScreenSaver*/ &MainFrame::OnSuspendScreenSaver,

            // Tools - Cheats
            /*CheatsList*/ &MainFrame::OnCheatsList,
            /*CheatsSearch*/ &MainFrame::OnCheatsSearch,
            /*CheatsAutoSaveLoad*/ &MainFrame::OnCheatsAutoSaveLoad,
            /*CheatsEnable*/ &MainFrame::OnCheatsEnable,

            // Tools - GDB
            /*DebugGDBBreak*/ &MainFrame::OnDebugGDBBreak,
            /*DebugGDBPort*/ &MainFrame::OnDebugGDBPort,
            /*DebugGDBBreakOnLoad*/ &MainFrame::OnDebugGDBBreakOnLoad,
            /*DebugGDBDisconnect*/ &MainFrame::OnDebugGDBDisconnect,

            // Tools - Viewers
            /*Disassemble*/ &MainFrame::OnDisassemble,
            /*Logging*/ &MainFrame::OnLogging,
            /*IOViewer*/ &MainFrame::OnIOViewer,
            /*MapViewer*/ &MainFrame::OnMapViewer,
            /*MemoryViewer*/ &MainFrame::OnMemoryViewer,
            /*OAMViewer*/ &MainFrame::OnOAMViewer,
            /*PaletteViewer*/ &MainFrame::OnPaletteViewer,
            /*TileViewer*/ &MainFrame::OnTileViewer,

            // Tools - Video Layers
            /*VideoLayersReset*/ &MainFrame::OnVideoLayersReset,
            /*VideoLayersBG0*/ &MainFrame::OnVideoLayersBG0,
            /*VideoLayersBG1*/ &MainFrame::OnVideoLayersBG1,
            /*VideoLayersBG2*/ &MainFrame::OnVideoLayersBG2,
            /*VideoLayersBG3*/ &MainFrame::OnVideoLayersBG3,
            /*VideoLayersOBJ*/ &MainFrame::OnVideoLayersOBJ,
            /*VideoLayersWIN0*/ &MainFrame::OnVideoLayersWIN0,
            /*VideoLayersWIN1*/ &MainFrame::OnVideoLayersWIN1,
            /*VideoLayersOBJWIN*/ &MainFrame::OnVideoLayersOBJWIN,

            // Tools - Sound Channels
            /*SoundChannel1*/ &MainFrame::OnSoundChannel1,
            /*SoundChannel2*/ &MainFrame::OnSoundChannel2,
            /*SoundChannel3*/ &MainFrame::OnSoundChannel3,
            /*SoundChannel4*/ &MainFrame::OnSoundChannel4,
            /*DirectSoundA*/ &MainFrame::OnDirectSoundA,
            /*DirectSoundB*/ &MainFrame::OnDirectSoundB,

            // Help
            /*BugReport*/ &MainFrame::OnBugReport,
            /*FAQ*/ &MainFrame::OnFAQ,
            /*Translate*/ &MainFrame::OnTranslate,
            /*UpdateEmu*/ &MainFrame::OnUpdateEmu,
            /*FactoryReset*/ &MainFrame::OnFactoryReset,
            /*About*/ &MainFrame::OnAbout,

            // Noop
            /*Noop*/ nullptr,
        };
        // clang-format on

        // If this fails, you forgot to add a handler to the map.
        assert(kHandlerMap[static_cast<size_t>(config::HandlerID::Noop)] == nullptr);

        return handler_map;
    }();

    return kHandlerMap;
}

// static
MainFrame::Handler MainFrame::GetHandler(config::HandlerID id) {
    const size_t index = static_cast<size_t>(id);
    assert(index < config::kNbHandlers);
    const auto& handler_map = GetHandlerMap();
    return handler_map.at(index);
}

void MainFrame::ExecuteHandler(config::HandlerID id) {
    const auto handler = GetHandler(id);
    handler(this);
}

#ifndef NO_LINK

void MainFrame::EnableNetworkMenu() {
    // Only set Link if the GBA Link type is not "nothing" and the local
    // prototype Link option is not enabled.
    cmd_enable_ &= ~widgets::CMDEN_LINK;

    if (gopts.gba_link_type != 0)
        cmd_enable_ |= widgets::CMDEN_LINK;

    if (OPTION(kGBALinkProto))
        cmd_enable_ &= ~widgets::CMDEN_LINK;

    enable_menus();
}

#endif  // NO_LINK
