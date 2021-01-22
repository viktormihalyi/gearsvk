#include "GoogleTestEnvironment.hpp"

#include "DrawRecordableInfo.hpp"
#include "Font.hpp"
#include "GLFWWindow.hpp"
#include "ImageData.hpp"
#include "RenderGraph.hpp"
#include "UniformReflection.hpp"
#include "VulkanEnvironment.hpp"
#include "VulkanWrapper.hpp"

#include "glmlib.hpp"

#include "GearsAPIv2.hpp"
#include "SequenceAdapter.hpp"


static const std::filesystem::path SequencesFolder = PROJECT_ROOT / "src" / "UserInterface" / "Project" / "Sequences";


class GearsTests : public GoogleTestEnvironmentBase {
protected:
    Ptr<Presentable>   pres;
    U<SequenceAdapter> sequenceAdapter;

    virtual void SetUp () override
    {
        env = VulkanEnvironment::Create (gtestDebugCallback);
    }

    virtual void TearDown () override
    {
        sequenceAdapter.reset ();
        pres.reset ();
        env.reset ();
    }

    void LoadFromFile (const std::filesystem::path& sequencePath)
    {
        sequenceAdapter = Gears::GetSequenceAdapterFromPyx (*env, sequencePath);
        if (GVK_ERROR (sequenceAdapter == nullptr)) {
            FAIL ();
            return;
        }

        pres = Presentable::Create (*env, HiddenGLFWWindow::Create (), DefaultSwapchainSettingsSingleImage ());

        sequenceAdapter->SetCurrentPresentable (pres);
    }

    void RenderAndCompare (uint32_t frameIndex, const std::string& checkName)
    {
        sequenceAdapter->RenderFrameIndex (frameIndex);
        sequenceAdapter->Wait ();

        std::vector<InheritedImageU> imgs = pres->GetSwapchain ().GetImageObjects ();

        ImageData img (GetDeviceExtra (), *imgs[0], 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        CompareImages (checkName, img);
    }
};


TEST_F (GearsTests, 04_velocity400)
{
    LoadFromFile (SequencesFolder / "4_MovingShapes" / "1_Bars" / "04_velocity400.pyx");

    RenderAndCompare (243, "400_velocity400_243");
}


TEST_F (GearsTests, 04_monkey_velocity1200)
{
    LoadFromFile (SequencesFolder / "4_MovingShapes" / "2_Rects" / "04_monkey_velocity1200.pyx");

    RenderAndCompare (122, "04_monkey_velocity1200_122");
    RenderAndCompare (480, "04_monkey_velocity1200_480");
    RenderAndCompare (780, "04_monkey_velocity1200_780");
    RenderAndCompare (1200, "04_monkey_velocity1200_1200");
}
