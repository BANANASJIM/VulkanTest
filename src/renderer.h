#pragma once

#include "window.h"
#include "device.h"
#include "swap_chain.h"

#include <cassert>
#include <memory>
#include <vector>

namespace vt
{
	class VtRenderer
	{

	public:
		VtRenderer(VtWindow &window, VtDevice &device);
		~VtRenderer();

		VtRenderer(const VtRenderer &) = delete;
		VtRenderer &operator=(const VtRenderer &) = delete;

		VkRenderPass getSwapChainRenderPass() const { return vtSwapChain->getRenderPass(); }
		float getAspectRatio() const {return vtSwapChain->extentAspectRatio();}
		bool isFrameInProgress() const { return isFrameStarted; }
		VkCommandBuffer getCurrentCommandBuffer() const
		{
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const{
			assert(isFrameInProgress() && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void drawFrame();
		void recreateSwapChain();

		VtWindow &vtWindow;
		VtDevice &vtDevice;
		std::unique_ptr<VtSwapChain> vtSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex{0};
		int currentFrameIndex{0};
		bool isFrameStarted{false};
	};
}