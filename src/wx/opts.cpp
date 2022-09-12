#include "wx/opts.h"

#include <limits>
#include <memory>
#include <unordered_set>

#include <wx/defs.h>
#include <wx/log.h>
#include <wx/xrc/xmlres.h>

#include "wx/config/bindings.h"
#include "wx/config/command.h"
#include "wx/config/emulated-gamepad.h"
#include "wx/config/handler-id.h"
#include "wx/config/option-observer.h"
#include "wx/config/option-proxy.h"
#include "wx/config/option.h"
#include "wx/config/user-input.h"
#include "wx/wxvbam.h"

namespace {

void SaveOption(config::Option* option) {
    wxConfigBase* cfg = wxConfigBase::Get();

    switch (option->type()) {
        case config::Option::Type::kNone:
            // Keyboard and Joypad are handled separately.
            break;
        case config::Option::Type::kBool:
            cfg->Write(option->config_name(), option->GetBool());
            break;
        case config::Option::Type::kDouble:
            cfg->Write(option->config_name(), option->GetDouble());
            break;
        case config::Option::Type::kInt:
            cfg->Write(option->config_name(), option->GetInt());
            break;
        case config::Option::Type::kUnsigned:
            cfg->Write(option->config_name(), option->GetUnsigned());
            break;
        case config::Option::Type::kString:
            cfg->Write(option->config_name(), option->GetString());
            break;
        case config::Option::Type::kFilter:
        case config::Option::Type::kInterframe:
        case config::Option::Type::kRenderMethod:
        case config::Option::Type::kAudioApi:
        case config::Option::Type::kAudioRate:
            cfg->Write(option->config_name(), option->GetEnumString());
            break;
        case config::Option::Type::kGbPalette:
            cfg->Write(option->config_name(), option->GetGbPaletteString());
            break;
    }
    cfg->Flush();
}

// Intitialize global observers to overwrite the configuration option when the
// option has been modified.
void InitializeOptionObservers() {
    static std::unordered_set<std::unique_ptr<config::OptionsObserver>>
        g_observers;
    g_observers.reserve(config::kNbOptions);
    for (config::Option& option : config::Option::All()) {
        g_observers.emplace(std::make_unique<config::OptionsObserver>(
            option.id(), &SaveOption));
    }
}

// Helper function to work around wxWidgets limitations when converting string
// to unsigned int.
uint32_t LoadUnsignedOption(wxConfigBase* cfg,
                            const wxString& option_name,
                            uint32_t default_value) {
    wxString temp;
    if (!cfg->Read(option_name, &temp)) {
        return default_value;
    }
    if (!temp.IsNumber()) {
        return default_value;
    }

    // Go through ulonglong to get enough space to work with. Also, older
    // versions do not have a conversion function for unsigned int.
    wxULongLong_t out;
    if (!temp.ToULongLong(&out)) {
        return default_value;
    }

    if (out > std::numeric_limits<uint32_t>::max()) {
        return default_value;
    }

    return out;
}

}  // namespace

opts_t gopts;

// This constructor only works with globally allocated gopts.
opts_t::opts_t() {}

// FIXME: simulate MakeInstanceFilename(vbam.ini) using subkeys (Slave%d/*)
void load_opts(bool first_time_launch) {
    // just for sanity...
    static bool did_init = false;
    assert(!did_init);
    did_init = true;

    // enumvals should not be translated, since they would cause config file
    // change after lang change
    // instead, translate when presented to user
    wxConfigBase* cfg = wxConfigBase::Get();
    cfg->SetPath("/");

    // enure there are no unknown options present
    // note that items cannot be deleted until after loop or loop will fail
    wxArrayString item_del, grp_del;
    long group_index;
    wxString group;
    bool cont;

    for (cont = cfg->GetFirstEntry(group, group_index); cont;
         cont = cfg->GetNextEntry(group, group_index)) {
        //wxLogWarning(_("Invalid option %s present; removing if possible"), s.c_str());
        item_del.push_back(group);
    }

    // Read the IniVersion now since the Option initialization code will reset
    // it to kIniLatestVersion if it is unset.
    uint32_t ini_version = 0;
    if (first_time_launch) {
        // Just go with the default values for the first time launch.
        ini_version = config::kIniLatestVersion;
    } else {
        // We want to default to 0 if the option is not set.
        ini_version = LoadUnsignedOption(cfg, "General/IniVersion", 0);
        if (ini_version > config::kIniLatestVersion) {
            wxLogWarning(
                _("The INI file was written for a more recent version of "
                  "VBA-M. Some INI option values may have been reset."));
            ini_version = config::kIniLatestVersion;
        }
    }

    for (cont = cfg->GetFirstGroup(group, group_index); cont;
         cont = cfg->GetNextGroup(group, group_index)) {
        // ignore wxWidgets-managed global library settings
        if (group == "Persistent_Options") {
            continue;
        }

        // ignore file history
        if (group == "Recent") {
            continue;
        }

        cfg->SetPath(group);
        long subgroup_index;
        wxString subgroup;
        for (cont = cfg->GetFirstGroup(subgroup, subgroup_index); cont;
             cont = cfg->GetNextGroup(subgroup, subgroup_index)) {
            // the only one with subgroups
            if (group == "Joypad" && subgroup.size() == 1 && subgroup[0] >= '1' &&
                subgroup[0] <= '4') {
                wxString path = group + '/' + subgroup + '/';
                cfg->SetPath(subgroup);

                wxString dummy_name;
                long dummy_index;
                for (cont = cfg->GetFirstGroup(dummy_name, dummy_index); cont;
                     cont = cfg->GetNextGroup(dummy_name, dummy_index)) {
                    wxString full_path = path + dummy_name;
                    wxLogWarning(_("Invalid option group %s present; removing if possible"),
                                 full_path.c_str());
                    grp_del.push_back(full_path);
                }

                wxString entry_name;
                long entry_index;
                for (cont = cfg->GetFirstEntry(entry_name, entry_index); cont;
                     cont = cfg->GetNextEntry(entry_name, entry_index)) {
                    if (!config::StringToGameKey(entry_name)) {
                        wxString full_path = path + entry_name;
                        wxLogWarning(_("Invalid option %s present; removing if possible"),
                                     full_path.c_str());
                        item_del.push_back(full_path);
                    }
                }

                cfg->SetPath("/");
                cfg->SetPath(group);
            } else {
                // Remove everything else.
                wxString full_path = group + '/' + subgroup;
                wxLogWarning(_("Invalid option group %s present; removing if possible"),
                             full_path.c_str());
                grp_del.push_back(full_path);
            }
        }

        wxString entry;
        long entry_index;
        for (cont = cfg->GetFirstEntry(entry, entry_index); cont;
             cont = cfg->GetNextEntry(entry, entry_index)) {
            const wxString full_path = group + '/' + entry;
            if (group == "Keyboard") {
                const nonstd::optional<config::Command> command =
                    config::Command::FromString(full_path);
                if (!command) {
                    // wxLogWarning(_("Invalid optio%s present; removing if possible"), full_path);
                    item_del.push_back(full_path);
                }
            } else {
                if (!config::Option::ByName(full_path) && full_path != "General/LastUpdated" &&
                    full_path != "General/LastUpdatedFileName") {
                    // wxLogWarning(_("Invalid option %s present; removing if possible"), s);
                    item_del.push_back(group);
                }
            }
        }

        cfg->SetPath("/");
    }

    for (size_t i = 0; i < item_del.size(); i++) {
        cfg->DeleteEntry(item_del[i]);
    }

    for (size_t i = 0; i < grp_del.size(); i++) {
        cfg->DeleteGroup(grp_del[i]);
    }

    // now read actual values and set to default if unset
    // config file will be updated with unset options
    cfg->SetRecordDefaults();

    // Deprecated / moved options handling.
    {
        // The SDL audio API is no longer supported.
        wxString temp;
        if (cfg->Read("Sound/AudioAPI", &temp) && temp == "sdl") {
#if defined(VBAM_ENABLE_XAUDIO2)
            cfg->Write("Sound/AudioAPI", "xaudio2");
#else
            cfg->Write("Sound/AudioAPI", "openal");
#endif
        }
    }

    // First access here will also initialize translations.
    for (config::Option& opt : config::Option::All()) {
        switch (opt.type()) {
        case config::Option::Type::kNone:
            // Keyboard or Joystick. Handled separately for now.
            break;
        case config::Option::Type::kBool: {
            bool temp;
            cfg->Read(opt.config_name(), &temp, opt.GetBool());
            opt.SetBool(temp);
            break;
        }
        case config::Option::Type::kDouble: {
            double temp;
            cfg->Read(opt.config_name(), &temp, opt.GetDouble());
            opt.SetDouble(temp);
            break;
        }
        case config::Option::Type::kInt: {
            int32_t temp;
            cfg->Read(opt.config_name(), &temp, opt.GetInt());
            opt.SetInt(temp);
            break;
        }
        case config::Option::Type::kUnsigned: {
            uint32_t temp =
                LoadUnsignedOption(cfg, opt.config_name(), opt.GetUnsigned());
            opt.SetUnsigned(temp);
            break;
        }
        case config::Option::Type::kString: {
            wxString temp;
            cfg->Read(opt.config_name(),  &temp, opt.GetString());
            opt.SetString(temp);
            break;
        }
        case config::Option::Type::kFilter:
        case config::Option::Type::kInterframe:
        case config::Option::Type::kRenderMethod:
        case config::Option::Type::kAudioApi:
        case config::Option::Type::kAudioRate: {
            wxString temp;
            if (cfg->Read(opt.config_name(), &temp) && !temp.empty()) {
                opt.SetEnumString(temp.MakeLower());
            }
            // This is necessary, in case the option we loaded was invalid.
            cfg->Write(opt.config_name(), opt.GetEnumString());
            break;
        }
        case config::Option::Type::kGbPalette: {
            wxString temp;
            cfg->Read(opt.config_name(), &temp, opt.GetGbPaletteString());
            opt.SetGbPaletteString(temp);
            break;
        }
        }
    }

    config::Bindings* const bindings = wxGetApp().bindings();

    // Read the keyboard entries, which have already been validated above.
    const wxString keyboard = "Keyboard/";
    cfg->SetPath("/Keyboard");
    wxString entry;
    long entry_index;
    for (cont = cfg->GetFirstEntry(entry, entry_index); cont;
         cont = cfg->GetNextEntry(entry, entry_index)) {
        const wxString full_path = keyboard + entry;
        const wxString value = cfg->Read(entry);
        const nonstd::optional<config::Command> command = config::Command::FromString(full_path);

        auto inputs = config::UserInput::FromConfigString(value);
        if (inputs.empty()) {
            wxLogWarning(_("Invalid key binding %s for %s"), value, entry);
        } else {
            for (const auto& input : inputs) {
                bindings->AssignInputToCommand(input, command.value());
            }
        }
    }

    // Overwrite the default Joypad configuration.
    cfg->SetPath("/");
    for (auto& iter : bindings->GetJoypadConfiguration()) {
        const wxString optname = iter.first.ToConfigString();
        if (cfg->Read(optname, &entry)) {
            const auto user_inputs = config::UserInput::FromConfigString(entry);
            if (!entry.empty() && user_inputs.empty()) {
                wxLogWarning(_("Invalid key binding %s for %s"), entry, optname);
            }
            bindings->AssignInputsToCommand(user_inputs, iter.first);
        } else {
            cfg->Write(optname, iter.second);
        }
    }

    cfg->SetPath("/");
    cfg->Flush();

    InitializeOptionObservers();

    // We default the MaxThreads option to 0, so set it to the CPU count here.
    config::OptionProxy<config::OptionID::kDispMaxThreads> max_threads;
    if (max_threads == 0) {
        // Handle erroneous thread count values appropriately.
        const int cpu_count = wxThread::GetCPUCount();
        if (cpu_count > 256) {
            max_threads = 256;
        } else if (cpu_count < 1) {
            max_threads = 1;
        } else {
            max_threads = cpu_count;
        }
    }

    // Apply Option updates.
    while (ini_version < config::kIniLatestVersion) {
        // Update the ini version as we go in case we fail halfway through.
        OPTION(kGenIniVersion) = ini_version;
        switch (ini_version) {
            case 0: { // up to 2.1.5 included.
#ifndef NO_LINK
                // Previous default was 1.
                if (OPTION(kGBALinkTimeout) == 1) {
                    OPTION(kGBALinkTimeout) = 500;
                }
#endif
                // Previous default was true.
                OPTION(kGBALCDFilter) = false;
            }
        }
        ini_version++;
    }

    // Finally, overwrite the value to the current version.
    OPTION(kGenIniVersion) = config::kIniLatestVersion;
}

// Note: run load_opts() first to guarantee all config opts exist
void update_opts() {
    for (config::Option& opt : config::Option::All()) {
        SaveOption(&opt);
    }
}

void update_shortcut_opts() {
    wxConfigBase* cfg = wxConfigBase::Get();

    // For keyboard shortcuts, it's easier to delete everything and start over.
    cfg->DeleteGroup("/Keyboard");
    cfg->SetPath("/Keyboard");
    for (const auto& iter : wxGetApp().bindings()->GetKeyboardConfiguration()) {
        cfg->Write(config::HandlerIDToConfigString(iter.first), iter.second);
    }

    cfg->SetPath("/");

    // For joypads, we just compare the strings.
    bool game_bindings_changed = false;
    for (const auto& iter : wxGetApp().bindings()->GetJoypadConfiguration()) {
        wxString option_name = iter.first.ToConfigString();
        wxString saved_config = cfg->Read(option_name, "");
        if (saved_config != iter.second) {
            game_bindings_changed = true;
            cfg->Write(option_name, iter.second);
        }
    }

    if (game_bindings_changed) {
        wxGetApp().emulated_gamepad()->Reset();
    }

    cfg->Flush();
}

void opt_set(const wxString& name, const wxString& val) {
    config::Option* opt = config::Option::ByName(name);

    // opt->is_none() means it is Keyboard or Joypad.
    if (opt && !opt->is_none()) {
        switch (opt->type()) {
        case config::Option::Type::kNone:
            wxLogWarning(_("Invalid option %s"), name);
            return;
        case config::Option::Type::kBool:
            if (val != '0' && val != '1') {
                wxLogWarning(_("Invalid value %s for option %s"),
                    name.c_str(), val.c_str());
                return;
            }
            opt->SetBool(val == '1');
            return;
        case config::Option::Type::kDouble: {
            double value;
            if (!val.ToDouble(&value)) {
                wxLogWarning(_("Invalid value %s for option %s"), val, name);
                return;
            }
            opt->SetDouble(value);
            return;
        }
        case config::Option::Type::kInt: {
            long value;
            if (!val.ToLong(&value)) {
                wxLogWarning(_("Invalid value %s for option %s"), val, name);
                return;
            }
            opt->SetInt(static_cast<int32_t>(value));
            return;
        }
        case config::Option::Type::kUnsigned: {
            unsigned long value;
            if (!val.ToULong(&value)) {
                wxLogWarning(_("Invalid value %s for option %s"), val, name);
                return;
            }
            opt->SetUnsigned(static_cast<uint32_t>(value));
            return;
        }
        case config::Option::Type::kString:
            opt->SetString(val);
            return;
        case config::Option::Type::kFilter:
        case config::Option::Type::kInterframe:
        case config::Option::Type::kRenderMethod:
        case config::Option::Type::kAudioApi:
        case config::Option::Type::kAudioRate:
            opt->SetEnumString(val);
            return;
        case config::Option::Type::kGbPalette:
            opt->SetGbPaletteString(val);
            return;
        }
    }

    nonstd::optional<config::Command> command = config::Command::FromString(name);
    if (command) {
        config::Bindings* const bindings = wxGetApp().bindings();
        const auto inputs = config::UserInput::FromConfigString(val);
        if (inputs.empty()) {
            wxLogWarning(_("Invalid key binding %s for %s"), val.c_str(), name.c_str());
        }
        bindings->AssignInputsToCommand(inputs, *command);
        return;
    }

    wxLogWarning(_("Unknown option %s with value %s"), name, val);
}
