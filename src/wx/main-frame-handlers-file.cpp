#include "wx/main-frame.h"

#include <wx/ctrlsub.h>
#include <wx/ffile.h>
#include <wx/filedlg.h>
#include <wx/filehistory.h>
#include <wx/msgdlg.h>
#include <wx/textctrl.h>
#include <wx/xrc/xmlres.h>

#include "core/gb/gb.h"
#include "core/gb/gbCheats.h"
#include "core/gba/gbaCheats.h"
#include "core/gba/gbaEeprom.h"
#include "core/gba/gbaGlobals.h"
#include "wx/config/file-history.h"
#include "wx/config/option-proxy.h"
#include "wx/dialogs/game-maker.h"
#include "wx/opts.h"
#include "wx/wxhead.h"

namespace {

void SetDialogLabel(wxDialog* dlg, const wxString& id, wxString ts, size_t l) {
    (void)l;  // unused params
    ts.Replace("&", "&&", true);
    (dynamic_cast<wxControl*>((*dlg).FindWindow(wxXmlResource::GetXRCID(id))))->SetLabel(ts);
}

// FIXME: Get rid of this.
void toggleBitVar(bool* menuValue, int* globalVar, int mask) {
    bool isEnabled = ((*globalVar) & (mask)) != (mask);
    if (*menuValue == isEnabled)
        *globalVar = ((*globalVar) & ~(mask)) | (!isEnabled ? (mask) : 0);
    else
        *globalVar = ((*globalVar) & ~(mask)) | (*menuValue ? (mask) : 0);
    *menuValue = ((*globalVar) & (mask)) != (mask);
}

}  // namespace

void MainFrame::OnOpenGba() {
    static int open_ft = 0;
    const wxString& gba_rom_dir = OPTION(kGBAROMDir);

    // FIXME: ignore if non-existent or not a dir
    wxString pats =
        _("Game Boy Advance Files (*.agb;*.gba;*.bin;*.elf;*.mb;*.zip;*.7z;*.rar)|"
          "*.agb;*.gba;*.bin;*.elf;*.mb;"
          "*.agb.gz;*.gba.gz;*.bin.gz;*.elf.gz;*.mb.gz;"
          "*.agb.z;*.gba.z;*.bin.z;*.elf.z;*.mb.z;"
          "*.zip;*.7z;*.rar|"
          "Game Boy Files (*.dmg;*.gb;*.gbc;*.cgb;*.sgb;*.zip;*.7z;*.rar)|"
          "*.dmg;*.gb;*.gbc;*.cgb;*.sgb;"
          "*.dmg.gz;*.gb.gz;*.gbc.gz;*.cgb.gz;*.sgb.gz;"
          "*.dmg.z;*.gb.z;*.gbc.z;*.cgb.z;*.sgb.z;"
          "*.zip;*.7z;*.rar|");
    pats.append(wxALL_FILES);
    wxFileDialog dlg(this, _("Open ROM file"), gba_rom_dir, "", pats,
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    dlg.SetFilterIndex(open_ft);

    if (ShowModal(&dlg) == wxID_OK)
        wxGetApp().pending_load = dlg.GetPath();

    open_ft = dlg.GetFilterIndex();
    if (gba_rom_dir.empty()) {
        OPTION(kGBAROMDir) = dlg.GetDirectory();
    }
}

void MainFrame::OnOpenGB() {
    static int open_ft = 0;
    const wxString& gb_rom_dir = OPTION(kGBROMDir);

    // FIXME: ignore if non-existent or not a dir
    wxString pats =
        _("Game Boy Files (*.dmg;*.gb;*.gbc;*.cgb;*.sgb;*.zip;*.7z;*.rar)|"
          "*.dmg;*.gb;*.gbc;*.cgb;*.sgb;"
          "*.dmg.gz;*.gb.gz;*.gbc.gz;*.cgb.gz;*.sgb.gz;"
          "*.dmg.z;*.gb.z;*.gbc.z;*.cgb.z;*.sgb.z;"
          "*.zip;*.7z;*.rar|");
    pats.append(wxALL_FILES);
    wxFileDialog dlg(this, _("Open GB ROM file"), gb_rom_dir, "", pats,
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    dlg.SetFilterIndex(open_ft);

    if (ShowModal(&dlg) == wxID_OK)
        wxGetApp().pending_load = dlg.GetPath();

    open_ft = dlg.GetFilterIndex();
    if (gb_rom_dir.empty()) {
        OPTION(kGBROMDir) = dlg.GetDirectory();
    }
}

void MainFrame::OnOpenGBC() {
    static int open_ft = 0;
    const wxString& gbc_rom_dir = OPTION(kGBGBCROMDir);

    // FIXME: ignore if non-existent or not a dir
    wxString pats =
        _("Game Boy Color Files (*.dmg;*.gb;*.gbc;*.cgb;*.sgb;*.zip;*.7z;*.rar)|"
          "*.dmg;*.gb;*.gbc;*.cgb;*.sgb;"
          "*.dmg.gz;*.gb.gz;*.gbc.gz;*.cgb.gz;*.sgb.gz;"
          "*.dmg.z;*.gb.z;*.gbc.z;*.cgb.z;*.sgb.z;"
          "*.zip;*.7z;*.rar|");
    pats.append(wxALL_FILES);
    wxFileDialog dlg(this, _("Open GBC ROM file"), gbc_rom_dir, "", pats,
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    dlg.SetFilterIndex(open_ft);

    if (ShowModal(&dlg) == wxID_OK)
        wxGetApp().pending_load = dlg.GetPath();

    open_ft = dlg.GetFilterIndex();
    if (gbc_rom_dir.empty()) {
        OPTION(kGBGBCROMDir) = dlg.GetDirectory();
    }
}

// File - Recent

void MainFrame::OnRecentReset() {
    // only save config if there were items to remove
    config_provider_->file_history()->ClearHistory();
}

void MainFrame::OnRecentFreeze() {
    GetMenuOptionConfig("RecentFreeze", config::OptionID::kGenFreezeRecent);
}

// following 10 should really be a single ranged handler
// former names: Recent01 .. Recent10
void MainFrame::OnRecentFile1() {
    GetPanel()->LoadGame(config_provider_->file_history()->GetHistoryFile(0).GetFullPath());
}

void MainFrame::OnRecentFile2() {
    GetPanel()->LoadGame(config_provider_->file_history()->GetHistoryFile(1).GetFullPath());
}

void MainFrame::OnRecentFile3() {
    GetPanel()->LoadGame(config_provider_->file_history()->GetHistoryFile(2).GetFullPath());
}

void MainFrame::OnRecentFile4() {
    GetPanel()->LoadGame(config_provider_->file_history()->GetHistoryFile(3).GetFullPath());
}

void MainFrame::OnRecentFile5() {
    GetPanel()->LoadGame(config_provider_->file_history()->GetHistoryFile(4).GetFullPath());
}

void MainFrame::OnRecentFile6() {
    GetPanel()->LoadGame(config_provider_->file_history()->GetHistoryFile(5).GetFullPath());
}

void MainFrame::OnRecentFile7() {
    GetPanel()->LoadGame(config_provider_->file_history()->GetHistoryFile(6).GetFullPath());
}

void MainFrame::OnRecentFile8() {
    GetPanel()->LoadGame(config_provider_->file_history()->GetHistoryFile(7).GetFullPath());
}

void MainFrame::OnRecentFile9() {
    GetPanel()->LoadGame(config_provider_->file_history()->GetHistoryFile(8).GetFullPath());
}

void MainFrame::OnRecentFile10() {
    GetPanel()->LoadGame(config_provider_->file_history()->GetHistoryFile(9).GetFullPath());
}

// File

void MainFrame::OnRomInformation() {
    wxString s;
#define setlab(id)                                 \
    do {                                           \
        /* SetLabelText is not in 2.8 */           \
        s.Replace(wxT("&"), wxT("&&"), true);      \
        XRCCTRL(*dlg, id, wxControl)->SetLabel(s); \
    } while (0)
#define setblab(id, b)                          \
    do {                                        \
        s.Printf(wxT("%02x"), (unsigned int)b); \
        setlab(id);                             \
    } while (0)
#define setlabs(id, ts, l)                               \
    do {                                                 \
        s = wxString((const char*)&(ts), wxConvLibc, l); \
        setlab(id);                                      \
    } while (0)

    switch (GetPanel()->game_type()) {
        case IMAGE_GB:
            ShowModal(GetXRCDialog("GBROMInfo"));
            break;
        case IMAGE_GBA: {
            IdentifyRom();
            wxDialog* dlg = GetXRCDialog("GBAROMInfo");
            wxString rom_crc32;
            rom_crc32.Printf(wxT("%08X"), panel->rom_crc32);
            SetDialogLabel(dlg, wxT("Title"), panel->rom_name, 30);
            setlabs("IntTitle", g_rom[0xa0], 12);
            SetDialogLabel(dlg, wxT("Scene"), panel->rom_scene_rls_name, 30);
            SetDialogLabel(dlg, wxT("Release"), panel->rom_scene_rls, 4);
            SetDialogLabel(dlg, wxT("CRC32"), rom_crc32, 8);
            setlabs("GameCode", g_rom[0xac], 4);
            setlabs("MakerCode", g_rom[0xb0], 2);
            s = dialogs::GetGameMakerName(s.ToStdString());
            setlab("MakerName");
            setblab("UnitCode", g_rom[0xb3]);
            s.Printf(wxT("%02x"), (unsigned int)g_rom[0xb4]);

            if (g_rom[0xb4] & 0x80)
                s.append(wxT(" (DACS)"));

            setlab("DeviceType");
            setblab("Version", g_rom[0xbc]);
            uint8_t crc = 0x19;

            for (int i = 0xa0; i < 0xbd; i++)
                crc += g_rom[i];

            crc = -crc;
            s.Printf(wxT("%02x (%02x)"), crc, g_rom[0xbd]);
            setlab("CRC");
            dlg->Fit();
            ShowModal(dlg);
        } break;

        default:
            break;
    }
}

// File - e-Reader

void MainFrame::OnResetLoadingDotCodeFile() {
    ResetLoadDotCodeFile();
}

void MainFrame::OnSetLoadingDotCodeFile() {
    static wxString loaddotcodefile_path;
    wxFileDialog dlg(this, _("Select Dot Code file"), loaddotcodefile_path, wxEmptyString,
                     _("e-Reader Dot Code (*.bin;*.raw)|"
                       "*.bin;*.raw"),
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    int ret = ShowModal(&dlg);

    if (ret != wxID_OK)
        return;

    loaddotcodefile_path = dlg.GetPath();
    SetLoadDotCodeFile(UTF8(loaddotcodefile_path));
}

void MainFrame::OnResetSavingDotCodeFile() {
    ResetLoadDotCodeFile();
}

void MainFrame::OnSetSavingDotCodeFile() {
    static wxString savedotcodefile_path;
    wxFileDialog dlg(this, _("Select Dot Code file"), savedotcodefile_path, wxEmptyString,
                     _("e-Reader Dot Code (*.bin;*.raw)|"
                       "*.bin;*.raw"),
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    int ret = ShowModal(&dlg);

    if (ret != wxID_OK)
        return;

    savedotcodefile_path = dlg.GetPath();
    SetSaveDotCodeFile(UTF8(savedotcodefile_path));
}

// File - Load State

void MainFrame::OnLoadStateRecent() {
    GetPanel()->LoadState();
}

void MainFrame::OnLoadStateSlot() {
    GetPanel()->LoadState(state_slot_ + 1);
}

void MainFrame::OnLoadStateAutoLoad() {
    GetMenuOptionConfig("LoadStateAutoLoad", config::OptionID::kGenAutoLoadLastState);
}

void MainFrame::OnLoadState01() {
    GetPanel()->LoadState(1);
}

void MainFrame::OnLoadState02() {
    GetPanel()->LoadState(2);
}

void MainFrame::OnLoadState03() {
    GetPanel()->LoadState(3);
}

void MainFrame::OnLoadState04() {
    GetPanel()->LoadState(4);
}

void MainFrame::OnLoadState05() {
    GetPanel()->LoadState(5);
}

void MainFrame::OnLoadState06() {
    GetPanel()->LoadState(6);
}

void MainFrame::OnLoadState07() {
    GetPanel()->LoadState(7);
}

void MainFrame::OnLoadState08() {
    GetPanel()->LoadState(8);
}

void MainFrame::OnLoadState09() {
    GetPanel()->LoadState(9);
}

void MainFrame::OnLoadState10() {
    GetPanel()->LoadState(10);
}

void MainFrame::OnLoadStateFile() {
    if (state_dir_.empty()) {
        state_dir_ = GetPanel()->state_dir();
    }

    wxFileDialog dlg(this, _("Select state file"), state_dir_, wxEmptyString,
                     _("Visual Boy Advance saved game files|*.sgm"),
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    int ret = ShowModal(&dlg);
    state_dir_ = dlg.GetDirectory();

    if (ret != wxID_OK) {
        return;
    }

    GetPanel()->LoadState(dlg.GetPath());
}

void MainFrame::OnLoadStateKeepSaves() {
    bool menuPress = false;
    GetMenuOptionBool("LoadStateKeepSaves", &menuPress);
    toggleBitVar(&menuPress, &coreOptions.skipSaveGameBattery, 1);
    SetMenuOption("LoadStateKeepSaves", menuPress ? 1 : 0);
    GetMenuOptionInt("LoadStateKeepSaves", &coreOptions.skipSaveGameBattery, 1);
    update_opts();
}

void MainFrame::OnLoadStateKeepCheats() {
    bool menuPress = false;
    GetMenuOptionBool("LoadStateKeepCheats", &menuPress);
    toggleBitVar(&menuPress, &coreOptions.skipSaveGameCheats, 1);
    SetMenuOption("LoadStateKeepCheats", menuPress ? 1 : 0);
    GetMenuOptionInt("LoadStateKeepCheats", &coreOptions.skipSaveGameCheats, 1);
    update_opts();
}

// File - Save State

void MainFrame::OnSaveStateOldest() {
    GetPanel()->SaveState();
}

void MainFrame::OnSaveStateSlot() {
    GetPanel()->SaveState(state_slot_ + 1);
}

void MainFrame::OnSaveStateIncreaseSlot() {
    state_slot_ = (state_slot_ + 1) % 10;
    GetPanel()->SaveState(state_slot_ + 1);
}

void MainFrame::OnSaveState01() {
    GetPanel()->SaveState(1);
}

void MainFrame::OnSaveState02() {
    GetPanel()->SaveState(2);
}

void MainFrame::OnSaveState03() {
    GetPanel()->SaveState(3);
}

void MainFrame::OnSaveState04() {
    GetPanel()->SaveState(4);
}

void MainFrame::OnSaveState05() {
    GetPanel()->SaveState(5);
}

void MainFrame::OnSaveState06() {
    GetPanel()->SaveState(6);
}

void MainFrame::OnSaveState07() {
    GetPanel()->SaveState(7);
}

void MainFrame::OnSaveState08() {
    GetPanel()->SaveState(8);
}

void MainFrame::OnSaveState09() {
    GetPanel()->SaveState(9);
}

void MainFrame::OnSaveState10() {
    GetPanel()->SaveState(10);
}

void MainFrame::OnSaveStateFile() {
    if (state_dir_.empty()) {
        state_dir_ = GetPanel()->state_dir();
    }

    wxFileDialog dlg(this, _("Select state file"), state_dir_, wxEmptyString,
                     _("Visual Boy Advance saved game files|*.sgm"),
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    int ret = ShowModal(&dlg);
    state_dir_ = dlg.GetDirectory();

    if (ret != wxID_OK)
        return;

    GetPanel()->SaveState(dlg.GetPath());
}

// File

void MainFrame::OnStateIncreaseSlot() {
    state_slot_ = (state_slot_ + 1) % 10;
    OnStateSlotChanged();
}

void MainFrame::OnStateDecreaseSlot() {
    state_slot_ = (state_slot_ + 9) % 10;
    OnStateSlotChanged();
}

// File - Import

void MainFrame::OnImportBatteryFile() {
    if (!battery_dir_.size())
        battery_dir_ = GetPanel()->bat_dir();

    wxFileDialog dlg(this, _("Select battery file"), battery_dir_, wxEmptyString,
                     _("Battery file (*.sav)|*.sav|Flash save (*.dat)|*.dat"),
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    int ret = ShowModal(&dlg);
    battery_dir_ = dlg.GetDirectory();

    if (ret != wxID_OK)
        return;

    wxString fn = dlg.GetPath();
    ret = wxMessageBox(_("Importing a battery file will erase any saved games (permanently after "
                         "the next write). Do you want to continue?"),
                       _("Confirm import"), wxYES_NO | wxICON_EXCLAMATION);

    if (ret == wxYES) {
        wxString msg;

        if (GetPanel()->emusys->emuReadBattery(UTF8(fn)))
            msg.Printf(_("Loaded battery %s"), fn.wc_str());
        else
            msg.Printf(_("Error loading battery %s"), fn.wc_str());

        systemScreenMessage(msg);
    }
}

void MainFrame::OnImportGamesharkCodeFile() {
    static wxString path;
    wxFileDialog dlg(this, _("Select code file"), path, wxEmptyString,
                     GetPanel()->game_type() == IMAGE_GBA
                         ? _("Game Shark Code File (*.spc;*.xpc)|*.spc;*.xpc")
                         : _("Game Shark Code File (*.gcf)|*.gcf"),
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    int ret = ShowModal(&dlg);
    path = dlg.GetDirectory();

    if (ret != wxID_OK)
        return;

    wxString fn = dlg.GetPath();
    ret = wxMessageBox(
        _("Importing a code file will replace any loaded cheats. Do you want to continue?"),
        _("Confirm import"), wxYES_NO | wxICON_EXCLAMATION);

    if (ret == wxYES) {
        wxString msg;
        bool res;

        if (GetPanel()->game_type() == IMAGE_GB)
            // FIXME: this routine will not work on big-endian systems
            // if the underlying file format is little-endian
            // (fix in gb/gbCheats.cpp)
            res = gbCheatReadGSCodeFile(UTF8(fn));
        else {
            // need to select game first
            wxFFile f(fn, wxT("rb"));

            if (!f.IsOpened()) {
                wxLogError(_("Cannot open file %s"), fn.c_str());
                return;
            }

            // FIXME: in my code, I assume file format is little-endian
            // however, in core code, it is assumed to be native-endian
            uint32_t len;
            char buf[14];

            if (f.Read(&len, sizeof(len)) != sizeof(len) || wxUINT32_SWAP_ON_BE(len) != 14 ||
                f.Read(buf, 14) != 14 || memcmp(buf, "SharkPortCODES", 14)) {
                wxLogError(_("Unsupported code file %s"), fn.c_str());
                return;
            }

            f.Seek(0x1e);

            if (f.Read(&len, sizeof(len)) != sizeof(len))
                len = 0;

            uint32_t game = 0;

            if (len > 1) {
                wxDialog* seldlg = GetXRCDialog("CodeSelect");
                wxControlWithItems* lst = XRCCTRL(*seldlg, "CodeList", wxControlWithItems);
                lst->Clear();

                while (len-- > 0) {
                    uint32_t slen;

                    if (f.Read(&slen, sizeof(slen)) != sizeof(slen) ||
                        slen > 1024)  // arbitrary upper bound
                        break;

                    char buf2[1024];

                    if (f.Read(buf2, slen) != slen)
                        break;

                    lst->Append(wxString(buf2, wxConvLibc, slen));
                    uint32_t ncodes;

                    if (f.Read(&ncodes, sizeof(ncodes)) != sizeof(ncodes))
                        break;

                    for (; ncodes > 0; ncodes--) {
                        if (f.Read(&slen, sizeof(slen)) != sizeof(slen))
                            break;

                        f.Seek(slen, wxFromCurrent);

                        if (f.Read(&slen, sizeof(slen)) != sizeof(slen))
                            break;

                        f.Seek(slen + 4, wxFromCurrent);

                        if (f.Read(&slen, sizeof(slen)) != sizeof(slen))
                            break;

                        f.Seek(slen * 12, wxFromCurrent);
                    }
                }

                int sel = ShowModal(seldlg);

                if (sel != wxID_OK)
                    return;

                game = lst->GetSelection();

                if ((int)game == wxNOT_FOUND)
                    game = 0;
            }

            bool v3 =
                fn.size() >= 4 && wxString(fn.substr(fn.size() - 4)).IsSameAs(wxT(".xpc"), false);
            // FIXME: this routine will not work on big-endian systems
            // if the underlying file format is little-endian
            // (fix in gba/Cheats.cpp)
            res = cheatsImportGSACodeFile(UTF8(fn), game, v3);
        }

        if (res)
            msg.Printf(_("Loaded code file %s"), fn.wc_str());
        else
            msg.Printf(_("Error loading code file %s"), fn.wc_str());

        systemScreenMessage(msg);
    }
}

void MainFrame::OnImportGamesharkActionReplaySnapshot() {
    wxFileDialog dlg(this, _("Select snapshot file"), gameshark_dir_, wxEmptyString,
                     GetPanel()->game_type() == IMAGE_GBA
                         ? _("Game Shark & PAC Snapshots (*.sps;*.xps)|*.sps;*.xps|Game Shark SP "
                             "Snapshots (*.gsv)|*.gsv")
                         : _("Game Boy Snapshot (*.gbs)|*.gbs"),
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    int ret = ShowModal(&dlg);
    gameshark_dir_ = dlg.GetDirectory();

    if (ret != wxID_OK)
        return;

    wxString fn = dlg.GetPath();
    ret = wxMessageBox(_("Importing a snapshot file will erase any saved games (permanently after "
                         "the next write). Do you want to continue?"),
                       _("Confirm import"), wxYES_NO | wxICON_EXCLAMATION);

    if (ret == wxYES) {
        wxString msg;
        bool res;

        if (GetPanel()->game_type() == IMAGE_GB)
            res = gbReadGSASnapshot(UTF8(fn));
        else {
            bool gsv =
                fn.size() >= 4 && wxString(fn.substr(fn.size() - 4)).IsSameAs(wxT(".gsv"), false);

            if (gsv)
                // FIXME: this will fail on big-endian machines if
                // file format is little-endian
                // fix in GBA.cpp
                res = CPUReadGSASPSnapshot(UTF8(fn));
            else
                // FIXME: this will fail on big-endian machines if
                // file format is little-endian
                // fix in GBA.cpp
                res = CPUReadGSASnapshot(UTF8(fn));
        }

        if (res)
            msg.Printf(_("Loaded snapshot file %s"), fn.wc_str());
        else
            msg.Printf(_("Error loading snapshot file %s"), fn.wc_str());

        systemScreenMessage(msg);
    }
}

// File - Export

void MainFrame::OnExportBatteryFile() {
    if (!battery_dir_.size())
        battery_dir_ = GetPanel()->bat_dir();

    wxFileDialog dlg(this, _("Select battery file"), battery_dir_, wxEmptyString,
                     _("Battery file (*.sav)|*.sav|Flash save (*.dat)|*.dat"),
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    int ret = ShowModal(&dlg);
    battery_dir_ = dlg.GetDirectory();

    if (ret != wxID_OK)
        return;

    wxString fn = dlg.GetPath();
    wxString msg;

    if (GetPanel()->emusys->emuWriteBattery(UTF8(fn)))
        msg.Printf(_("Wrote battery %s"), fn.wc_str());
    else
        msg.Printf(_("Error writing battery %s"), fn.wc_str());

    systemScreenMessage(msg);
}

void MainFrame::OnExportGamesharkSnapshot() {
    if (eepromInUse) {
        wxLogError(_("EEPROM saves cannot be exported"));
        return;
    }

    wxString def_name = GetPanel()->game_name();
    def_name.append(wxT(".sps"));
    wxFileDialog dlg(this, _("Select snapshot file"), gameshark_dir_, def_name,
                     _("Game Shark Snapshot (*.sps)|*.sps"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    int ret = ShowModal(&dlg);
    gameshark_dir_ = dlg.GetDirectory();

    if (ret != wxID_OK)
        return;

    wxString fn = dlg.GetPath();
    wxDialog* infodlg = GetXRCDialog("ExportSPS");
    wxTextCtrl *tit = XRCCTRL(*infodlg, "Title", wxTextCtrl),
               *dsc = XRCCTRL(*infodlg, "Description", wxTextCtrl),
               *n = XRCCTRL(*infodlg, "Notes", wxTextCtrl);
    tit->SetValue(wxString((const char*)&g_rom[0xa0], wxConvLibc, 12));
    dsc->SetValue(wxDateTime::Now().Format(wxT("%c")));
    n->SetValue(_("Exported from Visual Boy Advance-M"));

    if (ShowModal(infodlg) != wxID_OK)
        return;

    wxString msg;

    // FIXME: this will fail on big-endian machines if file format is
    // little-endian
    // fix in GBA.cpp
    if (CPUWriteGSASnapshot(fn.utf8_str(), tit->GetValue().utf8_str(), dsc->GetValue().utf8_str(),
                            n->GetValue().utf8_str()))
        msg.Printf(_("Saved snapshot file %s"), fn.wc_str());
    else
        msg.Printf(_("Error saving snapshot file %s"), fn.wc_str());

    systemScreenMessage(msg);
}

// File

void MainFrame::OnScreenCapture() {
    wxString scap_path = GetGamePath(OPTION(kGenScreenshotDir));
    wxString def_name = GetPanel()->game_name();

    const int capture_format = OPTION(kPrefCaptureFormat);
    if (capture_format == 0)
        def_name.append(".png");
    else
        def_name.append(".bmp");

    wxFileDialog dlg(this, _("Select output file"), scap_path, def_name,
                     _("PNG images|*.png|BMP images|*.bmp"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    dlg.SetFilterIndex(capture_format);
    int ret = ShowModal(&dlg);
    scap_path = dlg.GetDirectory();

    if (ret != wxID_OK)
        return;

    wxString fn = dlg.GetPath();
    int fmt = dlg.GetFilterIndex();

    if (fn.size() >= 4) {
        if (wxString(fn.substr(fn.size() - 4)).IsSameAs(wxT(".bmp"), false))
            fmt = 1;
        else if (wxString(fn.substr(fn.size() - 4)).IsSameAs(wxT(".png"), false))
            fmt = 0;
    }

    if (fmt == 0)
        GetPanel()->emusys->emuWritePNG(UTF8(fn));
    else
        GetPanel()->emusys->emuWriteBMP(UTF8(fn));

    wxString msg;
    msg.Printf(_("Wrote snapshot %s"), fn.wc_str());
    systemScreenMessage(msg);
}

// File - Record

void MainFrame::OnRecordSoundStartRecording() {
#ifndef NO_FFMPEG
    static wxString sound_exts;
    static int sound_extno;
    static wxString sound_path;

    if (!sound_exts.size()) {
        sound_extno = -1;
        int extno = 0;

        std::vector<char*> fmts = recording::getSupAudNames();
        std::vector<char*> exts = recording::getSupAudExts();

        for (size_t i = 0; i < fmts.size(); ++i) {
            sound_exts.append(wxString(fmts[i], wxConvLibc));
            sound_exts.append(_(" files ("));
            wxString ext(exts[i], wxConvLibc);
            ext.Replace(wxT(","), wxT(";*."));
            ext.insert(0, wxT("*."));

            if (sound_extno < 0 && ext.find(wxT("*.wav")) != wxString::npos)
                sound_extno = extno;

            sound_exts.append(ext);
            sound_exts.append(wxT(")|"));
            sound_exts.append(ext);
            sound_exts.append(wxT('|'));
            extno++;
        }

        sound_exts.append(wxALL_FILES);

        if (sound_extno < 0)
            sound_extno = extno;
    }

    sound_path = GetGamePath(OPTION(kGenRecordingDir));
    wxString def_name = GetPanel()->game_name();
    wxString extoff = sound_exts;

    for (int i = 0; i < sound_extno; i++) {
        extoff = extoff.Mid(extoff.Find(wxT('|')) + 1);
        extoff = extoff.Mid(extoff.Find(wxT('|')) + 1);
    }

    extoff = extoff.Mid(extoff.Find(wxT('|')) + 2);  // skip *
    def_name += extoff.Left(wxStrcspn(extoff, wxT(";|")));
    wxFileDialog dlg(this, _("Select output file"), sound_path, def_name, sound_exts,
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    dlg.SetFilterIndex(sound_extno);
    int ret = ShowModal(&dlg);
    sound_extno = dlg.GetFilterIndex();
    sound_path = dlg.GetDirectory();

    if (ret != wxID_OK)
        return;

    GetPanel()->StartSoundRecording(dlg.GetPath());
#endif
}

void MainFrame::OnRecordSoundStopRecording() {
#ifndef NO_FFMPEG
    GetPanel()->StopSoundRecording();
#endif
}

void MainFrame::OnRecordAVIStartRecording() {
#ifndef NO_FFMPEG
    static wxString vid_exts;
    static int vid_extno;
    static wxString vid_path;

    if (!vid_exts.size()) {
        vid_extno = -1;
        int extno = 0;

        std::vector<char*> fmts = recording::getSupVidNames();
        std::vector<char*> exts = recording::getSupVidExts();

        for (size_t i = 0; i < fmts.size(); ++i) {
            vid_exts.append(wxString(fmts[i], wxConvLibc));
            vid_exts.append(_(" files ("));
            wxString ext(exts[i], wxConvLibc);
            ext.Replace(wxT(","), wxT(";*."));
            ext.insert(0, wxT("*."));

            if (vid_extno < 0 && ext.find(wxT("*.avi")) != wxString::npos)
                vid_extno = extno;

            vid_exts.append(ext);
            vid_exts.append(wxT(")|"));
            vid_exts.append(ext);
            vid_exts.append(wxT('|'));
            extno++;
        }

        vid_exts.append(wxALL_FILES);

        if (vid_extno < 0)
            vid_extno = extno;
    }

    vid_path = GetGamePath(OPTION(kGenRecordingDir));
    wxString def_name = GetPanel()->game_name();
    wxString extoff = vid_exts;

    for (int i = 0; i < vid_extno; i++) {
        extoff = extoff.Mid(extoff.Find(wxT('|')) + 1);
        extoff = extoff.Mid(extoff.Find(wxT('|')) + 1);
    }

    extoff = extoff.Mid(extoff.Find(wxT('|')) + 2);  // skip *
    def_name += extoff.Left(wxStrcspn(extoff, wxT(";|")));
    wxFileDialog dlg(this, _("Select output file"), vid_path, def_name, vid_exts,
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    dlg.SetFilterIndex(vid_extno);
    int ret = ShowModal(&dlg);
    vid_extno = dlg.GetFilterIndex();
    vid_path = dlg.GetDirectory();

    if (ret != wxID_OK)
        return;

    GetPanel()->StartVidRecording(dlg.GetPath());
#endif
}

void MainFrame::OnRecordAVIStopRecording() {
#ifndef NO_FFMPEG
    GetPanel()->StopVidRecording();
#endif
}

void MainFrame::OnRecordMovieStartRecording() {
    static wxString mov_exts;
    static int mov_extno;
    static wxString mov_path;

    if (!mov_exts.size()) {
        mov_extno = -1;
        int extno = 0;

        std::vector<char*> fmts = getSupMovNamesToRecord();
        std::vector<char*> exts = getSupMovExtsToRecord();

        for (auto&& fmt : fmts) {
            mov_exts.append(wxString(fmt, wxConvLibc));
            mov_exts.append(_(" files ("));
            wxString ext(exts[extno], wxConvLibc);
            ext.Replace(wxT(","), wxT(";*."));
            ext.insert(0, wxT("*."));

            if (mov_extno < 0 && ext.find(wxT("*.vmv")) != wxString::npos)
                mov_extno = extno;

            mov_exts.append(ext);
            mov_exts.append(wxT(")|"));
            mov_exts.append(ext);
            mov_exts.append(wxT('|'));
            extno++;
        }

        mov_exts.append(wxALL_FILES);

        if (mov_extno < 0)
            mov_extno = extno;
    }

    mov_path = GetGamePath(OPTION(kGenRecordingDir));
    wxString def_name = GetPanel()->game_name();
    wxString extoff = mov_exts;

    for (int i = 0; i < mov_extno; i++) {
        extoff = extoff.Mid(extoff.Find(wxT('|')) + 1);
        extoff = extoff.Mid(extoff.Find(wxT('|')) + 1);
    }

    extoff = extoff.Mid(extoff.Find(wxT('|')) + 2);  // skip *
    def_name += extoff.Left(wxStrcspn(extoff, wxT(";|")));
    wxFileDialog dlg(this, _("Select output file"), mov_path, def_name, mov_exts,
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    dlg.SetFilterIndex(mov_extno);
    int ret = ShowModal(&dlg);
    mov_extno = dlg.GetFilterIndex();
    mov_path = dlg.GetDirectory();

    if (ret != wxID_OK)
        return;

    systemStartGameRecording(dlg.GetPath(), getSupMovFormatsToRecord()[mov_extno]);
}

void MainFrame::OnRecordMovieStopRecording() {
    systemStopGameRecording();
}

// File - Playback

void MainFrame::OnPlayMovieStartPlaying() {
    static wxString mov_exts;
    static int mov_extno;
    static wxString mov_path;

    if (!mov_exts.size()) {
        mov_extno = -1;
        int extno = 0;

        std::vector<char*> fmts = getSupMovNamesToPlayback();
        std::vector<char*> exts = getSupMovExtsToPlayback();

        for (size_t i = 0; i < fmts.size(); ++i) {
            mov_exts.append(wxString(fmts[i], wxConvLibc));
            mov_exts.append(_(" files ("));
            wxString ext(exts[i], wxConvLibc);
            ext.Replace(wxT(","), wxT(";*."));
            ext.insert(0, wxT("*."));

            if (mov_extno < 0 && ext.find(wxT("*.vmv")) != wxString::npos)
                mov_extno = extno;

            mov_exts.append(ext);
            mov_exts.append(wxT(")|"));
            mov_exts.append(ext);
            mov_exts.append(wxT('|'));
            extno++;
        }

        mov_exts.append(wxALL_FILES);

        if (mov_extno < 0)
            mov_extno = extno;
    }

    mov_path = GetGamePath(OPTION(kGenRecordingDir));
    systemStopGamePlayback();
    wxString def_name = GetPanel()->game_name();
    wxString extoff = mov_exts;

    for (int i = 0; i < mov_extno; i++) {
        extoff = extoff.Mid(extoff.Find(wxT('|')) + 1);
        extoff = extoff.Mid(extoff.Find(wxT('|')) + 1);
    }

    extoff = extoff.Mid(extoff.Find(wxT('|')) + 2);  // skip *
    def_name += extoff.Left(wxStrcspn(extoff, wxT(";|")));
    wxFileDialog dlg(this, _("Select file"), mov_path, def_name, mov_exts,
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    dlg.SetFilterIndex(mov_extno);
    int ret = ShowModal(&dlg);
    mov_extno = dlg.GetFilterIndex();
    mov_path = dlg.GetDirectory();

    if (ret != wxID_OK)
        return;

    systemStartGamePlayback(dlg.GetPath(), getSupMovFormatsToPlayback()[mov_extno]);
}

void MainFrame::OnPlayMovieStopPlaying() {
    systemStopGamePlayback();
}

// File

void MainFrame::OnClose() {
    GetPanel()->UnloadGame();
}

void MainFrame::OnExit() {
    Close(false);
}
