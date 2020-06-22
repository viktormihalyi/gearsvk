#ifndef PERSISTENT_HPP
#define PERSISTENT_HPP

#include "GearsVkAPI.hpp"

#include "Assert.hpp"
#include "Utils.hpp"

#include <cstring>
#include <filesystem>
#include <set>
#include <string>
#include <type_traits>


class BinaryIOStrategy {
public:
    template<typename T>
    void Read (const std::filesystem::path& file, T& val)
    {
        std::optional<std::vector<char>> binary = Utils::ReadBinaryFile (file);
        if (binary) {
            val = *reinterpret_cast<T*> (binary->data ());
        }
    }

    template<typename T>
    void Write (const std::filesystem::path& file, const T& value)
    {
        Utils::WriteBinaryFile (file, &value, sizeof (T));
    }
};


class VectorIOStrategy {
public:
    template<typename T>
    void Read (const std::filesystem::path& file, std::vector<T>& val)
    {
        ASSERT (false);
#if 0
        const uint32_t vectorSizeLength = sizeof (size_t);

        std::optional<std::vector<char>> binary = Utils::ReadBinaryFile (file);
        if (binary) {
            size_t vectorSize = 0;
            memcpy (&vectorSize, binary->data (), sizeof (size_t));

            val.clear ();
            val.resize (vectorSize);
            memcpy (val.data (), binary->data () + sizeof (size_t), binary->size () - sizeof (size_t));
        }
#endif
    }

    template<typename T>
    void Write (const std::filesystem::path& file, const std::vector<T>& value)
    {
        ASSERT (false);
#if 0
        const uint32_t vectorSizeLength = sizeof (size_t);

        std::vector<uint8_t> binaryData (vectorSizeLength + value.size () * sizeof (T));
        reinterpret_cast<size_t*> (binaryData->data ())[0] = value.size ();

        memcpy (binaryData->data () + vectorSizeLength, value.data (), value.size () * sizeof (T));

        Utils::WriteBinaryFile (file, binaryData->data (), binaryData->size ());
#endif
    }
};


class StringIOStrategy {
public:
    void Read (const std::filesystem::path& file, std::string& value)
    {
        std::optional<std::vector<char>> binary = Utils::ReadBinaryFile (file);
        if (binary) {
            value.resize (binary->size ());
            memcpy (value.data (), binary->data (), binary->size ());
        }
    }

    void Write (const std::filesystem::path& file, const std::string& value)
    {
        Utils::WriteBinaryFile (file, value.data (), value.size ());
    }
};


namespace detail {
GEARSVK_API
extern std::set<std::string> uniqueNames;
} // namespace detail

template<typename T, typename IOStrategy>
class PersistentVariable final : public Noncopyable {
private:
    const std::string           name;
    T                           value;
    const std::filesystem::path file;

    IOStrategy io;

    void AssertOnNameColision () const
    {
        const uint32_t previousSize = detail::uniqueNames.size ();
        detail::uniqueNames.insert (name);
        const uint32_t newSize = detail::uniqueNames.size ();
        ASSERT (newSize == previousSize + 1);
    }

    bool FileExists () const
    {
        return std::filesystem::exists (file);
    }

public:
    PersistentVariable (const std::string& name, const T& defaultValue = T ())
        : name (name)
        , file (std::filesystem::temp_directory_path () / "GearsVk" / (name + ".bin"))
    {
        AssertOnNameColision ();
        if (FileExists ()) {
            io.Read (file, value);
        } else {
            value = defaultValue;
        }
    }

    ~PersistentVariable ()
    {
        io.Write (file, value);
    }

    T& operator= (const T& other)
    {
        value = other;
    }


#define DEFOP(op)                   \
    T& operator op (const T& other) \
    {                               \
        value op other;             \
        return *this;               \
    }

    DEFOP (+=);
    DEFOP (-=);
    DEFOP (*=);
    DEFOP (/=);

#define DEFOP2(op)                 \
    T operator op (const T& other) \
    {                              \
        return value op other;     \
    }

    DEFOP2 (+);
    DEFOP2 (-);
    DEFOP2 (*);
    DEFOP2 (/);


    void Set (const T& other) { *this = other; }

    operator const T& () const { return value; }
    operator T& () { return value; }
    T& operator() () { return value; }
    T& operator* () { return value; }
    T& operator-> () { return value; }
    T& Get () { return value; }
};

using PersistentString = PersistentVariable<std::string, StringIOStrategy>;

template<typename T, typename = typename std::enable_if_t<std::is_standard_layout_v<T>>>
using Persistent = PersistentVariable<T, BinaryIOStrategy>;

template<typename T, typename = typename std::enable_if_t<std::is_standard_layout_v<T>>>
using PersistentVector = PersistentVariable<std::vector<T>, VectorIOStrategy>;

#endif