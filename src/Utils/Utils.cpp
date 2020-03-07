#include "Utils.hpp"
#include "Assert.hpp"

#include <fstream>

namespace Utils {

#ifdef PROJECT_ROOT_FULL_PATH
const std::filesystem::path PROJECT_ROOT (PROJECT_ROOT_FULL_PATH);
#else
#error "no project root path defined"
#endif


template<typename T, typename IteratorType = char>
static T ReadOpenedFile (std::ifstream& file)
{
    if (ERROR (!file.is_open ())) {
        return T ();
    }

    file.seekg (0, std::ios::end);
    T result;
    result.reserve (file.tellg ());
    file.seekg (0, std::ios::beg);

    result.assign (std::istreambuf_iterator<IteratorType> (file),
                   std::istreambuf_iterator<IteratorType> ());

    return result;
}


template<typename T, typename IteratorType = char>
static std::optional<T> OpenAndReadFile (const std::filesystem::path& filePath)
{
    std::ifstream file (filePath, std::ios::binary);

    if (!file.is_open ()) {
        return std::nullopt;
    }

    T result = ReadOpenedFile<T, IteratorType> (file);

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