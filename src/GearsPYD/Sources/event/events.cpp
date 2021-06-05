#include "event/events.h"

#ifdef _WIN32

#define NOMINMAX
#include <windows.h>

uint32_t Gears::Event::MouseMove::typeId (WM_MOUSEMOVE);
uint32_t Gears::Event::KeyPressed::typeId (WM_KEYDOWN);
uint32_t Gears::Event::KeyReleased::typeId (WM_KEYUP);
uint32_t Gears::Event::MousePressedLeft::typeId (WM_LBUTTONDOWN);
uint32_t Gears::Event::MouseReleasedLeft::typeId (WM_LBUTTONUP);
uint32_t Gears::Event::MousePressedMiddle::typeId (WM_MBUTTONDOWN);
uint32_t Gears::Event::MouseReleasedMiddle::typeId (WM_MBUTTONUP);
uint32_t Gears::Event::MousePressedRight::typeId (WM_RBUTTONDOWN);
uint32_t Gears::Event::MouseReleasedRight::typeId (WM_RBUTTONUP);
uint32_t Gears::Event::Wheel::typeId (WM_MOUSEWHEEL);
uint32_t Gears::Event::StimulusStart::typeId (WM_USER);
uint32_t Gears::Event::Frame::typeId (WM_USER + 1);
uint32_t Gears::Event::StimulusEnd::typeId (WM_USER + 2);

#elif __linux__

#include <X11/Xlib.h>
// TODO: right types for event handling
uint32_t Gears::Event::MouseMove::typeId (MotionNotify);
uint32_t Gears::Event::KeyPressed::typeId (KeyPress);
uint32_t Gears::Event::KeyReleased::typeId (KeyRelease);
uint32_t Gears::Event::MousePressedLeft::typeId (ButtonPress);
uint32_t Gears::Event::MouseReleasedLeft::typeId (ButtonRelease);
uint32_t Gears::Event::MousePressedMiddle::typeId (ButtonPress);
uint32_t Gears::Event::MouseReleasedMiddle::typeId (ButtonRelease);
uint32_t Gears::Event::MousePressedRight::typeId (ButtonPress);
uint32_t Gears::Event::MouseReleasedRight::typeId (ButtonRelease);
uint32_t Gears::Event::Wheel::typeId (ButtonPress);
uint32_t Gears::Event::StimulusStart::typeId (EnterNotify);
uint32_t Gears::Event::Frame::typeId (EnterNotify);
uint32_t Gears::Event::StimulusEnd::typeId (EnterNotify);
#endif