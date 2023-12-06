#include "material_system.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>

namespace vt{

bool load_shader_module(VkDevice device,const char* filePath, ShaderModule* outShaderModule)
{

	//open the file. With cursor at the end
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
        assert(false && "Cant Open shader file");
		return false;
	}

	//find what the size of the file is by looking up the location of the cursor
	//because the cursor is at the end, it gives the size directly in bytes
	size_t fileSize = (size_t)file.tellg();

	//spirv expects the buffer to be on uint32, so make sure to reserve a int vector big enough for the entire file
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	//put file cursor at beggining
	file.seekg(0);

	//load the entire file into the buffer
	file.read((char*)buffer.data(), fileSize);

	//now that the file is loaded into the buffer, we can close it
	file.close();

	//create a new shader module, using the buffer we loaded
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;

	//codeSize has to be in bytes, so multply the ints in the buffer by size of int to know the real size of the buffer
	createInfo.codeSize = buffer.size() * sizeof(uint32_t);
	createInfo.pCode = buffer.data();

	//check that the creation goes well.
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		return false;
	}

	outShaderModule->code = std::move(buffer);
	outShaderModule->module = shaderModule;
	return true;	
}

// FNV-1a 32bit hashing algorithm.
constexpr uint32_t fnv1a_32(char const* s, std::size_t count)
{
	return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
}

uint32_t hash_descriptor_layout_info(VkDescriptorSetLayoutCreateInfo* info)
{
	//we are going to put all the data into a string and then hash the string
	std::stringstream ss;

	ss << info->flags;
	ss << info->bindingCount;

	for (auto i = 0u; i < info->bindingCount; i++) {
		const VkDescriptorSetLayoutBinding &binding = info->pBindings[i];

		ss << binding.binding;
		ss << binding.descriptorCount;
		ss << binding.descriptorType;
		ss << binding.stageFlags;
	}

	auto str = ss.str();

	return fnv1a_32(str.c_str(),str.length());
}

void ShaderEffect::add_stage(ShaderModule* shaderModule, VkShaderStageFlagBits stage)
{
	ShaderStage newStage = { shaderModule,stage };
	stages.push_back(newStage);
}

struct DescriptorSetLayoutData {
	uint32_t set_number;
	VkDescriptorSetLayoutCreateInfo create_info;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
};
void ShaderEffect::reflect_layout(VkDevice device, ReflectionOverrides* overrides, int overrideCount)
{
	std::vector<DescriptorSetLayoutData> set_layouts;

	std::vector<VkPushConstantRange> constant_ranges;

	for (auto& s : stages) {	

		SpvReflectShaderModule spvmodule;
		SpvReflectResult result = spvReflectCreateShaderModule(s.shaderModule->code.size() * sizeof(uint32_t), s.shaderModule->code.data(), &spvmodule);
	
		uint32_t count = 0;
		result = spvReflectEnumerateDescriptorSets(&spvmodule, &count, NULL);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectDescriptorSet*> sets(count);
		result = spvReflectEnumerateDescriptorSets(&spvmodule, &count, sets.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);	

		for (size_t i_set = 0; i_set < sets.size(); ++i_set) {
			
			const SpvReflectDescriptorSet& refl_set = *(sets[i_set]);

			DescriptorSetLayoutData layout = {};

			layout.bindings.resize(refl_set.binding_count);
			for (uint32_t i_binding = 0; i_binding < refl_set.binding_count; ++i_binding) {
				const SpvReflectDescriptorBinding& refl_binding = *(refl_set.bindings[i_binding]);
				VkDescriptorSetLayoutBinding& layout_binding = layout.bindings[i_binding];
				layout_binding.binding = refl_binding.binding;
				layout_binding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);

				for (int ov = 0; ov < overrideCount; ov++)
				{
					if (strcmp(refl_binding.name, overrides[ov].name) == 0) {
						layout_binding.descriptorType = overrides[ov].overridenType;
					}
				}

				layout_binding.descriptorCount = 1;
				for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
					layout_binding.descriptorCount *= refl_binding.array.dims[i_dim];
				}
				layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(spvmodule.shader_stage);

				ReflectedBinding reflected;
				reflected.binding = layout_binding.binding;
				reflected.set = refl_set.set;
				reflected.type = layout_binding.descriptorType;

				bindings[refl_binding.name] = reflected;
			}
			layout.set_number = refl_set.set;
			layout.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layout.create_info.bindingCount = refl_set.binding_count;
			layout.create_info.pBindings = layout.bindings.data();

			set_layouts.push_back(layout);
		}

		//pushconstants	

		result = spvReflectEnumeratePushConstantBlocks(&spvmodule, &count, NULL);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectBlockVariable*> pconstants(count);
		result = spvReflectEnumeratePushConstantBlocks(&spvmodule, &count, pconstants.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		if (count > 0) {
			VkPushConstantRange pcs{};
			pcs.offset = pconstants[0]->offset;
			pcs.size = pconstants[0]->size;
			pcs.stageFlags = s.stage;

			constant_ranges.push_back(pcs);
        }
	}


	

	std::array<DescriptorSetLayoutData,1> merged_layouts;
	
	for (int i = 0; i < 1; i++) {

		DescriptorSetLayoutData &ly = merged_layouts[i];

		ly.set_number = i;

		ly.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

		std::unordered_map<int,VkDescriptorSetLayoutBinding> binds;
		for (auto& s : set_layouts) {
			if (s.set_number == i) {
				for (auto& b : s.bindings)
				{
					auto it = binds.find(b.binding);
					if (it == binds.end())
					{
						binds[b.binding] = b;
						//ly.bindings.push_back(b);
					}
					else {
						//merge flags
						binds[b.binding].stageFlags |= b.stageFlags;
					}
					
				}
			}
		}
		for (auto [k, v] : binds)
		{
			ly.bindings.push_back(v);
		}
		//sort the bindings, for hash purposes
		std::sort(ly.bindings.begin(), ly.bindings.end(), [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b) {			
			return a.binding < b.binding;
		});


		ly.create_info.bindingCount = (uint32_t)ly.bindings.size();
		ly.create_info.pBindings = ly.bindings.data();
		ly.create_info.flags = 0;
		ly.create_info.pNext = 0;
		

		if (ly.create_info.bindingCount > 0) {
			setHashes[i] = hash_descriptor_layout_info(&ly.create_info);
			vkCreateDescriptorSetLayout(device, &ly.create_info, nullptr, &setLayouts[i]);
		}
		else {
			setHashes[i] = 0;
			setLayouts[i] = VK_NULL_HANDLE;
		}
	}

	//we start from just the default empty pipeline layout info
	VkPipelineLayoutCreateInfo mesh_pipeline_layout_info = PipelineHelper::pipeline_layout_create_info();

	mesh_pipeline_layout_info.pPushConstantRanges = constant_ranges.data();
	mesh_pipeline_layout_info.pushConstantRangeCount = (uint32_t)constant_ranges.size();

	// std::array<VkDescriptorSetLayout,4> compactedLayouts;
	// int s = 0;
	// for (int i = 0; i < 2; i++) {
	// 	if (setLayouts[i] != VK_NULL_HANDLE) {
	// 		compactedLayouts[s] = setLayouts[i];
	// 		s++;
	// 	}
	// }

	mesh_pipeline_layout_info.setLayoutCount = setLayouts.size();
	mesh_pipeline_layout_info.pSetLayouts = setLayouts.data();

	
	vkCreatePipelineLayout(device, &mesh_pipeline_layout_info, nullptr, &builtLayout);

}


void ShaderEffect::fill_stages(std::vector<VkPipelineShaderStageCreateInfo>& pipelineStages)
{
	for (auto& s : stages)
	{
		pipelineStages.push_back(PipelineHelper::pipeline_shader_stage_create_info(s.stage, s.shaderModule->module));
	}
}

}