#include "Utils.hpp"
#include "Assert.hpp"

#include <fstream>

namespace Utils {

#ifdef PROJECT_ROOT_FULL_PATH
const std::filesystem::path PROJECT_ROOT (PROJECT_ROOT_FULL_PATH);
#else
#error "no project root path defined"
#endif


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
    return OpenAndReadFile<std::vector<uint32_t>> (filePath);
}

} // namespace Utils