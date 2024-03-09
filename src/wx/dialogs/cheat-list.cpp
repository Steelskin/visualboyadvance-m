#include "dialogs/cheat-list.h"

#include <wx/choice.h>
#include <wx/clrpicker.h>
#include <wx/dcclient.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/tokenzr.h>
#include <wx/toolbar.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>

#include <wx/xrc/xmlres.h>

#include "../gba/Cheats.h"
#include "config/cheat-manager.h"
#include "config/option-proxy.h"
#include "dialogs/cheat-edit.h"
#include "dialogs/validated-child.h"
#include "widgets/checked-list-ctrl.h"

namespace dialogs {

namespace {

// manage the cheat list dialog
class CheatList_t : public wxEvtHandler {
public:
    wxDialog* dlg;
    widgets::CheckedListCtrl* list;
    wxListItem item0, item1;
    int col1minw;
    wxString cheat_dir_name_, cheat_file_name_, deffn;
    bool is_gb_;

    // add/edit dialog
    wxString ce_desc_;
    wxString ce_codes_;
    wxChoice* ce_type_ch;
    wxControl* ce_codes_tc;
    int cheat_code_type_;
    config::CheatManagerProvider* const cheat_manager_provider_;
    CheatEdit* const cheat_edit_ = nullptr;

    void AddCheat() {
        wxStringTokenizer tk(ce_codes_.MakeUpper());

        while (tk.HasMoreTokens()) {
            wxString tok = tk.GetNextToken();

            if (is_gb_) {
                const config::CheatType cheat_type =
                    cheat_code_type_ ? config::CheatType(gbCheatType::GameGenie)
                                     : config::CheatType(gbCheatType::GameShark);
                cheat_manager_provider_->GetCheatManager()->AddCheat(config::Cheat{
                    cheat_type,
                    ce_desc_,
                    {tok.ToStdString()},
                    true,
                });
                return;
            }
            if (tok.Contains("=")) {
                // Flashcart CHT format
                // this basically converts to the "generic" format.
                ParseChtLine(ce_desc_, tok);
                return;
            }
            if (tok.Contains(":")) {
                // Generic Code
                cheatsAddCheatCode(tok.utf8_str(), ce_desc_.utf8_str());
                return;
            }

            // following determination of type by lengths is
            // same used by win32 and gtk code
            // and like win32/gtk code, user-chosen fmt is ignored
            if (tok.size() == 12) {
                tok = tok.substr(0, 8) + ' ' + tok.substr(8);
                cheatsAddCBACode(tok.utf8_str(), ce_desc_.utf8_str());
                return;
            }

            if (tok.size() == 16) {
                // not sure why 1-tok is !v3 and 2-tok is v3..
                cheatsAddGSACode(tok.utf8_str(), ce_desc_.utf8_str(), false);
                return;
            }

            // CBA codes are assumed to be N+4, and anything else
            // is assumed to be GSA v3 (although I assume the
            // actual formats should be 8+4 and 8+8)
            if (!tk.HasMoreTokens()) {
                // throw an error appropriate to chosen type
                if (cheat_code_type_ == (int)gbaCheatType::GameShark)  // GSA
                    cheatsAddGSACode(tok.utf8_str(), ce_desc_.utf8_str(), false);
                else
                    cheatsAddCBACode(tok.utf8_str(), ce_desc_.utf8_str());
            } else {
                wxString tok2 = tk.GetNextToken();

                if (tok2.size() == 4) {
                    tok += ' ' + tok2;
                    cheatsAddCBACode(tok.utf8_str(), ce_desc_.utf8_str());
                } else {
                    tok += tok2;
                    cheatsAddGSACode(tok.utf8_str(), ce_desc_.utf8_str(), true);
                }
            }
        }
    }

    void ParseChtLine(const wxString& desc, wxString tok) {
        const wxString cheat_name = tok.BeforeFirst('=');
        wxString cheat_set = tok.AfterFirst('=');
        wxStringTokenizer addr_tk(cheat_set.MakeUpper(), ";");

        while (addr_tk.HasMoreTokens()) {
            const wxString token = addr_tk.GetNextToken();
            const wxString cheat_address = token.BeforeFirst(',');
            wxString values = token.AfterFirst(',');
            const wxString cheat_desc = desc + ":" + cheat_name;
            wxString cheat_value;
            uint32_t address = 0;
            uint32_t value = 0;
            sscanf(cheat_address.utf8_str(), "%8x", &address);

            if (address < 0x40000)
                address += 0x2000000;
            else
                address += 0x3000000 - 0x40000;

            wxStringTokenizer value_tk(values.MakeUpper(), ",");

            while (value_tk.HasMoreTokens()) {
                const wxString value_token = value_tk.GetNextToken();
                sscanf(value_token.utf8_str(), "%2x", &value);
                const wxString cheat_line = wxString::Format("%08X:%02X", address, value);
                cheatsAddCheatCode(cheat_line.utf8_str(), cheat_desc.utf8_str());
                address++;
            }
        }
    }

    void AdjustDescWidth() {
        // why is it so hard to get an accurate measurement out of wx?
        // on msw, wxLIST_AUTOSIZE might actually be accurate.  On wxGTK,
        // and probably wxMAC (both of which use generic impl) wrong
        // font is used both for rendering (col 0's font) and for
        // wxLIST_AUTOSIZE calculation (the widget's font).
        // The only way to defeat this is to calculate size manually
        // Instead, this just allows user to set max size, and retains
        // it.
        int ow = list->GetColumnWidth(1);
        list->SetColumnWidth(1, wxLIST_AUTOSIZE);
        int cw = list->GetColumnWidth(1);
        // subtracted in renderer from width avail for text
        // but not added in wxLIST_AUTOSIZE
        cw += 8;

        if (cw < col1minw)
            cw = col1minw;

        if (cw < ow)
            cw = ow;

        list->SetColumnWidth(1, cw);
    }
};

}  // namespace

// static
CheatList* CheatList::NewInstance(wxWindow* parent, Delegate* const delegate) {
    assert(parent);
    return new CheatList(parent, delegate);
}

CheatList::CheatList(wxWindow* parent, Delegate* const delegate)
    : wxDialog(),
      cheat_edit_(CheatEdit::NewInstance(this)),
      delegate_(delegate),
      keep_on_top_styler_(this) {
#if !wxCHECK_VERSION(3, 1, 0)
    // This needs to be set before loading any element on the window. This also
    // has no effect since wx 3.1.0, where it became the default.
    this->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
#endif
    wxXmlResource::Get()->LoadDialog(this, parent, "CheatList");

    // Set escape key to the "OK" button.
    this->SetEscapeId(wxID_OK);

    cheat_list_ = GetValidatedChild<widgets::CheckedListCtrl>(this, "Cheats");
    [[maybe_unused]] const bool checked_list_init = cheat_list_->Init();
    assert(checked_list_init);

    // Set up the columns.
    cheat_list_->InsertColumn(0, _("Code"));
    cheat_list_->InsertColumn(1, _("Description"));

    // Set up the code and description items styles. We need to explicitly set the font here.
    const wxFont font = cheat_list_->GetFont();
    const wxFont new_font(font.GetPointSize(), wxFONTFAMILY_MODERN, font.GetStyle(),
                          font.GetWeight());

    code_item_.SetFont(new_font);
    code_item_.SetColumn(0);

    desc_item_.SetFont(new_font);
    desc_item_.SetColumn(1);

    // too bad I can't just set the size to windowwidth - other cols
    // default width is header width, but using following will probably
    // make it 80 pixels wide regardless
    // cheat_list_->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
    // on wxGTK, column 1 seems to inherit column 0's font regardless
    // of requested font
#if 0
    // the ideal way to set col 0's width would be to use
    // wxLIST_AUTOSIZE after setting value to a sample:
    code_item_.SetText("00000000 00000000");
    cheat_list_->InsertItem(code_item_);
    cheat_list_->SetColumnWidth(0, wxLIST_AUTOSIZE);
    cheat_list_->DeleteItem(0);
#else
    // however, the generic listctrl implementation uses the wrong
    // font to determine width (window vs. item), and does not
    // calculate the margins the same way in calculation vs. actual
    // drawing.  so calculate manually, using knowledge of underlying
    // code.  This is highly version-unportable, but better than using
    // buggy wx code..
    int width, height;
    cheat_list_->GetImageList(wxIMAGE_LIST_SMALL)->GetSize(0, width, height);
    width += 5;  // IMAGE_MARGIN_IN_REPORT_MODE
    // following is missing from wxLIST_AUTOSIZE
    width += 8;  // ??? subtracted from width avail for text
    {
        int charwidth, charheight;
        wxClientDC dc(cheat_list_);
        // following is item font instead of window font,
        // and so is missing from wxLIST_AUTOSIZE
        dc.SetFont(new_font);
        dc.GetTextExtent('M', &charwidth, &charheight);
        width += (8 + 1 + 8) * charwidth;
    }
    cheat_list_->SetColumnWidth(0, width);
#endif

    // Bind the Cheat List events.
    cheat_list_->Bind(CHECKED_LIST_ITEM_CHECKED, &CheatList::OnCheatChecked, this);
    cheat_list_->Bind(CHECKED_LIST_ITEM_UNCHECKED, &CheatList::OnCheatUnchecked, this);
    cheat_list_->Bind(wxEVT_LIST_ITEM_ACTIVATED, &CheatList::OnCheatEdition, this);

    // Populate the cheat list on show.
    this->Bind(wxEVT_SHOW, &CheatList::OnShow, this);

    // Bind the menu toolbar event.
    this->Bind(wxEVT_MENU, &CheatList::OnTool, this);

    this->Fit();
}

void CheatList::RepopulateCheats() {
    cheat_list_->ClearAll();
    for (size_t i = 0; i < delegate_->GetCheatManager()->cheats().size(); i++) {
        const config::Cheat& cheat = delegate_->GetCheatManager()->cheats()[i];

        code_item_.SetId(i);
        code_item_.SetText(cheat.codes[0]);
        cheat_list_->SetItem(code_item_);

        desc_item_.SetId(i);
        desc_item_.SetText(cheat.description);
        cheat_list_->SetItem(desc_item_);

        cheat_list_->Check(i, cheat.enabled);
    }
}

void CheatList::AddCheat(int index) {
    assert(index < static_cast<int>(delegate_->GetCheatManager()->size()));
    const config::Cheat& cheat = delegate_->GetCheatManager()->cheats()[index];

    code_item_.SetId(index);
    code_item_.SetText(cheat.codes[0]);
    cheat_list_->InsertItem(code_item_);

    desc_item_.SetId(index);
    desc_item_.SetText(cheat.description);
    cheat_list_->SetItem(desc_item_);
}

void CheatList::ReloadCheat(int index) {
    assert(index < static_cast<int>(delegate_->GetCheatManager()->size()));
    const config::Cheat& cheat = delegate_->GetCheatManager()->cheats()[index];

    code_item_.SetId(index);
    code_item_.SetText(cheat.codes[0]);
    cheat_list_->SetItem(code_item_);

    desc_item_.SetId(index);
    desc_item_.SetText(cheat.description);
    cheat_list_->SetItem(desc_item_);
}

void CheatList::OnShow(wxShowEvent& show_event) {
    if (show_event.IsShown()) {
        RepopulateCheats();
    }

    GetValidatedChild<wxToolBar>(this, "ToolBar")
        ->ToggleTool(XRCID("AutoLoadSave"), OPTION(kPrefAutoSaveLoadCheatList));

    // Let the event propagate.
    show_event.Skip();
}

void CheatList::OnCheatChecked(wxListEvent& event) {
    const int index = event.GetIndex();
    assert(index < static_cast<int>(delegate_->GetCheatManager()->size()));
    delegate_->GetCheatManager()->EnableCheat(index);
}

void CheatList::OnCheatUnchecked(wxListEvent& event) {
    const int index = event.GetIndex();
    assert(index < static_cast<int>(delegate_->GetCheatManager()->size()));
    delegate_->GetCheatManager()->DisableCheat(index);
}

void CheatList::OnCheatEdition(wxListEvent& event) {
    int index = event.GetIndex();

    const std::vector<config::Cheat>& cheats = delegate_->GetCheatManager()->cheats();
    if (index > static_cast<int>(cheats.size())) {
        return;
    }

    const config::Cheat cheat = cheats[index];
    cheat_edit_->SetCheat(cheat);
    if (cheat_edit_->ShowModal() != wxID_OK) {
        return;
    }

    if (!delegate_->GetCheatManager()->ReplaceCheat(index, cheat_edit_->cheat())) {
        wxMessageDialog(this, wxString::Format(_("Failed to replace cheat at index %d"), index),
                        _("Error"), wxICON_ERROR | wxOK)
            .ShowModal();
        return;
    };

    ReloadCheat(index);
}

void CheatList::OnTool(wxCommandEvent& event) {
    if (event.GetId() == XRCID("AutoLoadSave")) {
        OPTION(kPrefAutoSaveLoadCheatList) = event.IsChecked();
        return;
    }

    switch (event.GetId()) {
        case wxID_OPEN: {
            wxFileDialog subdlg(this, _("Select cheat file"), cheat_dir_name_, cheat_file_name_,
                                _("VBA cheat lists (*.clt)|*.clt|CHT cheat lists (*.cht)|*.cht"),
                                wxFD_OPEN | wxFD_FILE_MUST_EXIST);
            if (subdlg.ShowModal() != wxID_OK) {
                return;
            }
            cheat_dir_name_ = subdlg.GetDirectory();
            cheat_file_name_ = subdlg.GetPath();

            if (!delegate_->GetCheatManager()->LoadCheatList(cheat_file_name_)) {
                wxMessageDialog(this, _("Failed to load cheat list"), _("Error"),
                                wxICON_ERROR | wxOK)
                    .ShowModal();
                return;
            }

#if 0
            // FIXME: Move this.
            if (system_provider_() == VbamSystem::GameBoy) {
                 delegate_->GetCheatManager()->LoadCheatList(cheat_file_name_);
            } else {
                bool cld;

                if (cheat_file_name_.EndsWith(wxT(".clt"))) {
                    cld = cheatsLoadCheatList(cheat_file_name_);

                    if (cld) {
                        systemScreenMessage(_("Loaded cheats"));
                    }
                } else {
                    // cht format
                    wxFileInputStream input(cheat_file_name_);
                    wxTextInputStream text(input, wxT("\x09"), wxConvUTF8);
                    wxString cheat_desc = wxT("");

                    while (input.IsOk() && !input.Eof()) {
                        wxString line = text.ReadLine().Trim();

                        if (line.Contains(wxT("[")) && !line.Contains(wxT("="))) {
                            cheat_desc = line.AfterFirst('[').BeforeLast(']');
                        }

                        if (line.Contains(wxT("=")) && cheat_desc != wxT("GameInfo")) {
                            while ((input.IsOk() && !input.Eof()) &&
                                   (line.EndsWith(wxT(";")) || line.EndsWith(wxT(",")))) {
                                line = line + text.ReadLine().Trim();
                            }

                            ParseChtLine(cheat_desc, line);
                        }
                    }
                }
            }
#endif

            RepopulateCheats();
            return;
        }

        case wxID_SAVE: {
            wxFileDialog subdlg(this, _("Select cheat file"), cheat_dir_name_, cheat_file_name_,
                                _("VBA cheat lists (*.clt)|*.clt"),
                                wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
            if (subdlg.ShowModal() != wxID_OK) {
                return;
            }
            cheat_dir_name_ = subdlg.GetDirectory();
            cheat_file_name_ = subdlg.GetPath();

            // note that there is no way to test for succes of save
            delegate_->GetCheatManager()->SaveCheatList(cheat_file_name_);
            systemScreenMessage(_("Saved cheats"));
            return;
        }

        case wxID_ADD: {
            cheat_edit_->Reset(delegate_->GetSystem());
            if (cheat_edit_->ShowModal() != wxID_OK) {
                return;
            }

            const std::optional<size_t> new_index =
                delegate_->GetCheatManager()->AddCheat(cheat_edit_->cheat());
            if (!new_index) {
                wxMessageDialog(this, _("Failed to add cheat"), _("Error"), wxICON_ERROR | wxOK)
                    .ShowModal();
                return;
            }

            AddCheat(new_index.value());
            return;
        }

        case wxID_REMOVE: {
            for (int i = cheat_list_->GetItemCount() - 1; i >= 0; i--) {
                if (cheat_list_->GetItemState(i, wxLIST_STATE_SELECTED)) {
                    cheat_list_->DeleteItem(i);
                    delegate_->GetCheatManager()->RemoveCheat(i);
                }
            }
            return;
        }

        case wxID_CLEAR: {
            delegate_->GetCheatManager()->Reset();
            RepopulateCheats();
            return;
        }
    }
}

}  // namespace dialogs
