#ifndef VBAM_HANDLER_INTERNAL_INCLUDE
#error "Do not include "config/internal/handler-internal.h" outside of the implementation."
#endif

#include <array>
#include <unordered_map>
#include <map>

#include <wx/string.h>

#include "wx/config/handler-id.h"

namespace config {
namespace internal {

// Static data associated with a HandlerID.
struct HandlerData {
    // The handler ID.
    const HandlerID handler_id;
    // The string to use in the INI file for configuration.
    const wxString config_string;
    // The wxWidgets command ID.
    const int xrc_id;
};

// Returns the data for all handlers.
const std::array<HandlerData, kNbHandlers>& AllHandlersData();

// Returns the map from XRC ID to handler ID.
const std::unordered_map<int, HandlerID>& XRCIDToHandlerMap();

// Returns the map from config strings to handler ID.
const std::map<wxString, HandlerID>& ConfigStringToHandlerMap();

}  // namespace internal
}  // namespace config