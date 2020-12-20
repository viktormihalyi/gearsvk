#include "GraphSettings.hpp"

#include "Resource.hpp"

namespace RG {


void ConnectionSet::VisitOutputsOf (const Node* node, IResourceVisitor& visitor) const
{
    for (const ConnectionU& c : connections) {
        if (c->from.get () == node) {
            if (ResourceP asResource = std::dynamic_pointer_cast<Resource> (c->to)) {
                asResource->Visit (visitor);
            } else {
                GVK_BREAK ("???");
            }
        }
    }
}


void ConnectionSet::VisitInputsOf (const Node* node, IResourceVisitor& visitor) const
{
    for (const ConnectionU& c : connections) {
        if (c->to.get () == node) {
            if (ResourceP asResource = std::dynamic_pointer_cast<Resource> (c->from)) {
                asResource->Visit (visitor);
            } else {
                GVK_BREAK ("???");
            }
        }
    }
}


} // namespace RG
