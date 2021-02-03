#include "GoogleTestEnvironment.hpp"

#include "DrawRecordableInfo.hpp"
#include "Font.hpp"
#include "GLFWWindow.hpp"
#include "ImageData.hpp"
#include "RenderGraph.hpp"
#include "UniformReflection.hpp"
#include "VulkanEnvironment.hpp"
#include "VulkanWrapper.hpp"

#include "StaticInit.hpp"
#include "glmlib.hpp"

#include "GearsAPIv2.hpp"
#include "SequenceAdapter.hpp"


static const std::filesystem::path SequencesFolder = PROJECT_ROOT / "src" / "UserInterface" / "Project" / "Sequences";


StaticInit nosync ([] () {
    std::cout.sync_with_stdio (false);
});


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
        if (GVK_ERROR (!std::filesystem::exists (sequencePath))) {
            FAIL ();
            return;
        }

        sequenceAdapter = Gears::GetSequenceAdapterFromPyx (*env, sequencePath);
        if (GVK_ERROR (sequenceAdapter == nullptr)) {
            FAIL ();
            return;
        }

        pres = Presentable::Create (*env, HiddenGLFWWindow::Create (), defaultSwapchainSettingsSingleImage);

        bool success = false;
        try {
            sequenceAdapter->SetCurrentPresentable (pres);
            success = true;
        } catch (std::exception& ex) {
            std::cout << ex.what () << std::endl;
            success = false;
        }

        if (!success) {
            FAIL ();
            return;
        }
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


TEST_F (GearsTests, 2_chess_30Hz)
{
    LoadFromFile (SequencesFolder / "5_Randoms" / "2_Checkerboards" / "1_Binary" / "2_chess_30Hz.pyx");

    RenderAndCompare (122, "2_chess_30Hz_122");
    RenderAndCompare (480, "2_chess_30Hz_480");
    RenderAndCompare (780, "2_chess_30Hz_780");
    RenderAndCompare (1200, "2_chess_30Hz_1200");
}
