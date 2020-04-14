#ifndef PTR_HPP
#define PTR_HPP


#include <memory>

#define USING_PTR_ABSTRACT(T)               \
    using P    = std::shared_ptr<T>;        \
    using W    = std::weak_ptr<T>;          \
    using U    = std::unique_ptr<T>;        \
    using Ref  = std::reference_wrapper<T>; \
    using CP   = std::shared_ptr<const T>;  \
    using CW   = std::weak_ptr<const T>;    \
    using CU   = std::unique_ptr<const T>;  \
    using CRef = std::reference_wrapper<const T>

#define USING_PTR(T)                                                        \
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
    }                                                                       \
                                                                            \
    USING_PTR_ABSTRACT (T)


#endif