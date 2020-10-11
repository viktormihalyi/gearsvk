#ifndef RG_RESOURCE_HPP
#define RG_RESOURCE_HPP

#include "GearsVkAPI.hpp"

#include "Node.hpp"
#include "Ptr.hpp"


namespace RG {

class GraphSettings;


USING_PTR (Resource);
class GEARSVK_API Resource : public Node {
public:
    virtual ~Resource () = default;

    virtual void Compile (const GraphSettings&) = 0;
};


} // namespace RG

#endif
