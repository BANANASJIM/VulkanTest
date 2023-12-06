#pragma once
#include "material_system/shader_pass.h"
namespace vt
{

class CompositeShaderPass : public ShaderPass
{
public:

    CompositeShaderPass(VtDevice& _device,FrameBuffer* _osframebuffer,std::string _vertexShaderPath,std::string _fragShaderPath,VkRenderPass _renderPass);
    std::array<VkSubpassDependency, 2> dependencies;
    FrameBuffer* offScreenFrameBuf = nullptr;

    virtual void createPipeline() override;
    virtual void createDependency() override;
};
}
