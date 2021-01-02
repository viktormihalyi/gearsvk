#ifndef GRAPHSETTINGS_HPP
#define GRAPHSETTINGS_HPP

#include "GearsVkAPI.hpp"

#include "Connections.hpp"
#include "DeviceExtra.hpp"
#include "Node.hpp"
#include "Ptr.hpp"
#include <vulkan/vulkan.h>


namespace RG {

class IResourceVisitor;

USING_PTR (ConnectionSet);
class GEARSVK_API ConnectionSet final : public Noncopyable {
    USING_CREATE (ConnectionSet);

public:
    std::set<NodeP> nodes;

    USING_PTR (Connection);
    struct Connection {
        USING_CREATE (Connection);

        NodeP               from;
        NodeP               to;
        IConnectionBindingU binding;

        Connection (const NodeP&          from,
                    const NodeP&          to,
                    IConnectionBindingU&& binding)
            : from (from)
            , to (to)
            , binding (std::move (binding))
        {
        }
    };

private:
    std::vector<ConnectionU> connections;

public:
    ConnectionSet () = default;

    void VisitOutputsOf (const Node* node, IResourceVisitor& visitor) const;

    void VisitInputsOf (const Node* node, IResourceVisitor& visitor) const;

    template<typename T>
    std::vector<P<T>> GetPointingTo (const Node* node) const
    {
        std::vector<P<T>> result;

        for (const ConnectionU& c : connections) {
            if (c->from.get () == node) {
                if (auto asCasted = std::dynamic_pointer_cast<T> (c->to)) {
                    result.push_back (asCasted);
                }
            }
        }

        return result;
    }

    template<typename T>
    std::vector<P<T>> GetPointingHere (const Node* node) const
    {
        std::vector<P<T>> result;

        for (const ConnectionU& c : connections) {
            if (c->to.get () == node) {
                if (auto asCasted = std::dynamic_pointer_cast<T> (c->from)) {
                    result.push_back (asCasted);
                }
            }
        }

        return result;
    }

    void VisitOutputsOf (const Node* node, IConnectionBindingVisitor& visitor) const
    {
        for (const ConnectionU& c : connections) {
            if (c->from.get () == node) {
                c->binding->Visit (visitor);
            }
        }
    }

    template<typename Processor>
    void ProcessInputBindingsOf (const Node* node, const Processor& processor) const
    {
        for (const ConnectionU& c : connections) {
            if (c->to.get () == node) {
                processor (*c->binding);
            }
        }
    }
    template<typename Processor>
    void ProcessOutputBindingsOf (const Node* node, const Processor& processor) const
    {
        for (const ConnectionU& c : connections) {
            if (c->from.get () == node) {
                processor (*c->binding);
            }
        }
    }

    void VisitInputsOf (const Node* node, IConnectionBindingVisitor& visitor) const
    {
        for (const ConnectionU& c : connections) {
            if (c->to.get () == node) {
                c->binding->Visit (visitor);
            }
        }
    }


    void Add (const NodeP& from, const NodeP& to, IConnectionBindingU&& binding)
    {
        nodes.insert (from);
        nodes.insert (to);
        connections.push_back (Connection::Create (from, to, std::move (binding)));
    }

    void Add (const NodeP& node)
    {
        nodes.insert (node);
    }
};


USING_PTR (GraphSettings);
class GEARSVK_API GraphSettings {
    USING_CREATE (GraphSettings);

public:
    ConnectionSet      connectionSet;
    const DeviceExtra* device;
    uint32_t           framesInFlight;

    GraphSettings (const DeviceExtra& device, VkQueue queue, VkCommandPool commandPool, uint32_t framesInFlight);

    GraphSettings (const DeviceExtra& device, uint32_t framesInFlight);


    GraphSettings ();

    const DeviceExtra& GetDevice () const;

    const Queue& GetGrahpicsQueue () const;

    const CommandPool& GetCommandPool () const;
};

} // namespace RG

#endif