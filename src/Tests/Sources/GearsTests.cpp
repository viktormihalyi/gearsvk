#include "GoogleTestEnvironment.hpp"

#include "RenderGraph/DrawRecordable/DrawRecordable.hpp"
#include "RenderGraph/Font.hpp"
#include "RenderGraph/Window/GLFWWindow.hpp"
#include "RenderGraph/GraphRenderer.hpp"
#include "VulkanWrapper/Utils/ImageData.hpp"
#include "RenderGraph/RenderGraph.hpp"
#include "RenderGraph/UniformReflection.hpp"
#include "RenderGraph/VulkanEnvironment.hpp"
#include "VulkanWrapper/VulkanWrapper.hpp"
#include "Sequence/StimulusAdapter.hpp"

#include "Utils/StaticInit.hpp"
#include <glm/glm.hpp>

#define CEREAL_THREAD_SAFE 1 // doesnt compile otherwise

#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>

#include "Sequence/Pass.h"
#include "Sequence/Sequence.h"
#include "Sequence/Stimulus.h"
#include "Sequence/SpatialFilter.h"
#include "Sequence/Response.h"

#include "PySequence/core/PyPass.h"
#include "PySequence/core/PySequence.h"
#include "PySequence/core/PyStimulus.h"
#include "PySequence/core/PyResponse.h"


#include "GearsPYD/GearsAPIv2.hpp"
#include "Sequence/SequenceAdapter.hpp"


#include <sstream>

namespace glm
{

    template<class Archive> void serialize(Archive& archive, glm::vec2& v) { archive(v.x, v.y); }
    template<class Archive> void serialize(Archive& archive, glm::vec3& v) { archive(v.x, v.y, v.z); }
    template<class Archive> void serialize(Archive& archive, glm::vec4& v) { archive(v.x, v.y, v.z, v.w); }
    template<class Archive> void serialize(Archive& archive, glm::ivec2& v) { archive(v.x, v.y); }
    template<class Archive> void serialize(Archive& archive, glm::ivec3& v) { archive(v.x, v.y, v.z); }
    template<class Archive> void serialize(Archive& archive, glm::ivec4& v) { archive(v.x, v.y, v.z, v.w); }
    template<class Archive> void serialize(Archive& archive, glm::uvec2& v) { archive(v.x, v.y); }
    template<class Archive> void serialize(Archive& archive, glm::uvec3& v) { archive(v.x, v.y, v.z); }
    template<class Archive> void serialize(Archive& archive, glm::uvec4& v) { archive(v.x, v.y, v.z, v.w); }
    template<class Archive> void serialize(Archive& archive, glm::dvec2& v) { archive(v.x, v.y); }
    template<class Archive> void serialize(Archive& archive, glm::dvec3& v) { archive(v.x, v.y, v.z); }
    template<class Archive> void serialize(Archive& archive, glm::dvec4& v) { archive(v.x, v.y, v.z, v.w); }

    // glm matrices serialization
    template<class Archive> void serialize(Archive& archive, glm::mat2& m) { archive(m[0], m[1]); }
    template<class Archive> void serialize(Archive& archive, glm::dmat2& m) { archive(m[0], m[1]); }
    template<class Archive> void serialize(Archive& archive, glm::mat3& m) { archive(m[0], m[1], m[2]); }
    template<class Archive> void serialize(Archive& archive, glm::mat4& m) { archive(m[0], m[1], m[2], m[3]); }
    template<class Archive> void serialize(Archive& archive, glm::dmat4& m) { archive(m[0], m[1], m[2], m[3]); }

}


using namespace GVK;
using namespace GVK::RG;

static const std::filesystem::path SequencesFolder = std::filesystem::current_path () / "Project" / "Sequences";


StaticInit nosync ([] () {
    std::cout.sync_with_stdio (false);
});


class GearsTests : public GoogleTestEnvironmentBase {
protected:
    std::shared_ptr<Presentable>     pres;
    std::unique_ptr<SequenceAdapter> sequenceAdapter;

    virtual void SetUp () override
    {
        env = std::make_unique<VulkanEnvironment> (gtestDebugCallback, GetGLFWInstanceExtensions (), std::vector<const char*> { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
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

        ImageData img { GetDeviceExtra (), *imgs[0], 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };

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

    {
        
        std::stringstream ss; // any stream can be used
        cereal::JSONOutputArchive oarchive (ss); // Create an output archive

        oarchive (sequenceAdapter->GetSequence ());

        std::string val = ss.str ();
        Utils::WriteTextFile (std::filesystem::current_path () / "asd.txt", val);
    }


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


TEST_F (GearsTests, 3_Patterns_1_Gratings_3_dual)
{
    LoadFromFile (SequencesFolder / "3_Patterns" / "1_Gratings" / "3_dual.pyx");

    RenderAndCompare (480, "3_dual_480");
}


TEST_F (GearsTests, 3_Patterns_8_Other_3_mandelbrot)
{
    LoadFromFile (SequencesFolder / "3_Patterns" / "8_Other" / "3_mandelbrot.pyx");

    RenderAndCompare (122, "3_mandelbrot_122");
    RenderAndCompare (480, "3_mandelbrot_480");
    RenderAndCompare (780, "3_mandelbrot_780");
    RenderAndCompare (1000, "3_mandelbrot_1000");
}


// clang-format off

TEST_F (GearsTests, LoadOnly_0_Utility_1_Spots_1_tiny_red) { LoadFromFile (SequencesFolder / "0_Utility" / "1_Spots" / "1_tiny_red.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_0_Utility_1_Spots_2_big_red) { LoadFromFile (SequencesFolder / "0_Utility" / "1_Spots" / "2_big_red.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_0_Utility_1_Spots_3_interactive_red) { LoadFromFile (SequencesFolder / "0_Utility" / "1_Spots" / "3_interactive_red.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_0_Utility_1_Spots_7_tiny_red_short) { LoadFromFile (SequencesFolder / "0_Utility" / "1_Spots" / "7_tiny_red_short.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_0_Utility_1_Spots_8_electrodeGrid) { LoadFromFile (SequencesFolder / "0_Utility" / "1_Spots" / "8_electrodeGrid.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_0_Utility_2_FullField_1_long) { LoadFromFile (SequencesFolder / "0_Utility" / "2_FullField" / "1_long.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_0_Utility_2_FullField_2_wide_intensity_range_gradient) { LoadFromFile (SequencesFolder / "0_Utility" / "2_FullField" / "2_wide_intensity_range_gradient.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_0_Utility_2_FullField_3_wide_intensity_range_gradient_tone_test) { LoadFromFile (SequencesFolder / "0_Utility" / "2_FullField" / "3_wide_intensity_range_gradient_tone_test.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_1_Spots_1_VaryingSize_1_spot_and_fullfield) { LoadFromFile (SequencesFolder / "1_Spots" / "1_VaryingSize" / "1_spot_and_fullfield.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_1_Spots_1_VaryingSize_2_increasing_radii) { LoadFromFile (SequencesFolder / "1_Spots" / "1_VaryingSize" / "2_increasing_radii.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_1_Spots_1_VaryingSize_3_annuli_increasing_radii) { LoadFromFile (SequencesFolder / "1_Spots" / "1_VaryingSize" / "3_annuli_increasing_radii.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_1_Spots_2_ElectrodeTargeted_1_targeting_seqA) { LoadFromFile (SequencesFolder / "1_Spots" / "2_ElectrodeTargeted" / "1_targeting_seqA.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_1_Spots_2_ElectrodeTargeted_2_targeting_seqB) { LoadFromFile (SequencesFolder / "1_Spots" / "2_ElectrodeTargeted" / "2_targeting_seqB.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_1_Spots_2_ElectrodeTargeted_2_targeting_seqC) { LoadFromFile (SequencesFolder / "1_Spots" / "2_ElectrodeTargeted" / "3_targeting_seqC.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_1_Spots_3_Oscillations_1_sine_increasing_freqs) { LoadFromFile (SequencesFolder / "1_Spots" / "3_Oscillations" / "1_sine_increasing_freqs.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_1_Spots_3_SineChirp_1_spotchirp_freqmod_updown) { LoadFromFile (SequencesFolder / "1_Spots" / "3_SineChirp" / "1_spotchirp_freqmod_updown.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_1_Spots_3_SineChirp_2_spotchirp_freqmod_downup) { LoadFromFile (SequencesFolder / "1_Spots" / "3_SineChirp" / "2_spotchirp_freqmod_downup.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_1_Spots_3_SineChirp_3_spotchirp_ampmod_3Hz) { LoadFromFile (SequencesFolder / "1_Spots" / "3_SineChirp" / "3_spotchirp_ampmod_3Hz.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_1_Spots_3_SineChirp_4_spotchirp_ampmod_6Hz) { LoadFromFile (SequencesFolder / "1_Spots" / "3_SineChirp" / "4_spotchirp_ampmod_6Hz.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_1_Spots_3_SineChirp_5_spotchirp_ampmod_9Hz) { LoadFromFile (SequencesFolder / "1_Spots" / "3_SineChirp" / "5_spotchirp_ampmod_9Hz.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, DISABLED_LoadOnly_1_Spots_4_SampleFilters_1_Spatial_0_compare) { LoadFromFile (SequencesFolder / "1_Spots" / "4_SampleFilters" / "1_Spatial" / "0_compare.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, DISABLED_LoadOnly_1_Spots_4_SampleFilters_1_Spatial_test) { LoadFromFile (SequencesFolder / "1_Spots" / "4_SampleFilters" / "1_Spatial" / "test.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, DISABLED_LoadOnly_1_Spots_4_SampleFilters_2_Temporal_1_exponential) { LoadFromFile (SequencesFolder / "1_Spots" / "4_SampleFilters" / "2_Temporal" / "1_exponential.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, DISABLED_LoadOnly_1_Spots_4_SampleFilters_2_Temporal_2_cell) { LoadFromFile (SequencesFolder / "1_Spots" / "4_SampleFilters" / "2_Temporal" / "2_cell.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, DISABLED_LoadOnly_1_Spots_4_SampleFilters_2_Temporal_3_exponentialLti) { LoadFromFile (SequencesFolder / "1_Spots" / "4_SampleFilters" / "2_Temporal" / "3_exponentialLti.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, DISABLED_LoadOnly_1_Spots_4_SampleFilters_2_Temporal_4_cellLti3) { LoadFromFile (SequencesFolder / "1_Spots" / "4_SampleFilters" / "2_Temporal" / "4_cellLti3.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, DISABLED_LoadOnly_1_Spots_4_SampleFilters_2_Temporal_5_cellLti7) { LoadFromFile (SequencesFolder / "1_Spots" / "4_SampleFilters" / "2_Temporal" / "5_cellLti7.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, DISABLED_LoadOnly_1_Spots_4_SampleFilters_2_Temporal_6_cellLti3Color) { LoadFromFile (SequencesFolder / "1_Spots" / "4_SampleFilters" / "2_Temporal" / "6_cellLti3Color.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, DISABLED_LoadOnly_1_Spots_4_SampleFilters_2_Temporal_7_cellLti7Color) { LoadFromFile (SequencesFolder / "1_Spots" / "4_SampleFilters" / "2_Temporal" / "7_cellLti7Color.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, DISABLED_LoadOnly_1_Spots_4_SampleFilters_2_Temporal_8_explicitLti7Color) { LoadFromFile (SequencesFolder / "1_Spots" / "4_SampleFilters" / "2_Temporal" / "8_explicitLti7Color.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, DISABLED_LoadOnly_1_Spots_4_SampleFilters_test) { LoadFromFile (SequencesFolder / "1_Spots" / "4_SampleFilters" / "test.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_2_FullFields_1_Plain_2_fullfield_with_short_blacks) { LoadFromFile (SequencesFolder / "2_FullFields" / "1_Plain" / "2_fullfield_with_short_blacks.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_2_FullFields_1_Plain_3_offhunter) { LoadFromFile (SequencesFolder / "2_FullFields" / "1_Plain" / "3_offhunter.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_2_FullFields_1_Plain_4_melaparam) { LoadFromFile (SequencesFolder / "2_FullFields" / "1_Plain" / "4_melaparam.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_2_FullFields_2_Oscillations_1_square_oscillation) { LoadFromFile (SequencesFolder / "2_FullFields" / "2_Oscillations" / "1_square_oscillation.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_2_FullFields_2_Oscillations_2_sine_oscillation) { LoadFromFile (SequencesFolder / "2_FullFields" / "2_Oscillations" / "2_sine_oscillation.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_2_FullFields_2_Oscillations_3_mulifreq_synthesis) { LoadFromFile (SequencesFolder / "2_FullFields" / "2_Oscillations" / "3_mulifreq_synthesis.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_2_FullFields_2_Oscillations_4_mulifreq_floating) { LoadFromFile (SequencesFolder / "2_FullFields" / "2_Oscillations" / "4_mulifreq_floating.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_2_FullFields_3_Contrast_1_linear_descreasing) { LoadFromFile (SequencesFolder / "2_FullFields" / "3_Contrast" / "1_linear_descreasing.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_2_FullFields_3_Contrast_2_linear_increasing) { LoadFromFile (SequencesFolder / "2_FullFields" / "3_Contrast" / "2_linear_increasing.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_2_FullFields_3_Contrast_3_rep2_n6_) { LoadFromFile (SequencesFolder / "2_FullFields" / "3_Contrast" / "3_rep2_n6_.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_2_FullFields_3_Contrast_4_rep2_n6_05_1_0) { LoadFromFile (SequencesFolder / "2_FullFields" / "3_Contrast" / "4_rep2_n6_05_1_0.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_2_FullFields_4_SinChirp_1_chirp_freqmod_updown) { LoadFromFile (SequencesFolder / "2_FullFields" / "4_SinChirp" / "1_chirp_freqmod_updown.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_2_FullFields_4_SinChirp_2_chirp_freqmod_downup) { LoadFromFile (SequencesFolder / "2_FullFields" / "4_SinChirp" / "2_chirp_freqmod_downup.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_2_FullFields_4_SinChirp_3_chirp_ampmod_3Hz) { LoadFromFile (SequencesFolder / "2_FullFields" / "4_SinChirp" / "3_chirp_ampmod_3Hz.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_2_FullFields_4_SinChirp_4_chirp_ampmod_6Hz) { LoadFromFile (SequencesFolder / "2_FullFields" / "4_SinChirp" / "4_chirp_ampmod_6Hz.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_2_FullFields_4_SinChirp_5_chirp_ampmod_9Hz) { LoadFromFile (SequencesFolder / "2_FullFields" / "4_SinChirp" / "5_chirp_ampmod_9Hz.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_2_FullFields_4_SinChirp_9_15sec_chirp_freqmod_updown) { LoadFromFile (SequencesFolder / "2_FullFields" / "4_SinChirp" / "9_15sec_chirp_freqmod_updown.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_2_FullFields_4_SinChirp_g_chirp_lin_freqmod_updown) { LoadFromFile (SequencesFolder / "2_FullFields" / "4_SinChirp" / "g_chirp_lin_freqmod_updown.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_1_squarechirp_freqmod_updown) { LoadFromFile (SequencesFolder / "2_FullFields" / "5_SquareChirp" / "1_squarechirp_freqmod_updown.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_2_squarechirp_freqmod_downup) { LoadFromFile (SequencesFolder / "2_FullFields" / "5_SquareChirp" / "2_squarechirp_freqmod_downup.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_3_squarechirp_ampmod_3Hz) { LoadFromFile (SequencesFolder / "2_FullFields" / "5_SquareChirp" / "3_squarechirp_ampmod_3Hz.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_squarechirp_ampmod_6Hz) { LoadFromFile (SequencesFolder / "2_FullFields" / "5_SquareChirp" / "4_squarechirp_ampmod_6Hz.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_squarechirp_ampmod_9Hz) { LoadFromFile (SequencesFolder / "2_FullFields" / "5_SquareChirp" / "5_squarechirp_ampmod_9Hz.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_3_Patterns_1_Gratings_1_sine) { LoadFromFile (SequencesFolder / "3_Patterns" / "1_Gratings" / "1_sine.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_3_Patterns_1_Gratings_2_wheel) { LoadFromFile (SequencesFolder / "3_Patterns" / "1_Gratings" / "2_wheel.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_3_Patterns_1_Gratings_4_two_gratings) { LoadFromFile (SequencesFolder / "3_Patterns" / "1_Gratings" / "4_two_gratings.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_3_Patterns_2_MovingGrids_1_sin) { LoadFromFile (SequencesFolder / "3_Patterns" / "2_MovingGrids" / "1_sin.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_3_Patterns_2_MovingGrids_2_square) { LoadFromFile (SequencesFolder / "3_Patterns" / "2_MovingGrids" / "2_square.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_3_Patterns_2_MovingGrids_3_sinPhaseInvert) { LoadFromFile (SequencesFolder / "3_Patterns" / "2_MovingGrids" / "3_sinPhaseInvert.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_3_Patterns_8_Other_1_classicCampbellRobertson) { LoadFromFile (SequencesFolder / "3_Patterns" / "8_Other" / "1_classicCampbellRobertson.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_3_Patterns_8_Other_2_constCampbellRobertson) { LoadFromFile (SequencesFolder / "3_Patterns" / "8_Other" / "2_constCampbellRobertson.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_3_Patterns_8_Other_4_gradients) { LoadFromFile (SequencesFolder / "3_Patterns" / "8_Other" / "4_gradients.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_3_Patterns_8_Other_5_edges) { LoadFromFile (SequencesFolder / "3_Patterns" / "8_Other" / "5_edges.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_4_MovingShapes_1_Bars_01_velocity50) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "1_Bars" / "01_velocity50.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_1_Bars_02_velocity100) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "1_Bars" / "02_velocity100.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_1_Bars_03_velocity200) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "1_Bars" / "03_velocity200.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_1_Bars_04_velocity400) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "1_Bars" / "04_velocity400.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_1_Bars_05_velocity800) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "1_Bars" / "05_velocity800.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_1_Bars_06_velocity1200) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "1_Bars" / "06_velocity1200.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_1_Bars_07_velocity1200limited) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "1_Bars" / "07_velocity1200limited.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_1_Bars_08_velocity1200limitedModifiedComponents) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "1_Bars" / "08_velocity1200limitedModifiedComponents.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_1_Bars_11_300um_velocity50) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "1_Bars" / "11_300um_velocity50.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_1_Bars_12_300um_velocity100) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "1_Bars" / "12_300um_velocity100.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_1_Bars_13_300um_velocity200) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "1_Bars" / "13_300um_velocity200.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_1_Bars_14_300um_velocity400) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "1_Bars" / "14_300um_velocity400.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_1_Bars_15_300um_velocity800) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "1_Bars" / "15_300um_velocity800.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_1_Bars_16_300um_velocity1200) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "1_Bars" / "16_300um_velocity1200.pyx"); RenderFirstFrame (); }
// temporal filter
TEST_F (GearsTests, DISABLED_LoadOnly_4_MovingShapes_1_Bards_21_temp_filter_test) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "1_Bars" / "21_temp_filter_test.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_4_MovingShapes_2_Rects_01_monkey_velocity120) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "2_Rects" / "01_monkey_velocity120.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_2_Rects_02_monkey_velocity200) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "2_Rects" / "02_monkey_velocity200.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_2_Rects_03_monkey_velocity400) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "2_Rects" / "03_monkey_velocity400.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_2_Rects_04_monkey_velocity1200) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "2_Rects" / "04_monkey_velocity1200.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_2_Rects_05_monkey_velocity1600) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "2_Rects" / "05_monkey_velocity1600.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_2_Rects_11_monkey_scan_velocity120_rep3) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "2_Rects" / "11_monkey_scan_velocity120_rep3.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_2_Rects_12_monkey_scan_velocity1200_rep6) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "2_Rects" / "12_monkey_scan_velocity1200_rep6.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_2_Rects_21_dozer_velocity120_rep3) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "2_Rects" / "21_dozer_velocity120_rep3.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_2_Rects_22_dozer_velocity200_rep3) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "2_Rects" / "22_dozer_velocity200_rep3.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_2_Rects_23_dozer_velocity1200_rep6) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "2_Rects" / "23_dozer_velocity1200_rep6.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_2_Rects_31_comb_velocity120_rep3) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "2_Rects" / "31_comb_velocity120_rep3.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_2_Rects_32_comb_velocity1200_rep3) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "2_Rects" / "32_comb_velocity1200_rep3.pyx"); RenderFirstFrame (); }

// temporal filter
TEST_F (GearsTests, DISABLED_LoadOnly_4_MovingShapes_3_Spots_1_tempocell) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "3_Spots" / "1_tempocell.pyx"); RenderFirstFrame (); }
// instancing
TEST_F (GearsTests, DISABLED_LoadOnly_4_MovingShapes_3_Spots_9_quads) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "3_Spots" / "9_quads.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_4_MovingShapes_4_Interactive_1_mouseAdjustable) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "4_Interactive" / "1_mouseAdjustable.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_4_Interactive_2_mouseAdjustableBulldozer) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "4_Interactive" / "2_mouseAdjustableBulldozer.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_4_Interactive_3_mouseResizableRect) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "4_Interactive" / "3_mouseResizableRect.pyx"); RenderFirstFrame (); }
// spatial filter
TEST_F (GearsTests, DISABLED_LoadOnly_4_MovingShapes_4_Interactive_4_mouseResizableFilter) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "4_Interactive" / "4_mouseResizableFilter.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_4_Interactive_5_mouseColor) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "4_Interactive" / "5_mouseColor.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_4_Interactive_t_mouseResizableRectPair) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "4_Interactive" / "t_mouseResizableRectPair.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_4_MovingShapes_5_Scan_1_scanTinySquares) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "5_Scan" / "1_scanTinySquares.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_5_Scan_2_scanTinySquaresShuffled) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "5_Scan" / "2_scanTinySquaresShuffled.pyx"); RenderFirstFrame (); }

// GLSL shader nem fordul: figmotid
// instancing
TEST_F (GearsTests, DISABLED_LoadOnly_4_MovingShapes_6_Complex_1_brownian) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "6_Complex" / "1_brownian.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, DISABLED_LoadOnly_4_MovingShapes_6_Complex_5_showoff) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "6_Complex" / "5_showoff.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, DISABLED_LoadOnly_4_MovingShapes_6_Complex_6_showoffScale) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "6_Complex" / "6_showoffScale.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_4_MovingShapes_6_Complex_7_showoffChaos) { LoadFromFile (SequencesFolder / "4_MovingShapes" / "6_Complex" / "7_showoffChaos.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_5_Randoms_1_FullFields_1_Binary_1_ff_60Hz) { LoadFromFile (SequencesFolder / "5_Randoms" / "1_FullFields" / "1_Binary" / "1_ff_60Hz.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_1_FullFields_1_Binary_2_ff_30Hz) { LoadFromFile (SequencesFolder / "5_Randoms" / "1_FullFields" / "1_Binary" / "2_ff_30Hz.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_1_FullFields_1_Binary_3_ff_20Hz) { LoadFromFile (SequencesFolder / "5_Randoms" / "1_FullFields" / "1_Binary" / "3_ff_20Hz.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_1_FullFields_1_Binary_4_ff_60Hz_25_75) { LoadFromFile (SequencesFolder / "5_Randoms" / "1_FullFields" / "1_Binary" / "4_ff_60Hz_25-75.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_1_FullFields_1_Binary_5_ff_30Hz_25_75) { LoadFromFile (SequencesFolder / "5_Randoms" / "1_FullFields" / "1_Binary" / "5_ff_30Hz_25-75.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_1_FullFields_1_Binary_6_ff_20Hz_25_75) { LoadFromFile (SequencesFolder / "5_Randoms" / "1_FullFields" / "1_Binary" / "6_ff_20Hz_25-75.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_5_Randoms_1_FullFields_1_Binary_2_GreyScale_1_ff_60Hz) { LoadFromFile (SequencesFolder / "5_Randoms" / "1_FullFields" / "2_GreyScale" / "1_ff_60Hz.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_5_Randoms_1_FullFields_3_Color_1_color_60Hz) { LoadFromFile (SequencesFolder / "5_Randoms" / "1_FullFields" / "3_Color" / "1_color_60Hz.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_1_FullFields_3_Color_1_sat_color_60Hz) { LoadFromFile (SequencesFolder / "5_Randoms" / "1_FullFields" / "3_Color" / "1_sat_color_60Hz.pyx"); RenderFirstFrame (); }

// spatial filter
TEST_F (GearsTests, DISABLED_LoadOnly_5_Randoms_2_Checkerboards_1_Binary_1_chess_60Hz) { LoadFromFile (SequencesFolder / "5_Randoms" / "2_Checkerboards" / "1_Binary" / "1_chess_60Hz.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_2_Checkerboards_1_Binary_2_chess_30Hz) { LoadFromFile (SequencesFolder / "5_Randoms" / "2_Checkerboards" / "1_Binary" / "2_chess_30Hz.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_2_Checkerboards_1_Binary_3_chess_20Hz) { LoadFromFile (SequencesFolder / "5_Randoms" / "2_Checkerboards" / "1_Binary" / "3_chess_20Hz.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_2_Checkerboards_1_Binary_4_chess_60Hz_25_75) { LoadFromFile (SequencesFolder / "5_Randoms" / "2_Checkerboards" / "1_Binary" / "4_chess_60Hz_25-75.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_2_Checkerboards_1_Binary_5_chess_30Hz_25_75) { LoadFromFile (SequencesFolder / "5_Randoms" / "2_Checkerboards" / "1_Binary" / "5_chess_30Hz_25-75.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_2_Checkerboards_1_Binary_6_chess_20Hz_25_75) { LoadFromFile (SequencesFolder / "5_Randoms" / "2_Checkerboards" / "1_Binary" / "6_chess_20Hz_25-75.pyx"); RenderFirstFrame (); }
// NameError: name 'MaskedDefaultSequence' is not defined
TEST_F (GearsTests, DISABLED_LoadOnly_5_Randoms_2_Checkerboards_1_Binary_8_chess_pinch) { LoadFromFile (SequencesFolder / "5_Randoms" / "2_Checkerboards" / "1_Binary" / "8_chess_pinch.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, DISABLED_LoadOnly_5_Randoms_2_Checkerboards_1_Binary_9_chess_hires) { LoadFromFile (SequencesFolder / "5_Randoms" / "2_Checkerboards" / "1_Binary" / "9_chess_hires.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_5_Randoms_2_Checkerboards_2_GreyScale_1_soft_60Hz) { LoadFromFile (SequencesFolder / "5_Randoms" / "2_Checkerboards" / "2_GreyScale" / "1_soft_60Hz.pyx"); RenderFirstFrame (); }
// spatial filter
TEST_F (GearsTests, DISABLED_LoadOnly_5_Randoms_2_Checkerboards_2_GreyScale_2_soft_60Hz_big_dog) { LoadFromFile (SequencesFolder / "5_Randoms" / "2_Checkerboards" / "2_GreyScale" / "2_soft_60Hz_big_dog.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_5_Randoms_2_Checkerboards_3_Color_1_color_60Hz) { LoadFromFile (SequencesFolder / "5_Randoms" / "2_Checkerboards" / "3_Color" / "1_color_60Hz.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_2_Checkerboards_3_Color_1_sat_color_60Hz) { LoadFromFile (SequencesFolder / "5_Randoms" / "2_Checkerboards" / "3_Color" / "1_sat_color_60Hz.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_5_Randoms_2_Checkerboards_4_Masked_1_chess_annulus) { LoadFromFile (SequencesFolder / "5_Randoms" / "2_Checkerboards" / "4_Masked" / "1_chess_annulus.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_5_Randoms_3_Bullseye_1_chess_polar_spot) { LoadFromFile (SequencesFolder / "5_Randoms" / "3_Bullseye" / "1_chess_polar_spot.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_3_Bullseye_2_chess_polar_annulus) { LoadFromFile (SequencesFolder / "5_Randoms" / "3_Bullseye" / "2_chess_polar_annulus.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_3_Bullseye_3_chess_polar_geometricInverse) { LoadFromFile (SequencesFolder / "5_Randoms" / "3_Bullseye" / "3_chess_polar_geometricInverse.pyx"); RenderFirstFrame (); }

TEST_F (GearsTests, LoadOnly_5_Randoms_4_Barcode_1_barcode) { LoadFromFile (SequencesFolder / "5_Randoms" / "4_Barcode" / "1_barcode.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_4_Barcode_2_barcodeGrey) { LoadFromFile (SequencesFolder / "5_Randoms" / "4_Barcode" / "2_barcodeGrey.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_4_Barcode_3_barcodeSatColor) { LoadFromFile (SequencesFolder / "5_Randoms" / "4_Barcode" / "3_barcodeSatColor.pyx"); RenderFirstFrame (); }
TEST_F (GearsTests, LoadOnly_5_Randoms_4_Barcode_4_barcodeColor) { LoadFromFile (SequencesFolder / "5_Randoms" / "4_Barcode" / "4_barcodeColor.pyx"); RenderFirstFrame (); }

// clang-format on
