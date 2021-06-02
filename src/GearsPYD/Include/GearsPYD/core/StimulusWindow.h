#pragma once

#ifdef __linux__
#include <X11/Xlib.h> // Every Xlib program must include this
#include <X11/Xutil.h>
#endif

#include <algorithm>
#include <glm/glm.hpp>
#include <iomanip>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

class SequenceRenderer;


class StimulusWindow {
#ifdef _WIN32
    HDC   hdc;   // device context handle
    HGLRC hglrc; // OpenGL rendering context
    HWND  hwnd;  // window handle
    bool  fullscreen;
    int   indexPixelFormat; // number of available pixel formats

    HCURSOR arrowCursor;
    HCURSOR crossCursor;
    HCURSOR handCursor;

    static WNDCLASSEX ex;

    //static PFNWGLSWAPINTERVALEXTPROC    wglSwapIntervalEXT;
    //static PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;

    struct Monitor {
        HMONITOR hMonitor;
        HDC      hdcMonitor;
        RECT     rcMonitor;
    };
    std::vector<Monitor> monitors;
    uint                 currentMonitor;
#elif __linux__
    Display*                         display;
    Window                           wnd;
    GLXContext                       ctx;
    Colormap                         cmap;
    GLXFBConfig                      bestFbc;
    static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
    long                             eventMask = KeyPressMask;
#endif

    int  screenw;  // when window is resized, the new dimensions...
    int  screenh;  // ...are stored in these variables
    int  vscreenw; // when window is resized, the new dimensions...
    int  vscreenh; // ...are stored in these variables
    bool quit;     // indicates the state of application

    std::string           glSpecs;
    std::shared_ptr<SequenceRenderer> sequenceRenderer;
    std::shared_ptr<Ticker>           ticker;
    StimulusWindow ();

    pybind11::object onHideCallback;


    void swapBuffers ();
    void render ();
    void preRender ();
    void postRender ();

public:
    GEARS_SHARED_CREATE (StimulusWindow);
    static std::shared_ptr<StimulusWindow> instanceCreated;
    void                       createWindow (bool windowed, uint width, uint height);
    void                       run ();
    void                       closeWindow ();
    ~StimulusWindow () {}

#ifdef _WIN32
    static LRESULT CALLBACK WindowProc (
        _In_ HWND   hwnd,
        _In_ UINT   uMsg,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam);
    LRESULT     winProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static void registerClass ();
    void        shareCurrent ();
    void        setGLFormat (void);

    void addMonitor (
        HMONITOR hMonitor,
        HDC      hdcMonitor,
        LPRECT   lprcMonitor);

#elif __linux__
    void                             shareCurrent (unsigned int winId);
#endif

    std::string getSpecs ()
    {
        return glSpecs;
    }

    int  setSwapInterval (int swapInterval);
    void makeCurrent ();
    void setCursorPos ();

    void setSequenceRenderer (std::shared_ptr<SequenceRenderer> sequenceRenderer);

    void onHide (pybind11::object onHide)
    {
        onHideCallback = onHide;
    }
};
