#ifndef PTR_HPP
#define PTR_HPP

#include <memory>

template<typename T>
using Ptr = std::shared_ptr<T>;

template<typename T>
using PtrC = std::shared_ptr<T const>;

template<typename T>
using U = std::unique_ptr<T>;

#define USING_PTR(classname)                           \
    class classname;                                   \
    using classname##U   = std::unique_ptr<classname>; \
    using classname##Ref = std::reference_wrapper<classname>;

template<typename ObjectType, class... Parameters>
std::unique_ptr<ObjectType> Make (Parameters&&... parameters)
{
    return std::unique_ptr<ObjectType> (new ObjectType (std::forward<Parameters> (parameters)...));
}

#endif