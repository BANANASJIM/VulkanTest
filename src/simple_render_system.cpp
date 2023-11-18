#include "simple_render_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
namespace vt
{
	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{1.f};
		glm::mat4 normalMatrix{1.f};
	};

	SimpleRenderSystem::SimpleRenderSystem(VtDevice &device, VkRenderPass renderPass,VkDescriptorSetLayout globalSetLayout)
		: vtDevice{device}
	{
		createPipelineLayout();
		createPipeline(renderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(vtDevice.device(), pipelineLayout, nullptr);
	}

	void SimpleRenderSystem::createPipelineLayout()
	{
		effect.reset(build_effect(vtDevice.device(),"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv"));

	}

	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
	{
		pass.reset(buildShaderPass(renderPass,effect.get()));
		
		// assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		// PipelineConfigInfo pipelineConfig{};
		// VtPipeline::defaultPipelineConfigInfo(pipelineConfig);
		// pipelineConfig.renderPass = renderPass;
		// pipelineConfig.pipelineLayout = pipelineLayout;
		// vtPipeline = std::make_unique<VtPipeline>(
		// 	vtDevice,
		// 	"shaders/simple_shader.vert.spv",
		// 	"shaders/simple_shader.frag.spv",
		// 	pipelineConfig);
	}
	
	ShaderPass* SimpleRenderSystem::buildShaderPass(VkRenderPass renderPass, ShaderEffect* effect)
	{
	ShaderPass* pass = new ShaderPass();
	pass->effect = effect;
	pass->layout = effect->builtLayout;
	pipelineLayout = effect->builtLayout;
	assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
	PipelineConfigInfo pipelineConfig{};
	VtPipeline::defaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = effect->builtLayout;
	effect->fill_stages(pipelineConfig.shaderStages);

	vtPipeline = std::make_unique<VtPipeline>(
			vtDevice,
			pipelineConfig);

	pass->pipeline = vtPipeline->getGraphicPipeline();

	return pass;
	}

	void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo,
											   std::vector<VtGameObject> &gameObjects)
	{
		vtPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&frameInfo.globalDescriptorSet,
			0,
			nullptr
		);

		for (auto &obj : gameObjects)
		{
			SimplePushConstantData push{};
			push.modelMatrix = obj.transform.mat4();
			push.normalMatrix = obj.transform.normalMatrix();


			vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);
			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
	}

}