#include "UUID.hpp"

#include "Assert.hpp"
#include "CompilerDefinitions.hpp"
#include "Platform.hpp"

#if defined(PLATFORM_WINDOWS) && defined(COMPILER_MSVC)
#pragma comment(lib, "rpcrt4.lib")
#include <cstring>
#include <windows.h>

static std::array<uint8_t, 16> GenerateUUID ()
{
    static_assert (sizeof (UUID) == 16);

    UUID uuid;
    UuidCreate (&uuid);

    std::array<uint8_t, 16> result;
    memcpy (result.data (), &uuid, 16);
    return result;
}

#else // TODO handle other platforms

#include <uuid/uuid.h>

static std::array<uint8_t, 16> GenerateUUID ()
{
    static_assert (sizeof (uuid_t) == 16);

    uuid_t result;
    uuid_generate_random (result);

    std::array<uint8_t, 16> result;
    memcpy (result.data (), &uuid, 16);
    return result;
}

#endif


GearsVk::UUID::UUID ()
    : data (GenerateUUID ())
{
}


GearsVk::UUID::UUID (std::nullptr_t)
    : data ({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 })
{
}


std::string GearsVk::UUID::GetValue () const
{
    char str[39] = {};
    sprintf (str,
             "{%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
             data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
             data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
    return str;
}
