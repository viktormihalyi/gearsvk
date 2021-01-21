#include "GoogleTestEnvironment.hpp"

#include "DrawRecordableInfo.hpp"
#include "Font.hpp"
#include "ImageData.hpp"
#include "RenderGraph.hpp"
#include "UniformReflection.hpp"
#include "VulkanEnvironment.hpp"
#include "VulkanWrapper.hpp"

#include "glmlib.hpp"

#include "GearsAPIv2.hpp"

using GearsTests = HeadlessGoogleTestEnvironment;


TEST_F (GearsTests, SimpleSequence)
{
    SetOverriddenEnvironment (*env);

    SetRenderGraphFromPyxFileSequence (PROJECT_ROOT / "src" / "UserInterface" / "Project" / "Sequences" / "4_MovingShapes" / "1_Bars" / "04_velocity400.pyx");

    WindowU window = HiddenGLFWWindow::Create ();

    Ptr<Presentable> pres = Presentable::Create (*env, *window, DefaultSwapchainSettingsSingleImage ());

    SetCurrentPresentable (pres);

    RenderFrame (240);
    RenderFrame (241);
    RenderFrame (242);
    RenderFrame (243);

    std::vector<InheritedImageU> imgs = pres->GetSwapchain ().GetImageObjects ();

    ImageData img (GetDeviceExtra (), *imgs[0], 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    CompareImages ("Sequence01", img);

    DestroyEnvironment ();
}