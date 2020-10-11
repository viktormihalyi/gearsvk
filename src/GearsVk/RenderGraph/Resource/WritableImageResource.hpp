#ifndef RG_WRITABLEIMAGERESOURCE_HPP
#define RG_WRITABLEIMAGERESOURCE_HPP


#include "ImageResource.hpp"
#include "InputBindable.hpp"

#include "Sampler.hpp"

namespace RG {

USING_PTR (WritableImageResource);
class GEARSVK_API WritableImageResource : public ImageResource, public InputImageBindable {
    USING_CREATE (WritableImageResource);

private:
    SamplerU sampler;

    USING_PTR (SingleImageResource);
    struct SingleImageResource final;

public:
    const uint32_t width;
    const uint32_t height;
    const uint32_t arrayLayers;

    std::vector<SingleImageResourceU> images;

public:
    WritableImageResource (uint32_t width, uint32_t height, uint32_t arrayLayers = 1);

    virtual ~WritableImageResource () override = default;

    // overriding Resource
    virtual void Compile (const GraphSettings& graphSettings) override;

    // overriding ImageResource
    virtual void                    BindRead (uint32_t imageIndex, CommandBuffer& commandBuffer) override;
    virtual void                    BindWrite (uint32_t imageIndex, CommandBuffer& commandBuffer) override;
    virtual VkImageLayout           GetFinalLayout () const override;
    virtual VkFormat                GetFormat () const override;
    virtual uint32_t                GetDescriptorCount () const override;
    virtual std::vector<ImageBase*> GetImages () const override;

    // overriding InputImageBindable
    virtual VkImageView GetImageViewForFrame (uint32_t frameIndex, uint32_t layerIndex) override;
    virtual VkSampler   GetSampler () override;
};

} // namespace RG


#endif