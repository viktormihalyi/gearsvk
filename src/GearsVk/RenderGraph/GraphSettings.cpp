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


GraphSettings::GraphSettings (const DeviceExtra& device, VkQueue queue, VkCommandPool commandPool, uint32_t framesInFlight)
    : device (&device)
    , framesInFlight (framesInFlight)
{
}


GraphSettings::GraphSettings (const DeviceExtra& device, uint32_t framesInFlight)
    : GraphSettings (device, device.GetGraphicsQueue (), device.GetCommandPool (), framesInFlight)
{
}


GraphSettings::GraphSettings ()
    : device (nullptr)
    , framesInFlight (0)
{
}

const DeviceExtra& GraphSettings::GetDevice () const
{
    return *device;
}


const Queue& GraphSettings::GetGrahpicsQueue () const
{
    return device->GetGraphicsQueue ();
}


const CommandPool& GraphSettings::GetCommandPool () const
{
    return device->GetCommandPool ();
}


} // namespace RG
