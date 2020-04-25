#ifndef PERSISTENT_HPP
#define PERSISTENT_HPP

#include "Utils.hpp"

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
extern std::set<std::string> uniqueNames;
}

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
    USING_PTR (PersistentVariable);

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

#endif