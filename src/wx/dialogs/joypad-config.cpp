#include "wx/dialogs/joypad-config.h"

#include <wx/checkbox.h>

#include "wx/config/bindings.h"
#include "wx/config/command.h"
#include "wx/config/config-provider.h"
#include "wx/config/option-proxy.h"
#include "wx/dialogs/base-dialog.h"
#include "wx/widgets/client-data.h"
#include "wx/widgets/option-validator.h"
#include "wx/widgets/user-input-ctrl.h"
#include "wx/widgets/utils.h"

namespace dialogs {

namespace {

using GameJoyClientData = widgets::ClientData<config::GameJoy>;

// A validator for the UserInputCtrl. This validator is used to transfer the
// GameControl data to and from the UserInputCtrl. `config_provider` must
// outlive this object.
class UserInputCtrlValidator : public wxValidator {
public:
    explicit UserInputCtrlValidator(const config::GameCommand game_control,
                                    config::ConfigProvider* const config_provider);
    ~UserInputCtrlValidator() override = default;

    wxObject* Clone() const override;

protected:
    // wxValidator implementation.
    bool TransferToWindow() override;
    bool TransferFromWindow() override;
    bool Validate(wxWindow*) override { return true; }

    const config::GameCommand game_control_;
    config::ConfigProvider* const config_provider_;
};

UserInputCtrlValidator::UserInputCtrlValidator(const config::GameCommand game_control,
                                               config::ConfigProvider* const config_provider)
    : wxValidator(), game_control_(game_control), config_provider_(config_provider) {
    assert(config_provider);
}

wxObject* UserInputCtrlValidator::Clone() const {
    return new UserInputCtrlValidator(game_control_, config_provider_);
}

bool UserInputCtrlValidator::TransferToWindow() {
    widgets::UserInputCtrl* control = wxDynamicCast(GetWindow(), widgets::UserInputCtrl);
    assert(control);

    control->SetInputs(
        config_provider_->bindings()->InputsForCommand(config::Command(game_control_)));
    return true;
}

bool UserInputCtrlValidator::TransferFromWindow() {
    widgets::UserInputCtrl* control = wxDynamicCast(GetWindow(), widgets::UserInputCtrl);
    assert(control);

    config_provider_->bindings()->ClearCommandAssignments(config::Command(game_control_));
    for (const auto& input : control->inputs()) {
        config_provider_->bindings()->AssignInputToCommand(input, config::Command(game_control_));
    }

    return true;
}

}  // namespace

// static
JoypadConfig* JoypadConfig::NewInstance(wxWindow* parent,
                                        config::ConfigProvider* const config_provider) {
    assert(parent);
    assert(config_provider);
    return new JoypadConfig(parent, config_provider);
}

JoypadConfig::JoypadConfig(wxWindow* parent, config::ConfigProvider* const config_provider)
    : BaseDialog(parent, "JoypadConfig"), config_provider_(config_provider) {
    this->Bind(wxEVT_CHECKBOX, std::bind(&JoypadConfig::ToggleSDLGameControllerMode, this),
               XRCID("SDLGameControllerMode"));

    GetValidatedChild<wxCheckBox>("SDLGameControllerMode")
        ->SetValue(OPTION(kSDLGameControllerMode));

    for (const config::GameJoy& joypad : config::kAllGameJoys) {
        wxWindow* panel = GetValidatedChild(wxString::Format("joy%zu", joypad.ux_index()));
        panel->SetClientObject(new GameJoyClientData(joypad));

        widgets::GetValidatedChild(panel, "DefaultConfig")
            ->SetValidator(
                widgets::OptionSelectedValidator(config::OptionID::kJoyDefault, joypad.ux_index()));

        // Set up tab order so input is easy to configure. Note that there are
        // two tabs for each panel, so we must check for the parent before
        // setting up the tab order.
        wxWindow* prev = nullptr;
        wxWindow* prev_parent = nullptr;
        for (const config::GameKey& game_key : config::kAllGameKeys) {
            const wxString game_key_name = config::GameKeyToString(game_key);
            widgets::UserInputCtrl* game_key_control =
                widgets::GetValidatedChild<widgets::UserInputCtrl>(panel, game_key_name);
            wxWindow* current_parent = game_key_control->GetParent();

            game_key_control->SetValidator(
                UserInputCtrlValidator(config::GameCommand(joypad, game_key), config_provider));

            if (current_parent == prev_parent) {
                // The first control will be skipped here, but that's fine since
                // we don't care where it fits in the tab order.
                assert(prev);
                game_key_control->MoveAfterInTabOrder(prev);
            }
            prev = game_key_control;
            prev_parent = current_parent;

            // Bind the individual "Clear" key event.
            panel->Bind(wxEVT_BUTTON, std::bind(&widgets::UserInputCtrl::Clear, game_key_control),
                        XRCID(wxString("Clear" + config::GameKeyToString(game_key)).c_str()));
        }

        // Finally, bind the per-joypad "Defaults" and "Clear" events.
        panel->Bind(wxEVT_BUTTON, std::bind(&JoypadConfig::ResetToDefaults, this, panel),
                    XRCID("Defaults"));
        panel->Bind(wxEVT_BUTTON, std::bind(&JoypadConfig::ClearJoypad, this, panel),
                    XRCID("Clear"));
    }

    this->Fit();
}

void JoypadConfig::ResetToDefaults(wxWindow* panel) {
    const config::GameJoy& joypad = GameJoyClientData::From(panel);
    for (const config::GameKey& game_key : config::kAllGameKeys) {
        widgets::GetValidatedChild<widgets::UserInputCtrl>(panel, config::GameKeyToString(game_key))
            ->SetInputs(config_provider_->bindings()->DefaultInputsForCommand(
                config::GameCommand(joypad, game_key)));
    }
}

void JoypadConfig::ClearJoypad(wxWindow* panel) {
    for (const config::GameKey& game_key : config::kAllGameKeys) {
        widgets::GetValidatedChild<widgets::UserInputCtrl>(panel, config::GameKeyToString(game_key))
            ->Clear();
    }
}

void JoypadConfig::ToggleSDLGameControllerMode() {
    OPTION(kSDLGameControllerMode) =
        GetValidatedChild<wxCheckBox>("SDLGameControllerMode")->IsChecked();
    ClearAllJoypads();
}

void JoypadConfig::ClearAllJoypads() {
    for (unsigned joypad = 0; joypad < 4; joypad++) {
        wxWindow* panel = GetValidatedChild(wxString::Format("joy%d", joypad + 1));

        ClearJoypad(panel);
    }
}

}  // namespace dialogs
