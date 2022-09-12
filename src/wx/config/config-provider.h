#ifndef VBAM_WX_CONFIG_CONFIG_PROVIDER_H_
#define VBAM_WX_CONFIG_CONFIG_PROVIDER_H_

namespace config {

class Bindings;
class EmulatedGamepad;
class FileHistory;

class ConfigProvider {
public:
    ~ConfigProvider() = default;

    virtual Bindings* bindings() = 0;
    virtual EmulatedGamepad* emulated_gamepad() = 0;
    virtual FileHistory* file_history() = 0;
};

}  // namespace config

#endif  // VBAM_WX_CONFIG_CONFIG_PROVIDER_H_
