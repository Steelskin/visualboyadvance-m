#include "wx/main-frame.h"

#include <wx/choice.h>
#include <wx/control.h>
#include <wx/regex.h>
#include <wx/sstream.h>
#include <wx/textctrl.h>
#include <wx/wfstream.h>

#include "core/gb/gbGlobals.h"
#include "core/gb/gbPrinter.h"
#include "core/gb/gbSound.h"
#include "core/gba/gbaGlobals.h"
#include "core/gba/gbaPrint.h"
#include "core/gba/gbaSound.h"
#include "wx/background-input.h"
#include "wx/config/option-proxy.h"
#include "wx/opts.h"
#include "wx/widgets/shortcut-menu-item.h"
#include "wx/wxhead.h"

namespace {

#ifndef NO_LINK

void SetLinkTypeMenu(const char* type, int value) {
    MainFrame* mf = wxGetApp().frame;
    mf->SetMenuOption("LinkTypeNothing", 0);
    mf->SetMenuOption("LinkTypeCable", 0);
    mf->SetMenuOption("LinkTypeWireless", 0);
    mf->SetMenuOption("LinkTypeGameCube", 0);
    mf->SetMenuOption("LinkTypeGameboy", 0);
    mf->SetMenuOption(type, 1);
    gopts.gba_link_type = value;
    update_opts();
    CloseLink();
    mf->EnableNetworkMenu();
}

#endif  // NO_LINK

// FIXME: remove this.
void toggleBitVar(bool* menuValue, int* globalVar, int mask) {
    bool isEnabled = ((*globalVar) & (mask)) != (mask);
    if (*menuValue == isEnabled)
        *globalVar = ((*globalVar) & ~(mask)) | (!isEnabled ? (mask) : 0);
    else
        *globalVar = ((*globalVar) & ~(mask)) | (*menuValue ? (mask) : 0);
    *menuValue = ((*globalVar) & (mask)) != (mask);
}

}  // namespace

// Options - Link

void MainFrame::OnLinkStart() {
#ifndef NO_LINK
    LinkMode mode = GetLinkMode();

    if (mode != LINK_DISCONNECTED) {
        // while we could deactivate the command when connected, it is more
        // user-friendly to display a message indidcating why
        wxLogError(_("LAN link is already active. Disable link mode to disconnect."));
        return;
    }

    if (OPTION(kGBALinkProto)) {
        // see above comment
        wxLogError(_("Network is not supported in local mode."));
        return;
    }

    wxDialog* dlg = GetXRCDialog("NetLink");
    ShowModal(dlg);
    GetPanel()->SetFrameTitle();
#endif
}

// Options - Link - Type

void MainFrame::OnLinkTypeNothing() {
#ifndef NO_LINK
    SetLinkTypeMenu("LinkTypeNothing", 0);
#endif
}

void MainFrame::OnLinkTypeCable() {
#ifndef NO_LINK
    SetLinkTypeMenu("LinkTypeCable", 1);
#endif
}

void MainFrame::OnLinkTypeWireless() {
#ifndef NO_LINK
    SetLinkTypeMenu("LinkTypeWireless", 2);
#endif
}

void MainFrame::OnLinkTypeGameCube() {
#ifndef NO_LINK
    SetLinkTypeMenu("LinkTypeGameCube", 3);
#endif
}

void MainFrame::OnLinkTypeGameboy() {
#ifndef NO_LINK
    SetLinkTypeMenu("LinkTypeGameboy", 4);
#endif
}

// Options - Link

void MainFrame::OnLinkLocal() {
#ifndef NO_LINK
    GetMenuOptionConfig("LinkLocal", config::OptionID::kGBALinkProto);
    EnableNetworkMenu();
#endif
}

void MainFrame::OnLinkAuto() {
#ifndef NO_LINK
    GetMenuOptionConfig("LinkAuto", config::OptionID::kGBALinkAuto);
#endif
}

void MainFrame::OnLinkSpeedOn() {
#ifndef NO_LINK
    GetMenuOptionConfig("LinkSpeedOn", config::OptionID::kGBALinkFast);
#endif
}

void MainFrame::OnLinkConfigure() {
#ifndef NO_LINK
    if (ShowModal(GetXRCDialog("LinkConfig")) != wxID_OK) {
        return;
    }

    SetLinkTimeout(gopts.link_timeout);
    update_opts();
#endif
}

// Options - Video

void MainFrame::OnDisplayConfigure() {
    if (ShowModal(GetXRCDialog("DisplayConfig")) != wxID_OK) {
        return;
    }

    const int frame_skip = OPTION(kPrefFrameSkip);
    if (frame_skip != -1) {
        systemFrameSkip = frame_skip;
    }

    update_opts();
}

void MainFrame::OnFullscreen() {
    GetMenuOptionConfig("Fullscreen", config::OptionID::kGeomFullScreen);
}

void MainFrame::OnSetSize1x() {
    OPTION(kDispScale) = 1;
}

void MainFrame::OnSetSize2x() {
    OPTION(kDispScale) = 2;
}

void MainFrame::OnSetSize3x() {
    OPTION(kDispScale) = 3;
}

void MainFrame::OnSetSize4x() {
    OPTION(kDispScale) = 4;
}

void MainFrame::OnSetSize5x() {
    OPTION(kDispScale) = 5;
}

void MainFrame::OnSetSize6x() {
    OPTION(kDispScale) = 6;
}

void MainFrame::OnChangeFilter() {
    OPTION(kDispFilter).Next();
}

void MainFrame::OnChangeIFB() {
    OPTION(kDispIFB).Next();
}

void MainFrame::OnRetainAspect() {
    GetMenuOptionConfig("RetainAspect", config::OptionID::kDispStretch);
}

void MainFrame::OnBilinear() {
    GetMenuOptionConfig("Bilinear", config::OptionID::kDispBilinear);
}

void MainFrame::OnKeepOnTop() {
    GetMenuOptionConfig("KeepOnTop", config::OptionID::kDispKeepOnTop);
}

void MainFrame::OnNoStatusMsg() {
    GetMenuOptionConfig("NoStatusMsg", config::OptionID::kPrefDisableStatus);
}

void MainFrame::OnTransparent() {
    GetMenuOptionConfig("Transparent", config::OptionID::kPrefShowSpeedTransparent);
}

// Options - Audio

void MainFrame::OnSoundConfigure() {
    if (ShowModal(GetXRCDialog("SoundConfig")) != wxID_OK)
        return;

    // No point in observing these since they can only be set in this dialog.
    gb_effects_config.echo = (float)OPTION(kSoundGBEcho) / 100.0;
    gb_effects_config.stereo = (float)OPTION(kSoundGBStereo) / 100.0;
    soundFiltering = (float)OPTION(kSoundGBAFiltering) / 100.0f;
}

void MainFrame::OnIncreaseVolume() {
    OPTION(kSoundVolume) += 5;
}

void MainFrame::OnDecreaseVolume() {
    OPTION(kSoundVolume) -= 5;
}

void MainFrame::OnToggleSound() {
    bool en = gopts.sound_en == 0;
    gopts.sound_en = en ? 0x30f : 0;
    SetMenuOption("SoundChannel1", en);
    SetMenuOption("SoundChannel2", en);
    SetMenuOption("SoundChannel3", en);
    SetMenuOption("SoundChannel4", en);
    SetMenuOption("DirectSoundA", en);
    SetMenuOption("DirectSoundB", en);
    soundSetEnable(gopts.sound_en);
    update_opts();
    systemScreenMessage(en ? _("Sound enabled") : _("Sound disabled"));
}

void MainFrame::OnGBASoundInterpolation() {
    GetMenuOptionConfig("GBASoundInterpolation", config::OptionID::kSoundGBAInterpolation);
}

void MainFrame::OnGBEnhanceSound() {
    GetMenuOptionConfig("GBEnhanceSound", config::OptionID::kSoundGBEnableEffects);
}

void MainFrame::OnGBSurround() {
    GetMenuOptionConfig("GBSurround", config::OptionID::kSoundGBSurround);
}

void MainFrame::OnGBDeclicking() {
    GetMenuOptionConfig("GBDeclicking", config::OptionID::kSoundGBDeclicking);
}

// Options - Input

void MainFrame::OnJoypadConfigure() {
    if (ShowModal(GetXRCDialog("JoypadConfig")) == wxID_OK) {
        update_shortcut_opts();
    }
}

void MainFrame::OnAllowKeyboardBackgroundInput() {
    GetMenuOptionConfig("AllowKeyboardBackgroundInput",
                        config::OptionID::kUIAllowKeyboardBackgroundInput);

    disableKeyboardBackgroundInput();
    if (OPTION(kUIAllowKeyboardBackgroundInput)) {
        if (panel && panel->panel) {
            enableKeyboardBackgroundInput(panel->panel->GetWindow()->GetEventHandler());
        }
    }
}

void MainFrame::OnAllowJoystickBackgroundInput() {
    GetMenuOptionConfig("AllowKeyboardBackgroundInput",
                        config::OptionID::kUIAllowJoystickBackgroundInput);
}

// Options - Input - Autofire

void MainFrame::OnJoypadAutofireA() {
    bool menuPress = false;
    GetMenuOptionBool("JoypadAutofireA", &menuPress);
    toggleBitVar(&menuPress, &autofire, KEYM_A);
    SetMenuOption("JoypadAutofireA", menuPress ? 1 : 0);
    GetMenuOptionInt("JoypadAutofireA", &autofire, KEYM_A);
}

void MainFrame::OnJoypadAutofireB() {
    bool menuPress = false;
    GetMenuOptionBool("JoypadAutofireB", &menuPress);
    toggleBitVar(&menuPress, &autofire, KEYM_B);
    SetMenuOption("JoypadAutofireB", menuPress ? 1 : 0);
    GetMenuOptionInt("JoypadAutofireB", &autofire, KEYM_B);
}

void MainFrame::OnJoypadAutofireL() {
    bool menuPress = false;
    GetMenuOptionBool("JoypadAutofireL", &menuPress);
    toggleBitVar(&menuPress, &autofire, KEYM_L);
    SetMenuOption("JoypadAutofireL", menuPress ? 1 : 0);
    GetMenuOptionInt("JoypadAutofireL", &autofire, KEYM_L);
}

void MainFrame::OnJoypadAutofireR() {
    bool menuPress = false;
    GetMenuOptionBool("JoypadAutofireR", &menuPress);
    toggleBitVar(&menuPress, &autofire, KEYM_R);
    SetMenuOption("JoypadAutofireR", menuPress ? 1 : 0);
    GetMenuOptionInt("JoypadAutofireR", &autofire, KEYM_R);
}

// Options - Input Autohold

void MainFrame::OnJoypadAutoholdUp() {
    bool menuPress = false;
    char keyName[] = "JoypadAutoholdUp";
    int keym = KEYM_UP;
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &autohold, keym);
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &autohold, keym);
}

void MainFrame::OnJoypadAutoholdDown() {
    bool menuPress = false;
    char keyName[] = "JoypadAutoholdDown";
    int keym = KEYM_DOWN;
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &autohold, keym);
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &autohold, keym);
}

void MainFrame::OnJoypadAutoholdLeft() {
    bool menuPress = false;
    char keyName[] = "JoypadAutoholdLeft";
    int keym = KEYM_LEFT;
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &autohold, keym);
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &autohold, keym);
}

void MainFrame::OnJoypadAutoholdRight() {
    bool menuPress = false;
    char keyName[] = "JoypadAutoholdRight";
    int keym = KEYM_RIGHT;
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &autohold, keym);
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &autohold, keym);
}

void MainFrame::OnJoypadAutoholdA() {
    bool menuPress = false;
    char keyName[] = "JoypadAutoholdA";
    int keym = KEYM_A;
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &autohold, keym);
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &autohold, keym);
}

void MainFrame::OnJoypadAutoholdB() {
    bool menuPress = false;
    char keyName[] = "JoypadAutoholdB";
    int keym = KEYM_B;
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &autohold, keym);
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &autohold, keym);
}

void MainFrame::OnJoypadAutoholdL() {
    bool menuPress = false;
    char keyName[] = "JoypadAutoholdL";
    int keym = KEYM_L;
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &autohold, keym);
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &autohold, keym);
}

void MainFrame::OnJoypadAutoholdR() {
    bool menuPress = false;
    char keyName[] = "JoypadAutoholdR";
    int keym = KEYM_R;
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &autohold, keym);
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &autohold, keym);
}

void MainFrame::OnJoypadAutoholdSelect() {
    bool menuPress = false;
    char keyName[] = "JoypadAutoholdSelect";
    int keym = KEYM_SELECT;
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &autohold, keym);
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &autohold, keym);
}

void MainFrame::OnJoypadAutoholdStart() {
    bool menuPress = false;
    char keyName[] = "JoypadAutoholdStart";
    int keym = KEYM_START;
    GetMenuOptionBool(keyName, &menuPress);
    toggleBitVar(&menuPress, &autohold, keym);
    SetMenuOption(keyName, menuPress ? 1 : 0);
    GetMenuOptionInt(keyName, &autohold, keym);
}

// Options - Game Boy Advance

void MainFrame::OnGameBoyAdvanceConfigure() {
    wxDialog* dlg = GetXRCDialog("GameBoyAdvanceConfig");
    wxTextCtrl* ovcmt = XRCCTRL(*dlg, "Comment", wxTextCtrl);
    wxString cmt;
    wxChoice *ovrtc = XRCCTRL(*dlg, "OvRTC", wxChoice),
             *ovst = XRCCTRL(*dlg, "OvSaveType", wxChoice),
             *ovfs = XRCCTRL(*dlg, "OvFlashSize", wxChoice),
             *ovmir = XRCCTRL(*dlg, "OvMirroring", wxChoice);

    if (panel->game_type() == IMAGE_GBA) {
        wxString s = wxString((const char*)&g_rom[0xac], wxConvLibc, 4);
        XRCCTRL(*dlg, "GameCode", wxControl)->SetLabel(s);
        cmt = wxString((const char*)&g_rom[0xa0], wxConvLibc, 12);
        wxFileConfig* cfg = wxGetApp().overrides;

        if (cfg->HasGroup(s)) {
            cfg->SetPath(s);
            cmt = cfg->Read(wxT("comment"), cmt);
            ovcmt->SetValue(cmt);
            ovrtc->SetSelection(cfg->Read(wxT("rtcEnabled"), -1) + 1);
            ovst->SetSelection(cfg->Read(wxT("saveType"), -1) + 1);
            ovfs->SetSelection((cfg->Read(wxT("flashSize"), -1) >> 17) + 1);
            ovmir->SetSelection(cfg->Read(wxT("mirroringEnabled"), -1) + 1);
            cfg->SetPath(wxT("/"));
        } else {
            ovcmt->SetValue(cmt);
            ovrtc->SetSelection(0);
            ovst->SetSelection(0);
            ovfs->SetSelection(0);
            ovmir->SetSelection(0);
        }
    } else {
        XRCCTRL(*dlg, "GameCode", wxControl)->SetLabel(wxEmptyString);
        ovcmt->SetValue(wxEmptyString);
        ovrtc->SetSelection(0);
        ovst->SetSelection(0);
        ovfs->SetSelection(0);
        ovmir->SetSelection(0);
    }

    if (ShowModal(dlg) != wxID_OK)
        return;

    if (GetPanel()->game_type() == IMAGE_GBA) {
        agbPrintEnable(OPTION(kPrefAgbPrint));
        wxString s = wxString((const char*)&g_rom[0xac], wxConvLibc, 4);
        wxFileConfig* cfg = wxGetApp().overrides;
        bool chg;

        if (cfg->HasGroup(s)) {
            cfg->SetPath(s);
            chg = ovcmt->GetValue() != cmt ||
                  ovrtc->GetSelection() != cfg->Read(wxT("rtcEnabled"), -1) + 1 ||
                  ovst->GetSelection() != cfg->Read(wxT("saveType"), -1) + 1 ||
                  ovfs->GetSelection() != (cfg->Read(wxT("flashSize"), -1) >> 17) + 1 ||
                  ovmir->GetSelection() != cfg->Read(wxT("mirroringEnabled"), -1) + 1;
            cfg->SetPath(wxT("/"));
        } else
            chg = ovrtc->GetSelection() != 0 || ovst->GetSelection() != 0 ||
                  ovfs->GetSelection() != 0 || ovmir->GetSelection() != 0;

        if (chg) {
            wxString vba_over;
            wxFileName fn(wxGetApp().GetConfigurationPath(), wxT("vba-over.ini"));

            if (fn.FileExists()) {
                wxFileInputStream fis(fn.GetFullPath());
                wxStringOutputStream sos(&vba_over);
                fis.Read(sos);
            }

            if (cfg->HasGroup(s)) {
                cfg->SetPath(s);

                if (cfg->Read(wxT("path"), wxEmptyString) == fn.GetPath()) {
                    // EOL can be either \n (unix), \r\n (dos), or \r (old mac)
                    wxString res(
                        wxT("(^|[\n\r])"               // a new line
                            L"("                       // capture group as \2
                            L"(#[^\n\r]*(\r?\n|\r))?"  // an optional comment line
                            L"\\["));                  // the group header
                    res += s;
                    res += wxT(
                        "\\]"
                        L"([^[#]"                    // non-comment non-group-start chars
                        L"|[^\r\n \t][ \t]*[[#]"     // or comment/grp start chars in middle of line
                        L"|#[^\n\r]*(\r?\n|\r)[^[]"  // or comments not followed by grp start
                        L")*"
                        L")"  // end of group
                        // no need to try to describe what's next
                        // as the regex should maximize match size
                    );
                    wxRegEx re(res);

                    // there may be more than one group if it was hand-edited
                    // so remove them all
                    // could use re.Replace(), but this is more reliable
                    while (re.Matches(vba_over)) {
                        size_t beg, end;
                        re.GetMatch(&beg, &end, 2);
                        vba_over.erase(beg, end - beg);
                    }
                }

                cfg->SetPath(wxT("/"));
                cfg->DeleteGroup(s);
            }

            cfg->SetPath(s);
            cfg->Write(wxT("path"), fn.GetPath());
            cfg->Write(wxT("comment"), ovcmt->GetValue());
            vba_over.append(wxT("# "));
            vba_over.append(ovcmt->GetValue());
            vba_over.append(wxTextFile::GetEOL());
            vba_over.append(wxT('['));
            vba_over.append(s);
            vba_over.append(wxT(']'));
            vba_over.append(wxTextFile::GetEOL());
            int sel;
#define appendval(n)                                   \
    do {                                               \
        vba_over.append(wxT(n));                       \
        vba_over.append(wxT('='));                     \
        vba_over.append((wxChar)(wxT('0') + sel - 1)); \
        vba_over.append(wxTextFile::GetEOL());         \
        cfg->Write(wxT(n), sel - 1);                   \
    } while (0)

            if ((sel = ovrtc->GetSelection()) > 0)
                appendval("rtcEnabled");

            if ((sel = ovst->GetSelection()) > 0)
                appendval("saveType");

            if ((sel = ovfs->GetSelection()) > 0) {
                vba_over.append(wxT("flashSize="));
                vba_over.append(sel == 1 ? wxT("65536") : wxT("131072"));
                vba_over.append(wxTextFile::GetEOL());
                cfg->Write(wxT("flashSize"), 0x10000 << (sel - 1));
            }

            if ((sel = ovmir->GetSelection()) > 0)
                appendval("mirroringEnabled");

            cfg->SetPath(wxT("/"));
            vba_over.append(wxTextFile::GetEOL());
            fn.Mkdir(0777, wxPATH_MKDIR_FULL);
            wxTempFileOutputStream fos(fn.GetFullPath());
            fos.Write(vba_over.c_str(), vba_over.size());
            fos.Commit();
        }
    }

    update_opts();
}

void MainFrame::OnRTC() {
    GetMenuOptionInt("RTC", &coreOptions.rtcEnabled, 1);
    update_opts();
}

void MainFrame::OnBootRomGba() {
    GetMenuOptionConfig("BootRomGba", config::OptionID::kPrefUseBiosGBA);
}

void MainFrame::OnAGBPrinter() {
    GetMenuOptionConfig("AGBPrinter", config::OptionID::kPrefAgbPrint);
}

void MainFrame::OnGBALcdFilter() {
    GetMenuOptionConfig("GBALcdFilter", config::OptionID::kGBALCDFilter);
}

// Options - Game Boy

void MainFrame::OnGameBoyConfigure() {
    ShowModal(GetXRCDialog("GameBoyConfig"));
}

void MainFrame::OnGBColorOption() {
    GetMenuOptionConfig("GBColorOption", config::OptionID::kGBColorOption);
}

void MainFrame::OnGBLcdFilter() {
    GetMenuOptionConfig("GBLcdFilter", config::OptionID::kGBLCDFilter);
}

void MainFrame::OnColorizerHack() {
    GetMenuOptionConfig("ColorizerHack", config::OptionID::kGBColorizerHack);
    if (OPTION(kGBColorizerHack) && OPTION(kPrefUseBiosGB)) {
        wxLogError(_("Cannot use Colorizer Hack when Game Boy BIOS File is enabled."));
        SetMenuOption("ColorizerHack", 0);
        OPTION(kGBColorizerHack) = false;
    }
}

void MainFrame::OnPrinter() {
    GetMenuOptionInt("Printer", &coreOptions.winGbPrinterEnabled, 1);
#if (defined __WIN32__ || defined _WIN32)
#ifndef NO_LINK
    gbSerialFunction = gbStartLink;
#else
    gbSerialFunction = NULL;
#endif
#endif
    if (coreOptions.winGbPrinterEnabled)
        gbSerialFunction = gbPrinterSend;

    update_opts();
}

void MainFrame::OnPrintGather() {
    GetMenuOptionConfig("PrintGather", config::OptionID::kGBPrintAutoPage);
}

void MainFrame::OnPrintSnap() {
    GetMenuOptionConfig("PrintSnap", config::OptionID::kGBPrintScreenCap);
}

void MainFrame::OnBootRomGB() {
    GetMenuOptionConfig("BootRomGB", config::OptionID::kPrefUseBiosGB);
    if (OPTION(kPrefUseBiosGB) && OPTION(kGBColorizerHack)) {
        wxLogError(_("Cannot use Game Boy BIOS when Colorizer Hack is enabled."));
        SetMenuOption("BootRomGB", 0);
        OPTION(kPrefUseBiosGB) = false;
    }
}

void MainFrame::OnBootRomGBC() {
    GetMenuOptionConfig("BootRomGBC", config::OptionID::kPrefUseBiosGBC);
}

// Options

void MainFrame::OnGeneralConfigure() {
    int rew = gopts.rewind_interval;
    wxDialog* dlg = GetXRCDialog("GeneralConfig");

    if (ShowModal(dlg) == wxID_OK)
        update_opts();

    if (GetPanel()->game_type() != IMAGE_UNKNOWN)
        soundSetThrottle(coreOptions.throttle);

    if (rew != gopts.rewind_interval) {
        if (!gopts.rewind_interval) {
            if (GetPanel()->num_rewind_states) {
                cmd_enable_ &= ~widgets::CMDEN_REWIND;
                enable_menus();
            }

            GetPanel()->num_rewind_states = 0;
            GetPanel()->do_rewind = false;
        } else {
            if (!GetPanel()->num_rewind_states)
                GetPanel()->do_rewind = true;

            GetPanel()->rewind_time = gopts.rewind_interval * 6;
        }
    }
}

void MainFrame::OnSpeedupConfigure() {
    wxDialog* dlg = GetXRCDialog("SpeedupConfig");

    unsigned save_speedup_throttle = coreOptions.speedup_throttle;
    unsigned save_speedup_frame_skip = coreOptions.speedup_frame_skip;
    bool save_speedup_throttle_frame_skip = coreOptions.speedup_throttle_frame_skip;

    if (ShowModal(dlg) == wxID_OK)
        update_opts();
    else {
        // Restore values if cancel pressed.
        coreOptions.speedup_throttle = save_speedup_throttle;
        coreOptions.speedup_frame_skip = save_speedup_frame_skip;
        coreOptions.speedup_throttle_frame_skip = save_speedup_throttle_frame_skip;
    }
}

void MainFrame::OnEmulatorDirectories() {
    ShowModal(GetXRCDialog("DirectoriesConfig"));
}

void MainFrame::OnCustomize() {
    if (ShowModal(GetXRCDialog("AccelConfig")) == wxID_OK) {
        update_shortcut_opts();
        ResetMenuAccelerators();
    }
}

// Options - UI Settings

void MainFrame::OnStatusBar() {
    GetMenuOptionConfig("StatusBar", config::OptionID::kGenStatusBar);
}

void MainFrame::OnHideMenuBar() {
    GetMenuOptionConfig("HideMenuBar", config::OptionID::kUIHideMenuBar);
}

void MainFrame::OnSuspendScreenSaver() {
    GetMenuOptionConfig("SuspendScreenSaver", config::OptionID::kUISuspendScreenSaver);
}
