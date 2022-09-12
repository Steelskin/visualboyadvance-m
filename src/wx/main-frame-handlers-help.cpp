#include "wx/main-frame.h"

#include <wx/aboutdlg.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>

#include "core/base/version.h"

#ifndef NO_ONLINEUPDATES
#include "autoupdater/autoupdater.h"
#endif  // NO_ONLINEUPDATES

// Help

void MainFrame::OnBugReport() {
    wxLaunchDefaultBrowser("https://github.com/visualboyadvance-m/visualboyadvance-m/issues");
}

void MainFrame::OnFAQ() {
    wxLaunchDefaultBrowser("https://github.com/visualboyadvance-m/visualboyadvance-m/");
}

void MainFrame::OnTranslate() {
    wxLaunchDefaultBrowser("http://www.transifex.com/projects/p/vba-m");
}

void MainFrame::OnUpdateEmu() {
#ifndef NO_ONLINEUPDATES
    checkUpdatesUi();
#endif  // NO_ONLINEUPDATES
}

void MainFrame::OnFactoryReset() {
    wxMessageDialog dlg(nullptr, _("YOUR CONFIGURATION WILL BE DELETED!\n\nAre you sure?"),
                        _("FACTORY RESET"), wxYES_NO | wxNO_DEFAULT | wxCENTRE);

    if (dlg.ShowModal() == wxID_YES) {
        wxConfigBase::Get()->DeleteAll();
        wxExecute(wxStandardPaths::Get().GetExecutablePath(), wxEXEC_ASYNC);
        Close(true);
    }
}

void MainFrame::OnAbout() {
    wxAboutDialogInfo ai;
    ai.SetName("VisualBoyAdvance-M");
    wxString version(kVbamVersion);
    ai.SetVersion(version);
    // setting website, icon, license uses custom aboutbox on win32 & macosx
    // but at least win32 standard about is nothing special
    ai.SetWebSite("http://visualboyadvance-m.org/");
    ai.SetIcon(GetIcons().GetIcon(wxSize(32, 32), wxIconBundle::FALLBACK_NEAREST_LARGER));
    ai.SetDescription(_("Nintendo Game Boy / Color / Advance emulator."));
    ai.SetCopyright(
        _("Copyright (C) 1999-2003 Forgotten\nCopyright (C) 2004-2006 VBA development "
          "team\nCopyright (C) 2007-2020 VBA-M development team"));
    ai.SetLicense(
        _("This program is free software: you can redistribute it and / or modify\n"
          "it under the terms of the GNU General Public License as published by\n"
          "the Free Software Foundation, either version 2 of the License, or\n"
          "(at your option) any later version.\n"
          "\n"
          "This program is distributed in the hope that it will be useful,\n"
          "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
          "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
          "GNU General Public License for more details.\n"
          "\n"
          "You should have received a copy of the GNU General Public License\n"
          "along with this program. If not, see http://www.gnu.org/licenses ."));
    // from gtk
    ai.AddDeveloper(wxT("Forgotten"));
    ai.AddDeveloper(wxT("kxu"));
    ai.AddDeveloper(wxT("Pokemonhacker"));
    ai.AddDeveloper(wxT("Spacy51"));
    ai.AddDeveloper(wxT("mudlord"));
    ai.AddDeveloper(wxT("Nach"));
    ai.AddDeveloper(wxT("jbo_85"));
    ai.AddDeveloper(wxT("bgK"));
    ai.AddArtist(wxT("Matteo Drera"));
    ai.AddArtist(wxT("Jakub Steiner"));
    ai.AddArtist(wxT("Jones Lee"));
    // from win32
    ai.AddDeveloper(wxT("Jonas Quinn"));
    ai.AddDeveloper(wxT("DJRobX"));
    ai.AddDeveloper(wxT("Spacy"));
    ai.AddDeveloper(wxT("Squall Leonhart"));
    // wx
    ai.AddDeveloper(wxT("Thomas J. Moore"));
    // from win32 "thanks"
    ai.AddDeveloper(wxT("blargg"));
    ai.AddDeveloper(wxT("Costis"));
    ai.AddDeveloper(wxT("chrono"));
    ai.AddDeveloper(wxT("xKiv"));
    ai.AddDeveloper(wxT("skidau"));
    ai.AddDeveloper(wxT("TheCanadianBacon"));
    ai.AddDeveloper(wxT("rkitover"));
    ai.AddDeveloper(wxT("Mystro256"));
    ai.AddDeveloper(wxT("retro-wertz"));
    ai.AddDeveloper(wxT("denisfa"));
    ai.AddDeveloper(wxT("orbea"));
    ai.AddDeveloper(wxT("Orig. VBA team"));
    ai.AddDeveloper(wxT("... many contributors who send us patches/PRs"));
    wxAboutBox(ai);
}
