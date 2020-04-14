#ifdef _WIN32
#include <windows.h>
#endif

class PortHandler {
#ifdef _WIN32
    HANDLE port;
#elif __linux__
    int port;
#endif
public:
    PortHandler ();
    PortHandler (const char* name);
    PortHandler (const PortHandler& o) = delete;
    PortHandler& operator= (const PortHandler& o) = delete;
    PortHandler (PortHandler&& o);
    void setCommand (int cmd);
    bool isInvalid ();
    void close ();
    ~PortHandler ()
    {
        close ();
    }
};