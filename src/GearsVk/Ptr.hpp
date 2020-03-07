#ifndef PTR_HPP
#define PTR_HPP


#include <memory>


template<typename T>
using P = std::shared_ptr<T>;

template<typename T>
using U = std::unique_ptr<T>;

template<typename T>
using CP = std::shared_ptr<T>;

template<typename T>
using CU = std::unique_ptr<T>;

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using CRef = std::unique_ptr<T>;

#define USING_PTR(T)                                                        \
    using P  = std::shared_ptr<T>;                                          \
    using CP = std::shared_ptr<const T>;                                    \
    using U  = std::unique_ptr<T>;                                          \
    using CU = std::unique_ptr<const T>;                                    \
                                                                            \
    template<class... Types>                                                \
    static T::U Create (Types&&... _Args)                                   \
    {                                                                       \
        return std::unique_ptr<T> (new T (std::forward<Types> (_Args)...)); \
    }                                                                       \
                                                                            \
    template<class... Types>                                                \
    static T::P CreateShared (Types&&... _Args)                             \
    {                                                                       \
        return std::shared_ptr<T> (new T (std::forward<Types> (_Args)...)); \
    }                                                                       \
                                                                            \
    using Ref  = std::reference_wrapper<T>;                                 \
    using CRef = std::reference_wrapper<const T>


#endif