#include <cstddef>
#include "wxvbam.h"
#include "wx/sdljoy.h"
#include "SDL.h"
#include <SDL_events.h>
#include "../common/range.hpp"
#include "../common/contains.h"

using namespace Range;

// For testing a GameController as a Joystick:
//#define SDL_IsGameController(x) false

DEFINE_EVENT_TYPE(wxEVT_SDLJOY)

wxSDLJoy::wxSDLJoy()
    : wxTimer()
{
    // Start up joystick if not already started
    // FIXME: check for errors
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    SDL_GameControllerEventState(SDL_ENABLE);
    SDL_JoystickEventState(SDL_ENABLE);

    for (uint8_t joy : range(0, SDL_NumJoysticks()))
        ConnectController(joy);
}

wxSDLJoy::~wxSDLJoy()
{
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

static int16_t axisval(int16_t x)
{
    if (x > 0x1fff)
        return 1;
    else if (x < -0x1fff)
        return -1;

    return 0;
}

void wxSDLJoy::CreateAndSendEvent(unsigned short joy, unsigned short ctrl_type, unsigned short ctrl_idx, short ctrl_val, short prev_val)
{
    auto handler = wxGetApp().frame->GetJoyEventHandler();
    if (!handler)
        return;

    wxSDLJoyEvent *ev = new wxSDLJoyEvent(wxEVT_SDLJOY);
    ev->joy           = joy;
    ev->ctrl_type     = ctrl_type;
    ev->ctrl_idx      = ctrl_idx;
    ev->ctrl_val      = ctrl_val;
    ev->prev_val      = prev_val;

    wxQueueEvent(handler, ev);
}

void wxSDLJoy::Poll()
{
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            {
                SDL_JoystickID joy = e.cbutton.which;

                if (!SDL_IsGameController(joy))
                    break;

                if (contains(joystate_, joy)) {
                    auto but      = e.cbutton.button;
                    auto val      = e.cbutton.state;
                    auto prev_val = joystate_[joy].button[but];

                    if (val != prev_val) {
                        CreateAndSendEvent(joy, WXSDLJOY_BUTTON, but, val, prev_val);
                        joystate_[joy].button[but] = val;
                        wxLogDebug("GOT SDL_CONTROLLERBUTTON: joy:%d but:%d val:%d prev_val:%d", joy, but, val, prev_val);
                    }
                }

                break;
            }
            case SDL_CONTROLLERAXISMOTION:
            {
                SDL_JoystickID joy = e.caxis.which;

                if (!SDL_IsGameController(joy))
                    break;

                if (contains(joystate_, joy)) {
                    auto axis     = e.caxis.axis;
                    auto val      = axisval(e.caxis.value);
                    auto prev_val = joystate_[joy].axis[axis];

                    if (val != prev_val) {
                        CreateAndSendEvent(joy, WXSDLJOY_AXIS, axis, val, prev_val);
                        joystate_[joy].axis[axis] = val;
                        wxLogDebug("GOT SDL_CONTROLLERAXISMOTION: joy:%d axis:%d val:%d prev_val:%d", joy, axis, val, prev_val);
                    }
                }

                break;
            }
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMAPPED:
            {
                SDL_JoystickID joy = e.cdevice.which;

                if (!SDL_IsGameController(joy))
                    break;

                if (contains(joystate_, joy)) {
                    DisconnectController(joy);
                    ConnectController(joy);
                    systemScreenMessage(wxString::Format(_("Connected game controller %d"), joy + 1));
                }

                break;
            }
            case SDL_CONTROLLERDEVICEREMOVED:
            {
                SDL_JoystickID joy = e.cdevice.which;

                if (contains(joystate_, joy)) {
                    DisconnectController(joy);
                    systemScreenMessage(wxString::Format(_("Disconnected game controller %d"), joy + 1));
                }

                break;
            }

            // Joystick events for non-GameControllers.
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
            {
                SDL_JoystickID joy = e.jbutton.which;

                if (SDL_IsGameController(joy))
                    break;

                if (contains(joystate_, joy)) {
                    auto but      = e.jbutton.button;
                    auto val      = e.jbutton.state;
                    auto prev_val = joystate_[joy].button[but];

                    if (val != prev_val) {
                        CreateAndSendEvent(joy, WXSDLJOY_BUTTON, but, val, prev_val);
                        joystate_[joy].button[but] = val;
                        wxLogDebug("GOT SDL_JOYBUTTON: joy:%d but:%d val:%d prev_val:%d", joy, but, val, prev_val);
                    }
                }

                break;
            }
            case SDL_JOYAXISMOTION:
            {
                auto joy = e.jaxis.which;

                if (SDL_IsGameController(joy))
                    break;

                if (contains(joystate_, joy)) {
                    auto axis     = e.jaxis.axis;
                    auto val      = axisval(e.jaxis.value);
                    auto prev_val = joystate_[joy].axis[axis];

                    if (val != prev_val) {
                        CreateAndSendEvent(joy, WXSDLJOY_AXIS, axis, val, prev_val);
                        joystate_[joy].axis[axis] = val;
                        wxLogDebug("GOT SDL_JOYAXISMOTION: joy:%d axis:%d val:%d prev_val:%d", joy, axis, val, prev_val);
                    }
                }

                break;
            }
            case SDL_JOYDEVICEADDED:
            {
                auto joy = e.cdevice.which;

                if (SDL_IsGameController(joy))
                    break;

                if (contains(joystate_, joy)) {
                    DisconnectController(joy);
                    ConnectController(joy);
                    systemScreenMessage(wxString::Format(_("Connected joystick %d"), joy + 1));
                }

                break;
            }
            case SDL_JOYDEVICEREMOVED:
            {
                auto joy = e.cdevice.which;

                if (SDL_IsGameController(joy))
                    break;

                if (contains(joystate_, joy)) {
                    DisconnectController(joy);
                    systemScreenMessage(wxString::Format(_("Disconnected joystick %d"), joy + 1));
                }

                break;
            }
        }
    }

}

void wxSDLJoy::ConnectController(uint8_t joy)
{
    if (SDL_IsGameController(joy)) {
        if (!(joystate_[joy].dev = SDL_GameControllerOpen(joy))) {
            wxLogDebug("SDL_GameControllerOpen(%d) failed: %s", joy, SDL_GetError());
            return;
        }
    } else {
        if (!(joystate_[joy].dev = SDL_JoystickOpen(joy))) {
            wxLogDebug("SDL_JoystickOpen(%d) failed: %s", joy, SDL_GetError());
            return;
        }
    }
}

void wxSDLJoy::DisconnectController(uint8_t joy)
{
    if (auto& dev = joystate_[joy].dev) {
        if (SDL_IsGameController(joy)) {
            if (SDL_GameControllerGetAttached(dev))
                SDL_GameControllerClose(dev);
        } else {
            if (SDL_JoystickGetAttached(dev))
                SDL_JoystickClose(dev);
        }

        dev = nullptr;
    }
}

void wxSDLJoy::SetRumble(bool do_rumble)
{
    rumbling_ = do_rumble;

#if SDL_VERSION_ATLEAST(2, 0, 9)
    // Do rumble only on device 0, and only if it's a GameController.
    auto dev = joystate_[0].dev;
    if (dev && SDL_IsGameController(0)) {
        if (rumbling_) {
            SDL_GameControllerRumble(dev, 0xFFFF, 0xFFFF, 300);
            if (!IsRunning())
                Start(150);
        }
        else {
            SDL_GameControllerRumble(dev, 0, 0, 0);
            Stop();
        }
    }
#endif
}

void wxSDLJoy::Notify()
{
    SetRumble(rumbling_);
}

wxSDLJoyDev::operator SDL_GameController*&()
{
    return dev_gc;
}

SDL_GameController*& wxSDLJoyDev::operator=(SDL_GameController* ptr)
{
    dev_gc = ptr;
    return dev_gc;
}

wxSDLJoyDev::operator SDL_Joystick*&()
{
    return dev_js;
}

SDL_Joystick*& wxSDLJoyDev::operator=(SDL_Joystick* ptr)
{
    dev_js = ptr;
    return dev_js;
}

wxSDLJoyDev::operator bool()
{
    return dev_gc != nullptr;
}

std::nullptr_t& wxSDLJoyDev::operator=(std::nullptr_t&& null_ptr)
{
    dev_gc = null_ptr;
    return null_ptr;
}
