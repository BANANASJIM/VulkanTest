#pragma once
#include "device.h"
#include <string>
#include <vector>
namespace vt
{

    struct PipelineConfigInfo
    {
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineVertexInputStateCreateInfo vertexInputInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        uint32_t subpass = 0;
        
        void ConfigVertexInputState(const std::vector<VkVertexInputBindingDescription> &vertexBindingDescriptions,
                                    const std::vector<VkVertexInputAttributeDescription> &vertexAttributeDescriptions);
    };

    struct PipelineHelper
    {
        static VkPipelineLayoutCreateInfo pipeline_layout_create_info()
        {
            VkPipelineLayoutCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            info.pNext = nullptr;

            // empty defaults
            info.flags = 0;
            info.setLayoutCount = 0;
            info.pSetLayouts = nullptr;
            info.pushConstantRangeCount = 0;
            info.pPushConstantRanges = nullptr;
            return info;
        }

        static VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule)
        {
            VkPipelineShaderStageCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            info.pNext = nullptr;

            // shader stage
            info.stage = stage;
            // module containing the code for this shader stage
            info.module = shaderModule;
            // the entry point of the shader
            info.pName = "main";
            return info;
        }

        static VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
			VkColorComponentFlags colorWriteMask,
			VkBool32 blendEnable)
		{
			VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState {};
			pipelineColorBlendAttachmentState.colorWriteMask = colorWriteMask;
			pipelineColorBlendAttachmentState.blendEnable = blendEnable;
			return pipelineColorBlendAttachmentState;
		}
    };

    class VtPipeline
    {
    public:
        VtPipeline(VtDevice &device, const PipelineConfigInfo &configInfo);
        ~VtPipeline();

        VtPipeline(const VtPipeline &) = delete;
        VtPipeline operator=(const VtPipeline &) = delete;

        void bind(VkCommandBuffer commandBuffer);

        VkPipeline& getGraphicPipeline() {return graphicPipeline;}

        static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);


    private:
        static std::vector<char> readFile(const std::string &filepath);

        void createGraphicPipeline(const PipelineConfigInfo &configInfo);

        void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

        VtDevice &vtDevice;
        VkPipeline graphicPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };
}