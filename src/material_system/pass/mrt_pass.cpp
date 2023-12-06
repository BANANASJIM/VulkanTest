#include "mrt_pass.h"

namespace vt
{
    MRTShaderPass::MRTShaderPass(VtDevice& _device,FrameBuffer* _osframebuffer,std::string _vertexShaderPath,std::string _fragShaderPath)
    :ShaderPass( _device, _vertexShaderPath, _fragShaderPath),offScreenFrameBuf{_osframebuffer}
    {
        build();
    }

    void MRTShaderPass::createPipeline()
    {
        PipelineConfigInfo pipelineConfig{};
        VtPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = effect->builtLayout;
        pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;

        // override deafault
        VkPipelineColorBlendStateCreateInfo &colorBlendState = pipelineConfig.colorBlendInfo;
        std::array<VkPipelineColorBlendAttachmentState, 3> blendAttachmentStates = {
            PipelineHelper::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
            PipelineHelper::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
            PipelineHelper::pipelineColorBlendAttachmentState(0xf, VK_FALSE)};

        colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
        colorBlendState.pAttachments = blendAttachmentStates.data();
        
        const auto bindingDescriptions = VtModel::Vertex::getBindingDescriptions();
        const auto attributeDescriptions = VtModel::Vertex::getAttributeDescriptions();
        pipelineConfig.ConfigVertexInputState(bindingDescriptions,attributeDescriptions);

        effect->fill_stages(pipelineConfig.shaderStages);
        vtPipeline = std::make_unique<VtPipeline>(
            device,
            pipelineConfig);
    }
    
    void MRTShaderPass::createFramebuffer()
    {
        std::array<VkImageView, 4> attachments;
        attachments[0] = offScreenFrameBuf->position.view;
        attachments[1] = offScreenFrameBuf->normal.view;
        attachments[2] = offScreenFrameBuf->albedo.view;
        attachments[3] = offScreenFrameBuf->depth.view;

        VkFramebufferCreateInfo fbufCreateInfo{};
        fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbufCreateInfo.pNext = NULL;
        fbufCreateInfo.renderPass = renderPass;
        fbufCreateInfo.pAttachments = attachments.data();
        fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        fbufCreateInfo.width = offScreenFrameBuf->width;
        fbufCreateInfo.height = offScreenFrameBuf->height;
        fbufCreateInfo.layers = 1;
        vkCreateFramebuffer(device.device(), &fbufCreateInfo, nullptr, &offScreenFrameBuf->frameBuffer);
    }
    void MRTShaderPass::createRenderPass()
    {

        // Set up separate renderpass with references to the color and depth attachments
        std::array<VkAttachmentDescription, 4> attachmentDescs = {};
        // Init attachment properties
        for (uint32_t i = 0; i < 4; ++i)
        {
            attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            if (i == 3)
            {
                attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
            else
            {
                attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
        }

        // Formats
        attachmentDescs[0].format = offScreenFrameBuf->position.format;
        attachmentDescs[1].format = offScreenFrameBuf->normal.format;
        attachmentDescs[2].format = offScreenFrameBuf->albedo.format;
        attachmentDescs[3].format = offScreenFrameBuf->depth.format;

        std::vector<VkAttachmentReference> colorReferences;
        colorReferences.push_back({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        colorReferences.push_back({1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        colorReferences.push_back({2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});

        VkAttachmentReference depthReference = {};
        depthReference.attachment = 3;
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.pColorAttachments = colorReferences.data();
        subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
        subpass.pDepthStencilAttachment = &depthReference;

        createDependency();

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.pAttachments = attachmentDescs.data();
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = dependencies.data();

        vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass);

        createFramebuffer();

    }
    
    void MRTShaderPass::createDependency()
    {
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }
    
    void MRTShaderPass::build()
    {
        createRenderPass();
        createPipeline();
    }

}