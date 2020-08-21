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

#define USING_CREATE(T)                                                     \
public:                                                                     \
    template<class... Types>                                                \
    static std::unique_ptr<T> Create (Types&&... _Args)                     \
    {                                                                       \
        return std::unique_ptr<T> (new T (std::forward<Types> (_Args)...)); \
    }                                                                       \
                                                                            \
    template<class... Types>                                                \
    static std::shared_ptr<T> CreateShared (Types&&... _Args)               \
    {                                                                       \
        return std::shared_ptr<T> (new T (std::forward<Types> (_Args)...)); \
    }


#endif