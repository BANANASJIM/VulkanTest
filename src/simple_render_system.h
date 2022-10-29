#pragma once

#include "pipeline.h"
#include "device.h"
#include "game_object.h"

#include <memory>
#include <vector>

namespace vt
{
	class SimpleRenderSystem
	{

	public:

		SimpleRenderSystem(VtDevice &device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem &) = delete;
		SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

		void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<VtGameObject> &gameObjects);
	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

		VtDevice &vtDevice;

		std::unique_ptr<VtPipeline> vtPipeline;
		VkPipelineLayout pipelineLayout;
	};
}