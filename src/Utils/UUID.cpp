#include "UUID.hpp"

#include "Assert.hpp"
#include "Platform.hpp"
#include "CompilerDefinitions.hpp"

#if defined(PLATFORM_WINDOWS) && defined(COMPILER_MSVC)
#pragma comment(lib, "rpcrt4.lib")
#include <windows.h>
#include <cstring>

static std::string GenerateUUID ()
{
    constexpr size_t uuidSize = 16;
    static_assert (sizeof (UUID) == uuidSize);

    UUID uuid;
    UuidCreate (&uuid);
    std::string result;
    result.resize (uuidSize);
    memcpy (result.data (), &uuid, uuidSize);
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
    GVK_ASSERT (value.size () == 16);
}


GearsVk::UUID::UUID (std::nullptr_t)
    : value ("0000-00-00-00-0000")
{
    GVK_ASSERT (value.size () == 16);
}