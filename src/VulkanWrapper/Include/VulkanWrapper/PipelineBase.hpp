#ifndef VULKANWRAPPER_PIPELINEBASE_HPP
#define VULKANWRAPPER_PIPELINEBASE_HPP

#include "VulkanWrapper/VulkanWrapperExport.hpp"
#include <vulkan/vulkan.h>

#include "Utils/Assert.hpp"
#include "Utils/MovablePtr.hpp"
#include "VulkanObject.hpp"

namespace GVK {

class VULKANWRAPPER_DLL_EXPORT PipelineBase : public VulkanObject {
public:
    PipelineBase () = default;

    virtual ~PipelineBase () override;

    PipelineBase (PipelineBase&&) = default;
    PipelineBase& operator= (PipelineBase&&) = default;

    virtual VkObjectType GetObjectTypeForName () const override { return VK_OBJECT_TYPE_PIPELINE; }

    virtual operator VkPipeline () const = 0;
};

} // namespace GVK

#endif