#include "dialogs/cheat-edit.h"

#include <wx/choice.h>
#include <wx/msgdlg.h>
#include <wx/textctrl.h>
#include <wx/valgen.h>
#include <wx/validate.h>
#include <wx/xrc/xmlres.h>

#include "dialogs/validated-child.h"
#include "wx/string.h"

namespace dialogs {

namespace {

class CheatTypeClientData : public wxClientData {
public:
    explicit CheatTypeClientData(config::CheatType type) : type_(type) {}
    ~CheatTypeClientData() override = default;

    config::CheatType type() const { return type_; }

private:
    const config::CheatType type_;
};

class CodeTypeValidator final : public wxValidator {
public:
    explicit CodeTypeValidator(config::CheatType* const cheat_type) : cheat_type_(cheat_type) {
        assert(cheat_type_);
    }
    ~CodeTypeValidator() override = default;

    wxObject* Clone() const override { return new CodeTypeValidator(cheat_type_); }

    // wxValidator implementation.
    bool TransferFromWindow() final {
        const wxChoice* choice = wxDynamicCast(GetWindow(), wxChoice);
        assert(choice);

        const int selection = choice->GetSelection();
        if (selection == wxNOT_FOUND) {
            return false;
        }
        *cheat_type_ =
            static_cast<CheatTypeClientData*>(choice->GetClientObject(selection))->type();

        return true;
    }

    bool Validate(wxWindow*) final {
        const wxChoice* choice = wxDynamicCast(GetWindow(), wxChoice);
        assert(choice);
        return choice->GetSelection() != wxNOT_FOUND;
    }

    bool TransferToWindow() final {
        wxChoice* choice = wxDynamicCast(GetWindow(), wxChoice);
        assert(choice);

        for (unsigned int i = 0; i < choice->GetCount(); i++) {
            if (*cheat_type_ ==
                static_cast<CheatTypeClientData*>(choice->GetClientObject(i))->type()) {
                choice->SetSelection(i);
                return true;
            }
        }

        return false;
    }

private:
    config::CheatType* const cheat_type_;
};

class MultiLineStringValidator : public wxValidator {
public:
    explicit MultiLineStringValidator(config::Cheat* const cheat) : cheat_(cheat) {
        assert(cheat_);
    }
    ~MultiLineStringValidator() override = default;

    wxObject* Clone() const override { return new MultiLineStringValidator(cheat_); }

    // wxValidator implementation.
    bool TransferFromWindow() final {
        const wxTextCtrl* text_ctrl = wxDynamicCast(GetWindow(), wxTextCtrl);
        assert(text_ctrl);
        cheat_->codes.clear();
        for (int i = 0; i < text_ctrl->GetNumberOfLines(); i++) {
            cheat_->codes.push_back(text_ctrl->GetLineText(i).ToStdString());
        }
        return true;
    }

    bool Validate(wxWindow*) final {
        if (cheat_->codes.empty()) {
            wxMessageDialog dialog(GetWindow(), _("No cheat code provided."), _("Error"),
                                   wxICON_ERROR | wxOK);
            dialog.ShowModal();
            return false;
        }

        for (const std::string& code : cheat_->codes) {
            switch (cheat_->system()) {
                case VbamSystem::GameBoy:
                    if (!gbCheatManager::isCodeValid(code, cheat_->type.gb())) {
                        wxMessageDialog dialog(GetWindow(),
                                               wxString::Format(_("Invalid code: %s"), code),
                                               _("Error"), wxICON_ERROR | wxOK);
                        return false;
                    }
                    break;
                case VbamSystem::GameBoyAdvance:
                    // FIXME
                    break;
            }
        }

        return true;
    }

    bool TransferToWindow() final {
        wxTextCtrl* text_ctrl = wxDynamicCast(GetWindow(), wxTextCtrl);
        assert(text_ctrl);
        text_ctrl->Clear();
        for (const std::string& code : cheat_->codes) {
            text_ctrl->AppendText(wxString::FromUTF8(code) + "\n");
        }
        return true;
    }

private:
    config::Cheat* const cheat_;
};

}  // namespace

// static
CheatEdit* CheatEdit::NewInstance(wxWindow* parent) {
    assert(parent);
    return new CheatEdit(parent);
}

CheatEdit::CheatEdit(wxWindow* parent)
    : wxDialog(),
      gb_cheats_(std::vector<std::pair<wxString, config::CheatType>>{
          {"Game Shark", config::CheatType(gbCheatType::GameShark)},
          {"Game Genie", config::CheatType(gbCheatType::GameGenie)},
      }),
      gba_cheats_(std::vector<std::pair<wxString, config::CheatType>>{
          {_("Generic Code"), config::CheatType(gbaCheatType::Generic)},
          {"Game Shark Advance", config::CheatType(gbaCheatType::GameShark)},
          {"Code Breaker Advance", config::CheatType(gbaCheatType::CodeBreaker)},
          {"Flashcart CHT", config::CheatType(gbaCheatType::Flashcart)},
      }),
      keep_on_top_styler_(this) {
#if !wxCHECK_VERSION(3, 1, 0)
    // This needs to be set before loading any element on the window. This also
    // has no effect since wx 3.1.0, where it became the default.
    this->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
#endif
    wxXmlResource::Get()->LoadDialog(this, parent, "CheatEdit");

    GetValidatedChild(this, "Type")->SetValidator(CodeTypeValidator(&cheat_.type));
    GetValidatedChild(this, "Desc")->SetValidator(wxGenericValidator(&cheat_.description));
    GetValidatedChild(this, "Codes")->SetValidator(MultiLineStringValidator(&cheat_));
    this->Fit();
}

void CheatEdit::SetCheat(const config::Cheat& cheat) {
    cheat_ = cheat;
    this->ResetChoices();
    this->TransferDataToWindow();
}

void CheatEdit::Reset(VbamSystem system) {
    switch (system) {
        case VbamSystem::GameBoy:
            cheat_.type = config::CheatType(gbCheatType::GameGenie);
            break;
        case VbamSystem::GameBoyAdvance:
            cheat_.type = config::CheatType(gbaCheatType::Generic);
            break;
    }
    cheat_.codes.clear();
    cheat_.description.clear();
    cheat_.enabled = true;
    this->ResetChoices();
    this->TransferDataToWindow();
}

void CheatEdit::ResetChoices() {
    wxChoice* choice = GetValidatedChild<wxChoice>(this, "Type");
    choice->Clear();
    switch (cheat_.system()) {
        case VbamSystem::GameBoy:
            for (const auto& cheat : gb_cheats_) {
                choice->Append(cheat.first, new CheatTypeClientData(cheat.second));
            }
            break;
        case VbamSystem::GameBoyAdvance:
            for (const auto& cheat : gba_cheats_) {
                choice->Append(cheat.first, new CheatTypeClientData(cheat.second));
            }
            break;
    }
}

}  // namespace dialogs
