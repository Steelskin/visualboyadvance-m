#ifndef VBAM_WX_CONFIG_HANDLER_ID_H_
#define VBAM_WX_CONFIG_HANDLER_ID_H_

#include <optional.hpp>

#include <wx/string.h>

// wxWidgets only goes up to `wxID_FILE9` but we want 10 recent files.
static constexpr int wxID_FILE10 = wxID_FILE9 + 1;

namespace config {

// Unique identifier for each command handler. For the sake of sanity, keep
// these in the same order as their position in the menu.
enum class HandlerID : size_t {
    // File
    OpenGba,
    OpenGb,
    OpenGbc,

    // File - Recent
    RecentReset,
    RecentFreeze,
    RecentFile1,
    RecentFile2,
    RecentFile3,
    RecentFile4,
    RecentFile5,
    RecentFile6,
    RecentFile7,
    RecentFile8,
    RecentFile9,
    RecentFile10,

    // File
    RomInformation,

    // File - e-Reader
    ResetLoadingDotCodeFile,
    SetLoadingDotCodeFile,
    ResetSavingDotCodeFile,
    SetSavingDotCodeFile,

    // File - Load State
    LoadStateRecent,
    LoadStateSlot,
    LoadStateAutoLoad,
    LoadState01,
    LoadState02,
    LoadState03,
    LoadState04,
    LoadState05,
    LoadState06,
    LoadState07,
    LoadState08,
    LoadState09,
    LoadState10,
    LoadStateFile,
    LoadStateKeepSaves,
    LoadStateKeepCheats,

    // File - Save State
    SaveStateOldest,
    SaveStateSlot,
    SaveStateIncreaseSlot,
    SaveState01,
    SaveState02,
    SaveState03,
    SaveState04,
    SaveState05,
    SaveState06,
    SaveState07,
    SaveState08,
    SaveState09,
    SaveState10,
    SaveStateFile,

    // File
    StateIncreaseSlot,
    StateDecreaseSlot,

    // File - Import
    ImportBatteryFile,
    ImportGamesharkCodeFile,
    ImportGamesharkActionReplaySnapshot,

    // File - Export
    ExportBatteryFile,
    ExportGamesharkSnapshot,

    // File
    ScreenCapture,

    // File - Record
    RecordSoundStartRecording,
    RecordSoundStopRecording,
    RecordAVIStartRecording,
    RecordAVIStopRecording,
    RecordMovieStartRecording,
    RecordMovieStopRecording,

    // File - Playback
    PlayMovieStartPlaying,
    PlayMovieStopPlaying,

    // File
    Close,
    Exit,

    // Emulation
    Pause,
    NextFrame,
    Rewind,
    ToggleFullscreen,
    EmulatorSpeedupToggle,
    VSync,
    FrameSkipAuto,
    SkipIntro,
    ApplyPatches,
    PauseWhenInactive,
    Reset,

    // Options - Link
    LinkStart,

    // Options - Link - Type
    LinkTypeNothing,
    LinkTypeCable,
    LinkTypeWireless,
    LinkTypeGameCube,
    LinkTypeGameboy,

    // Options - Link
    LinkLocal,
    LinkAuto,
    LinkSpeedOn,
    LinkConfigure,

    // Options - Video
    DisplayConfigure,
    Fullscreen,
    SetSize1x,
    SetSize2x,
    SetSize3x,
    SetSize4x,
    SetSize5x,
    SetSize6x,
    ChangeFilter,
    ChangeIFB,
    RetainAspect,
    Bilinear,
    KeepOnTop,
    NoStatusMsg,
    Transparent,

    // Options - Audio
    SoundConfigure,
    IncreaseVolume,
    DecreaseVolume,
    ToggleSound,
    GBASoundInterpolation,
    GBEnhanceSound,
    GBSurround,
    GBDeclicking,

    // Options - Input
    JoypadConfigure,
    AllowKeyboardBackgroundInput,
    AllowJoystickBackgroundInput,

    // Options - Input - Autofire
    JoypadAutofireA,
    JoypadAutofireB,
    JoypadAutofireL,
    JoypadAutofireR,

    // Options - Input - Autohold
    JoypadAutoholdUp,
    JoypadAutoholdDown,
    JoypadAutoholdLeft,
    JoypadAutoholdRight,
    JoypadAutoholdA,
    JoypadAutoholdB,
    JoypadAutoholdL,
    JoypadAutoholdR,
    JoypadAutoholdSelect,
    JoypadAutoholdStart,

    // Options - Game Boy Advance
    GameBoyAdvanceConfigure,
    RTC,
    BootRomGba,
    AGBPrinter,
    GBALcdFilter,

    // Options - Game Boy
    GameBoyConfigure,
    GBColorOption,
    GBLcdFilter,
    ColorizerHack,
    Printer,
    PrintGather,
    PrintSnap,
    BootRomGB,
    BootRomGBC,

    // Options
    GeneralConfigure,
    SpeedupConfigure,
    EmulatorDirectories,
    Customize,

    // Options - UI Settings
    StatusBar,
    HideMenuBar,
    SuspendScreenSaver,

    // Tools - Cheats
    CheatsList,
    CheatsSearch,
    CheatsAutoSaveLoad,
    CheatsEnable,

    // Tools - GDB
    DebugGDBBreak,
    DebugGDBPort,
    DebugGDBBreakOnLoad,
    DebugGDBDisconnect,

    // Tools - Viewers
    Disassemble,
    Logging,
    IOViewer,
    MapViewer,
    MemoryViewer,
    OAMViewer,
    PaletteViewer,
    TileViewer,

    // Tools - Video Layers
    VideoLayersReset,
    VideoLayersBG0,
    VideoLayersBG1,
    VideoLayersBG2,
    VideoLayersBG3,
    VideoLayersOBJ,
    VideoLayersWIN0,
    VideoLayersWIN1,
    VideoLayersOBJWIN,

    // Tools - Sound Channels
    SoundChannel1,
    SoundChannel2,
    SoundChannel3,
    SoundChannel4,
    DirectSoundA,
    DirectSoundB,

    // Help
    BugReport,
    FAQ,
    Translate,
    UpdateEmu,
    FactoryReset,
    About,

    // Special. Add nothing under this ID.
    Noop,
};

static constexpr size_t kNbHandlers = static_cast<size_t>(HandlerID::Noop) + 1;

// Returns the HandlerID for the given zero-based recent file index.
// `index` must be between 0 and 9, will assert otherwise.
HandlerID GetRecentHandlerID(size_t index);

// Returns the HandlerID for the given zero-based load state index.
// `index` must be between 0 and 9, will assert otherwise.
HandlerID GetLoadStateHandlerID(size_t index);

// Returns the HandlerID for the given zero-based save state index.
// `index` must be between 0 and 9, will assert otherwise.
HandlerID GetSaveStateHandlerID(size_t index);

// Returns true if the given HandlerID is a state handler (load or save).
bool HandlerIsState(HandlerID handler_id);

// Returns the corresponding HandlerID for the given XRC ID.
// Returns HandlerID::Noop if the XRC ID is not recognized.
HandlerID XRCIDToHandlerID(int xrc_id);

// Returns the corresponding XRC ID for the given HandlerID.
int HandlerIDToXRCID(HandlerID handler_id);

// Returns the corresponding vbam.ini ID for the given HandlerID.
wxString HandlerIDToConfigString(HandlerID handler_id);

// Returns the corresponding HandlerID for the given vbam.ini ID.
nonstd::optional<HandlerID> ConfigStringToHandlerID(const wxString& config_string);

}  // namespace config

#endif  // VBAM_WX_CONFIG_HANDLER_ID_H_
