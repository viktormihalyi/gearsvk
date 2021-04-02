#include "Utils.hpp"
#include "Assert.hpp"
#include "BuildType.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>


#ifdef WIN32

#include <windows.h>

std::filesystem::path Utils::GetProjectRoot ()
{
    char buff[MAX_PATH] = {};
    GetModuleFileName (NULL, buff, (sizeof (buff)));
    return std::filesystem::path (buff).parent_path ();
}

#else

#include <limits.h>
#include <unistd.h>

std::filesystem::path Utils::GetProjectRoot ()
{
    char    buff[PATH_MAX];
    ssize_t len = ::readlink ("/proc/self/exe", buff, sizeof (buff) - 1);
    GVK_ASSERT (len != -1);
    buff[len] = '\0';
    return std::filesystem::path (buff).parent_path ();
}

#endif

namespace Utils {


template<typename T>
static T ReadOpenedFile (std::ifstream& file)
{
    if (GVK_ERROR (!file.is_open ())) {
        return T ();
    }

    file.seekg (0, std::ios::end);
    T result;
    result.reserve (file.tellg ());
    file.seekg (0, std::ios::beg);

    result.assign (std::istreambuf_iterator<char> (file),
                   std::istreambuf_iterator<char> ());

    return result;
}


bool WriteBinaryFile (const std::filesystem::path& filePath, const std::vector<uint8_t>& data)
{
    return WriteBinaryFile (filePath, data.data (), data.size ());
}


void EnsureParentFolderExists (const std::filesystem::path& filePath)
{
    if (!std::filesystem::exists (filePath.parent_path ())) {
        std::filesystem::create_directories (filePath.parent_path ());
    }
}


bool WriteBinaryFile (const std::filesystem::path& filePath, const void* data, size_t size)
{
    EnsureParentFolderExists (filePath);

    std::ofstream file (filePath, std::ios::out | std::ios::binary);

    if (!file.is_open ()) {
        return false;
    }

    file.write (reinterpret_cast<const char*> (data), size);

    return true;
}


bool WriteTextFile (const std::filesystem::path& filePath, const std::string& text)
{
    EnsureParentFolderExists (filePath);

    std::ofstream file (filePath, std::ios::out | std::ios::binary);

    if (!file.is_open ()) {
        return false;
    }

    file << text;

    return true;
}


template<typename T>
static std::optional<T> OpenAndReadFile (const std::filesystem::path& filePath)
{
    std::ifstream file (filePath, std::ios::binary);

    if (!file.is_open ()) {
        return std::nullopt;
    }

    T result = ReadOpenedFile<T> (file);

    if (GVK_ERROR (file.fail ())) {
        return std::nullopt;
    }

    return result;
}


std::optional<std::string> ReadTextFile (const std::filesystem::path& filePath)
{
    return OpenAndReadFile<std::string> (filePath);
}


std::optional<std::vector<char>> ReadBinaryFile (const std::filesystem::path& filePath)
{
    return OpenAndReadFile<std::vector<char>> (filePath);
}


std::optional<std::vector<uint32_t>> ReadBinaryFile4Byte (const std::filesystem::path& filePath)
{
    const std::optional<std::vector<char>>
        readResult = OpenAndReadFile<std::vector<char>> (filePath);
    if (!readResult.has_value ()) {
        return std::nullopt;
    }

    GVK_ASSERT (readResult->size () % 4 == 0);
    uint32_t binarySize = readResult->size () / 4;
    while (binarySize % 4 != 0) {
        ++binarySize;
    }
    std::vector<uint32_t> result (binarySize);
    memset (result.data (), 0, result.size () * sizeof (uint32_t));

    memcpy (result.data (), readResult->data (), readResult->size ());

    return result;
}


std::vector<std::string> SplitString (const std::string& str, const char delim, const bool keepEmpty)
{
    std::vector<std::string> result;

    size_t prev = 0;
    size_t pos  = 0;

    do {
        pos = str.find (delim, prev);
        if (pos == std::string::npos) {
            pos = str.length ();
        }

        std::string token = str.substr (prev, pos - prev);

        if (keepEmpty || !token.empty ()) {
            result.push_back (std::move (token));
        }

        prev = pos + 1;

    } while (pos < str.length () && prev < str.length ());

    return result;
}


std::vector<std::string> SplitString (const std::string& str, const std::string& delim, const bool keepEmpty)
{
    std::vector<std::string> result;

    size_t prev = 0;
    size_t pos  = 0;

    do {
        pos = str.find (delim, prev);
        if (pos == std::string::npos) {
            pos = str.length ();
        }

        std::string token = str.substr (prev, pos - prev);

        if (keepEmpty || !token.empty ()) {
            result.push_back (std::move (token));
        }

        prev = pos + delim.length ();

    } while (pos < str.length () && prev < str.length ());

    return result;
}


std::string ReplaceAll (const std::string& str, const std::string& substringToReplace, const std::function<std::string ()>& replacementSubstring)
{
    const std::vector<std::string> split = SplitString (str, substringToReplace, true);

    std::string result;

    for (const std::string& s : split) {
        if (s == substringToReplace) {
            result += replacementSubstring ();
        } else {
            result += s;
        }
    }

    return result;
}


bool StringContains (const std::string& str, const std::string& substr)
{
    return str.find (substr) != std::string::npos;
}


} // namespace Utils