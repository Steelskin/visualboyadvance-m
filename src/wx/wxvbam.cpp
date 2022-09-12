#include "wx/wxvbam.h"

#include <wx/cmdline.h>
#include <wx/confbase.h>
#include <wx/fs_arc.h>
#include <wx/fs_mem.h>
#include <wx/image.h>
#include <wx/imagxpm.h>
#include <wx/mstream.h>
#include <wx/object.h>
#include <wx/regex.h>
#include <wx/sstream.h>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>
#include <wx/wxcrtvararg.h>
#include <wx/xrc/xmlres.h>

#ifdef __WXMSW__
#include <windows.h>
#endif

#ifdef __WXGTK__
#include <gdk/gdk.h>
#endif

#include "components/user_config/user_config.h"
#include "wx/builtin-over.h"
#include "wx/builtin-xrc.h"
#include "wx/config/bindings.h"
#include "wx/config/emulated-gamepad.h"
#include "wx/config/file-history.h"
#include "wx/config/option-proxy.h"
#include "wx/main-frame.h"
#include "wx/opts.h"
#include "wx/strutils.h"
#include "wx/wayland.h"
#include "wx/widgets/shortcut-menu-bar.h"
#include "wx/widgets/shortcut-menu-item.h"
#include "wx/widgets/utils.h"
#include "wx/widgets/widgets.h"

#if defined(VBAM_ENABLE_DEBUGGER)
#include "core/gba/gbaRemote.h"
#endif  // defined(VBAM_ENABLE_DEBUGGER)

namespace {

static const wxString kOldConfigFileName("vbam.conf");
static const wxString knewConfigFileName("vbam.ini");
static const char kDotDir[] = "visualboyadvance-m";

}  // namespace

#if defined(VBAM_ENABLE_DEBUGGER)
void(*dbgMain)() = remoteStubMain;
void(*dbgSignal)(int, int) = remoteStubSignal;
void(*dbgOutput)(const char *, uint32_t) = debuggerOutput;
#endif  // defined(VBAM_ENABLE_DEBUGGER)

#ifdef __WXMSW__

int __stdcall WinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPSTR lpCmdLine,
                      int nCmdShow) {
    bool console_attached = AttachConsole(ATTACH_PARENT_PROCESS) != FALSE;
#ifdef DEBUG
    // In debug builds, create a console if none is attached.
    if (!console_attached) {
        console_attached = AllocConsole() != FALSE;
    }
#endif  // DEBUG

    // Redirect stdout/stderr to the console if one is attached.
    // This code was taken from Dolphin.
    // https://github.com/dolphin-emu/dolphin/blob/6cf99195c645f54d54c72322ad0312a0e56bc985/Source/Core/DolphinQt/Main.cpp#L112
    HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_attached && stdout_handle) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }

    // Set up logging.
#ifdef DEBUG
    wxLog::SetLogLevel(wxLOG_Trace);
#else   // DEBUG
    wxLog::SetLogLevel(wxLOG_Info);
#endif  // DEBUG

    // Redirect e.g. --help to stderr.
    wxMessageOutput::Set(new wxMessageOutputStderr());

    // This will be freed on wxEntry exit.
    wxApp::SetInstance(new wxvbamApp());
    return wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

#else  // __WXMSW__

int main(int argc, char** argv) {
    // Set up logging.
#ifdef DEBUG
    wxLog::SetLogLevel(wxLOG_Trace);
#else   // DEBUG
    wxLog::SetLogLevel(wxLOG_Info);
#endif  // DEBUG

    // Launch under xwayland on Wayland if EGL is not available.
#if defined(__WXGTK__) && !defined(HAVE_WAYLAND_EGL)
    wxString xdg_session_type = wxGetenv("XDG_SESSION_TYPE");
    wxString wayland_display  = wxGetenv("WAYLAND_DISPLAY");

    if (xdg_session_type == "wayland" || wayland_display.Contains("wayland"))
        gdk_set_allowed_backends("x11,*");
#endif

    // This will be freed on wxEntry exit.
    wxApp::SetInstance(new wxvbamApp());
    return wxEntry(argc, argv);
}

#endif  // __WXMSW__

wxvbamApp& wxGetApp() {
    return *static_cast<wxvbamApp*>(wxApp::GetInstance());
}

#ifndef NO_ONLINEUPDATES
#include "autoupdater/autoupdater.h"
#endif // NO_ONLINEUPDATES

// generate config file path
static void get_config_path(wxPathList& path, bool exists = true)
{
    wxString current_app_name = wxGetApp().GetAppName();

    //   local config dir first, then global
    //   locale-specific res first, then main
    wxStandardPathsBase& stdp = wxStandardPaths::Get();
#define add_path(p)                                                                                          \
    do {                                                                                                     \
        const wxString& s = stdp.p;                                                                          \
        wxFileName parent = wxFileName::DirName(s + wxT("//.."));                                            \
        parent.MakeAbsolute();                                                                               \
        if ((wxDirExists(s) && wxIsWritable(s)) || ((!exists || !wxDirExists(s)) && parent.IsDirWritable())) \
            path.Add(s);                                                                                     \
    } while (0)
#define add_nonstandard_path(p)                                                                              \
    do {                                                                                                     \
        const wxString& s = p;                                                                               \
        wxFileName parent = wxFileName::DirName(s + wxT("//.."));                                            \
        parent.MakeAbsolute();                                                                               \
        if ((wxDirExists(s) && wxIsWritable(s)) || ((!exists || !wxDirExists(s)) && parent.IsDirWritable())) \
            path.Add(s);                                                                                     \
    } while (0)

    static bool debug_dumped = false;

    if (!debug_dumped) {
        wxLogDebug(wxT("GetUserLocalDataDir(): %s"), stdp.GetUserLocalDataDir().c_str());
        wxLogDebug(wxT("GetUserDataDir(): %s"), stdp.GetUserDataDir().c_str());
        wxLogDebug(wxT("GetLocalizedResourcesDir(wxGetApp().locale.GetCanonicalName()): %s"), stdp.GetLocalizedResourcesDir(wxGetApp().locale.GetCanonicalName()).c_str());
        wxLogDebug(wxT("GetResourcesDir(): %s"), stdp.GetResourcesDir().c_str());
        wxLogDebug(wxT("GetDataDir(): %s"), stdp.GetDataDir().c_str());
        wxLogDebug(wxT("GetLocalDataDir(): %s"), stdp.GetLocalDataDir().c_str());
        wxLogDebug(wxT("plugins_dir: %s"), wxGetApp().GetPluginsDir().c_str());
        wxLogDebug(wxT("XdgConfigDir: %s"), (wxString(get_xdg_user_config_home().c_str(), wxConvLibc) + current_app_name).c_str());
        debug_dumped = true;
    }

// When native support for XDG dirs is available (wxWidgets >= 3.1),
// this will be no longer necessary
#if defined(__WXGTK__)
    // XDG spec manual support
    // ${XDG_CONFIG_HOME:-$HOME/.config}/`appname`
    wxString old_config = wxString(getenv("HOME"), wxConvLibc) + kFileSep + ".vbam";
    wxString new_config(get_xdg_user_config_home().c_str(), wxConvLibc);
    if (!wxDirExists(old_config) && wxIsWritable(new_config))
    {
        wxFileName new_path(new_config, wxEmptyString);
        new_path.AppendDir(current_app_name);
        new_path.MakeAbsolute();

        add_nonstandard_path(new_path.GetFullPath());
    }
    else
    {
        // config is in $HOME/.vbam/
        add_nonstandard_path(old_config);
    }
#endif

    // NOTE: this does not support XDG (freedesktop.org) paths
    add_path(GetUserLocalDataDir());
    add_path(GetUserDataDir());
    add_path(GetLocalizedResourcesDir(wxGetApp().locale.GetCanonicalName()));
    add_path(GetResourcesDir());
    add_path(GetDataDir());
    add_path(GetLocalDataDir());
    add_nonstandard_path(wxGetApp().GetPluginsDir());
}

static void tack_full_path(wxString& s, const wxString& app = wxEmptyString)
{
    // regenerate path, including nonexistent dirs this time
    wxPathList full_config_path;
    get_config_path(full_config_path, false);

    for (size_t i = 0; i < full_config_path.size(); i++)
        s += wxT("\n\t") + full_config_path[i] + app;
}

wxvbamApp::wxvbamApp()
    : wxApp(),
      pending_fullscreen(false),
      frame(nullptr),
      using_wayland(false),
      bindings_(std::make_unique<config::Bindings>()),
      emulated_gamepad_(std::make_unique<config::EmulatedGamepad>(this)),
      file_history_(std::make_unique<config::FileHistory>()),
      sdl_poller_(std::bind(&wxvbamApp::GetJoyEventHandler, this)) {}

const wxString wxvbamApp::GetPluginsDir()
{
    return wxStandardPaths::Get().GetPluginsDir();
}

wxString wxvbamApp::GetConfigurationPath() {
    // first check if config files exists in reverse order
    // (from system paths to more local paths.)
    if (data_path.empty()) {
        get_config_path(config_path);

        for (int i = config_path.size() - 1; i >= 0; i--) {
            wxFileName fn(config_path[i], knewConfigFileName);

            if (fn.FileExists() && fn.IsFileWritable()) {
                data_path = config_path[i];
                break;
            }
        }
    }

    // if no config file was not found, search for writable config
    // dir or parent to create it in in OnInit in normal order
    // (from user paths to system paths.)
    if (data_path.empty()) {
        for (size_t i = 0; i < config_path.size(); i++) {
            // Check if path is writeable
            if (wxIsWritable(config_path[i])) {
                data_path = config_path[i];
                break;
            }

            // check if parent of path is writable, so we can
            // create the path in OnInit
            wxFileName parent_dir = wxFileName::DirName(config_path[i] + wxT("//.."));
            parent_dir.MakeAbsolute();

            if (parent_dir.IsDirWritable()) {
                data_path = config_path[i];
                break;
            }
        }
    }

    return data_path;
}

wxString wxvbamApp::GetAbsolutePath(wxString path)
{
    wxFileName fn(path);

    if (fn.IsRelative()) {
        fn.MakeRelativeTo(GetConfigurationPath());
        fn.Normalize(wxPATH_NORM_ENV_VARS | wxPATH_NORM_DOTS | wxPATH_NORM_TILDE |
                     wxPATH_NORM_CASE | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG |
                     wxPATH_NORM_SHORTCUT);
        return fn.GetFullPath();
    }

    return path;
}

bool wxvbamApp::OnInit() {
    using_wayland = IsWayland();

    // use consistent names for config, DO NOT TRANSLATE
    SetAppName("visualboyadvance-m");
#if (wxMAJOR_VERSION >= 3)
    SetAppDisplayName("VisualBoyAdvance-M");
#endif
    // load system default locale, if available
    locale.Init();
    locale.AddCatalog("wxvbam");
    // make built-in xrc file available
    // this has to be done before parent OnInit() so xrc dump works
    wxFileSystem::AddHandler(new wxMemoryFSHandler);
    wxFileSystem::AddHandler(new wxArchiveFSHandler);
    wxMemoryFSHandler::AddFileWithMimeType(wxT("wxvbam.xrs"), builtin_xrs, sizeof(builtin_xrs), wxT("application/zip"));

    if (!wxApp::OnInit())
        return false;

    if (console_mode)
        return true;

    // prepare for loading xrc files
    wxXmlResource* xr = widgets::InitializeXmlHandlers();
    wxImage::AddHandler(new wxXPMHandler());

    get_config_path(config_path);
    // first, load override xrcs
    // this can only override entire root nodes
    // 2.9 has LoadAllFiles(), but this is 2.8, so we'll do it manually
    wxString cwd = wxGetCwd();

    for (size_t i = 0; i < config_path.size(); i++)
        if (wxDirExists(config_path[i]) && wxSetWorkingDirectory(config_path[i])) {
            // *.xr[cs] doesn't work (double the number of scans)
            // 2.9 gives errors for no files found, so manual precheck needed
            // (yet another double the number of scans)
            if (!wxFindFirstFile(wxT("*.xrc")).empty())
                xr->Load(wxT("*.xrc"));

            if (!wxFindFirstFile(wxT("*.xrs")).empty())
                xr->Load(wxT("*.xrs"));
        }

    wxFileName xrcDir(GetConfigurationPath() + wxT("//xrc"), wxEmptyString);

    if (xrcDir.DirExists() && wxSetWorkingDirectory(xrcDir.GetFullPath()) && !wxFindFirstFile(wxT("*.xrc")).empty()) {
        xr->Load(wxT("*.xrc"));
    } else {
        // finally, load built-in xrc
        xr->Load(wxT("memory:wxvbam.xrs"));
    }

    wxSetWorkingDirectory(cwd);

    if (!config_file_.IsOk()) {
        // Set up the default configuration file.
        // This needs to be in a subdir to support other config as well.
        // NOTE: this does not support XDG (freedesktop.org) paths.
        // We rely on wx to build the paths in a cross-platform manner. However,
        // the wxFileName APIs are weird and don't quite work as intended so we
        // use the wxString APIs for files instead.
        const wxString old_conf_file(
            wxFileName(GetConfigurationPath(), kOldConfigFileName)
                .GetFullPath());
        const wxString new_conf_file(
            wxFileName(GetConfigurationPath(), knewConfigFileName)
                .GetFullPath());

        if (wxDirExists(new_conf_file)) {
            wxLogError(_("Invalid configuration file provided: %s"),
                       new_conf_file);
            return false;
        }

        // /MIGRATION
        // Migrate from 'vbam.conf' to 'vbam.ini' to manage a single config
        // file for all platforms.
        if (!wxFileExists(new_conf_file) && wxFileExists(old_conf_file)) {
            wxRenameFile(old_conf_file, new_conf_file, false);
        }
        // /END_MIGRATION

        config_file_ = new_conf_file;
    }

    if (!config_file_.IsOk() || wxDirExists(config_file_.GetFullPath())) {
        wxLogError(_("Invalid configuration file provided: %s"),
                   config_file_.GetFullPath());
        return false;
    }

    // wx takes ownership of the wxFileConfig here. It will be deleted on app
    // destruction.
    wxConfigBase::DontCreateOnDemand();
    wxConfigBase::Set(new wxFileConfig("vbam", wxEmptyString,
                                       config_file_.GetFullPath(),
                                       wxEmptyString, wxCONFIG_USE_LOCAL_FILE));

    // wx does not create the directories by itself so do it here, if needed.
    if (!wxDirExists(config_file_.GetPath())) {
       config_file_.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }

    // Load the default options.
    load_opts(!config_file_.Exists());

    // Load the FileHistory.
    file_history_->LoadConfig();

    // wxGLCanvas segfaults under wayland before wx 3.2
#if defined(HAVE_WAYLAND_SUPPORT) && !defined(HAVE_WAYLAND_EGL)
    if (UsingWayland()) {
        OPTION(kDispRenderMethod) = config::RenderMethod::kSimple;
    }
#endif

    // process command-line options
    for (size_t i = 0; i < pending_optset.size(); i++) {
        auto parts = strutils::split(pending_optset[i], wxT('='));
        opt_set(parts[0], parts[1]);
    }

    pending_optset.clear();
    wxFileName vba_over(GetConfigurationPath(), wxT("vba-over.ini"));
    wxFileName rdb(GetConfigurationPath(), wxT("Nintendo - Game Boy Advance*.dat"));
    wxFileName scene_rdb(GetConfigurationPath(), wxT("Nintendo - Game Boy Advance (Scene)*.dat"));
    wxFileName nointro_rdb(GetConfigurationPath(), wxT("Official No-Intro Nintendo Gameboy Advance Number (Date).xml"));
    wxString f = wxFindFirstFile(nointro_rdb.GetFullPath(), wxFILE);

    if (!f.empty() && wxFileName(f).IsFileReadable())
        rom_database_nointro = f;

    f = wxFindFirstFile(scene_rdb.GetFullPath(), wxFILE);

    if (!f.empty() && wxFileName(f).IsFileReadable())
        rom_database_scene = f;

    f = wxFindFirstFile(rdb.GetFullPath(), wxFILE);

    while (!f.empty()) {
        if (f == rom_database_scene.GetFullPath()) {
            f = wxFindNextFile();
        } else if (wxFileName(f).IsFileReadable()) {
            rom_database = f;
            break;
        }
    }

    // load vba-over.ini
    // rather than dealing with wxConfig's broken search path, just use
    // the same one that the xrc overrides use
    // this also allows us to override a group at a time, add commments, and
    // add the file from which the group came
    wxMemoryInputStream mis(builtin_over, sizeof(builtin_over));
    overrides = new wxFileConfig(mis);
    wxRegEx cmtre;
    // not the most efficient thing to do: read entire file into a string
    // just to parse the comments out
    wxString bovs((const char*)builtin_over, wxConvUTF8, sizeof(builtin_over));
    bool cont;
    wxString s;
    long grp_idx;
#define CMT_RE_START wxT("(^|[\n\r])# ?([^\n\r]*)(\r?\n|\r)\\[")

    for (cont = overrides->GetFirstGroup(s, grp_idx); cont;
         cont = overrides->GetNextGroup(s, grp_idx)) {
        // apparently even MacOSX sometimes uses the old \r by itself
        wxString cmt(CMT_RE_START);
        cmt += s + wxT("\\]");

        if (cmtre.Compile(cmt) && cmtre.Matches(bovs))
            cmt = cmtre.GetMatch(bovs, 2);
        else
            cmt = wxEmptyString;

        overrides->Write(s + wxT("/comment"), cmt);
    }

    if (vba_over.FileExists()) {
        wxStringOutputStream sos;
        wxFileInputStream fis(vba_over.GetFullPath());
        // not the most efficient thing to do: read entire file into a string
        // just to parse the comments out
        fis.Read(sos);
        // rather than assuming the file is seekable, use the string we just
        // read as an input stream
        wxStringInputStream sis(sos.GetString());
        wxFileConfig ov(sis);

        for (cont = ov.GetFirstGroup(s, grp_idx); cont;
             cont = ov.GetNextGroup(s, grp_idx)) {
            overrides->DeleteGroup(s);
            overrides->SetPath(s);
            ov.SetPath(s);
            overrides->Write(wxT("path"), GetConfigurationPath());
            // apparently even MacOSX sometimes uses \r by itself
            wxString cmt(CMT_RE_START);
            cmt += s + wxT("\\]");

            if (cmtre.Compile(cmt) && cmtre.Matches(sos.GetString()))
                cmt = cmtre.GetMatch(sos.GetString(), 2);
            else
                cmt = wxEmptyString;

            overrides->Write(wxT("comment"), cmt);
            long ent_idx;

            for (cont = ov.GetFirstEntry(s, ent_idx); cont;
                 cont = ov.GetNextEntry(s, ent_idx))
                overrides->Write(s, ov.Read(s, wxEmptyString));

            ov.SetPath(wxT("/"));
            overrides->SetPath(wxT("/"));
        }
    }

    // We need to gather this information before crating the MainFrame as the
    // OnSize / OnMove event handlers can fire during construction.
    const wxRect client_rect(
        OPTION(kGeomWindowX).Get(),
        OPTION(kGeomWindowY).Get(),
        OPTION(kGeomWindowWidth).Get(),
        OPTION(kGeomWindowHeight).Get());
    const bool is_fullscreen = OPTION(kGeomFullScreen);
    const bool is_maximized = OPTION(kGeomIsMaximized);

    // Create the main window.
    frame = wxDynamicCast(xr->LoadFrame(nullptr, "MainFrame"), MainFrame);
    if (!frame) {
        wxLogError(_("Could not create main window"));
        return false;
    }

    // Create() cannot be overridden easily
    if (!frame->BindControls()) {
        return false;
    }

    // Ensure we are not drawing out of bounds.
    if (widgets::GetDisplayRect().Intersects(client_rect)) {
        frame->SetSize(client_rect);
    }

    if (is_maximized) {
        frame->Maximize();
    }
    if (is_fullscreen && wxGetApp().pending_load != wxEmptyString)
        frame->ShowFullScreen(is_fullscreen);

    frame->Show(true);

#ifndef NO_ONLINEUPDATES
    initAutoupdater();
#endif
    return true;
}

int wxvbamApp::OnRun()
{
    if (console_mode)
    {
        // we could check for our own error codes here...
        return console_status;
    }
    else
    {
        return wxApp::OnRun();
    }
}

// called on --help
bool wxvbamApp::OnCmdLineHelp(wxCmdLineParser& parser)
{
    wxApp::OnCmdLineHelp(parser);
    console_mode = true;
    return true;
}

bool wxvbamApp::OnCmdLineError(wxCmdLineParser& parser)
{
    wxApp::OnCmdLineError(parser);
    console_mode = true;
    console_status = 1;
    return true;
}

void wxvbamApp::OnInitCmdLine(wxCmdLineParser& cl)
{
    wxApp::OnInitCmdLine(cl);
    cl.SetLogo(wxT("VisualBoyAdvance-M\n"));
// 2.9 decided to change all of these from wxChar to char for some
// reason
#if wxCHECK_VERSION(2, 9, 0)
// N_(x) returns x
#define t(x) x
#else
// N_(x) returns wxT(x)
#define t(x) wxT(x)
#endif
    // while I would rather the long options be translated, there is merit
    // to the idea that command-line syntax should not change based on
    // locale
    static wxCmdLineEntryDesc opttab[] = {
        { wxCMD_LINE_OPTION, NULL, t("save-xrc"),
            N_("Save built-in XRC file and exit"),
            wxCMD_LINE_VAL_STRING, 0 },
        { wxCMD_LINE_OPTION, NULL, t("save-over"),
            N_("Save built-in vba-over.ini and exit"),
            wxCMD_LINE_VAL_STRING, 0 },
        { wxCMD_LINE_SWITCH, NULL, t("print-cfg-path"),
            N_("Print configuration path and exit"),
            wxCMD_LINE_VAL_NONE, 0 },
        { wxCMD_LINE_SWITCH, t("f"), t("fullscreen"),
            N_("Start in full-screen mode"),
            wxCMD_LINE_VAL_NONE, 0 },
        { wxCMD_LINE_OPTION, t("c"), t("config"),
            N_("Set a configuration file"),
            wxCMD_LINE_VAL_STRING, 0 },
#if !defined(NO_LINK) && !defined(__WXMSW__)
        { wxCMD_LINE_SWITCH, t("s"), t("delete-shared-state"),
            N_("Delete shared link state first, if it exists"),
            wxCMD_LINE_VAL_NONE, 0 },
#endif
        // stupid wx cmd line parser doesn't support duplicate options
        //    { wxCMD_LINE_OPTION, t("o"),  t("option"),
        //        _("Set configuration option; <opt>=<value> or help for list"),
        { wxCMD_LINE_SWITCH, t("o"), t("list-options"),
            N_("List all settable options and exit"),
            wxCMD_LINE_VAL_NONE, 0 },
        { wxCMD_LINE_PARAM, NULL, NULL,
            N_("ROM file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
        { wxCMD_LINE_PARAM, NULL, NULL,
            N_("<config>=<value>"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_MULTIPLE | wxCMD_LINE_PARAM_OPTIONAL },
        { wxCMD_LINE_NONE, NULL, NULL, NULL, wxCMD_LINE_VAL_NONE, 0 }
    };
// 2.9 automatically translates desc, but 2.8 doesn't
#if !wxCHECK_VERSION(2, 9, 0)

    for (int i = 0; opttab[i].kind != wxCMD_LINE_NONE; i++)
        opttab[i].description = wxGetTranslation(opttab[i].description);

#endif
    // note that "SetDesc" actually Adds rather than replaces
    // so system standard (port-dependent) options are still included:
    //   -h/--help  --verbose --theme --mode
    cl.SetDesc(opttab);
}

bool wxvbamApp::OnCmdLineParsed(wxCmdLineParser& cl)
{
    if (!wxApp::OnCmdLineParsed(cl))
        return false;

    wxString s;

    if (cl.Found(wxT("save-xrc"), &s)) {
        // This was most likely done on a command line, so use
        // stderr instead of gui for messages
        wxLog::SetActiveTarget(new wxLogStderr);
        wxFileSystem fs;
        wxFSFile* f = fs.OpenFile(wxT("memory:wxvbam.xrs#zip:wxvbam.xrs$wxvbam.xrc"));

        if (!f) {
            wxLogError(_("Configuration / build error: can't find built-in xrc"));
            return false;
        }

        wxFileOutputStream os(s);
        os.Write(*f->GetStream());
        delete f;
        wxString lm;
        lm.Printf(_("Wrote built-in configuration to %s.\n"
                    "To override, remove all but changed root node(s). First found root node of correct name in any ."
                    "xrc or .xrs files in following search path overrides built-in:"),
            s.c_str());
        tack_full_path(lm);
        wxLogMessage(lm);
        console_mode = true;
        return true;
    }

    if (cl.Found(wxT("print-cfg-path"))) {
        // This was most likely done on a command line, so use
        // stderr instead of gui for messages
        wxLog::SetActiveTarget(new wxLogStderr);
        wxString lm(_("Configuration is read from, in order:"));
        tack_full_path(lm);
        wxLogMessage(lm);
        console_mode = true;
        return true;
    }

    if (cl.Found(wxT("save-over"), &s)) {
        // This was most likely done on a command line, so use
        // stderr instead of gui for messages
        wxLog::SetActiveTarget(new wxLogStderr);
        wxFileOutputStream os(s);
        os.Write(builtin_over, sizeof(builtin_over));
        wxString lm;
        lm.Printf(_("Wrote built-in override file to %s\n"
                    "To override, delete all but changed section. First found section is used from search path:"),
            s.c_str());
        wxString oi = wxFileName::GetPathSeparator();
        oi += wxT("vba-over.ini");
        tack_full_path(lm, oi);
        lm.append(_("\n\tbuilt-in"));
        wxLogMessage(lm);
        console_mode = true;
        return true;
    }

    if (cl.Found(wxT("f"))) {
        pending_fullscreen = true;
    }

    if (cl.Found(wxT("o"))) {
        // This was most likely done on a command line, so use
        // stderr instead of gui for messages
        wxLog::SetActiveTarget(new wxLogStderr);

        wxPrintf(_("Options set from the command line are saved if any"
                   " configuration changes are made in the user interface.\n\n"
                   "For flag options, true and false are specified as 1 and 0, respectively.\n\n"));

        for (const config::Option& opt : config::Option::All()) {
            wxPrintf("%s\n", opt.ToHelperString());
        }

        // We need to load the menu bar to do this.

        wxPrintf(_("The commands available for the Keyboard/* option are:\n\n"));
        widgets::ShortcutMenuBar* menu_bar = wxDynamicCast(
            wxXmlResource::Get()->LoadObject(nullptr, "MainMenu",
                                             widgets::ShortcutMenuBar::ms_classInfo.GetClassName()),
            widgets::ShortcutMenuBar);
        for (const auto& item : menu_bar->item_map()) {
            wxPrintf("%s (%s)\n", HandlerIDToConfigString(item.first), item.second->GetItemLabel());
        }

        [[maybe_unused]] const bool success = menu_bar->Destroy();
        assert(success);
        menu_bar = nullptr;

        console_mode = true;
        return true;
    }

    if (cl.Found(wxT("c"), &s)) {
        wxFileName vbamconf(s);
        if (!vbamconf.FileExists()) {
            wxLogError(_("Configuration file not found."));
            return false;
        }
        config_file_ = s;
    }

#if !defined(NO_LINK) && !defined(__WXMSW__)

    if (cl.Found(wxT("s"))) {
        CleanLocalLink();
    }

#endif
    int nparm = cl.GetParamCount();
    bool complained = false, gotfile = false;

    for (int i = 0; i < nparm; i++) {
        auto p     = cl.GetParam(i);
        auto parts = strutils::split(p, wxT('='));

        if (parts.size() > 1) {
            opt_set(parts[0], parts[1]);

            pending_optset.push_back(p);
        }
        else {
            if (!gotfile) {
                pending_load = p;
                gotfile = true;
            } else {
                if (!complained) {
                    wxFprintf(stderr, _("Bad configuration option or multiple ROM files given:\n"));
                    wxFprintf(stderr, wxT("%s\n"), pending_load.c_str());
                    complained = true;
                }

                wxFprintf(stderr, wxT("%s\n"), p.c_str());
            }
        }
    }

    return true;
}

wxString wxvbamApp::GetConfigDir()
{
    return GetAbsolutePath(wxString((get_xdg_user_config_home() + kDotDir).c_str(), wxConvLibc));
}

wxString wxvbamApp::GetDataDir()
{
    return GetAbsolutePath(wxString((get_xdg_user_data_home() + kDotDir).c_str(), wxConvLibc));
}

wxvbamApp::~wxvbamApp() {
    if (home != NULL)
    {
        free(home);
        home = NULL;
    }
    delete overrides;

#ifndef NO_ONLINEUPDATES
    shutdownAutoupdater();
#endif
}

wxEvtHandler* wxvbamApp::GetJoyEventHandler() {
    // Use the active window, if any.
    wxWindow* focused_window = wxWindow::FindFocus();
    if (focused_window) {
        return focused_window;
    }

    if (!frame) {
        return nullptr;
    }

    auto panel = frame->GetPanel();
    if (!panel || !panel->panel) {
        return nullptr;
    }

    if (OPTION(kUIAllowJoystickBackgroundInput)) {
        // Use the game panel, if the background polling option is enabled.
        return panel->panel->GetWindow()->GetEventHandler();
    }

    return nullptr;
}

// global event filter
// apparently required for win32; just setting accel table still misses
// a few keys (e.g. only ctrl-x works for exit, but not esc & ctrl-q;
// ctrl-w does not work for close).  It's possible another entity is
// grabbing those keys, but I can't track it down.
int wxvbamApp::FilterEvent(wxEvent& event)
{
    if (frame) {
        return frame->FilterEvent(event);
    }
    return wxApp::FilterEvent(event);
}
