#pragma once

#include <vector>
#include <array>
#include <unordered_map>
#include "model.h"
#include "pipeline.h"
#include "spirv_reflect.h"
#include <iostream>
#include <string>

#include <memory>

namespace vt {
	
struct ShaderModule {
	std::vector<uint32_t> code;
	VkShaderModule module;
};


	//loads a shader module from a spir-v file. Returns false if it errors	
	bool load_shader_module(VkDevice device, const char* filePath, ShaderModule* outShaderModule);

	uint32_t hash_descriptor_layout_info(VkDescriptorSetLayoutCreateInfo* info);





struct ShaderEffect {

	struct ReflectionOverrides {
		const char* name;
		VkDescriptorType overridenType;
	};
    ShaderEffect()= default;
	ShaderEffect(ShaderEffect&& other) noexcept = default;
	ShaderEffect& operator=(ShaderEffect&& other) noexcept = default;


	void add_stage(ShaderModule* shaderModule, VkShaderStageFlagBits stage);

	void reflect_layout( const VkDevice device, ReflectionOverrides* overrides, int overrideCount);

	void fill_stages(std::vector<VkPipelineShaderStageCreateInfo>& pipelineStages);
	VkPipelineLayout builtLayout;

	VkDescriptorSetLayout& getSetLayout() {return setLayouts[0];}

	struct ReflectedBinding {
		uint32_t set;
		uint32_t binding;
		VkDescriptorType type;
	};
	std::unordered_map<std::string, ReflectedBinding> bindings;
	std::array<VkDescriptorSetLayout, 1> setLayouts;
	std::array<uint32_t, 1> setHashes;
private:
	struct ShaderStage {
		ShaderModule* shaderModule;
		VkShaderStageFlagBits stage;
	};

	std::vector<ShaderStage> stages;
};

}
