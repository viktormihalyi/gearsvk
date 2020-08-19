#include "Pipeline.hpp"

Pipeline::Pipeline (VkDevice                                              device,
                    uint32_t                                              width,
                    uint32_t                                              height,
                    uint32_t                                              attachmentCount,
                    VkPipelineLayout                                      pipelineLayout,
                    VkRenderPass                                          renderPass,
                    const std::vector<VkPipelineShaderStageCreateInfo>&   shaderStages,
                    const std::vector<VkVertexInputBindingDescription>&   vertexBindingDescriptions,
                    const std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions,
                    VkPrimitiveTopology                                   topology)
    : device (device)
{
    VkPipelineVertexInputStateCreateInfo vertexInputInfo     = {};
    vertexInputInfo.sType                                    = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount            = static_cast<uint32_t> (vertexBindingDescriptions.size ());
    vertexInputInfo.pVertexBindingDescriptions               = vertexBindingDescriptions.data ();
    vertexInputInfo.vertexAttributeDescriptionCount          = static_cast<uint32_t> (vertexAttributeDescriptions.size ());
    vertexInputInfo.pVertexAttributeDescriptions             = vertexAttributeDescriptions.data ();
    VkPipelineInputAssemblyStateCreateInfo inputAssembly     = {};
    inputAssembly.sType                                      = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology                                   = topology; // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable                     = VK_FALSE;
    VkViewport viewport                                      = {};
    viewport.x                                               = 0.0f;
    viewport.y                                               = 0.0f;
    viewport.width                                           = static_cast<float> (width);
    viewport.height                                          = static_cast<float> (height);
    viewport.minDepth                                        = 0.0f;
    viewport.maxDepth                                        = 1.0f;
    VkRect2D scissor                                         = {};
    scissor.offset                                           = {0, 0};
    scissor.extent                                           = {width, height};
    VkPipelineViewportStateCreateInfo viewportState          = {};
    viewportState.sType                                      = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount                              = 1;
    viewportState.pViewports                                 = &viewport;
    viewportState.scissorCount                               = 1;
    viewportState.pScissors                                  = &scissor;
    VkPipelineRasterizationStateCreateInfo rasterizer        = {};
    rasterizer.sType                                         = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable                              = VK_FALSE;
    rasterizer.rasterizerDiscardEnable                       = VK_FALSE;
    rasterizer.polygonMode                                   = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth                                     = 1.0f;
    rasterizer.cullMode                                      = VK_CULL_MODE_NONE;
    rasterizer.frontFace                                     = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable                               = VK_FALSE;
    rasterizer.depthBiasConstantFactor                       = 0.0f; // Optional
    rasterizer.depthBiasClamp                                = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor                          = 0.0f; // Optional
    VkPipelineMultisampleStateCreateInfo multisampling       = {};
    multisampling.sType                                      = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable                        = VK_FALSE;
    multisampling.rasterizationSamples                       = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading                           = 1.0f;     // Optional
    multisampling.pSampleMask                                = nullptr;  // Optional
    multisampling.alphaToCoverageEnable                      = VK_FALSE; // Optional
    multisampling.alphaToOneEnable                           = VK_FALSE; // Optional
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask                      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable                         = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor                 = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor                 = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp                        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor                 = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor                 = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp                        = VK_BLEND_OP_ADD;
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments (attachmentCount, colorBlendAttachment);
    VkPipelineColorBlendStateCreateInfo              colorBlending = {};
    colorBlending.sType                                            = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable                                    = VK_FALSE;
    colorBlending.logicOp                                          = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount                                  = attachmentCount;
    colorBlending.pAttachments                                     = colorBlendAttachments.data ();
    colorBlending.blendConstants[0]                                = 0.0f; // Optional
    colorBlending.blendConstants[1]                                = 0.0f; // Optional
    colorBlending.blendConstants[2]                                = 0.0f; // Optional
    colorBlending.blendConstants[3]                                = 0.0f; // Optional
    VkPipelineDynamicStateCreateInfo dynamicState                  = {};
    dynamicState.sType                                             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount                                 = 0;
    dynamicState.pDynamicStates                                    = nullptr;


    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount                   = static_cast<uint32_t> (shaderStages.size ());
    pipelineInfo.pStages                      = shaderStages.data ();
    pipelineInfo.pVertexInputState            = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState          = &inputAssembly;
    pipelineInfo.pViewportState               = &viewportState;
    pipelineInfo.pRasterizationState          = &rasterizer;
    pipelineInfo.pMultisampleState            = &multisampling;
    pipelineInfo.pDepthStencilState           = nullptr; // Optional
    pipelineInfo.pColorBlendState             = &colorBlending;
    pipelineInfo.pDynamicState                = nullptr; // Optional
    pipelineInfo.layout                       = pipelineLayout;
    pipelineInfo.renderPass                   = renderPass;
    pipelineInfo.subpass                      = 0;
    pipelineInfo.basePipelineHandle           = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex            = -1;             // Optional

    if (GVK_ERROR (vkCreateGraphicsPipelines (device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &handle) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create pipeline");
    }
}
