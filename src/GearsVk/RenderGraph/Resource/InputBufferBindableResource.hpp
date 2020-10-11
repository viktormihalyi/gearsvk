#ifndef RG_INPUTBUFFERBINDABLERESOURCE_HPP
#define RG_INPUTBUFFERBINDABLERESOURCE_HPP

#include "GearsVkAPI.hpp"

#include "InputBindable.hpp"
#include "Resource.hpp"

#include "Ptr.hpp"


namespace RG {


USING_PTR (InputBufferBindableResource);
class GEARSVK_API InputBufferBindableResource : public Resource, public InputBufferBindable {
public:
    virtual ~InputBufferBindableResource () = default;
};


} // namespace RG

#endif