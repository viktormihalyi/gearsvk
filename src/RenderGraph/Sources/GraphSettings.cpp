#include "GraphSettings.hpp"

#include "Resource.hpp"


namespace RG {

void ConnectionSet::VisitOutputsOf (const Node* node, IResourceVisitor& visitor) const
{
    for (const std::unique_ptr<Connection>& c : connections) {
        if (c->from.get () == node) {
            if (std::shared_ptr<Resource> asResource = std::dynamic_pointer_cast<Resource> (c->to)) {
                asResource->Visit (visitor);
            } else {
                GVK_BREAK ("???");
            }
        }
    }
}


void ConnectionSet::VisitInputsOf (const Node* node, IResourceVisitor& visitor) const
{
    for (const std::unique_ptr<Connection>& c : connections) {
        if (c->to.get () == node) {
            if (std::shared_ptr<Resource> asResource = std::dynamic_pointer_cast<Resource> (c->from)) {
                asResource->Visit (visitor);
            } else {
                GVK_BREAK ("???");
            }
        }
    }
}


GraphSettings::GraphSettings (const GVK::DeviceExtra& device, ConnectionSet&& connectionSet, uint32_t framesInFlight)
    : device (&device)
    , framesInFlight (framesInFlight)
    , connectionSet (std::move (connectionSet))
{
}



GraphSettings::GraphSettings (const GVK::DeviceExtra& device, uint32_t framesInFlight)
    : device (&device)
    , framesInFlight (framesInFlight)
{
}


GraphSettings::GraphSettings ()
    : device (nullptr)
    , framesInFlight (0)
{
}

const GVK::DeviceExtra& GraphSettings::GetDevice () const
{
    GVK_ASSERT (device != nullptr);
    return *device;
}


const GVK::Queue& GraphSettings::GetGrahpicsQueue () const
{
    return device->GetGraphicsQueue ();
}


const GVK::CommandPool& GraphSettings::GetCommandPool () const
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
    , nodeSet (std::move (other.nodeSet))
    , insertionOrder (std::move (other.insertionOrder))
{
    other.connections.clear ();
    other.nodeSet.clear ();
    other.insertionOrder.clear ();
}


ConnectionSet& ConnectionSet::operator= (ConnectionSet&& other)
{
    if (this != &other) {
        connections    = std::move (other.connections);
        nodeSet        = std::move (other.nodeSet);
        insertionOrder = std::move (other.insertionOrder);

        other.connections.clear ();
        other.nodeSet.clear ();
        other.insertionOrder.clear ();
    }

    return *this;
}


} // namespace RG
