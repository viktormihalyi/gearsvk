#include "GraphSettings.hpp"

#include "Resource.hpp"

namespace GVK {

namespace RG {


void ConnectionSet::VisitOutputsOf (const Node* node, IResourceVisitor& visitor) const
{
    for (const ConnectionU& c : connections) {
        if (c->from.get () == node) {
            if (Ptr<Resource> asResource = std::dynamic_pointer_cast<Resource> (c->to)) {
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
            if (Ptr<Resource> asResource = std::dynamic_pointer_cast<Resource> (c->from)) {
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


GraphSettings::GraphSettings (GraphSettings&& other)
    : connectionSet (std::move (other.connectionSet))
    , device (other.device)
    , framesInFlight (other.framesInFlight)
{
    other.device         = nullptr;
    other.framesInFlight = 0;
}


GraphSettings& GraphSettings::operator= (GraphSettings&& other)
{
    if (this != &other) {
        connectionSet  = std::move (other.connectionSet);
        device         = other.device;
        framesInFlight = other.framesInFlight;

        other.device         = nullptr;
        other.framesInFlight = 0;
    }

    return *this;
}


ConnectionSet::ConnectionSet () = default;


ConnectionSet::ConnectionSet (ConnectionSet&& other)
    : connections (std::move (other.connections))
    , nodes (std::move (other.nodes))
{
    other.connections.clear ();
    other.nodes.clear ();
}


ConnectionSet& ConnectionSet::operator= (ConnectionSet&& other)
{
    if (this != &other) {
        connections = std::move (other.connections);
        nodes       = std::move (other.nodes);

        other.connections.clear ();
        other.nodes.clear ();
    }

    return *this;
}


} // namespace RG

}
