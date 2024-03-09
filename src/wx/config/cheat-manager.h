#ifndef VBAM_WX_CONFIG_CHEAT_MANAGER_H_
#define VBAM_WX_CONFIG_CHEAT_MANAGER_H_

#include <string>
#include <vector>

#include <wx/string.h>
#include <zlib.h>

#include "../common/system.h"
#include "../gb/gbCheats.h"
#include "../gba/Cheats.h"
#include "nonstd/optional.hpp"
#include "nonstd/variant.hpp"

namespace config {

class CheatType {
public:
    CheatType() : CheatType(gbCheatType::GameGenie) {}
    explicit CheatType(gbCheatType type) : type_(type), system_(VbamSystem::GameBoy) {}
    explicit CheatType(gbaCheatType type) : type_(type), system_(VbamSystem::GameBoyAdvance) {}
    ~CheatType() = default;

    VbamSystem system() const { return system_; }

    bool is_gb() const { return system_ == VbamSystem::GameBoy; }
    bool is_gba() const { return system_ == VbamSystem::GameBoyAdvance; }

    gbCheatType gb() const {
        assert(is_gb());
        return nonstd::get<gbCheatType>(type_);
    }
    gbaCheatType gba() const {
        assert(is_gba());
        return nonstd::get<gbaCheatType>(type_);
    }

    bool operator==(const CheatType& other) const {
        return type_ == other.type_ && system_ == other.system_;
    }

private:
    nonstd::variant<gbCheatType, gbaCheatType> type_;
    VbamSystem system_;
};

struct Cheat {
    CheatType type;
    wxString description;
    std::vector<std::string> codes;
    bool enabled;

    VbamSystem system() const { return type.system(); }
};

class CheatManager final : public gbCheatSaveStateDelegate {
public:
    CheatManager() = default;
    ~CheatManager() final = default;

    // Not copyable or movable
    CheatManager(const CheatManager&) = delete;
    CheatManager& operator=(const CheatManager&) = delete;
    CheatManager(CheatManager&&) = delete;
    CheatManager& operator=(CheatManager&&) = delete;

    // Accessors.
    const std::vector<Cheat>& cheats() const { return cheats_; }
    size_t size() const { return cheats_.size(); }
    bool empty() const { return cheats_.empty(); }

    void Reset();
    nonstd::optional<size_t> AddCheat(Cheat cheat);
    bool RemoveCheat(size_t index);
    bool ReplaceCheat(size_t index, Cheat cheat);
    bool EnableCheat(size_t index);
    bool DisableCheat(size_t index);

    void SaveCheatList(const wxString& file_name) const;
    bool LoadCheatList(const wxString& file_name);
    bool ReadGSCodeFile(const wxString& file_name);

private:
    // gbCheatStateDelegate implementation.
    void SaveCheatsToFile(gzFile file) final;
    bool LoadCheatsFromFile(gzFile file, bool activate) final;

    std::vector<Cheat> cheats_;
};

class CheatManagerProvider {
public:
    virtual ~CheatManagerProvider() = default;
    virtual CheatManager* GetCheatManager() = 0;
};

}  // namespace config

#endif  // VBAM_WX_CONFIG_CHEAT_MANAGER_H_
