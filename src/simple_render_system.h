#pragma once

#include "camera.h"
#include "pipeline.h"
#include "device.h"
#include "game_object.h"
#include "frame_info.h"
#include "material_system/material_system.h"
#include <memory>
#include <vector>

namespace vt
{
	class SimpleRenderSystem
	{

	public:
		SimpleRenderSystem(VtDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem &) = delete;
		SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

		void renderGameObjects(FrameInfo &frameInfo,
							   std::vector<VtGameObject> &gameObjects);


	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

		ShaderPass* buildShaderPass(VkRenderPass renderPass, ShaderEffect* effect);
		VtDevice &vtDevice;

		std::unique_ptr<VtPipeline> vtPipeline;
		VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
		std::unique_ptr<ShaderPass> pass;
		std::unique_ptr<ShaderEffect> effect;
	};
}