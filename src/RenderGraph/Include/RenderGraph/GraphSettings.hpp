#ifndef GRAPHSETTINGS_HPP
#define GRAPHSETTINGS_HPP

#include "RenderGraph/RenderGraphAPI.hpp"

#include "RenderGraph/Connections.hpp"
#include "RenderGraph/Node.hpp"
#include "VulkanWrapper/DeviceExtra.hpp"

#include <memory>
#include <set>
#include <vulkan/vulkan.h>


namespace RG {

class IResourceVisitor;

class GVK_RENDERER_API NodeConnection {
public:
    std::shared_ptr<Node>               from;
    std::shared_ptr<Node>               to;
    std::unique_ptr<IConnectionBinding> binding;

    NodeConnection  (const std::shared_ptr<Node>&          from,
                     const std::shared_ptr<Node>&          to,
                     std::unique_ptr<IConnectionBinding>&& binding)
        : from (from)
        , to (to)
        , binding (std::move (binding))
    {
    }
};

class GVK_RENDERER_API ConnectionSet final : public Noncopyable {
public:

private:
    std::vector<std::unique_ptr<NodeConnection>> connections;
    
    std::set<std::shared_ptr<Node>> nodeSet;

public:
    std::vector<std::shared_ptr<Node>> insertionOrder;


public:
    ConnectionSet ();
    ConnectionSet (ConnectionSet&&);
    ConnectionSet& operator= (ConnectionSet&&);

    void VisitOutputsOf (const Node* node, IResourceVisitor& visitor) const;

    void VisitInputsOf (const Node* node, IResourceVisitor& visitor) const;

    template<typename T>
    std::vector<std::shared_ptr<T>> GetPointingTo (const Node* node) const
    {
        std::vector<std::shared_ptr<T>> result;

        for (const std::unique_ptr<NodeConnection>& c : connections) {
            if (c->from.get () == node) {
                if (auto asCasted = std::dynamic_pointer_cast<T> (c->to)) {
                    result.push_back (asCasted);
                }
            }
        }

        return result;
    }

    template<typename T>
    std::vector<std::shared_ptr<T>> GetPointingHere (const Node* node) const
    {
        std::vector<std::shared_ptr<T>> result;

        for (const std::unique_ptr<NodeConnection>& c : connections) {
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
        for (const std::unique_ptr<NodeConnection>& c : connections) {
            if (c->from.get () == node) {
                c->binding->Visit (visitor);
            }
        }
    }

    template<typename Processor>
    void ProcessInputBindingsOf (const Node* node, const Processor& processor) const
    {
        for (const std::unique_ptr<NodeConnection>& c : connections) {
            if (c->to.get () == node) {
                processor (*c->binding);
            }
        }
    }
    template<typename Processor>
    void ProcessOutputBindingsOf (const Node* node, const Processor& processor) const
    {
        for (const std::unique_ptr<NodeConnection>& c : connections) {
            if (c->from.get () == node) {
                processor (*c->binding);
            }
        }
    }


    void VisitInputsOf (const Node* node, IConnectionBindingVisitor& visitor) const
    {
        for (const std::unique_ptr<NodeConnection>& c : connections) {
            if (c->to.get () == node) {
                c->binding->Visit (visitor);
            }
        }
    }

    
    void Add (std::unique_ptr<NodeConnection>&& connection)
    {
        Add (connection->from);
        Add (connection->to);

        connections.push_back (std::move (connection));
    }


    void Add (const std::shared_ptr<Node>& from, const std::shared_ptr<Node>& to, std::unique_ptr<IConnectionBinding>&& binding)
    {
        Add (std::make_unique<NodeConnection> (from, to, std::move (binding)));
    }


    void Add (const std::shared_ptr<Node>& node)
    {
        if (nodeSet.insert (node).second)
            insertionOrder.push_back (node);
    }
};


class GVK_RENDERER_API GraphSettings {
public:
    ConnectionSet      connectionSet;
    const GVK::DeviceExtra* device;
    uint32_t           framesInFlight;

    GraphSettings (const GVK::DeviceExtra& device, ConnectionSet&& connectionSet, uint32_t framesInFlight);
    GraphSettings (const GVK::DeviceExtra& device, uint32_t framesInFlight);

    GraphSettings ();
    GraphSettings (GraphSettings&&);
    GraphSettings& operator= (GraphSettings&&);

    const GVK::DeviceExtra& GetDevice () const;

    const GVK::Queue& GetGrahpicsQueue () const;

    const GVK::CommandPool& GetCommandPool () const;
};

} // namespace RG

#endif