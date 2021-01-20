#ifndef PTR_HPP
#define PTR_HPP


#include <memory>

template<typename T>
using Ptr = std::shared_ptr<T>;

template<typename T>
using U = std::unique_ptr<T>;

template<typename T>
using Unique = std::unique_ptr<T>;

#define USING_PTR(classname)                           \
    class classname;                                   \
    using classname##U   = std::unique_ptr<classname>; \
    using classname##Ref = std::reference_wrapper<classname>;

#define USING_PTR2(classname, basename)               \
    using classname      = basename;                  \
    using classname##U   = std::unique_ptr<basename>; \
    using classname##Ref = std::reference_wrapper<basename>;


template<class T, class... Parameters>
U<T> Make (Parameters&&... parameters)
{
    return U<classname> (new classname (std::forward<Parameters> (parameters)...));
}

#define USING_CREATE(classname)                                                                       \
public:                                                                                               \
    template<class... Parameters>                                                                     \
    static std::unique_ptr<classname> Create (Parameters&&... parameters)                             \
    {                                                                                                 \
        return std::unique_ptr<classname> (new classname (std::forward<Parameters> (parameters)...)); \
    }                                                                                                 \
                                                                                                      \
    template<class... Parameters>                                                                     \
    static std::shared_ptr<classname> CreateShared (Parameters&&... parameters)                       \
    {                                                                                                 \
        return std::shared_ptr<classname> (new classname (std::forward<Parameters> (parameters)...)); \
    }


#endif