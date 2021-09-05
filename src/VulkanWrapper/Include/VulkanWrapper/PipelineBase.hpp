#ifndef VULKANWRAPPER_PIPELINEBASE_HPP
#define VULKANWRAPPER_PIPELINEBASE_HPP

#include "VulkanWrapper/VulkanWrapperAPI.hpp"
#include <vulkan/vulkan.h>

#include "Utils/Assert.hpp"
#include "Utils/MovablePtr.hpp"
#include "VulkanObject.hpp"

namespace GVK {

class VULKANWRAPPER_API PipelineBase : public VulkanObject {
public:
    virtual ~PipelineBase () override;

    virtual VkObjectType GetObjectTypeForName () const override { return VK_OBJECT_TYPE_PIPELINE; }

    virtual operator VkPipeline () const = 0;
};

} // namespace GVK

#endif