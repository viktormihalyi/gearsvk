#ifndef COMMANDBUFFER_HPP
#define COMMANDBUFFER_HPP

#include <vulkan/vulkan.h>

#include <unordered_map>

#include "VulkanWrapper/DeviceExtra.hpp"
#include "Utils/MovablePtr.hpp"
#include "VulkanWrapper/VulkanObject.hpp"
#include "VulkanWrapper/Command.hpp"

namespace GVK {

class Image;


class VULKANWRAPPER_API CommandBuffer : public VulkanObject {
private:
    GVK::MovablePtr<VkDevice>        device;
    GVK::MovablePtr<VkCommandPool>   commandPool;
    GVK::MovablePtr<VkCommandBuffer> handle;

    bool canRecordCommands;

public:
    std::vector<std::unique_ptr<Command>> recordedAbstractCommands;

public:
    CommandBuffer (VkDevice device, VkCommandPool commandPool);
    CommandBuffer (const DeviceExtra& device);

    CommandBuffer (CommandBuffer&&) = default;
    CommandBuffer& operator= (CommandBuffer&&) = default;

    virtual ~CommandBuffer () override;
    
    virtual void* GetHandleForName () const override { return handle; }

    virtual VkObjectType GetObjectTypeForName () const override { return VK_OBJECT_TYPE_COMMAND_BUFFER; }

    void Begin (VkCommandBufferUsageFlags flags = 0);

    void End ();

    void Reset (bool releaseResources = true);

    Command& RecordCommand (std::unique_ptr<Command>&& command);

    template<typename CommandType, typename... CommandParameters>
    Command& Record (CommandParameters&&... parameters)
    {
        return RecordCommand (std::make_unique<CommandType> (std::forward<CommandParameters> (parameters)...));
    }

    VkCommandBuffer GetHandle () const { return handle; }
};


} // namespace GVK

#endif