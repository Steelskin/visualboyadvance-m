#ifndef VBAM_WX_WXHEAD_H_
#define VBAM_WX_WXHEAD_H_

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <wx/xrc/xmlres.h>

#ifndef NO_OGL
// glcanvas must be included before SFML for MacOSX
// originally, this was confined to drawing.h.
#include <wx/glcanvas.h>
// besides that, other crap gets #defined
#ifdef Status
#undef Status
#endif
#ifdef BadRequest
#undef BadRequest
#endif
#endif

// wxrc helpers (for dynamic strings instead of constant)
#define XRCID_D(str) wxXmlResource::GetXRCID(str)
//#define XRCCTRL_D(win, id, type) (wxStaticCast((win).FindWindow(XRCID_D(id)), type))
//#define XRCCTRL_I(win, id, type) (wxStaticCast((win).FindWindow(id), type))
// XRCCTRL is broken.
// In debug mode, it uses wxDynamicCast, which numerous wx classes fail on
// due to not correctly specifying parents in CLASS() declarations
// In standard mode, it does a static cast, which is unsafe for user input
// So instead I'll always do a (slow, possibly unportable) dynamic_cast().
// If your compiler doesn't support rtti, there are other pieces of code where
// I bypassed wx's stuff to use real dynamic_cast as well, so get a better
// compiler.
#undef XRCCTRL
#define XRCCTRL_I(win, id, type) (dynamic_cast<type*>((win).FindWindow(id)))
#define XRCCTRL(win, id, type) XRCCTRL_I(win, XRCID(id), type)
#define XRCCTRL_D(win, id, type) XRCCTRL_I(win, XRCID_D(id), type)

// Keep a single entry point for converting wxString to UTF8.
// Use this function whenever we want to get
static inline const wxCharBuffer UTF8(wxString str)
{
    return str.mb_str(wxConvUTF8);
}

#endif // VBAM_WX_WXHEAD_H_
