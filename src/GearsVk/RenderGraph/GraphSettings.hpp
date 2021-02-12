#ifndef GRAPHSETTINGS_HPP
#define GRAPHSETTINGS_HPP

#include "GearsVkAPI.hpp"

#include "Connections.hpp"
#include "DeviceExtra.hpp"
#include "Node.hpp"
#include "Ptr.hpp"
#include <vulkan/vulkan.h>


namespace GVK {

namespace RG {

class IResourceVisitor;

USING_PTR (ConnectionSet);
class GVK_RENDERER_API ConnectionSet final : public Noncopyable {
public:
    USING_PTR (Connection);
    class Connection {
    public:
        Ptr<Node>           from;
        Ptr<Node>           to;
        IConnectionBindingU binding;

        Connection (const Ptr<Node>&      from,
                    const Ptr<Node>&      to,
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
    std::set<Ptr<Node>> nodes;


public:
    ConnectionSet ();
    ConnectionSet (ConnectionSet&&);
    ConnectionSet& operator= (ConnectionSet&&);

    void VisitOutputsOf (const Node* node, IResourceVisitor& visitor) const;

    void VisitInputsOf (const Node* node, IResourceVisitor& visitor) const;

    template<typename T>
    std::vector<Ptr<T>> GetPointingTo (const Node* node) const
    {
        std::vector<Ptr<T>> result;

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
    std::vector<Ptr<T>> GetPointingHere (const Node* node) const
    {
        std::vector<Ptr<T>> result;

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


    void Add (const Ptr<Node>& from, const Ptr<Node>& to, IConnectionBindingU&& binding)
    {
        nodes.insert (from);
        nodes.insert (to);
        connections.push_back (Make<Connection> (from, to, std::move (binding)));
    }


    void Add (const Ptr<Node>& node)
    {
        nodes.insert (node);
    }
};


USING_PTR (GraphSettings);
class GVK_RENDERER_API GraphSettings {
public:
    ConnectionSet      connectionSet;
    const DeviceExtra* device;
    uint32_t           framesInFlight;

    GraphSettings (const DeviceExtra& device, VkQueue queue, VkCommandPool commandPool, uint32_t framesInFlight);

    GraphSettings (const DeviceExtra& device, uint32_t framesInFlight);

    GraphSettings ();
    GraphSettings (GraphSettings&&);
    GraphSettings& operator= (GraphSettings&&);

    const DeviceExtra& GetDevice () const;

    const Queue& GetGrahpicsQueue () const;

    const CommandPool& GetCommandPool () const;
};

} // namespace RG

} // namespace GVK

#endif