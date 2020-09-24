#include "UUID.hpp"

#include "Assert.hpp"
#include "Platform.hpp"

#ifdef PLATFORM_WINDOWS
#pragma comment(lib, "rpcrt4.lib")
#include <windows.h>

static std::string GenerateUUID ()
{
    UUID uuid;
    UuidCreate (&uuid);
    char* str;
    UuidToStringA (&uuid, (RPC_CSTR*)&str);
    const std::string result (str);
    RpcStringFreeA ((RPC_CSTR*)&str);
    return result;
}

#else // TODO handle other platforms

#include <random>
#include <sstream>

static std::random_device              rd;
static std::mt19937                    gen (rd ());
static std::uniform_int_distribution<> dis (0, 15);
static std::uniform_int_distribution<> dis2 (8, 11);

static std::string GenerateUUID ()
{
    std::stringstream ss;
    int               i;
    ss << std::hex;
    for (i = 0; i < 8; i++) {
        ss << dis (gen);
    }
    ss << "-";
    for (i = 0; i < 4; i++) {
        ss << dis (gen);
    }
    ss << "-4";
    for (i = 0; i < 3; i++) {
        ss << dis (gen);
    }
    ss << "-";
    ss << dis2 (gen);
    for (i = 0; i < 3; i++) {
        ss << dis (gen);
    }
    ss << "-";
    for (i = 0; i < 12; i++) {
        ss << dis (gen);
    }
    return ss.str ();
}

#endif


GearsVk::UUID::UUID ()
    : value (GenerateUUID ())
{
    GVK_ASSERT (value.size () == 36);
}


GearsVk::UUID::UUID (std::nullptr_t)
    : value ("00000000-0000-0000-0000-000000000000")
{
    GVK_ASSERT (value.size () == 36);
}