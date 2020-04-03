#include "stdafx.h"

#ifdef _WIN32
#include "StimulusWindow.h"

#include "Event/events.h"
#include <iostream>
#include <sstream>

//extern HINSTANCE gearsDllInstance;

StimulusWindow::StimulusWindow ()
{
    quit             = false;
    sequenceRenderer = nullptr;
    ticker           = nullptr;
    //wglSwapIntervalEXT    = nullptr;
    //wglGetSwapIntervalEXT = nullptr;
}

BOOL CALLBACK MonitorEnumProc (
    _In_ HMONITOR hMonitor,
    _In_ HDC      hdcMonitor,
    _In_ LPRECT   lprcMonitor,
    _In_ LPARAM   dwData)
{
    ((StimulusWindow*)dwData)->addMonitor (hMonitor, hdcMonitor, lprcMonitor);
    return true;
}

void StimulusWindow::createWindow (bool windowed, uint width, uint height)
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    currentMonitor = 0;
    screenw        = GetSystemMetrics (SM_CXSCREEN);
    screenh        = GetSystemMetrics (SM_CYSCREEN);
    vscreenw       = GetSystemMetrics (SM_CXVIRTUALSCREEN);
    vscreenh       = GetSystemMetrics (SM_CYVIRTUALSCREEN);
    // center position of the window
    int posx = (screenw / 2) - (width / 2);
    int posy = (screenh / 2) - (height / 2);

    // set up the window for a windowed application by default
    long wndStyle = WS_OVERLAPPEDWINDOW;
    fullscreen    = false;

    EnumDisplayMonitors (NULL, NULL, MonitorEnumProc, (LPARAM)this);

    if (!windowed) // create a full-screen application if requested
    {
        wndStyle   = WS_POPUP;
        fullscreen = true;
        posx       = 0;
        posy       = 0;

        /*		// change resolution before the window is created
		DEVMODE dmode;

		memset(&dmode, 0, sizeof(DEVMODE));
		dmode.dmSize = sizeof(DEVMODE);
		dmode.dmPelsWidth = screenw;
		dmode.dmPelsHeight = screenh;
		dmode.dmBitsPerPel = 16;
		dmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

		EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dmode);

		// change resolution, if possible
		LONG dispResult = ChangeDisplaySettings(&dmode, CDS_FULLSCREEN);
		if(dispResult != DISP_CHANGE_SUCCESSFUL)
		{
			// if not... failed to change resolution
			fullscreen = false;

			std::stringstream ss;
			ss << "Could not change display mode to fullscreen. " << std::endl;
			std::cerr << ss.str();
			PyErr_SetString(PyExc_RuntimeError, ss.str().c_str());
			boost::python::throw_error_already_set();

			closeWindow();
		}*/
        width  = screenw;
        height = screenh;
    }

    instanceCreated = this->getSharedPtr ();
    // create the window
    hwnd = CreateWindowEx (NULL,
                           "GearsStimulusWindow",
                           "Stimulus window",
                           wndStyle | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                           posx, posy,
                           width, height,
                           NULL,
                           NULL,
                           gearsDllInstance,
                           NULL);

    SetWindowLong (hwnd, GWLP_USERDATA, (long)this);

    arrowCursor = LoadCursorFromFile ("Gui/BlackRedN.cur");
    crossCursor = LoadCursorFromFile ("Gui/BlackRedCross.cur");
    handCursor  = LoadCursorFromFile ("Gui/BlackRedHand.cur");
    // at this point WM_CREATE message is sent/received
    // the WM_CREATE branch inside WinProc function will execute here

    HWND splash = FindWindow ("gearsSplashScreen", "Loading Gears...");
    ShowWindow (splash, 0);

#endif
}


void StimulusWindow::run ()
{
    MSG msg;

    //	DEVMODE dmode;
    //	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dmode);
    //	LONG dispResult = ChangeDisplaySettings(&dmode, CDS_FULLSCREEN);

    currentMonitor = sequenceRenderer->getSequence ()->getMonitorIndex ();
    if (currentMonitor >= this->monitors.size ())
        currentMonitor = this->monitors.size ();
    Monitor& m = monitors[currentMonitor];
    MoveWindow (hwnd, m.rcMonitor.left, m.rcMonitor.top, m.rcMonitor.right - m.rcMonitor.left, m.rcMonitor.bottom - m.rcMonitor.top, false);

    ShowWindow (hwnd, SW_SHOW); // everything went OK, show the window
    UpdateWindow (hwnd);

    GetFocus ();

    preRender ();

    quit = false;
    while (!quit) {
        if (PeekMessage (&msg, NULL, NULL, NULL, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                quit = true;

            TranslateMessage (&msg);
            DispatchMessage (&msg);
        }

        render ();

        //if(GetAsyncKeyState(VK_ESCAPE))
        //{
        //	quit = true;
        //}
    }

    postRender ();

    while (PeekMessage (&msg, hwnd, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
        ;
    ShowWindow (hwnd, SW_HIDE);
}

void StimulusWindow::swapBuffers ()
{
    SwapBuffers (hdc);
}

WNDCLASSEX StimulusWindow::ex;

//PFNWGLSWAPINTERVALEXTPROC    StimulusWindow::wglSwapIntervalEXT (nullptr);
//PFNWGLGETSWAPINTERVALEXTPROC StimulusWindow::wglGetSwapIntervalEXT (nullptr);

void StimulusWindow::registerClass ()
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    ex.cbSize        = sizeof (WNDCLASSEX);
    ex.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    ex.lpfnWndProc   = StimulusWindow::WindowProc;
    ex.cbClsExtra    = 0;
    ex.cbWndExtra    = 0;
    ex.hInstance     = gearsDllInstance;
    ex.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
    ex.hCursor       = LoadCursor (NULL, IDC_CROSS);
    ex.hbrBackground = NULL;
    ex.lpszMenuName  = NULL;
    ex.lpszClassName = "GearsStimulusWindow";
    ex.hIconSm       = NULL;
#endif

    RegisterClassEx (&ex);
}

void StimulusWindow::closeWindow ()
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    wglMakeCurrent (hdc, NULL); // release device context in use by rc
    wglDeleteContext (hglrc);   // delete rendering context

    PostQuitMessage (0); // make sure the window will be destroyed
#endif
    //	if(fullscreen)	// if FULLSCREEN, change back to original resolution
    //		ChangeDisplaySettings(NULL, 0);
}

LRESULT CALLBACK StimulusWindow::WindowProc (
    _In_ HWND   hwnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam)
{
    StimulusWindow* c = (StimulusWindow*)GetWindowLong (hwnd, GWLP_USERDATA);

    if (c == NULL)
        if (instanceCreated) {
            return instanceCreated->winProc (hwnd, uMsg, wParam, lParam);
        } else
            return DefWindowProc (hwnd, uMsg, wParam, lParam);

    return c->winProc (hwnd, uMsg, wParam, lParam);
}

LRESULT StimulusWindow::winProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    switch (uMsg) {
        case WM_CREATE: {
            if ((hdc = GetDC (hwnd)) == NULL) // get device context
            {
                //TODO error
                closeWindow ();
                return FALSE;
            }
            setGLFormat ();                               // select pixel format
            if ((hglrc = wglCreateContext (hdc)) == NULL) // create the rendering context
            {
                //TODO error
                //MessageBox(hwnd, "Failed to Create the OpenGL Rendering Context",
                //	"OpenGL Rendering Context Error", MB_OK);
                closeWindow ();
                return FALSE;
            }
            if ((wglMakeCurrent (hdc, hglrc)) == false) // make hglrc current rc
            {
                //TODO error
                //MessageBox(hwnd, "Failed to make OpenGL Rendering Context current",
                //					 "OpenGL Rendering Context Error", MB_OK);
                closeWindow ();
                return FALSE;
            }
            glDisable (GL_DEPTH_TEST);
            glDepthMask (GL_FALSE);
            GLenum err = glGetError ();

            //glewSequenceal = GL_TRUE;
            err = glewInit ();

            std::stringstream specs;

            if (GLEW_OK != err) {
                std::stringstream ss;
                ss << "GLEW/OpenGL Error: " << glewGetErrorString (err) << std::endl;
                std::cerr << ss.str ();
                PyErr_SetString (PyExc_RuntimeError, ss.str ().c_str ());
                boost::python::throw_error_already_set ();
            } else {
                specs << "Using GLEW " << glewGetString (GLEW_VERSION) << std::endl;
                specs << "Vendor: " << glGetString (GL_VENDOR) << std::endl;
                specs << "Renderer: " << glGetString (GL_RENDERER) << std::endl;
                specs << "Version: " << glGetString (GL_VERSION) << std::endl;
                specs << "GLSL: " << glGetString (GL_SHADING_LANGUAGE_VERSION) << std::endl;

                std::cout << specs.str ();

                if (!GLEW_VERSION_3_3) {
                    PyErr_SetString (PyExc_RuntimeError, "OpenGL 3.3 not supported. Verify your HW specs and update your drivers.");
                    boost::python::throw_error_already_set ();
                }
            }
            err = glGetError ();
            if (err == GL_NO_ERROR)
                std::cout << "GLEW initialized.\n";
            else
                std::cerr << "OpenGL error: " << err << "\n";

            glSpecs = specs.str ();

            wglSwapIntervalEXT    = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress ("wglSwapIntervalEXT");
            wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress ("wglGetSwapIntervalEXT");
            if (wglSwapIntervalEXT == nullptr || wglGetSwapIntervalEXT == nullptr) {
                std::cerr << "OpenGL extension wglSwapIntervalEXT or wglGetSwapIntervalEXT not supported.\n";
                PyErr_SetString (PyExc_RuntimeError, "OpenGL extension wglSwapIntervalEXT or wglGetSwapIntervalEXT not supported.");
                boost::python::throw_error_already_set ();
            }
            err = glGetError ();
            if (err != GL_NO_ERROR)
                std::cerr << "OpenGL error: " << err << "\n";

            break;
        }
        case WM_KILLFOCUS: {
            break;
        }
        case WM_SETFOCUS: {
            makeCurrent ();
            glViewport (0, 0, screenw, screenh);
            break;
        }
        case WM_DESTROY: {
            // release memory and shutdown
            closeWindow ();
            break;
        }
        case WM_KEYDOWN: {
            bool         handled  = false;
            Stimulus::CP stimulus = sequenceRenderer->getCurrentStimulus ();
            handled               = handled || stimulus->executeCallbacks<Gears::Event::KeyPressed> (Gears::Event::KeyPressed::create (uMsg, wParam, lParam));
            Response::CP response = sequenceRenderer->getCurrentResponse ();
            if (response)
                handled = handled || response->executeCallbacks<Gears::Event::KeyPressed> (Gears::Event::KeyPressed::create (uMsg, wParam, lParam));

            if (!handled)
                switch (wParam) {
                    case 'Q':
                    case VK_ESCAPE:
                        sequenceRenderer->skip (100000000);
                        break;
                    case VK_RIGHT:
                    case 'S':
                        sequenceRenderer->skip (1);
                        break;
                    case VK_LEFT:
                    case 'B':
                        sequenceRenderer->skip (-1);
                        break;
                    case 'P':
                        sequenceRenderer->pause ();
                        break;
                        //		case 'Y':
                        //			//TODO instant spike
                        //			//gears.instantlyClearSignal(sequence.onKeySpikeChannel)
                        //			//gears.instantlyRaiseSignal(sequence.onKeySpikeChannel)
                        //			break;
                        //		case 'H':
                        //			//TODO instant spike
                        //			//gears.instantlyClearSignal(sequence.onKeySpikeChannel)
                        //			//gears.instantlyRaiseSignal(sequence.onKeySpikeChannel)
                        //			break;
                    case VK_TAB:
                        currentMonitor++;
                        if (currentMonitor >= monitors.size ())
                            currentMonitor = 0;
                        {
                            Monitor& m = monitors[currentMonitor];
                            MoveWindow (hwnd, m.rcMonitor.left, m.rcMonitor.top, m.rcMonitor.right - m.rcMonitor.left, m.rcMonitor.bottom - m.rcMonitor.top, false);
                        }
                        break;
                    case VK_INSERT:
                        MoveWindow (hwnd, 0, 0, vscreenw, vscreenh, false);
                        break;
                    case VK_SPACE:
                        sequenceRenderer->setText ("_____toggle info", "Press SPACE to hide this text.");
                        sequenceRenderer->showText ();
                        break;
                }
            break;
        }
        case WM_MOUSEMOVE: {
            Stimulus::CP stimulus = sequenceRenderer->getCurrentStimulus ();
            stimulus->executeCallbacks<Gears::Event::MouseMove> (Gears::Event::MouseMove::create (uMsg, wParam, lParam, screenw, screenh));
            //Response::CP response = sequenceRenderer->getCurrentResponse();
            //response->executeCallbacks<Gears::Event::MouseMove>(Gears::Event::MouseMove::create(uMsg, wParam, lParam, screenw, screenh));
            break;
        }
        case WM_MOUSEWHEEL: {
            Stimulus::CP stimulus = sequenceRenderer->getCurrentStimulus ();
            stimulus->executeCallbacks<Gears::Event::Wheel> (Gears::Event::Wheel::create (uMsg, wParam, lParam));
            break;
        }
        case WM_KEYUP: {
            Stimulus::CP stimulus = sequenceRenderer->getCurrentStimulus ();
            stimulus->executeCallbacks<Gears::Event::KeyReleased> (Gears::Event::KeyReleased::create (uMsg, wParam, lParam));
            Response::CP Response = sequenceRenderer->getCurrentResponse ();
            if (Response)
                Response->executeCallbacks<Gears::Event::KeyReleased> (Gears::Event::KeyReleased::create (uMsg, wParam, lParam));
            break;
        }
        case WM_LBUTTONUP: {
            Response::CP response = sequenceRenderer->getCurrentResponse ();
            if (response)
                response->executeCallbacks<Gears::Event::MouseReleasedLeft> (Gears::Event::MouseReleasedLeft::create (uMsg, wParam, lParam, screenw, screenh));
            break;
        }
        /*def mouseMoveEvent(self, event):			 
		stimulus = gears.getCurrentStimulus().getPythonObject()
		try:
		for cb in stimulus.onMouse :
		cb(event)
		except AttributeError :
		pass

		def mousePressEvent(self, event):
		stimulus = gears.getCurrentStimulus().getPythonObject()
		try:
		for cb in stimulus.onMouseClick :
		cb(event)
		except AttributeError :
		pass

		def wheelEvent(self, event):
		stimulus = gears.getCurrentStimulus().getPythonObject()
		try:
		for cb in stimulus.onWheel :
		cb(event)
		except AttributeError :
		pass
		return 0;*/
        case WM_SIZE: {
            // restore the viewport after the window had been resized
            screenw = LOWORD (lParam);
            screenh = HIWORD (lParam);

            break;
        }
        case WM_SETCURSOR: {
            if (LOWORD (lParam) == HTCLIENT) {
                if (sequenceRenderer->isShowingCursor ())
                    //SetCursor(LoadCursor(NULL, IDC_CROSS));
                    SetCursor (crossCursor);
                else
                    SetCursor (NULL);
            }
            break;
        }
    }
#endif;
    return TRUE;
}


void StimulusWindow::setGLFormat (void)
{
    PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof (PIXELFORMATDESCRIPTOR),
            1,
            PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
            PFD_TYPE_RGBA,
            16,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // useless parameters
            16,
            0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0};

    // Choose the closest pixel format available
    if (!(indexPixelFormat = ChoosePixelFormat (hdc, &pfd))) {
        //MessageBox(hwnd, "Failed to find pixel format", "Pixel Format Error", MB_OK);
        //todo error
        closeWindow ();
    }

    // Set the pixel format for the provided window DC
    if (!SetPixelFormat (hdc, indexPixelFormat, &pfd)) {
        //TODO error
        //MessageBox(hwnd, "Failed to Set Pixel Format", "Pixel Format Error", MB_OK);
        closeWindow ();
    }
}

int StimulusWindow::setSwapInterval (int swapInterval)
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    wglSwapIntervalEXT (swapInterval);
    return wglGetSwapIntervalEXT ();
#endif;
    return 0;
}

void StimulusWindow::makeCurrent ()
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    if ((wglMakeCurrent (hdc, hglrc)) == false) // make hglrc current rc
    {
        //TODO error
        //MessageBox(hwnd, "Failed to make OpenGL Rendering Context current",
        //					 "OpenGL Rendering Context Error", MB_OK);
        closeWindow ();
    }
#endif
}

void StimulusWindow::shareCurrent ()
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    HGLRC current = wglGetCurrentContext ();
    if (!wglShareLists (hglrc, current)) {
        std::stringstream ss;
        ss << "WGL Error: " << GetLastError () << std::endl;
        std::cerr << ss.str ();
    }
#endif
}

void StimulusWindow::setCursorPos ()
{
    //TODO: Windows implementation
}

StimulusWindow::P StimulusWindow::instanceCreated (nullptr);

void StimulusWindow::addMonitor (
    HMONITOR hMonitor,
    HDC      hdcMonitor,
    LPRECT   lprcMonitor)
{
    Monitor m;
    m.hMonitor   = hMonitor;
    m.hdcMonitor = hdcMonitor;
    m.rcMonitor  = *lprcMonitor;

    monitors.push_back (m);
}

#endif //_WIN32