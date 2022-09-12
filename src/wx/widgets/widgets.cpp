#include "wx/widgets/widgets.h"

#include <wx/xrc/xh_bmp.h>
#include <wx/xrc/xh_bttn.h>
#include <wx/xrc/xh_chckb.h>
#include <wx/xrc/xh_choic.h>
#include <wx/xrc/xh_clrpicker.h>
#include <wx/xrc/xh_dirpicker.h>
#include <wx/xrc/xh_dlg.h>
#include <wx/xrc/xh_filepicker.h>
#include <wx/xrc/xh_frame.h>
#include "wx/xrc/xh_listb.h"
#include <wx/xrc/xh_listc.h>
#include <wx/xrc/xh_notbk.h>
#include <wx/xrc/xh_panel.h>
#include <wx/xrc/xh_radbt.h>
#include <wx/xrc/xh_scwin.h>
#include <wx/xrc/xh_sizer.h>
#include <wx/xrc/xh_slidr.h>
#include <wx/xrc/xh_spin.h>
#include <wx/xrc/xh_statbar.h>
#include <wx/xrc/xh_sttxt.h>
#include <wx/xrc/xh_text.h>
#include <wx/xrc/xh_toolb.h>
#include <wx/xrc/xh_tree.h>
#include <wx/xrc/xh_unkwn.h>
#include <wx/xrc/xmlres.h>

#include "wx/widgets/group-check-box.h"
#include "wx/widgets/shortcut-menu-bar.h"
#include "wx/widgets/shortcut-menu-item.h"
#include "wx/widgets/shortcut-menu.h"
#include "wx/widgets/user-input-ctrl.h"

namespace widgets {

wxXmlResource* InitializeXmlHandlers() {
    wxXmlResource* xr = wxXmlResource::Get();

    // wxWidgets handlers. We only register the ones we actually need here to
    // reduce binary size in static builds.
    xr->AddHandler(new wxButtonXmlHandler());
    xr->AddHandler(new wxCheckBoxXmlHandler());
    xr->AddHandler(new wxChoiceXmlHandler());
    xr->AddHandler(new wxColourPickerCtrlXmlHandler());
    xr->AddHandler(new wxDialogXmlHandler());
    xr->AddHandler(new wxDirPickerCtrlXmlHandler());
    xr->AddHandler(new wxFilePickerCtrlXmlHandler());
    xr->AddHandler(new wxFrameXmlHandler());
    xr->AddHandler(new wxIconXmlHandler());
    xr->AddHandler(new wxListBoxXmlHandler());
    xr->AddHandler(new wxListCtrlXmlHandler());
    xr->AddHandler(new wxNotebookXmlHandler());
    xr->AddHandler(new wxPanelXmlHandler());
    xr->AddHandler(new wxRadioButtonXmlHandler());
    xr->AddHandler(new wxScrolledWindowXmlHandler());
    xr->AddHandler(new wxSizerXmlHandler());
    xr->AddHandler(new wxSliderXmlHandler());
    xr->AddHandler(new wxSpinCtrlXmlHandler());
    xr->AddHandler(new wxStaticTextXmlHandler());
    xr->AddHandler(new wxStatusBarXmlHandler());
    xr->AddHandler(new wxStdDialogButtonSizerXmlHandler());
    xr->AddHandler(new wxTextCtrlXmlHandler());
    xr->AddHandler(new wxToolBarXmlHandler());
    xr->AddHandler(new wxTreeCtrlXmlHandler());
    xr->AddHandler(new wxUnknownWidgetXmlHandler());

    // Our additional handlers.
    xr->AddHandler(new widgets::GroupCheckBoxXmlHandler());
    xr->AddHandler(new widgets::UserInputCtrlXmlHandler());
    xr->AddHandler(new widgets::ShortcutMenuBarXmlHandler());
    xr->AddHandler(new widgets::ShortcutMenuItemXmlHandler());
    xr->AddHandler(new widgets::ShortcutMenuXmlHandler());

    return xr;
}

}  // namespace widgets
