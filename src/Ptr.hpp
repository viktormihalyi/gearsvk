#ifndef PTR_HPP
#define PTR_HPP

#include <memory>

template<typename T>
using Ptr = std::shared_ptr<T>;

template<typename T>
using UPtr = std::unique_ptr<T>;

#define DEFINE_PTR(T)                              \
    class T;                                       \
    using T##Ptr       = std::shared_ptr<T>;       \
    using T##ConstPtr  = std::shared_ptr<const T>; \
    using T##UPtr      = std::unique_ptr<T>;       \
    using T##ConstUPtr = std::unique_ptr<const T>


#endif