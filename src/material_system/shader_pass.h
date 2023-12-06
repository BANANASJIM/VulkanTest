#pragma once
#include "material_system.h"
#include "texture.h"
//#include "simple_render_system.h"

namespace vt
{
    class SimpleRenderSystem;

    //TODO use texture
	struct FrameBufferAttachment
	{
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
		VkFormat format;
        
        //std::unique_ptr<texture> attachment;
	};

    struct FrameBuffer
    {
        int32_t width, height;
        VkFramebuffer frameBuffer;
        FrameBufferAttachment position, normal, albedo;
        FrameBufferAttachment depth;
        VkRenderPass renderPass;
        VkSampler colorSampler;
    };

    class ShaderPass
    {
    public:

        ShaderPass(VtDevice& _device,std::string _vertexShaderPath,std::string _fragShaderPath);
        ~ShaderPass(){};
        ShaderPass(const ShaderPass &) = delete;
        ShaderPass &operator=(const ShaderPass &) = delete;

        void execute(VkCommandBuffer commandBuffer);
        virtual void initialize(std::string_view vertexShader, std::string_view fragmentShader);
        void cleanup();

        ShaderEffect* build_effect(std::string_view vertexShader, std::string_view fragmentShader);

        virtual void createPipeline() = 0;
        virtual void createFramebuffer() {};
        virtual void createRenderPass() {};
        virtual void createDependency();

        VkRenderPass& getRenderPass() {return renderPass;}

        ShaderEffect& getEffect() {return *effect;}
        VkDescriptorSetLayout& getSetLayout() {return effect->getSetLayout();}
        VkPipelineLayout& getPipelineLayout() {return effect->builtLayout;}
        VkPipeline& getPipeline() {return vtPipeline->getGraphicPipeline();}
        
    protected:
        //TODO need to initialize
        std::unique_ptr<ShaderEffect> effect;
        std::unique_ptr<VtPipeline> vtPipeline;
        //std::unique_ptr<VtDescriptorSetLayout> layout;
        VtDevice& device;
        std::string vertexShaderPath;
        std::string fragShaderPath;
        VkRenderPass renderPass;
    };
}


