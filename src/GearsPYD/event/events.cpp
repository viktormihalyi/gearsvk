#include "event/events.h"
#include "stdafx.h"

#ifdef _WIN32

#define NOMINMAX
#include <windows.h>

uint Gears::Event::MouseMove::typeId (WM_MOUSEMOVE);
uint Gears::Event::KeyPressed::typeId (WM_KEYDOWN);
uint Gears::Event::KeyReleased::typeId (WM_KEYUP);
uint Gears::Event::MousePressedLeft::typeId (WM_LBUTTONDOWN);
uint Gears::Event::MouseReleasedLeft::typeId (WM_LBUTTONUP);
uint Gears::Event::MousePressedMiddle::typeId (WM_MBUTTONDOWN);
uint Gears::Event::MouseReleasedMiddle::typeId (WM_MBUTTONUP);
uint Gears::Event::MousePressedRight::typeId (WM_RBUTTONDOWN);
uint Gears::Event::MouseReleasedRight::typeId (WM_RBUTTONUP);
uint Gears::Event::Wheel::typeId (WM_MOUSEWHEEL);
uint Gears::Event::StimulusStart::typeId (WM_USER);
uint Gears::Event::Frame::typeId (WM_USER + 1);
uint Gears::Event::StimulusEnd::typeId (WM_USER + 2);

#elif __linux__

#include <X11/Xlib.h>
// TODO: right types for event handling
uint Gears::Event::MouseMove::typeId (MotionNotify);
uint Gears::Event::KeyPressed::typeId (KeyPress);
uint Gears::Event::KeyReleased::typeId (KeyRelease);
uint Gears::Event::MousePressedLeft::typeId (ButtonPress);
uint Gears::Event::MouseReleasedLeft::typeId (ButtonRelease);
uint Gears::Event::MousePressedMiddle::typeId (ButtonPress);
uint Gears::Event::MouseReleasedMiddle::typeId (ButtonRelease);
uint Gears::Event::MousePressedRight::typeId (ButtonPress);
uint Gears::Event::MouseReleasedRight::typeId (ButtonRelease);
uint Gears::Event::Wheel::typeId (ButtonPress);
uint Gears::Event::StimulusStart::typeId (EnterNotify);
uint Gears::Event::Frame::typeId (EnterNotify);
uint Gears::Event::StimulusEnd::typeId (EnterNotify);
#endif