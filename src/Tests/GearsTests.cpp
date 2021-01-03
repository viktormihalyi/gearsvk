#include "GoogleTestEnvironment.hpp"

#include "DrawRecordableInfo.hpp"
#include "Font.hpp"
#include "ImageData.hpp"
#include "RenderGraph.hpp"
#include "UniformReflection.hpp"
#include "VulkanWrapper.hpp"

#include "glmlib.hpp"

#include "GearsAPIv2.hpp"

using GearsTests = HeadlessGoogleTestEnvironment;


TEST_F (GearsTests, SimpleSequence)
{
    SetRenderGraphFromPyxFileSequence (std::filesystem::path ("src") / "UserInterface" / "Project" / "Sequences" / "4_MovingShapes" / "1_Bars" / "04_velocity400.pyx");
}