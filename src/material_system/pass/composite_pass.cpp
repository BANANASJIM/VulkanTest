#include "composite_pass.h"
#include <cassert>
#include <memory>

namespace vt
{
    CompositeShaderPass::CompositeShaderPass(VtDevice& _device,FrameBuffer* _osframebuffer,std::string _vertexShaderPath,std::string _fragShaderPath,VkRenderPass _renderPass)
    :ShaderPass( _device,_vertexShaderPath, _fragShaderPath),offScreenFrameBuf{_osframebuffer}
    {        
        renderPass = _renderPass;
        createPipeline();

    }
    
    
    void CompositeShaderPass::createDependency()
    {

    }
    
    void CompositeShaderPass::createPipeline()
    {
        PipelineConfigInfo pipelineConfig{};
        VtPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = effect->builtLayout;
        VkPipelineVertexInputStateCreateInfo emptyInputState{};
        emptyInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        pipelineConfig.vertexInputInfo = emptyInputState;

        effect->fill_stages(pipelineConfig.shaderStages);
        vtPipeline = std::make_unique<VtPipeline>(
            device,
            pipelineConfig);
    }

}