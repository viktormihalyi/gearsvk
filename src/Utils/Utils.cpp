#include "Utils.hpp"
#include "Assert.hpp"
#include "BuildType.hpp"
#include "Dummy.hpp"

#include <cstring>
#include <fstream>
#include <iostream>


namespace Utils {

inline bool PLSfunc (bool condition, const SourceLocation& srcLoc)
{
    if constexpr (IsDebugBuild) {
        if (!condition) {
            Utils::detail::ShowAssertPopup ("PLS failed", "here", srcLoc.ToString (), dummy<bool>);
        }
    }
    return condition;
}

inline bool PLSNOfunc (bool condition, const SourceLocation& srcLoc)
{
    if constexpr (IsDebugBuild) {
        if (condition) {
            Utils::detail::ShowAssertPopup ("PLSNO failed", "here", srcLoc.ToString (), dummy<bool>);
        }
    }
    return condition;
}

#define CURRENTSRCLOC \
    SourceLocation { __FILE__, __LINE__, __func__ }

#define PLS(cond) PLSfunc (cond, CURRENTSRCLOC)
#define PLSNO(cond) PLSfunc (cond, CURRENTSRCLOC)

void Test ()
{
    if (PLSNO (false)) {
    }
}

std::filesystem::path GetProjectRoot ()
{
#ifdef PROJECT_ROOT_FULL_PATH
    static const std::filesystem::path projectRoot (PROJECT_ROOT_FULL_PATH);
    return projectRoot;
#else
#error "no project root path defined"
#endif
}


template<typename T>
static T ReadOpenedFile (std::ifstream& file)
{
    if (ERROR (!file.is_open ())) {
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


static void EnsureParentFolder (const std::filesystem::path& filePath)
{
    if (!std::filesystem::exists (filePath.parent_path ())) {
        std::filesystem::create_directories (filePath.parent_path ());
    }
}

bool WriteBinaryFile (const std::filesystem::path& filePath, const void* data, size_t size)
{
    EnsureParentFolder (filePath);

    std::ofstream file (filePath, std::ios::out | std::ios::binary);

    if (!file.is_open ()) {
        return false;
    }

    file.write (reinterpret_cast<const char*> (data), size);

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

    if (ERROR (file.fail ())) {
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
    std::optional<std::vector<char>> readResult = OpenAndReadFile<std::vector<char>> (filePath);
    if (!readResult.has_value ()) {
        return std::nullopt;
    }

    ASSERT (readResult->size () % 4 == 0);
    uint32_t binarySize = readResult->size () / 4;
    while (binarySize % 4 != 0) {
        ++binarySize;
    }
    std::vector<uint32_t> result (binarySize);
    memset (result.data (), 0, result.size () * sizeof (uint32_t));

    memcpy (result.data (), readResult->data (), readResult->size ());

    return result;
}

} // namespace Utils