#include "wx/main-frame.h"

#include "components/filters_interframe/interframe.h"
#include "wx/config/option-proxy.h"
#include "wx/opts.h"

namespace {

// FIXME: remove this.
void toggleBooleanVar(bool* menuValue, bool* globalVar) {
    if (*menuValue == *globalVar)  // used accelerator
        *globalVar = !(*globalVar);
    else  // used menu item
        *globalVar = *menuValue;
}

}  // namespace

void MainFrame::OnPause() {
    bool menuPress = false;
    GetMenuOptionBool("Pause", &menuPress);
    toggleBooleanVar(&menuPress, &paused);
    SetMenuOption("Pause", paused ? 1 : 0);

    if (paused)
        panel->Pause();
    else if (!IsPaused())
        panel->Resume();

    // undo next-this's zeroing of frameskip
    const int frame_skip = OPTION(kPrefFrameSkip);
    if (frame_skip != -1) {
        systemFrameSkip = frame_skip;
    }
}

void MainFrame::OnNextFrame() {
    SetMenuOption("Pause", true);
    paused = true;
    pause_next = true;

    if (!IsPaused())
        GetPanel()->Resume();

    systemFrameSkip = 0;
}

void MainFrame::OnRewind() {
    int rew_st = (panel->next_rewind_state + NUM_REWINDS - 1) % NUM_REWINDS;

    // if within 5 seconds of last one, and > 1 state, delete last state & move back
    // FIXME: 5 should actually be user-configurable
    // maybe instead of 5, 10% of rewind_interval
    if (panel->num_rewind_states > 1 &&
        (gopts.rewind_interval <= 5 || (int)panel->rewind_time / 6 > gopts.rewind_interval - 5)) {
        --panel->num_rewind_states;
        GetPanel()->next_rewind_state = rew_st;

        if (gopts.rewind_interval > 5)
            rew_st = (rew_st + NUM_REWINDS - 1) % NUM_REWINDS;
    }

    GetPanel()->emusys->emuReadMemState(&panel->rewind_mem[rew_st * REWIND_SIZE], REWIND_SIZE);
    InterframeCleanup();
    // FIXME: if(paused) blank screen
    GetPanel()->do_rewind = false;
    GetPanel()->rewind_time = gopts.rewind_interval * 6;
    //    systemScreenMessage(_("Rewinded"));
}

void MainFrame::OnToggleFullscreen() {
    GetPanel()->ShowFullScreen(!IsFullScreen());
}

void MainFrame::OnEmulatorSpeedupToggle() {
    bool menuPress = false;
    GetMenuOptionBool("EmulatorSpeedupToggle", &menuPress);
    toggleBooleanVar(&menuPress, &turbo);
    SetMenuOption("EmulatorSpeedupToggle", turbo ? 1 : 0);
}

void MainFrame::OnVSync() {
    GetMenuOptionConfig("VSync", config::OptionID::kPrefVsync);
}

void MainFrame::OnFrameSkipAuto() {
    GetMenuOptionConfig("FrameSkipAuto", config::OptionID::kPrefAutoFrameSkip);
}

void MainFrame::OnSkipIntro() {
    GetMenuOptionConfig("SkipIntro", config::OptionID::kPrefSkipBios);
}

void MainFrame::OnApplyPatches() {
    GetMenuOptionConfig("ApplyPatches", config::OptionID::kPrefAutoPatch);
}

void MainFrame::OnPauseWhenInactive() {
    GetMenuOptionConfig("PauseWhenInactive", config::OptionID::kPrefPauseWhenInactive);
}

void MainFrame::OnReset() {
    GetPanel()->emusys->emuReset();
    // systemScreenMessage("Reset");
}
