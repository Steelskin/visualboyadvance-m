#include "wx/config/handler-id.h"

#include <array>
#include <cassert>
#include <unordered_map>

#include <wx/xrc/xmlres.h>

#define VBAM_HANDLER_INTERNAL_INCLUDE
#include "wx/config/internal/handler-internal.h"
#undef VBAM_HANDLER_INTERNAL_INCLUDE

namespace config {
namespace internal {

const std::array<HandlerData, kNbHandlers>& AllHandlersData() {
    // One-time thread-safe execution.
    static const std::array<HandlerData, kNbHandlers> kAllHandlersData = []() {
        // clang-format off
        const std::array<HandlerData, kNbHandlers> handlers_data = {
            // File
            HandlerData {
                /*handler_id=*/HandlerID::OpenGba,
                /*config_string=*/"OPEN",
                /*xrc_id=*/wxID_OPEN,
            },
            HandlerData {
                /*handler_id=*/HandlerID::OpenGb,
                /*config_string=*/"OpenGB",
                /*xrc_id=*/XRCID("OpenGB"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::OpenGbc,
                /*config_string=*/"OpenGBC",
                /*xrc_id=*/XRCID("OpenGBC"),
            },

            // File - Recent
            HandlerData {
                /*handler_id=*/HandlerID::RecentReset,
                /*config_string=*/"RecentReset",
                /*xrc_id=*/XRCID("RecentReset"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::RecentFreeze,
                /*config_string=*/"RecentFreeze",
                /*xrc_id=*/XRCID("RecentFreeze"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::RecentFile1,
                /*config_string=*/"FILE1",
                /*xrc_id=*/wxID_FILE1,
            },
            HandlerData {
                /*handler_id=*/HandlerID::RecentFile2,
                /*config_string=*/"FILE2",
                /*xrc_id=*/wxID_FILE2,
            },
            HandlerData {
                /*handler_id=*/HandlerID::RecentFile3,
                /*config_string=*/"FILE3",
                /*xrc_id=*/wxID_FILE3,
            },
            HandlerData {
                /*handler_id=*/HandlerID::RecentFile4,
                /*config_string=*/"FILE4",
                /*xrc_id=*/wxID_FILE4,
            },
            HandlerData {
                /*handler_id=*/HandlerID::RecentFile5,
                /*config_string=*/"FILE5",
                /*xrc_id=*/wxID_FILE5,
            },
            HandlerData {
                /*handler_id=*/HandlerID::RecentFile6,
                /*config_string=*/"FILE6",
                /*xrc_id=*/wxID_FILE6,
            },
            HandlerData {
                /*handler_id=*/HandlerID::RecentFile7,
                /*config_string=*/"FILE7",
                /*xrc_id=*/wxID_FILE7,
            },
            HandlerData {
                /*handler_id=*/HandlerID::RecentFile8,
                /*config_string=*/"FILE8",
                /*xrc_id=*/wxID_FILE8,
            },
            HandlerData {
                /*handler_id=*/HandlerID::RecentFile9,
                /*config_string=*/"FILE9",
                /*xrc_id=*/wxID_FILE9,
            },
            HandlerData {
                /*handler_id=*/HandlerID::RecentFile10,
                /*config_string=*/"FILE10",
                /*xrc_id=*/wxID_FILE10,
            },

            // File
            HandlerData {
                /*handler_id=*/HandlerID::RomInformation,
                /*config_string=*/"RomInformation",
                /*xrc_id=*/XRCID("RomInformation"),
            },

            // File - e-Reader
            HandlerData {
                /*handler_id=*/HandlerID::ResetLoadingDotCodeFile,
                /*config_string=*/"ResetLoadingDotCodeFile",
                /*xrc_id=*/XRCID("ResetLoadingDotCodeFile"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SetLoadingDotCodeFile,
                /*config_string=*/"SetLoadingDotCodeFile",
                /*xrc_id=*/XRCID("SetLoadingDotCodeFile"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::ResetSavingDotCodeFile,
                /*config_string=*/"ResetSavingDotCodeFile",
                /*xrc_id=*/XRCID("ResetSavingDotCodeFile"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SetSavingDotCodeFile,
                /*config_string=*/"SetSavingDotCodeFile",
                /*xrc_id=*/XRCID("SetSavingDotCodeFile"),
            },

            // File - Load State
            HandlerData {
                /*handler_id=*/HandlerID::LoadStateRecent,
                /*config_string=*/"LoadGameRecent",
                /*xrc_id=*/XRCID("LoadStateRecent"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LoadStateSlot,
                /*config_string=*/"LoadGameSlot",
                /*xrc_id=*/XRCID("LoadStateSlot"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LoadStateAutoLoad,
                /*config_string=*/"LoadGameAutoLoad",
                /*xrc_id=*/XRCID("LoadStateAutoLoad"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LoadState01,
                /*config_string=*/"LoadGame01",
                /*xrc_id=*/XRCID("LoadState01"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LoadState02,
                /*config_string=*/"LoadGame02",
                /*xrc_id=*/XRCID("LoadState02"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LoadState03,
                /*config_string=*/"LoadGame03",
                /*xrc_id=*/XRCID("LoadState03"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LoadState04,
                /*config_string=*/"LoadGame04",
                /*xrc_id=*/XRCID("LoadState04"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LoadState05,
                /*config_string=*/"LoadGame05",
                /*xrc_id=*/XRCID("LoadState05"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LoadState06,
                /*config_string=*/"LoadGame06",
                /*xrc_id=*/XRCID("LoadState06"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LoadState07,
                /*config_string=*/"LoadGame07",
                /*xrc_id=*/XRCID("LoadState07"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LoadState08,
                /*config_string=*/"LoadGame08",
                /*xrc_id=*/XRCID("LoadState08"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LoadState09,
                /*config_string=*/"LoadGame09",
                /*xrc_id=*/XRCID("LoadState09"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LoadState10,
                /*config_string=*/"LoadGame10",
                /*xrc_id=*/XRCID("LoadState10"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LoadStateFile,
                /*config_string=*/"Load",
                /*xrc_id=*/XRCID("LoadStateFile"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LoadStateKeepSaves,
                /*config_string=*/"KeepSaves",
                /*xrc_id=*/XRCID("LoadStateKeepSaves"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LoadStateKeepCheats,
                /*config_string=*/"KeepCheats",
                /*xrc_id=*/XRCID("LoadStateKeepCheats"),
            },

            // File - Save State
            HandlerData {
                /*handler_id=*/HandlerID::SaveStateOldest,
                /*config_string=*/"SaveGameOldest",
                /*xrc_id=*/XRCID("SaveStateOldest"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SaveStateSlot,
                /*config_string=*/"SaveGameSlot",
                /*xrc_id=*/XRCID("SaveStateSlot"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SaveStateIncreaseSlot,
                /*config_string=*/"IncrGameSlotSave",
                /*xrc_id=*/XRCID("SaveStateIncreaseSlot"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SaveState01,
                /*config_string=*/"SaveGame01",
                /*xrc_id=*/XRCID("SaveState01"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SaveState02,
                /*config_string=*/"SaveGame02",
                /*xrc_id=*/XRCID("SaveState02"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SaveState03,
                /*config_string=*/"SaveGame03",
                /*xrc_id=*/XRCID("SaveState03"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SaveState04,
                /*config_string=*/"SaveGame04",
                /*xrc_id=*/XRCID("SaveState04"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SaveState05,
                /*config_string=*/"SaveGame05",
                /*xrc_id=*/XRCID("SaveState05"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SaveState06,
                /*config_string=*/"SaveGame06",
                /*xrc_id=*/XRCID("SaveState06"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SaveState07,
                /*config_string=*/"SaveGame07",
                /*xrc_id=*/XRCID("SaveState07"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SaveState08,
                /*config_string=*/"SaveGame08",
                /*xrc_id=*/XRCID("SaveState08"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SaveState09,
                /*config_string=*/"SaveGame09",
                /*xrc_id=*/XRCID("SaveState09"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SaveState10,
                /*config_string=*/"SaveGame10",
                /*xrc_id=*/XRCID("SaveState10"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SaveStateFile,
                /*config_string=*/"Save",
                /*xrc_id=*/XRCID("SaveStateFile"),
            },

            // File
            HandlerData {
                /*handler_id=*/HandlerID::StateIncreaseSlot,
                /*config_string=*/"IncrGameSlot",
                /*xrc_id=*/XRCID("StateIncreaseSlot"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::StateDecreaseSlot,
                /*config_string=*/"DecrGameSlot",
                /*xrc_id=*/XRCID("StateDecreaseSlot"),
            },

            // File - Import
            HandlerData {
                /*handler_id=*/HandlerID::ImportBatteryFile,
                /*config_string=*/"ImportBatteryFile",
                /*xrc_id=*/XRCID("ImportBatteryFile"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::ImportGamesharkCodeFile,
                /*config_string=*/"ImportGamesharkCodeFile",
                /*xrc_id=*/XRCID("ImportGamesharkCodeFile"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::ImportGamesharkActionReplaySnapshot,
                /*config_string=*/"ImportGamesharkActionReplaySnapshot",
                /*xrc_id=*/XRCID("ImportGamesharkActionReplaySnapshot"),
            },

            // File - Export
            HandlerData {
                /*handler_id=*/HandlerID::ExportBatteryFile,
                /*config_string=*/"ExportBatteryFile",
                /*xrc_id=*/XRCID("ExportBatteryFile"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::ExportGamesharkSnapshot,
                /*config_string=*/"ExportGamesharkSnapshot",
                /*xrc_id=*/XRCID("ExportGamesharkSnapshot"),
            },

            // File
            HandlerData {
                /*handler_id=*/HandlerID::ScreenCapture,
                /*config_string=*/"ScreenCapture",
                /*xrc_id=*/XRCID("ScreenCapture"),
            },

            // File - Record
            HandlerData {
                /*handler_id=*/HandlerID::RecordSoundStartRecording,
                /*config_string=*/"RecordSoundStartRecording",
                /*xrc_id=*/XRCID("RecordSoundStartRecording"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::RecordSoundStopRecording,
                /*config_string=*/"RecordSoundStopRecording",
                /*xrc_id=*/XRCID("RecordSoundStopRecording"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::RecordAVIStartRecording,
                /*config_string=*/"RecordAVIStartRecording",
                /*xrc_id=*/XRCID("RecordAVIStartRecording"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::RecordAVIStopRecording,
                /*config_string=*/"RecordAVIStopRecording",
                /*xrc_id=*/XRCID("RecordAVIStopRecording"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::RecordMovieStartRecording,
                /*config_string=*/"RecordMovieStartRecording",
                /*xrc_id=*/XRCID("RecordMovieStartRecording"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::RecordMovieStopRecording,
                /*config_string=*/"RecordMovieStopRecording",
                /*xrc_id=*/XRCID("RecordMovieStopRecording"),
            },

            // File -Playback
            HandlerData {
                /*handler_id=*/HandlerID::PlayMovieStartPlaying,
                /*config_string=*/"PlayMovieStartPlaying",
                /*xrc_id=*/XRCID("PlayMovieStartPlaying"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::PlayMovieStopPlaying,
                /*config_string=*/"PlayMovieStopPlaying",
                /*xrc_id=*/XRCID("PlayMovieStopPlaying"),
            },

            // File
            HandlerData {
                /*handler_id=*/HandlerID::Close,
                /*config_string=*/"CLOSE",
                /*xrc_id=*/wxID_CLOSE,
            },
            HandlerData {
                /*handler_id=*/HandlerID::Exit,
                /*config_string=*/"EXIT",
                /*xrc_id=*/wxID_EXIT,
            },

            // Emulation
            HandlerData {
                /*handler_id=*/HandlerID::Pause,
                /*config_string=*/"Pause",
                /*xrc_id=*/XRCID("Pause"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::NextFrame,
                /*config_string=*/"NextFrame",
                /*xrc_id=*/XRCID("NextFrame"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::Rewind,
                /*config_string=*/"Rewind",
                /*xrc_id=*/XRCID("Rewind"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::ToggleFullscreen,
                /*config_string=*/"ToggleFullscreen",
                /*xrc_id=*/XRCID("ToggleFullscreen"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::EmulatorSpeedupToggle,
                /*config_string=*/"EmulatorSpeedupToggle",
                /*xrc_id=*/XRCID("EmulatorSpeedupToggle"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::VSync,
                /*config_string=*/"VSync",
                /*xrc_id=*/XRCID("VSync"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::FrameSkipAuto,
                /*config_string=*/"FrameSkipAuto",
                /*xrc_id=*/XRCID("FrameSkipAuto"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SkipIntro,
                /*config_string=*/"SkipIntro",
                /*xrc_id=*/XRCID("SkipIntro"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::ApplyPatches,
                /*config_string=*/"ApplyPatches",
                /*xrc_id=*/XRCID("ApplyPatches"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::PauseWhenInactive,
                /*config_string=*/"PauseWhenInactive",
                /*xrc_id=*/XRCID("PauseWhenInactive"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::Reset,
                /*config_string=*/"Reset",
                /*xrc_id=*/XRCID("Reset"),
            },

            // Options - Link
            HandlerData {
                /*handler_id=*/HandlerID::LinkStart,
                /*config_string=*/"LinkStart",
                /*xrc_id=*/XRCID("LinkStart"),
            },

            // Options - Link - Type
            HandlerData {
                /*handler_id=*/HandlerID::LinkTypeNothing,
                /*config_string=*/"LinkTypeNothing",
                /*xrc_id=*/XRCID("LinkTypeNothing"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LinkTypeCable,
                /*config_string=*/"LinkTypeCable",
                /*xrc_id=*/XRCID("LinkTypeCable"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LinkTypeWireless,
                /*config_string=*/"LinkTypeWireless",
                /*xrc_id=*/XRCID("LinkTypeWireless"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LinkTypeGameCube,
                /*config_string=*/"LinkTypeGameCube",
                /*xrc_id=*/XRCID("LinkTypeGameCube"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LinkTypeGameboy,
                /*config_string=*/"LinkTypeGameboy",
                /*xrc_id=*/XRCID("LinkTypeGameboy"),
            },

            // Options - Link
            HandlerData {
                /*handler_id=*/HandlerID::LinkLocal,
                /*config_string=*/"LinkLocal",
                /*xrc_id=*/XRCID("LinkLocal"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LinkAuto,
                /*config_string=*/"LinkAuto",
                /*xrc_id=*/XRCID("LinkAuto"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LinkSpeedOn,
                /*config_string=*/"LinkSpeedOn",
                /*xrc_id=*/XRCID("LinkSpeedOn"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::LinkConfigure,
                /*config_string=*/"LinkConfigure",
                /*xrc_id=*/XRCID("LinkConfigure"),
            },

            // Options - Video
            HandlerData {
                /*handler_id=*/HandlerID::DisplayConfigure,
                /*config_string=*/"DisplayConfigure",
                /*xrc_id=*/XRCID("DisplayConfigure"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::Fullscreen,
                /*config_string=*/"Fullscreen",
                /*xrc_id=*/XRCID("Fullscreen"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SetSize1x,
                /*config_string=*/"SetSize1x",
                /*xrc_id=*/XRCID("SetSize1x"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SetSize2x,
                /*config_string=*/"SetSize2x",
                /*xrc_id=*/XRCID("SetSize2x"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SetSize3x,
                /*config_string=*/"SetSize3x",
                /*xrc_id=*/XRCID("SetSize3x"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SetSize4x,
                /*config_string=*/"SetSize4x",
                /*xrc_id=*/XRCID("SetSize4x"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SetSize5x,
                /*config_string=*/"SetSize5x",
                /*xrc_id=*/XRCID("SetSize5x"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SetSize6x,
                /*config_string=*/"SetSize6x",
                /*xrc_id=*/XRCID("SetSize6x"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::ChangeFilter,
                /*config_string=*/"ChangeFilter",
                /*xrc_id=*/XRCID("ChangeFilter"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::ChangeIFB,
                /*config_string=*/"ChangeIFB",
                /*xrc_id=*/XRCID("ChangeIFB"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::RetainAspect,
                /*config_string=*/"RetainAspect",
                /*xrc_id=*/XRCID("RetainAspect"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::Bilinear,
                /*config_string=*/"Bilinear",
                /*xrc_id=*/XRCID("Bilinear"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::KeepOnTop,
                /*config_string=*/"KeepOnTop",
                /*xrc_id=*/XRCID("KeepOnTop"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::NoStatusMsg,
                /*config_string=*/"NoStatusMsg",
                /*xrc_id=*/XRCID("NoStatusMsg"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::Transparent,
                /*config_string=*/"Transparent",
                /*xrc_id=*/XRCID("Transparent"),
            },

            // Options - Audio
            HandlerData {
                /*handler_id=*/HandlerID::SoundConfigure,
                /*config_string=*/"SoundConfigure",
                /*xrc_id=*/XRCID("SoundConfigure"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::IncreaseVolume,
                /*config_string=*/"IncreaseVolume",
                /*xrc_id=*/XRCID("IncreaseVolume"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::DecreaseVolume,
                /*config_string=*/"DecreaseVolume",
                /*xrc_id=*/XRCID("DecreaseVolume"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::ToggleSound,
                /*config_string=*/"ToggleSound",
                /*xrc_id=*/XRCID("ToggleSound"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::GBASoundInterpolation,
                /*config_string=*/"GBASoundInterpolation",
                /*xrc_id=*/XRCID("GBASoundInterpolation"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::GBEnhanceSound,
                /*config_string=*/"GBEnhanceSound",
                /*xrc_id=*/XRCID("GBEnhanceSound"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::GBSurround,
                /*config_string=*/"GBSurround",
                /*xrc_id=*/XRCID("GBSurround"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::GBDeclicking,
                /*config_string=*/"GBDeclicking",
                /*xrc_id=*/XRCID("GBDeclicking"),
            },

            // Options - Input
            HandlerData {
                /*handler_id=*/HandlerID::JoypadConfigure,
                /*config_string=*/"JoypadConfigure",
                /*xrc_id=*/XRCID("JoypadConfigure"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::AllowKeyboardBackgroundInput,
                /*config_string=*/"AllowKeyboardBackgroundInput",
                /*xrc_id=*/XRCID("AllowKeyboardBackgroundInput"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::AllowJoystickBackgroundInput,
                /*config_string=*/"AllowJoystickBackgroundInput",
                /*xrc_id=*/XRCID("AllowJoystickBackgroundInput"),
            },

            // Options - Input - Autofire
            HandlerData {
                /*handler_id=*/HandlerID::JoypadAutofireA,
                /*config_string=*/"JoypadAutofireA",
                /*xrc_id=*/XRCID("JoypadAutofireA"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::JoypadAutofireB,
                /*config_string=*/"JoypadAutofireB",
                /*xrc_id=*/XRCID("JoypadAutofireB"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::JoypadAutofireL,
                /*config_string=*/"JoypadAutofireL",
                /*xrc_id=*/XRCID("JoypadAutofireL"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::JoypadAutofireR,
                /*config_string=*/"JoypadAutofireR",
                /*xrc_id=*/XRCID("JoypadAutofireR"),
            },

            // Options - Input - Autohold
            HandlerData {
                /*handler_id=*/HandlerID::JoypadAutoholdUp,
                /*config_string=*/"JoypadAutoholdUp",
                /*xrc_id=*/XRCID("JoypadAutoholdUp"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::JoypadAutoholdDown,
                /*config_string=*/"JoypadAutoholdDown",
                /*xrc_id=*/XRCID("JoypadAutoholdDown"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::JoypadAutoholdLeft,
                /*config_string=*/"JoypadAutoholdLeft",
                /*xrc_id=*/XRCID("JoypadAutoholdLeft"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::JoypadAutoholdRight,
                /*config_string=*/"JoypadAutoholdRight",
                /*xrc_id=*/XRCID("JoypadAutoholdRight"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::JoypadAutoholdA,
                /*config_string=*/"JoypadAutoholdA",
                /*xrc_id=*/XRCID("JoypadAutoholdA"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::JoypadAutoholdB,
                /*config_string=*/"JoypadAutoholdB",
                /*xrc_id=*/XRCID("JoypadAutoholdB"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::JoypadAutoholdL,
                /*config_string=*/"JoypadAutoholdL",
                /*xrc_id=*/XRCID("JoypadAutoholdL"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::JoypadAutoholdR,
                /*config_string=*/"JoypadAutoholdR",
                /*xrc_id=*/XRCID("JoypadAutoholdR"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::JoypadAutoholdSelect,
                /*config_string=*/"JoypadAutoholdSelect",
                /*xrc_id=*/XRCID("JoypadAutoholdSelect"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::JoypadAutoholdStart,
                /*config_string=*/"JoypadAutoholdStart",
                /*xrc_id=*/XRCID("JoypadAutoholdStart"),
            },

            // Options - Game Boy Advance
            HandlerData {
                /*handler_id=*/HandlerID::GameBoyAdvanceConfigure,
                /*config_string=*/"GameBoyAdvanceConfigure",
                /*xrc_id=*/XRCID("GameBoyAdvanceConfigure"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::RTC,
                /*config_string=*/"RTC",
                /*xrc_id=*/XRCID("RTC"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::BootRomGba,
                /*config_string=*/"BootRomGba",
                /*xrc_id=*/XRCID("BootRomGba"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::AGBPrinter,
                /*config_string=*/"AGBPrinter",
                /*xrc_id=*/XRCID("AGBPrinter"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::GBALcdFilter,
                /*config_string=*/"GBALcdFilter",
                /*xrc_id=*/XRCID("GBALcdFilter"),
            },

            // Options - Game Boy
            HandlerData {
                /*handler_id=*/HandlerID::GameBoyConfigure,
                /*config_string=*/"GameBoyConfigure",
                /*xrc_id=*/XRCID("GameBoyConfigure"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::GBColorOption,
                /*config_string=*/"GBColorOption",
                /*xrc_id=*/XRCID("GBColorOption"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::GBLcdFilter,
                /*config_string=*/"GBLcdFilter",
                /*xrc_id=*/XRCID("GBLcdFilter"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::ColorizerHack,
                /*config_string=*/"ColorizerHack",
                /*xrc_id=*/XRCID("ColorizerHack"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::Printer,
                /*config_string=*/"Printer",
                /*xrc_id=*/XRCID("Printer"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::PrintGather,
                /*config_string=*/"PrintGather",
                /*xrc_id=*/XRCID("PrintGather"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::PrintSnap,
                /*config_string=*/"PrintSnap",
                /*xrc_id=*/XRCID("PrintSnap"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::BootRomGB,
                /*config_string=*/"BootRomGB",
                /*xrc_id=*/XRCID("BootRomGB"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::BootRomGBC,
                /*config_string=*/"BootRomGBC",
                /*xrc_id=*/XRCID("BootRomGBC"),
            },

            // Options
            HandlerData {
                /*handler_id=*/HandlerID::GeneralConfigure,
                /*config_string=*/"GeneralConfigure",
                /*xrc_id=*/XRCID("GeneralConfigure"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SpeedupConfigure,
                /*config_string=*/"SpeedupConfigure",
                /*xrc_id=*/XRCID("SpeedupConfigure"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::EmulatorDirectories,
                /*config_string=*/"EmulatorDirectories",
                /*xrc_id=*/XRCID("EmulatorDirectories"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::Customize,
                /*config_string=*/"Customize",
                /*xrc_id=*/XRCID("Customize"),
            },

            // Options - UI Settings
            HandlerData {
                /*handler_id=*/HandlerID::StatusBar,
                /*config_string=*/"StatusBar",
                /*xrc_id=*/XRCID("StatusBar"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::HideMenuBar,
                /*config_string=*/"HideMenuBar",
                /*xrc_id=*/XRCID("HideMenuBar"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SuspendScreenSaver,
                /*config_string=*/"SuspendScreenSaver",
                /*xrc_id=*/XRCID("SuspendScreenSaver"),
            },

            // Tools - Cheats
            HandlerData {
                /*handler_id=*/HandlerID::CheatsList,
                /*config_string=*/"CheatsList",
                /*xrc_id=*/XRCID("CheatsList"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::CheatsSearch,
                /*config_string=*/"CheatsSearch",
                /*xrc_id=*/XRCID("CheatsSearch"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::CheatsAutoSaveLoad,
                /*config_string=*/"CheatsAutoSaveLoad",
                /*xrc_id=*/XRCID("CheatsAutoSaveLoad"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::CheatsEnable,
                /*config_string=*/"CheatsEnable",
                /*xrc_id=*/XRCID("CheatsEnable"),
            },

            // Tools - GDB
            HandlerData {
                /*handler_id=*/HandlerID::DebugGDBBreak,
                /*config_string=*/"DebugGDBBreak",
                /*xrc_id=*/XRCID("DebugGDBBreak"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::DebugGDBPort,
                /*config_string=*/"DebugGDBPort",
                /*xrc_id=*/XRCID("DebugGDBPort"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::DebugGDBBreakOnLoad,
                /*config_string=*/"DebugGDBBreakOnLoad",
                /*xrc_id=*/XRCID("DebugGDBBreakOnLoad"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::DebugGDBDisconnect,
                /*config_string=*/"DebugGDBDisconnect",
                /*xrc_id=*/XRCID("DebugGDBDisconnect"),
            },

            // Tools - Viewers
            HandlerData {
                /*handler_id=*/HandlerID::Disassemble,
                /*config_string=*/"Disassemble",
                /*xrc_id=*/XRCID("Disassemble"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::Logging,
                /*config_string=*/"Logging",
                /*xrc_id=*/XRCID("Logging"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::IOViewer,
                /*config_string=*/"IOViewer",
                /*xrc_id=*/XRCID("IOViewer"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::MapViewer,
                /*config_string=*/"MapViewer",
                /*xrc_id=*/XRCID("MapViewer"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::MemoryViewer,
                /*config_string=*/"MemoryViewer",
                /*xrc_id=*/XRCID("MemoryViewer"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::OAMViewer,
                /*config_string=*/"OAMViewer",
                /*xrc_id=*/XRCID("OAMViewer"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::PaletteViewer,
                /*config_string=*/"PaletteViewer",
                /*xrc_id=*/XRCID("PaletteViewer"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::TileViewer,
                /*config_string=*/"TileViewer",
                /*xrc_id=*/XRCID("TileViewer"),
            },

            // Tools - Video Layers
            HandlerData {
                /*handler_id=*/HandlerID::VideoLayersReset,
                /*config_string=*/"VideoLayersReset",
                /*xrc_id=*/XRCID("VideoLayersReset"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::VideoLayersBG0,
                /*config_string=*/"VideoLayersBG0",
                /*xrc_id=*/XRCID("VideoLayersBG0"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::VideoLayersBG1,
                /*config_string=*/"VideoLayersBG1",
                /*xrc_id=*/XRCID("VideoLayersBG1"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::VideoLayersBG2,
                /*config_string=*/"VideoLayersBG2",
                /*xrc_id=*/XRCID("VideoLayersBG2"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::VideoLayersBG3,
                /*config_string=*/"VideoLayersBG3",
                /*xrc_id=*/XRCID("VideoLayersBG3"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::VideoLayersOBJ,
                /*config_string=*/"VideoLayersOBJ",
                /*xrc_id=*/XRCID("VideoLayersOBJ"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::VideoLayersWIN0,
                /*config_string=*/"VideoLayersWIN0",
                /*xrc_id=*/XRCID("VideoLayersWIN0"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::VideoLayersWIN1,
                /*config_string=*/"VideoLayersWIN1",
                /*xrc_id=*/XRCID("VideoLayersWIN1"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::VideoLayersOBJWIN,
                /*config_string=*/"VideoLayersOBJWIN",
                /*xrc_id=*/XRCID("VideoLayersOBJWIN"),
            },

            // Tools - Sound Channels
            HandlerData {
                /*handler_id=*/HandlerID::SoundChannel1,
                /*config_string=*/"SoundChannel1",
                /*xrc_id=*/XRCID("SoundChannel1"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SoundChannel2,
                /*config_string=*/"SoundChannel2",
                /*xrc_id=*/XRCID("SoundChannel2"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SoundChannel3,
                /*config_string=*/"SoundChannel3",
                /*xrc_id=*/XRCID("SoundChannel3"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::SoundChannel4,
                /*config_string=*/"SoundChannel4",
                /*xrc_id=*/XRCID("SoundChannel4"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::DirectSoundA,
                /*config_string=*/"DirectSoundA",
                /*xrc_id=*/XRCID("DirectSoundA"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::DirectSoundB,
                /*config_string=*/"DirectSoundB",
                /*xrc_id=*/XRCID("DirectSoundB"),
            },

            // Help
            HandlerData {
                /*handler_id=*/HandlerID::BugReport,
                /*config_string=*/"BugReport",
                /*xrc_id=*/XRCID("BugReport"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::FAQ,
                /*config_string=*/"FAQ",
                /*xrc_id=*/XRCID("FAQ"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::Translate,
                /*config_string=*/"Translate",
                /*xrc_id=*/XRCID("Translate"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::UpdateEmu,
                /*config_string=*/"UpdateEmu",
                /*xrc_id=*/XRCID("UpdateEmu"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::FactoryReset,
                /*config_string=*/"FactoryReset",
                /*xrc_id=*/XRCID("FactoryReset"),
            },
            HandlerData {
                /*handler_id=*/HandlerID::About,
                /*config_string=*/"ABOUT",
                /*xrc_id=*/wxID_ABOUT,
            },

            // Noop
            HandlerData {
                /*handler_id=*/HandlerID::Noop,
                /*config_string=*/"NOOP",
                /*xrc_id=*/XRCID("NOOP"),
            },
        };
        // clang-format on

        // If this fails, you forgot to add data to the array.
        assert(handlers_data[static_cast<size_t>(HandlerID::Noop)].handler_id == HandlerID::Noop);

        return handlers_data;
    }();


    return kAllHandlersData;
}

const std::unordered_map<int, HandlerID>& XRCIDToHandlerMap() {
    // One-time thread-safe execution.
    static const std::unordered_map<int, HandlerID> kCommandToHandler = [] {
        std::unordered_map<int, HandlerID> command_to_handler;
        for (const auto& handler_data : AllHandlersData()) {
            command_to_handler[handler_data.xrc_id] = handler_data.handler_id;
        }
        return command_to_handler;
    }();

    return kCommandToHandler;
}

const std::map<wxString, HandlerID>& ConfigStringToHandlerMap() {
    // One-time thread-safe execution.
    static const std::map<wxString, HandlerID> kConfigStringToHandler = [] {
        std::map<wxString, HandlerID> config_string_to_handler;
        for (const auto& handler_data : AllHandlersData()) {
            config_string_to_handler[handler_data.config_string] = handler_data.handler_id;
        }
        return config_string_to_handler;
    }();

    return kConfigStringToHandler;
}

}  // namespace internal
}  // namespace config