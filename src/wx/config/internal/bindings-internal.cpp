#define VBAM_BINDINGS_INTERNAL_INCLUDE
#include "wx/config/internal/bindings-internal.h"
#undef VBAM_BINDINGS_INTERNAL_INCLUDE

#include <unordered_set>

#include <wx/xrc/xmlres.h>

#include "wx/config/command.h"
#include "wx/config/handler-id.h"

namespace config {
namespace internal {

const std::unordered_map<Command, std::unordered_set<UserInput>>& DefaultInputs() {
    // clang-format off
    static const std::unordered_map<Command, std::unordered_set<UserInput>> kDefaultInputs = {
        {ShortcutCommand(HandlerID::CheatsList),
            {
                KeyboardInput('C', wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::NextFrame),
            {
                KeyboardInput('N', wxMOD_CMD)
            }},
        // this was annoying people A LOT #334
        // {ShortcutCommand(HandlerID::Exit),
        //     {
        //         KeyboardInput(WXK_ESCAPE, wxMOD_NONE)
        //     }},
        // this was annoying people #298
        // {ShortcutCommand(HandlerID::Exit),
        //     {
        //         KeyboardInput('X', wxMOD_CMD)
        //     }},
        {ShortcutCommand(HandlerID::Exit),
            {
                KeyboardInput('Q', wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::Close),
            {
                KeyboardInput('W', wxMOD_CMD)
            }},
        // load most recent is more commonly used than load state
        // {ShortcutCommand(HandlerID::LoadStateFile),
        //     {
        //         KeyboardInput('L', wxMOD_CMD)
        // }},
        {ShortcutCommand(HandlerID::LoadStateRecent),
            {
                KeyboardInput('L', wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::LoadState01),
            {
                KeyboardInput(WXK_F1, wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::LoadState02),
            {
                KeyboardInput(WXK_F2, wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::LoadState03),
            {
                KeyboardInput(WXK_F3, wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::LoadState04),
            {
                KeyboardInput(WXK_F4, wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::LoadState05),
            {
                KeyboardInput(WXK_F5, wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::LoadState06),
            {
                KeyboardInput(WXK_F6, wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::LoadState07),
            {
                KeyboardInput(WXK_F7, wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::LoadState08),
            {
                KeyboardInput(WXK_F8, wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::LoadState09),
            {
                KeyboardInput(WXK_F9, wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::LoadState10),
            {
                KeyboardInput(WXK_F10, wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::Pause),
         {KeyboardInput(WXK_PAUSE, wxMOD_NONE), KeyboardInput('P', wxMOD_CMD)}},
        {ShortcutCommand(HandlerID::Reset),
            {
                KeyboardInput('R', wxMOD_CMD)
            }},
        // add shortcuts for original size multiplier #415
        {ShortcutCommand(HandlerID::SetSize1x),
            {
                KeyboardInput('1', wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::SetSize2x),
            {
                KeyboardInput('2', wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::SetSize3x),
            {
                KeyboardInput('3', wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::SetSize4x),
            {
                KeyboardInput('4', wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::SetSize5x),
            {
                KeyboardInput('5', wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::SetSize6x),
            {
                KeyboardInput('6', wxMOD_NONE)
            }},
        // save oldest is more commonly used than save other
        // {ShortcutCommand(HandlerID::SaveStateFile),
        //     {
        //         KeyboardInput('S', wxMOD_CMD)
        //     }},
        {ShortcutCommand(HandlerID::SaveStateOldest),
            {
                KeyboardInput('S', wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::SaveState01),
            {
                KeyboardInput(WXK_F1, wxMOD_SHIFT)
            }},
        {ShortcutCommand(HandlerID::SaveState02),
            {
                KeyboardInput(WXK_F2, wxMOD_SHIFT)
            }},
        {ShortcutCommand(HandlerID::SaveState03),
            {
                KeyboardInput(WXK_F3, wxMOD_SHIFT)
            }},
        {ShortcutCommand(HandlerID::SaveState04),
            {
                KeyboardInput(WXK_F4, wxMOD_SHIFT)
            }},
        {ShortcutCommand(HandlerID::SaveState05),
            {
                KeyboardInput(WXK_F5, wxMOD_SHIFT)
            }},
        {ShortcutCommand(HandlerID::SaveState06),
            {
                KeyboardInput(WXK_F6, wxMOD_SHIFT)
            }},
        {ShortcutCommand(HandlerID::SaveState07),
            {
                KeyboardInput(WXK_F7, wxMOD_SHIFT)
            }},
        {ShortcutCommand(HandlerID::SaveState08),
            {
                KeyboardInput(WXK_F8, wxMOD_SHIFT)
            }},
        {ShortcutCommand(HandlerID::SaveState09),
            {
                KeyboardInput(WXK_F9, wxMOD_SHIFT)
            }},
        {ShortcutCommand(HandlerID::SaveState10),
            {
                KeyboardInput(WXK_F10, wxMOD_SHIFT)
            }},
        // I prefer the SDL ESC key binding
        // {ShortcutCommand(HandlerID::ToggleFullscreen),
        //     {
        //         KeyboardInput(WXK_ESCAPE, wxMOD_NONE)
        //     }},
        // alt-enter is more standard anyway
        {ShortcutCommand(HandlerID::ToggleFullscreen),
            {
                KeyboardInput(WXK_RETURN, wxMOD_ALT)
            }},
        {ShortcutCommand(HandlerID::JoypadAutofireA),
            {
                KeyboardInput('1', wxMOD_ALT)
            }},
        {ShortcutCommand(HandlerID::JoypadAutofireB),
            {
                KeyboardInput('2', wxMOD_ALT)
            }},
        {ShortcutCommand(HandlerID::JoypadAutofireL),
            {
                KeyboardInput('3', wxMOD_ALT)
            }},
        {ShortcutCommand(HandlerID::JoypadAutofireR),
            {
                KeyboardInput('4', wxMOD_ALT)
            }},
        {ShortcutCommand(HandlerID::VideoLayersBG0),
            {
                KeyboardInput('1', wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::VideoLayersBG1),
            {
                KeyboardInput('2', wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::VideoLayersBG2),
            {
                KeyboardInput('3', wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::VideoLayersBG3),
            {
                KeyboardInput('4', wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::VideoLayersOBJ),
            {
                KeyboardInput('5', wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::VideoLayersWIN0),
            {
                KeyboardInput('6', wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::VideoLayersWIN1),
            {
                KeyboardInput('7', wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::VideoLayersOBJWIN),
            {
                KeyboardInput('8', wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::Rewind),
            {
                KeyboardInput('B', wxMOD_CMD)
            }},
        // The following commands do not have the dafault wxWidgets shortcut.
        // The HandlerID::RecentFile1 shortcut is active when the first recent menu entry is populated.
        // The same goes for the others, HandlerID::RecentFile2 is active when the second recent menu entry is
        // populated, etc.
        {ShortcutCommand(HandlerID::RecentFile1),
            {
                KeyboardInput(WXK_F1, wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::RecentFile2),
            {
                KeyboardInput(WXK_F2, wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::RecentFile3),
            {
                KeyboardInput(WXK_F3, wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::RecentFile4),
            {
                KeyboardInput(WXK_F4, wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::RecentFile5),
            {
                KeyboardInput(WXK_F5, wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::RecentFile6),
            {
                KeyboardInput(WXK_F6, wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::RecentFile7),
            {
                KeyboardInput(WXK_F7, wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::RecentFile8),
            {
                KeyboardInput(WXK_F8, wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::RecentFile9),
            {
                KeyboardInput(WXK_F9, wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::RecentFile10),
            {
                KeyboardInput(WXK_F10, wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::VideoLayersReset),
            {
                KeyboardInput('0', wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::ChangeFilter),
            {
                KeyboardInput('G', wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::ChangeIFB),
            {
                KeyboardInput('I', wxMOD_CMD)
            }},
        {ShortcutCommand(HandlerID::IncreaseVolume),
            {
                KeyboardInput(WXK_NUMPAD_ADD, wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::DecreaseVolume),
            {
                KeyboardInput(WXK_NUMPAD_SUBTRACT, wxMOD_NONE)
            }},
        {ShortcutCommand(HandlerID::ToggleSound),
            {
                KeyboardInput(WXK_NUMPAD_ENTER, wxMOD_NONE)
            }},

        // Player 1 controls.
        {GameCommand(GameJoy(0), config::GameKey::Up),
         {
             KeyboardInput('W'),
             JoyInput(JoyId(0), JoyControl::Button, 11),
             JoyInput(JoyId(0), JoyControl::AxisMinus, 1),
             JoyInput(JoyId(0), JoyControl::AxisMinus, 3),
             JoyInput(JoyId(0), JoyControl::HatNorth, 0),
         }},
        {GameCommand(GameJoy(0), config::GameKey::Down),
            {
                KeyboardInput('S'),
                JoyInput(JoyId(0), JoyControl::Button, 12),
                JoyInput(JoyId(0), JoyControl::AxisPlus, 1),
                JoyInput(JoyId(0), JoyControl::AxisPlus, 3),
                JoyInput(JoyId(0), JoyControl::HatSouth, 0),
            }},
        {GameCommand(GameJoy(0), config::GameKey::Left),
            {
                KeyboardInput('A'),
                JoyInput(JoyId(0), JoyControl::Button, 13),
                JoyInput(JoyId(0), JoyControl::AxisMinus, 0),
                JoyInput(JoyId(0), JoyControl::AxisMinus, 2),
                JoyInput(JoyId(0), JoyControl::HatWest, 0),
            }},
        {GameCommand(GameJoy(0), config::GameKey::Right),
            {
                KeyboardInput('D'),
                JoyInput(JoyId(0), JoyControl::Button, 14),
                JoyInput(JoyId(0), JoyControl::AxisPlus, 0),
                JoyInput(JoyId(0), JoyControl::AxisPlus, 2),
                JoyInput(JoyId(0), JoyControl::HatEast, 0),
            }},
        {GameCommand(GameJoy(0), config::GameKey::A),
            {
                KeyboardInput('L'),
                JoyInput(JoyId(0), JoyControl::Button, 1),
            }},
        {GameCommand(GameJoy(0), config::GameKey::B),
            {
                KeyboardInput('K'),
                JoyInput(JoyId(0), JoyControl::Button, 0),
            }},
        {GameCommand(GameJoy(0), config::GameKey::L),
            {
                KeyboardInput('I'),
                JoyInput(JoyId(0), JoyControl::Button, 2),
                JoyInput(JoyId(0), JoyControl::Button, 9),
                JoyInput(JoyId(0), JoyControl::AxisPlus, 4),
            }},
        {GameCommand(GameJoy(0), config::GameKey::R),
            {
                KeyboardInput('O'),
                JoyInput(JoyId(0), JoyControl::Button, 3),
                JoyInput(JoyId(0), JoyControl::Button, 10),
                JoyInput(JoyId(0), JoyControl::AxisPlus, 5),
            }},
        {GameCommand(GameJoy(0), config::GameKey::Select),
            {
                KeyboardInput(WXK_BACK),
                JoyInput(JoyId(0), JoyControl::Button, 4),
            }},
        {GameCommand(GameJoy(0), config::GameKey::Start),
            {
                KeyboardInput(WXK_RETURN),
                JoyInput(JoyId(0), JoyControl::Button, 6),
            }},
        {GameCommand(GameJoy(0), config::GameKey::MotionUp), {}},
        {GameCommand(GameJoy(0), config::GameKey::MotionDown), {}},
        {GameCommand(GameJoy(0), config::GameKey::MotionLeft), {}},
        {GameCommand(GameJoy(0), config::GameKey::MotionRight), {}},
        {GameCommand(GameJoy(0), config::GameKey::MotionIn), {}},
        {GameCommand(GameJoy(0), config::GameKey::MotionOut), {}},
        {GameCommand(GameJoy(0), config::GameKey::AutoA), {}},
        {GameCommand(GameJoy(0), config::GameKey::AutoB), {}},
        {GameCommand(GameJoy(0), config::GameKey::Speed),
            {
                KeyboardInput(WXK_SPACE),
            }},
        {GameCommand(GameJoy(0), config::GameKey::Capture), {}},
        {GameCommand(GameJoy(0), config::GameKey::Gameshark), {}},

        // Player 2 controls.
        {GameCommand(GameJoy(1), config::GameKey::Up),
            {
                JoyInput(JoyId(1), JoyControl::Button, 11),
                JoyInput(JoyId(1), JoyControl::AxisMinus, 1),
                JoyInput(JoyId(1), JoyControl::AxisMinus, 3),
                JoyInput(JoyId(1), JoyControl::HatNorth, 0),
            }},
        {GameCommand(GameJoy(1), config::GameKey::Down),
            {
                JoyInput(JoyId(1), JoyControl::Button, 12),
                JoyInput(JoyId(1), JoyControl::AxisPlus, 1),
                JoyInput(JoyId(1), JoyControl::AxisPlus, 3),
                JoyInput(JoyId(1), JoyControl::HatSouth, 0),
            }},
        {GameCommand(GameJoy(1), config::GameKey::Left),
            {
                JoyInput(JoyId(1), JoyControl::Button, 13),
                JoyInput(JoyId(1), JoyControl::AxisMinus, 0),
                JoyInput(JoyId(1), JoyControl::AxisMinus, 2),
                JoyInput(JoyId(1), JoyControl::HatWest, 0),
            }},
        {GameCommand(GameJoy(1), config::GameKey::Right),
            {
                JoyInput(JoyId(1), JoyControl::Button, 14),
                JoyInput(JoyId(1), JoyControl::AxisPlus, 0),
                JoyInput(JoyId(1), JoyControl::AxisPlus, 2),
                JoyInput(JoyId(1), JoyControl::HatEast, 0),
            }},
        {GameCommand(GameJoy(1), config::GameKey::A),
            {
                JoyInput(JoyId(1), JoyControl::Button, 1),
            }},
        {GameCommand(GameJoy(1), config::GameKey::B),
            {
                JoyInput(JoyId(1), JoyControl::Button, 0),
            }},
        {GameCommand(GameJoy(1), config::GameKey::L),
            {
                JoyInput(JoyId(1), JoyControl::Button, 2),
                JoyInput(JoyId(1), JoyControl::Button, 9),
                JoyInput(JoyId(1), JoyControl::AxisPlus, 4),
            }},
        {GameCommand(GameJoy(1), config::GameKey::R),
            {
                JoyInput(JoyId(1), JoyControl::Button, 3),
                JoyInput(JoyId(1), JoyControl::Button, 10),
                JoyInput(JoyId(1), JoyControl::AxisPlus, 5),
            }},
        {GameCommand(GameJoy(1), config::GameKey::Select),
            {
                JoyInput(JoyId(1), JoyControl::Button, 4),
            }},
        {GameCommand(GameJoy(1), config::GameKey::Start),
            {
                JoyInput(JoyId(1), JoyControl::Button, 6),
            }},
        {GameCommand(GameJoy(1), config::GameKey::MotionUp), {}},
        {GameCommand(GameJoy(1), config::GameKey::MotionDown), {}},
        {GameCommand(GameJoy(1), config::GameKey::MotionLeft), {}},
        {GameCommand(GameJoy(1), config::GameKey::MotionRight), {}},
        {GameCommand(GameJoy(1), config::GameKey::MotionIn), {}},
        {GameCommand(GameJoy(1), config::GameKey::MotionOut), {}},
        {GameCommand(GameJoy(1), config::GameKey::AutoA), {}},
        {GameCommand(GameJoy(1), config::GameKey::AutoB), {}},
        {GameCommand(GameJoy(1), config::GameKey::Speed), {}},
        {GameCommand(GameJoy(1), config::GameKey::Capture), {}},
        {GameCommand(GameJoy(1), config::GameKey::Gameshark), {}},

        // Player 3 controls.
        {GameCommand(GameJoy(2), config::GameKey::Up),
            {
                JoyInput(JoyId(2), JoyControl::Button, 11),
                JoyInput(JoyId(2), JoyControl::AxisMinus, 1),
                JoyInput(JoyId(2), JoyControl::AxisMinus, 3),
                JoyInput(JoyId(2), JoyControl::HatNorth, 0),
            }},
        {GameCommand(GameJoy(2), config::GameKey::Down),
            {
                JoyInput(JoyId(2), JoyControl::Button, 12),
                JoyInput(JoyId(2), JoyControl::AxisPlus, 1),
                JoyInput(JoyId(2), JoyControl::AxisPlus, 3),
                JoyInput(JoyId(2), JoyControl::HatSouth, 0),
            }},
        {GameCommand(GameJoy(2), config::GameKey::Left),
            {
                JoyInput(JoyId(2), JoyControl::Button, 13),
                JoyInput(JoyId(2), JoyControl::AxisMinus, 0),
                JoyInput(JoyId(2), JoyControl::AxisMinus, 2),
                JoyInput(JoyId(2), JoyControl::HatWest, 0),
            }},
        {GameCommand(GameJoy(2), config::GameKey::Right),
            {
                JoyInput(JoyId(2), JoyControl::Button, 14),
                JoyInput(JoyId(2), JoyControl::AxisPlus, 0),
                JoyInput(JoyId(2), JoyControl::AxisPlus, 2),
                JoyInput(JoyId(2), JoyControl::HatEast, 0),
            }},
        {GameCommand(GameJoy(2), config::GameKey::A),
            {
                JoyInput(JoyId(2), JoyControl::Button, 1),
            }},
        {GameCommand(GameJoy(2), config::GameKey::B),
            {
                JoyInput(JoyId(2), JoyControl::Button, 0),
            }},
        {GameCommand(GameJoy(2), config::GameKey::L),
            {
                JoyInput(JoyId(2), JoyControl::Button, 2),
                JoyInput(JoyId(2), JoyControl::Button, 9),
                JoyInput(JoyId(2), JoyControl::AxisPlus, 4),
            }},
        {GameCommand(GameJoy(2), config::GameKey::R),
            {
                JoyInput(JoyId(2), JoyControl::Button, 3),
                JoyInput(JoyId(2), JoyControl::Button, 10),
                JoyInput(JoyId(2), JoyControl::AxisPlus, 5),
            }},
        {GameCommand(GameJoy(2), config::GameKey::Select),
            {
                JoyInput(JoyId(2), JoyControl::Button, 4),
            }},
        {GameCommand(GameJoy(2), config::GameKey::Start),
            {
                JoyInput(JoyId(2), JoyControl::Button, 6),
            }},
        {GameCommand(GameJoy(2), config::GameKey::MotionUp), {}},
        {GameCommand(GameJoy(2), config::GameKey::MotionDown), {}},
        {GameCommand(GameJoy(2), config::GameKey::MotionLeft), {}},
        {GameCommand(GameJoy(2), config::GameKey::MotionRight), {}},
        {GameCommand(GameJoy(2), config::GameKey::MotionIn), {}},
        {GameCommand(GameJoy(2), config::GameKey::MotionOut), {}},
        {GameCommand(GameJoy(2), config::GameKey::AutoA), {}},
        {GameCommand(GameJoy(2), config::GameKey::AutoB), {}},
        {GameCommand(GameJoy(2), config::GameKey::Speed), {}},
        {GameCommand(GameJoy(2), config::GameKey::Capture), {}},
        {GameCommand(GameJoy(2), config::GameKey::Gameshark), {}},

        // Player 4 controls.
        {GameCommand(GameJoy(3), config::GameKey::Up),
            {
                JoyInput(JoyId(3), JoyControl::Button, 11),
                JoyInput(JoyId(3), JoyControl::AxisMinus, 1),
                JoyInput(JoyId(3), JoyControl::AxisMinus, 3),
                JoyInput(JoyId(3), JoyControl::HatNorth, 0),
            }},
        {GameCommand(GameJoy(3), config::GameKey::Down),
            {
                JoyInput(JoyId(3), JoyControl::Button, 12),
                JoyInput(JoyId(3), JoyControl::AxisPlus, 1),
                JoyInput(JoyId(3), JoyControl::AxisPlus, 3),
                JoyInput(JoyId(3), JoyControl::HatSouth, 0),
            }},
        {GameCommand(GameJoy(3), config::GameKey::Left),
            {
                JoyInput(JoyId(3), JoyControl::Button, 13),
                JoyInput(JoyId(3), JoyControl::AxisMinus, 0),
                JoyInput(JoyId(3), JoyControl::AxisMinus, 2),
                JoyInput(JoyId(3), JoyControl::HatWest, 0),
            }},
        {GameCommand(GameJoy(3), config::GameKey::Right),
            {
                JoyInput(JoyId(3), JoyControl::Button, 14),
                JoyInput(JoyId(3), JoyControl::AxisPlus, 0),
                JoyInput(JoyId(3), JoyControl::AxisPlus, 2),
                JoyInput(JoyId(3), JoyControl::HatEast, 0),
            }},
        {GameCommand(GameJoy(3), config::GameKey::A),
            {
                JoyInput(JoyId(3), JoyControl::Button, 1),
            }},
        {GameCommand(GameJoy(3), config::GameKey::B),
            {
                JoyInput(JoyId(3), JoyControl::Button, 0),
            }},
        {GameCommand(GameJoy(3), config::GameKey::L),
            {
                JoyInput(JoyId(3), JoyControl::Button, 2),
                JoyInput(JoyId(3), JoyControl::Button, 9),
                JoyInput(JoyId(3), JoyControl::AxisPlus, 4),
            }},
        {GameCommand(GameJoy(3), config::GameKey::R),
            {
                JoyInput(JoyId(3), JoyControl::Button, 3),
                JoyInput(JoyId(3), JoyControl::Button, 10),
                JoyInput(JoyId(3), JoyControl::AxisPlus, 5),
            }},
        {GameCommand(GameJoy(3), config::GameKey::Select),
            {
                JoyInput(JoyId(3), JoyControl::Button, 4),
            }},
        {GameCommand(GameJoy(3), config::GameKey::Start),
            {
                JoyInput(JoyId(3), JoyControl::Button, 6),
            }},
        {GameCommand(GameJoy(3), config::GameKey::MotionUp), {}},
        {GameCommand(GameJoy(3), config::GameKey::MotionDown), {}},
        {GameCommand(GameJoy(3), config::GameKey::MotionLeft), {}},
        {GameCommand(GameJoy(3), config::GameKey::MotionRight), {}},
        {GameCommand(GameJoy(3), config::GameKey::MotionIn), {}},
        {GameCommand(GameJoy(3), config::GameKey::MotionOut), {}},
        {GameCommand(GameJoy(3), config::GameKey::AutoA), {}},
        {GameCommand(GameJoy(3), config::GameKey::AutoB), {}},
        {GameCommand(GameJoy(3), config::GameKey::Speed), {}},
        {GameCommand(GameJoy(3), config::GameKey::Capture), {}},
        {GameCommand(GameJoy(3), config::GameKey::Gameshark), {}},
    };
    // clang-format on
    return kDefaultInputs;
}

const std::unordered_set<UserInput>& DefaultInputsForCommand(const Command& command) {
    const auto& iter = DefaultInputs().find(command);
    if (iter != DefaultInputs().end()) {
        return iter->second;
    }
    static const std::unordered_set<UserInput> kEmptySet;
    return kEmptySet;
}

bool IsDefaultInputForCommand(const Command& command, const UserInput& input) {
    const auto& iter = DefaultInputs().find(command);
    if (iter != DefaultInputs().end()) {
        return iter->second.find(input) != iter->second.end();
    }
    return false;
}

}  // namespace internal
}  // namespace config
