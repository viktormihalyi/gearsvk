#ifndef PTR_HPP
#define PTR_HPP


#include <memory>

#define USING_PTR(classname)                           \
    class classname;                                   \
    using classname##U   = std::unique_ptr<classname>; \
    using classname##P   = std::shared_ptr<classname>; \
    using classname##W   = std::weak_ptr<classname>;   \
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