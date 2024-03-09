#include "config/cheat-manager.h"

namespace config {

nonstd::optional<size_t> CheatManager::AddCheat(const Cheat& cheat) {
    cheats_.push_back(cheat);
}



}  // namespace config
