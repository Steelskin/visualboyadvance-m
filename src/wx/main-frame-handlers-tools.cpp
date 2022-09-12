#include "wx/main-frame.h"

#include <wx/generic/numdlgg.h>
#include <wx/menuitem.h>
#include <wx/xrc/xmlres.h>

#include "core/gba/gbaGlobals.h"
#include "core/gba/gbaSound.h"
#include "wx/dialogs/log.h"
#include "wx/opts.h"
#include "wx/widgets/shortcut-menu-item.h"

// Defined in gbRemote.cpp. TODO: Export this symbol from the core.
extern int remotePort;

namespace {

// FIXME - Remove this.
void toggleBitVar(bool* menuValue, int* globalVar, int mask) {
    bool isEnabled = ((*globalVar) & (mask)) != (mask);
    if (*menuValue == isEnabled)
        *globalVar = ((*globalVar) & ~(mask)) | (!isEnabled ? (mask) : 0);
    else
        *globalVar = ((*globalVar) & ~(mask)) | (*menuValue ? (mask) : 0);
    *menuValue = ((*globalVar) & (mask)) != (mask);
}

}  // namespace

// Tools - Cheats

void MainFrame::OnCheatsList() {
    wxDialog* dlg = GetXRCDialog("CheatList");
    ShowModal(dlg);
}

void MainFrame::OnCheatsSearch() {
    wxDialog* dlg = GetXRCDialog("CheatCreate");
    ShowModal(dlg);
}

void MainFrame::OnCheatsAutoSaveLoad() {
    GetMenuOptionConfig("CheatsAutoSaveLoad", config::OptionID::kPrefAutoSaveLoadCheatList);
}

void MainFrame::OnCheatsEnable() {
    bool menuPress = false;
    GetMenuOptionBool("CheatsEnable", &menuPress);
    toggleBitVar(&menuPress, &coreOptions.cheatsEnabled, 1);
    SetMenuOption("CheatsEnable", menuPress ? 1 : 0);
    GetMenuOptionInt("CheatsEnable", &coreOptions.cheatsEnabled, 1);
    update_opts();
}

// Tools - GDB

void MainFrame::OnDebugGDBBreak() {
#if defined(VBAM_ENABLE_DEBUGGER)
    GDBBreak();
#endif  // defined(VBAM_ENABLE_DEBUGGER)
}

void MainFrame::OnDebugGDBPort() {
#if defined(VBAM_ENABLE_DEBUGGER)
    int port_selected = GetGDBPort();

    if (port_selected != -1) {
        gopts.gdb_port = port_selected;
        update_opts();
    }
#endif  // defined(VBAM_ENABLE_DEBUGGER)
}

void MainFrame::OnDebugGDBBreakOnLoad() {
#if defined(VBAM_ENABLE_DEBUGGER)
    GetMenuOptionConfig("DebugGDBBreakOnLoad", config::OptionID::kPrefGDBBreakOnLoad);
#endif  // defined(VBAM_ENABLE_DEBUGGER)
}

void MainFrame::OnDebugGDBDisconnect() {
#if defined(VBAM_ENABLE_DEBUGGER)
    debugger = false;
    dbgMain = NULL;
    dbgSignal = NULL;
    dbgOutput = NULL;
    remotePort = 0;
    remoteCleanUp();
    cmd_enable_ &= ~widgets::CMDEN_GDB;
    enable_menus();
#endif  // defined(VBAM_ENABLE_DEBUGGER)
}

// Tools - Viewers

void MainFrame::OnDisassemble() {
    Disassemble();
}

void MainFrame::OnLogging() {
    logdlg->SetWindowStyle(wxCAPTION | wxRESIZE_BORDER);
    logdlg->Show();
    logdlg->Raise();
}

void MainFrame::OnIOViewer() {
    IOViewer();
}

void MainFrame::OnMapViewer() {
    MapViewer();
}

void MainFrame::OnMemoryViewer() {
    MemViewer();
}

void MainFrame::OnOAMViewer() {
    OAMViewer();
}

void MainFrame::OnPaletteViewer() {
    PaletteViewer();
}

void MainFrame::OnTileViewer() {
    TileViewer();
}

// Tools - Video Layers

void MainFrame::OnVideoLayersReset() {
#define set_vl(s)                                \
    do {                                         \
        const int id = XRCID(s);                 \
        const auto iter = checkable_mi.find(id); \
        iter->second.mi->Check(true);            \
    } while (0)
    coreOptions.layerSettings = 0x7f00;
    coreOptions.layerEnable = DISPCNT & coreOptions.layerSettings;
    set_vl("VideoLayersBG0");
    set_vl("VideoLayersBG1");
    set_vl("VideoLayersBG2");
    set_vl("VideoLayersBG3");
    set_vl("VideoLayersOBJ");
    set_vl("VideoLayersWIN0");
    set_vl("VideoLayersWIN1");
    set_vl("VideoLayersOBJWIN");
    CPUUpdateRenderBuffers(false);
}

void MainFrame::OnVideoLayersBG0() {
    bool menuPress = false;
    char keyName[] = "VideoLayersBG0";
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &coreOptions.layerSettings, (1 << 8));
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &coreOptions.layerSettings, (1 << 8));
    coreOptions.layerEnable = DISPCNT & coreOptions.layerSettings;
    CPUUpdateRenderBuffers(false);
}

void MainFrame::OnVideoLayersBG1() {
    bool menuPress = false;
    char keyName[] = "VideoLayersBG1";
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &coreOptions.layerSettings, (1 << 9));
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &coreOptions.layerSettings, (1 << 9));
    coreOptions.layerEnable = DISPCNT & coreOptions.layerSettings;
    CPUUpdateRenderBuffers(false);
}

void MainFrame::OnVideoLayersBG2() {
    bool menuPress = false;
    char keyName[] = "VideoLayersBG2";
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &coreOptions.layerSettings, (1 << 10));
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &coreOptions.layerSettings, (1 << 10));
    coreOptions.layerEnable = DISPCNT & coreOptions.layerSettings;
    CPUUpdateRenderBuffers(false);
}

void MainFrame::OnVideoLayersBG3() {
    bool menuPress = false;
    char keyName[] = "VideoLayersBG3";
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &coreOptions.layerSettings, (1 << 11));
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &coreOptions.layerSettings, (1 << 11));
    coreOptions.layerEnable = DISPCNT & coreOptions.layerSettings;
    CPUUpdateRenderBuffers(false);
}

void MainFrame::OnVideoLayersOBJ() {
    bool menuPress = false;
    char keyName[] = "VideoLayersOBJ";
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &coreOptions.layerSettings, (1 << 12));
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &coreOptions.layerSettings, (1 << 12));
    coreOptions.layerEnable = DISPCNT & coreOptions.layerSettings;
    CPUUpdateRenderBuffers(false);
}

void MainFrame::OnVideoLayersWIN0() {
    bool menuPress = false;
    char keyName[] = "VideoLayersWIN0";
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &coreOptions.layerSettings, (1 << 13));
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &coreOptions.layerSettings, (1 << 13));
    coreOptions.layerEnable = DISPCNT & coreOptions.layerSettings;
    CPUUpdateRenderBuffers(false);
}

void MainFrame::OnVideoLayersWIN1() {
    bool menuPress = false;
    char keyName[] = "VideoLayersWIN1";
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &coreOptions.layerSettings, (1 << 14));
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &coreOptions.layerSettings, (1 << 14));
    coreOptions.layerEnable = DISPCNT & coreOptions.layerSettings;
    CPUUpdateRenderBuffers(false);
}

void MainFrame::OnVideoLayersOBJWIN() {
    bool menuPress = false;
    char keyName[] = "VideoLayersOBJWIN";
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &coreOptions.layerSettings, (1 << 15));
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &coreOptions.layerSettings, (1 << 15));
    coreOptions.layerEnable = DISPCNT & coreOptions.layerSettings;
    CPUUpdateRenderBuffers(false);
}

// Tools - Sound Channels

void MainFrame::OnSoundChannel1() {
    bool menuPress = false;
    char keyName[] = "SoundChannel1";
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &gopts.sound_en, (1 << 0));
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &gopts.sound_en, (1 << 0));
    soundSetEnable(gopts.sound_en);
    update_opts();
}

void MainFrame::OnSoundChannel2() {
    bool menuPress = false;
    char keyName[] = "SoundChannel2";
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &gopts.sound_en, (1 << 1));
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &gopts.sound_en, (1 << 1));
    soundSetEnable(gopts.sound_en);
    update_opts();
}

void MainFrame::OnSoundChannel3() {
    bool menuPress = false;
    char keyName[] = "SoundChannel3";
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &gopts.sound_en, (1 << 2));
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &gopts.sound_en, (1 << 2));
    soundSetEnable(gopts.sound_en);
    update_opts();
}

void MainFrame::OnSoundChannel4() {
    bool menuPress = false;
    char keyName[] = "SoundChannel4";
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &gopts.sound_en, (1 << 3));
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &gopts.sound_en, (1 << 3));
    soundSetEnable(gopts.sound_en);
    update_opts();
}

void MainFrame::OnDirectSoundA() {
    bool menuPress = false;
    char keyName[] = "DirectSoundA";
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &gopts.sound_en, (1 << 8));
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &gopts.sound_en, (1 << 8));
    soundSetEnable(gopts.sound_en);
    update_opts();
}

void MainFrame::OnDirectSoundB() {
    bool menuPress = false;
    char keyName[] = "DirectSoundB";
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &gopts.sound_en, (1 << 9));
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &gopts.sound_en, (1 << 9));
    soundSetEnable(gopts.sound_en);
    update_opts();
}
