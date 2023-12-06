#pragma once

#include "material_system/shader_pass.h"

namespace vt
{
class MRTShaderPass : public ShaderPass
{
    
public:
    MRTShaderPass(VtDevice& _device,FrameBuffer* _framebuffer,std::string _vertexShaderPath,std::string _fragShaderPath);

    FrameBuffer* offScreenFrameBuf = nullptr;
    std::array<VkSubpassDependency, 2> dependencies;

    virtual void createPipeline() override;
    virtual void createFramebuffer() override;
    virtual void createRenderPass() override;
    virtual void createDependency() override;

    void build();
};

}
