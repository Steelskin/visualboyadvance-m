#include "wx/config/handler-id.h"

#include <array>
#include <cstddef>
#include <unordered_map>

#include <wx/string.h>
#include <wx/stringimpl.h>
#include <wx/xrc/xmlres.h>

#define VBAM_HANDLER_INTERNAL_INCLUDE
#include "wx/config/internal/handler-internal.h"
#undef VBAM_HANDLER_INTERNAL_INCLUDE

namespace config {

HandlerID GetRecentHandlerID(size_t index) {
    assert(index < 10);
    return static_cast<HandlerID>(static_cast<size_t>(HandlerID::RecentFile1) + index);
}

HandlerID GetLoadStateHandlerID(size_t index) {
    assert(index < 10);
    return static_cast<HandlerID>(static_cast<size_t>(HandlerID::LoadState01) + index);
}

HandlerID GetSaveStateHandlerID(size_t index) {
    assert(index < 10);
    return static_cast<HandlerID>(static_cast<size_t>(HandlerID::SaveState01) + index);
}

bool HandlerIsState(HandlerID handler_id) {
    return (handler_id >= HandlerID::LoadState01 && handler_id <= HandlerID::SaveState10) ||
           (handler_id >= HandlerID::LoadState01 && handler_id <= HandlerID::SaveState10);
}

HandlerID XRCIDToHandlerID(int xrc_id) {
    const auto& map = internal::XRCIDToHandlerMap();
    const auto iter = map.find(xrc_id);
    if (iter == map.end()) {
        return HandlerID::Noop;
    }

    return iter->second;
}

int HandlerIDToXRCID(HandlerID handler_id) {
    if (handler_id > HandlerID::Noop) {
        return wxID_NONE;
    }

    return internal::AllHandlersData()[static_cast<size_t>(handler_id)].xrc_id;
}

wxString HandlerIDToConfigString(HandlerID handler_id) {
    if (handler_id > HandlerID::Noop) {
        return wxEmptyString;
    }

    return internal::AllHandlersData()[static_cast<size_t>(handler_id)].config_string;
}

nonstd::optional<HandlerID> ConfigStringToHandlerID(const wxString &config_string) {
    const auto& map = internal::ConfigStringToHandlerMap();
    const auto iter = map.find(config_string);
    if (iter == map.end()) {
        return nonstd::nullopt;
    }

    return iter->second;
}

}  // namespace config
