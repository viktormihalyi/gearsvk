#ifndef GRAPHSETTINGS_HPP
#define GRAPHSETTINGS_HPP

struct GraphSettings {
    const uint32_t framesInFlight;
    const uint32_t width;
    const uint32_t height;

    GraphSettings (uint32_t framesInFlight, uint32_t width, uint32_t height)
        : framesInFlight (framesInFlight)
        , width (width)
        , height (height)
    {
    }

    GraphSettings (const Swapchain& swapchain)
        : framesInFlight (swapchain.GetImageCount ())
        , width (swapchain.GetWidth ())
        , height (swapchain.GetHeight ())
    {
    }
};

#endif