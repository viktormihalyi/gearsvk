#ifndef GRAPHSETTINGS_HPP
#define GRAPHSETTINGS_HPP

#include "GearsVk/GearsVkAPI.hpp"

#include "GearsVk/RenderGraph/Connections.hpp"
#include "GearsVk/RenderGraph/Node.hpp"
#include "VulkanWrapper/DeviceExtra.hpp"

#include <memory>
#include <vulkan/vulkan.h>


namespace GVK {

namespace RG {

class IResourceVisitor;

class GVK_RENDERER_API ConnectionSet final : public Noncopyable {
public:
    class Connection {
    public:
        std::shared_ptr<Node>               from;
        std::shared_ptr<Node>               to;
        std::unique_ptr<IConnectionBinding> binding;

        Connection (const std::shared_ptr<Node>&          from,
                    const std::shared_ptr<Node>&          to,
                    std::unique_ptr<IConnectionBinding>&& binding)
            : from (from)
            , to (to)
            , binding (std::move (binding))
        {
        }
    };

private:
    std::vector<std::unique_ptr<Connection>> connections;

public:
    std::set<std::shared_ptr<Node>> nodes;


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

        for (const std::unique_ptr<Connection>& c : connections) {
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

        for (const std::unique_ptr<Connection>& c : connections) {
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
        for (const std::unique_ptr<Connection>& c : connections) {
            if (c->from.get () == node) {
                c->binding->Visit (visitor);
            }
        }
    }

    template<typename Processor>
    void ProcessInputBindingsOf (const Node* node, const Processor& processor) const
    {
        for (const std::unique_ptr<Connection>& c : connections) {
            if (c->to.get () == node) {
                processor (*c->binding);
            }
        }
    }
    template<typename Processor>
    void ProcessOutputBindingsOf (const Node* node, const Processor& processor) const
    {
        for (const std::unique_ptr<Connection>& c : connections) {
            if (c->from.get () == node) {
                processor (*c->binding);
            }
        }
    }


    void VisitInputsOf (const Node* node, IConnectionBindingVisitor& visitor) const
    {
        for (const std::unique_ptr<Connection>& c : connections) {
            if (c->to.get () == node) {
                c->binding->Visit (visitor);
            }
        }
    }


    void Add (const std::shared_ptr<Node>& from, const std::shared_ptr<Node>& to, std::unique_ptr<IConnectionBinding>&& binding)
    {
        nodes.insert (from);
        nodes.insert (to);
        connections.push_back (std::make_unique<Connection> (from, to, std::move (binding)));
    }


    void Add (const std::shared_ptr<Node>& node)
    {
        nodes.insert (node);
    }
};


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