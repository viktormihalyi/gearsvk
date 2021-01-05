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
    SetRenderGraphFromPyxFileSequence (PROJECT_ROOT / "src" / "UserInterface" / "Project" / "Sequences" / "4_MovingShapes" / "1_Bars" / "04_velocity400.pyx");
}