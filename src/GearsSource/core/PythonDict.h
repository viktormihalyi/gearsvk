#if 0
#pragma once
#include <boost/python.hpp>
#include <functional>
#include <glm/glm.hpp>
#include <sstream>

namespace Gears {
/// Helper class to access fields of lua tables provided as boost::python::dict.
class PythonDict {
public:
    /// The lua table.
    //boost::python::dict const& dictionary;

    void throwError (const std::string& errorMessage)
    {
        PyErr_SetString (PyExc_ValueError, "for some reason...");
        boost::python::throw_error_already_set ();
    }

    PythonDict (boost::python::dict const& dictionary)
        : dictionary (dictionary)
    {
        if (dictionary.is_none ()) {
            throwError ("Python dict expected.");
        }
    }

    boost::python::dict getSubDict (const std::string& key)
    {
        using namespace boost::python;
        return extract<dict> (dictionary[key]);
    }

    boost::python::dict getSubDict (uint index)
    {
        using namespace boost::python;
        return extract<dict> (dictionary[index]);
    }

    std::string getString (const std::string& key, const std::string& defaultValue)
    {
        return getString (key, defaultValue.c_str ());
    }

    /// Retrieves a table field as a string.
    /// @param key the name of the field
    /// @param defaultValue the string that should be returned if the specified field does not exit, or NULL to indicate a required field
    /// @return the value of the table field
    std::string getString (const std::string& key, const char* defaultValue = NULL)
    {
        using namespace boost::python;
        extract<std::string> gs (dictionary[key]);
        if (!gs.check ()) {
            if (defaultValue == NULL) {
                throwError (std::string ("Required parameter '") + key + "' not found.");
            }
            return defaultValue;
        }
        return gs ();
    }

    /// Retrieves a table field as a bool.
    /// @param key the name of the field
    /// @param defaultValue the value that should be returned if the specified field does not exit
    /// @return the value of the table field
    bool getBool (const std::string& key, bool defaultValue = false)
    {
        using namespace boost::python;
        if (!dictionary.has_key (key))
            return defaultValue;
        object        a = dictionary[key];
        extract<bool> gs (a);
        if (!gs.check ()) {
            std::stringstream ss;
            ss << "Conversion of '" << key << "' value to bool failed.";
            throwError (ss.str ());
        }
        return gs ();
    }

    /// Retrieves a table field as integer.
    /// @param key the name of the field
    /// @param defaultValue the value that should be returned if the specified field does not exit
    /// @return the value of the table field
    int getInt (const std::string& key, int defaultValue = 0)
    {
        using namespace boost::python;
        if (!dictionary.has_key (key))
            return defaultValue;
        object       a = dictionary[key];
        extract<int> gs (a);
        if (!gs.check ()) {
            std::stringstream ss;
            ss << "Conversion of '" << key << "' value to int failed.";
            throwError (ss.str ());
        }
        return gs ();
    }

    /// Retrieves a table field as unsigned integer.
    /// @param key the name of the field
    /// @param defaultValue the value that should be returned if the specified field does not exit
    /// @return the value of the table field
    uint getUint (const std::string& key, uint defaultValue = 0)
    {
        using namespace boost::python;
        if (!dictionary.has_key (key))
            return defaultValue;
        object        a = dictionary[key];
        extract<uint> gs (a);
        if (!gs.check ()) {
            std::stringstream ss;
            ss << "Conversion of '" << key << "' value to uint failed.";
            throwError (ss.str ());
        }
        return gs ();
    }

    /// Retrieves a table field (which must be a table itself) as a int3.
    /// @param key the name of the field
    /// @param defaultValue the value that should be returned if the specified field does not exist
    /// @return the value of the table field
    Gears::Math::int3 getInt3 (const std::string& key, Gears::Math::int3 defaultValue = Gears::Math::int3 (1, 1, 1))
    {
        using namespace boost::python;
        if (!dictionary.has_key (key))
            return defaultValue;
        object        a = dictionary[key];
        extract<dict> gs (a);
        if (!gs.check ()) {
            std::stringstream ss;
            ss << "Dict expected for int3 parameter '" << key << "'.";
            throwError (ss.str ());
        }
        PythonDict vectorElements (gs ());
        return Gears::Math::int3 (
            vectorElements.getInt ("x"),
            vectorElements.getInt ("y"),
            vectorElements.getInt ("z"));
    }

    /// Evaluates the table as a int3.
    /// @param defaultValue the values that should be returned where the channel fields do not exist
    /// @return the value of the vactor
    Gears::Math::int3 asInt3 (Gears::Math::int3 defaultValue = Gears::Math::int3 (1, 1, 1))
    {
        return Gears::Math::int3 (
            getInt ("x", defaultValue.x),
            getInt ("y", defaultValue.y),
            getInt ("z", defaultValue.z));
    }

    Gears::Math::int4 getInt4 (const std::string& key, Gears::Math::int4 defaultValue = Gears::Math::int4 (1, 1, 1, 1))
    {
        using namespace boost::python;
        if (!dictionary.has_key (key))
            return defaultValue;
        object        a = dictionary[key];
        extract<dict> gs (a);
        if (!gs.check ()) {
            std::stringstream ss;
            ss << "Dict expected for int4 parameter '" << key << "'.";
            throwError (ss.str ());
        }
        PythonDict vectorElements (gs ());
        return Gears::Math::int4 (
            vectorElements.getInt ("x"),
            vectorElements.getInt ("y"),
            vectorElements.getInt ("z"),
            vectorElements.getInt ("w"));
    }

    /// Evaluates the table as a int4.
    /// @param defaultValue the values that should be returned where the channel fields do not exist
    /// @return the value of the int4
    Gears::Math::int4 asInt4 (Gears::Math::int4 defaultValue = Gears::Math::int4 (1, 1, 1, 1))
    {
        return Gears::Math::int4 (
            getInt ("x", defaultValue.x),
            getInt ("y", defaultValue.y),
            getInt ("z", defaultValue.z),
            getInt ("w", defaultValue.w));
    }

    /// Evaluates the table as a uint4.
    /// @param defaultValue the values that should be returned where the channel fields do not exist
    /// @return the value of the int4
    Gears::Math::uint4 asUint4 (Gears::Math::uint4 defaultValue = Gears::Math::uint4 (1, 1, 1, 1))
    {
        return Gears::Math::uint4 (
            (int)getUint ("x", defaultValue.x),
            (int)getUint ("y", defaultValue.y),
            (int)getUint ("z", defaultValue.z),
            (int)getUint ("w", defaultValue.w));
    }

    /// Retrieves a table field as a float.
    /// @param key the name of the field
    /// @param defaultValue the value that should be returned if the specified field does not exit
    /// @return the value of the table field
    float getFloat (const std::string& key, float defaultValue = 0)
    {
        using namespace boost::python;
        if (!dictionary.has_key (key))
            return defaultValue;
        object         a = dictionary[key];
        extract<float> gs (a);
        if (!gs.check ()) {
            std::stringstream ss;
            ss << "Conversion of '" << key << "' value to float failed.";
            throwError (ss.str ());
        }
        return gs ();
    }

    /// Retrieves a table field (which must be a table itself) as a float3.
    /// @param key the name of the field
    /// @param defaultValue the value that should be returned if the specified field does not exist
    /// @return the value of the table field
    glm::vec3 getFloat3 (const std::string& key, glm::vec3 defaultValue = glm::vec3 (1, 1, 1))
    {
        using namespace boost::python;
        if (!dictionary.has_key (key))
            return defaultValue;
        object        a = dictionary[key];
        extract<dict> gs (a);
        if (!gs.check ()) {
            std::stringstream ss;
            ss << "Dict expected for float3 parameter '" << key << "'.";
            throwError (ss.str ());
        }
        PythonDict vectorElements (gs ());
        return glm::vec3 (
            vectorElements.getFloat ("x"),
            vectorElements.getFloat ("y"),
            vectorElements.getFloat ("z"));
    }

    /// Evaluates the table as a float3.
    /// @param defaultValue the values that should be returned where the channel fields do not exist
    /// @return the value of the vactor
    glm::vec3 asFloat3 (glm::vec3 defaultValue = glm::vec3 (1, 1, 1))
    {
        return glm::vec3 (
            getFloat ("x", defaultValue.x),
            getFloat ("y", defaultValue.y),
            getFloat ("z", defaultValue.z));
    }

    Gears::Math::float4 getFloat4 (const std::string& key, Gears::Math::float4 defaultValue = Gears::Math::float4 (1, 1, 1, 1))
    {
        using namespace boost::python;
        if (!dictionary.has_key (key))
            return defaultValue;
        object        a = dictionary[key];
        extract<dict> gs (a);
        if (!gs.check ()) {
            std::stringstream ss;
            ss << "Dict expected for float4 parameter '" << key << "'.";
            throwError (ss.str ());
        }
        PythonDict vectorElements (gs ());
        return Gears::Math::float4 (
            vectorElements.getFloat ("x"),
            vectorElements.getFloat ("y"),
            vectorElements.getFloat ("z"),
            vectorElements.getFloat ("w"));
    }

    /// Evaluates the table as a float4.
    /// @param defaultValue the values that should be returned where the channel fields do not exist
    /// @return the value of the float4
    Gears::Math::float4 asFloat4 (Gears::Math::float4 defaultValue = Gears::Math::float4 (1, 1, 1, 1))
    {
        return Gears::Math::float4 (
            getFloat ("x", defaultValue.x),
            getFloat ("y", defaultValue.y),
            getFloat ("z", defaultValue.z),
            getFloat ("w", defaultValue.w));
    }

    template<typename T>
    std::shared_ptr<T> get (const std::string& key, std::shared_ptr<T> defaultValue)
    {
        using namespace boost::python;
        if (!dictionary.has_key (key))
            return defaultValue;
        object                      a = dictionary[key];
        extract<std::shared_ptr<T>> gs (a);
        if (!gs.check ()) {
            std::stringstream ss;
            ss << "Conversion of '" << key << "' value to reference failed.";
            throwError (ss.str ());
        }
        return gs ();
    }

    template<typename T>
    std::shared_ptr<T> get (const std::string& key)
    {
        using namespace boost::python;
        if (!dictionary.has_key (key))
            throwError (std::string (": Required lua table entry '") + key + "' not found or is 'None'.");
        object                      a = dictionary[key];
        extract<std::shared_ptr<T>> gs (a);
        if (!gs.check ()) {
            std::stringstream ss;
            ss << "Conversion of '" << key << "' value to reference failed.";
            throwError (ss.str ());
        }
        return gs ();
    }

    void process (std::function<void (std::string)> f)
    {
        using namespace boost::python;
        list keys      = dictionary.keys ();
        uint nSettings = len (keys);
        for (unsigned int c = 0; c < nSettings; c++) {
            std::string key = extract<std::string> (keys[c]);
            f (key);
        }
    }

    void forEach (std::string key, std::function<void (pybind11::object)> f)
    {
        using namespace boost::python;
        if (!dictionary.has_key (key))
            throwError (std::string ("Required lua table entry '") + key + "' not found or is 'None'.");
        object        a = dictionary[key];
        extract<list> gs (a);
        if (!gs.check ()) {
            std::stringstream ss;
            ss << "Conversion of '" << key << "' value to list failed.";
            throwError (ss.str ());
        }
        list l         = gs ();
        uint nElements = len (l);
        for (unsigned int c = 0; c < nElements; c++) {
            f (l[c]);
        }
    }
};
} // namespace Gears
#endif