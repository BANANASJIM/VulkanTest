#include "shader_pass.h"
#include <cassert>

namespace vt
{
    ShaderPass::ShaderPass(VtDevice& _device,std::string _vertexShaderPath,std::string _fragShaderPath)
        :device{_device}, vertexShaderPath{_vertexShaderPath}, fragShaderPath{_fragShaderPath}
    {
        initialize(vertexShaderPath,fragShaderPath);
    }

    void ShaderPass::initialize(std::string_view vertexShader, std::string_view fragmentShader)
    {
        effect.reset(build_effect(vertexShader,fragmentShader));
    }

    ShaderEffect *ShaderPass::build_effect(std::string_view vertexShader, std::string_view fragmentShader)
    {
        ShaderEffect *effect = new ShaderEffect;

        ShaderModule *vertShadermodule = new ShaderModule;
        assert(load_shader_module(device.device(), vertexShader.data(), vertShadermodule) && "Failed to load shader module.");
        ShaderModule *fragShadermodule = new ShaderModule;
        assert(load_shader_module(device.device(), fragmentShader.data(), fragShadermodule) && "Failed to load shader module.");

        effect->add_stage(vertShadermodule, VK_SHADER_STAGE_VERTEX_BIT);
        effect->add_stage(fragShadermodule, VK_SHADER_STAGE_FRAGMENT_BIT);

        effect->reflect_layout(device.device(), nullptr, 0);
        
        return effect;
    }
    
    void ShaderPass::createDependency()
    {
        assert(false&&"Implement in child class.");
    }
    

} // namespace vt