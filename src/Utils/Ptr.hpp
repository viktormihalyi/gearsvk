#ifndef PTR_HPP
#define PTR_HPP


#include <memory>

template<typename T>
using P = std::shared_ptr<T>;

template<typename T>
using PC = std::shared_ptr<const T>;

template<typename T>
using U = std::unique_ptr<T>;

template<typename T>
using UC = std::unique_ptr<const T>;

template<typename T>
using W = std::weak_ptr<T>;

#define USING_PTR(classname)                                 \
    class classname;                                         \
    using classname##U   = std::unique_ptr<classname>;       \
    using classname##UC  = std::unique_ptr<const classname>; \
    using classname##P   = std::shared_ptr<classname>;       \
    using classname##PC  = std::shared_ptr<const classname>; \
    using classname##W   = std::weak_ptr<classname>;         \
    using classname##Ref = std::reference_wrapper<classname>;

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