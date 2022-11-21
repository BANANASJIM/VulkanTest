#pragma once

#include "camera.h"
#include "pipeline.h"
#include "device.h"
#include "game_object.h"
#include "frame_info.h"

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
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		VtDevice &vtDevice;

		std::unique_ptr<VtPipeline> vtPipeline;
		VkPipelineLayout pipelineLayout;
	};
}