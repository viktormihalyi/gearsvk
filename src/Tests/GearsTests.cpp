#include "GoogleTestEnvironment.hpp"

#include "DrawRecordableInfo.hpp"
#include "Font.hpp"
#include "GLFWWindow.hpp"
#include "GraphRenderer.hpp"
#include "ImageData.hpp"
#include "RenderGraph.hpp"
#include "UniformReflection.hpp"
#include "VulkanEnvironment.hpp"
#include "VulkanWrapper.hpp"

#include "StaticInit.hpp"
#include "glmlib.hpp"

#include "GearsAPIv2.hpp"
#include "SequenceAdapter.hpp"


using namespace GVK;
using namespace GVK::RG;

static const std::filesystem::path SequencesFolder = PROJECT_ROOT / "Project" / "Sequences";


StaticInit nosync ([] () {
    std::cout.sync_with_stdio (false);
});


class GearsTests : public GoogleTestEnvironmentBase {
protected:
    std::shared_ptr<Presentable>     pres;
    std::unique_ptr<SequenceAdapter> sequenceAdapter;

    virtual void SetUp () override
    {
        env = std::make_unique<VulkanEnvironment> (gtestDebugCallback);
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

        pres = std::make_unique<Presentable> (*env, std::make_unique<HiddenGLFWWindow> (), defaultSwapchainSettingsSingleImage);

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

        std::vector<std::unique_ptr<InheritedImage>> imgs = pres->GetSwapchain ().GetImageObjects ();

        ImageData img (GetDeviceExtra (), *imgs[0], 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        CompareImages (checkName, img);
    }

    void RenderFirstFrame ()
    {
        sequenceAdapter->RenderFrameIndex (1);
        sequenceAdapter->Wait ();
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


TEST_F (GearsTests, 1_fullfield_whites)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "1_Plain" / "1_fullfield_whites.pyx");

    RenderAndCompare (122, "1_fullfield_whites_122");
    RenderAndCompare (480, "1_fullfield_whites_480");
    RenderAndCompare (780, "1_fullfield_whites_780");
    RenderAndCompare (1200, "1_fullfield_whites_1200");
}


TEST_F (GearsTests, LoadOnly_2_fullfield_with_short_blacks)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "1_Plain" / "2_fullfield_with_short_blacks.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_3_offhunter)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "1_Plain" / "3_offhunter.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_4_melaparam)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "1_Plain" / "4_melaparam.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_1_square_oscillation)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "2_Oscillations" / "1_square_oscillation.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_2_sine_oscillation)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "2_Oscillations" / "2_sine_oscillation.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_3_mulifreq_synthesis)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "2_Oscillations" / "3_mulifreq_synthesis.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_4_mulifreq_floating)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "2_Oscillations" / "4_mulifreq_floating.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_1_linear_descreasing)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "3_Contrast" / "1_linear_descreasing.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_2_linear_increasing)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "3_Contrast" / "2_linear_increasing.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_3_rep2_n6_)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "3_Contrast" / "3_rep2_n6_.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_4_rep2_n6_05_1_0)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "3_Contrast" / "4_rep2_n6_05_1_0.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_1_chirp_freqmod_updown)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "4_SinChirp" / "1_chirp_freqmod_updown.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_2_chirp_freqmod_downup)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "4_SinChirp" / "2_chirp_freqmod_downup.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_3_chirp_ampmod_3Hz)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "4_SinChirp" / "3_chirp_ampmod_3Hz.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_4_chirp_ampmod_6Hz)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "4_SinChirp" / "4_chirp_ampmod_6Hz.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_5_chirp_ampmod_9Hz)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "4_SinChirp" / "5_chirp_ampmod_9Hz.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_9_15sec_chirp_freqmod_updown)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "4_SinChirp" / "9_15sec_chirp_freqmod_updown.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_g_chirp_lin_freqmod_updown)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "4_SinChirp" / "g_chirp_lin_freqmod_updown.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_1_squarechirp_freqmod_updown)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "5_SquareChirp" / "1_squarechirp_freqmod_updown.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_2_squarechirp_freqmod_downup)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "5_SquareChirp" / "2_squarechirp_freqmod_downup.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_3_squarechirp_ampmod_3Hz)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "5_SquareChirp" / "3_squarechirp_ampmod_3Hz.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_4_squarechirp_ampmod_6Hz)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "5_SquareChirp" / "4_squarechirp_ampmod_6Hz.pyx");
    RenderFirstFrame ();
}


TEST_F (GearsTests, LoadOnly_5_squarechirp_ampmod_9Hz)
{
    LoadFromFile (SequencesFolder / "2_FullFields" / "5_SquareChirp" / "5_squarechirp_ampmod_9Hz.pyx");
    RenderFirstFrame ();
}
