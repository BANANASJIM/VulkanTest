#pragma once

#include "window.h"
#include "device.h"
#include "swap_chain.h"
#include "vulkanInitializer.h"
#include "descriptors.h"
#include "material_system/pass/mrt_pass.h"
#include "material_system/pass/composite_pass.h"
#include "simple_render_system.h"
#include <cassert>
#include <memory>
#include <vector>

namespace vt
{


	//TODO 同步 和 commandbuffer 在这里记录和缓存，pass放这里
	class VtRenderer
	{

	public:
		FrameBuffer offScreenFrameBuf;

		VtRenderer(VtWindow &window, VtDevice &device, SimpleRenderSystem &_rendersystem);
		~VtRenderer();

		VtRenderer(const VtRenderer &) = delete;
		VtRenderer &operator=(const VtRenderer &) = delete;

		VkRenderPass getSwapChainRenderPass() const { return vtSwapChain->getRenderPass(); }
		VkExtent2D getWindowExtent() const { return vtSwapChain->getSwapChainExtent(); }
		VkCommandBuffer getDefferredCommandBuffer() {return DefferedComandBuffer;}
		VkPipelineLayout getMRTLayout() {return mrtPass->getPipelineLayout();}
		VkDescriptorSetLayout getMRTSetLayout(){return mrtPass->getSetLayout();}
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

		VtDevice& getDevice() const {return vtDevice;}
		void beginFrame();
		void endFrame();

		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		void buildPasses();
		void createSampler();
		void createFramebuffer();
		void createAttachments();
		//For pipeline
		void setupDescriptorSet(VtDescriptorPool &descriptorPool);
		void buildCommandBuffers();
		void beginBuildDeferredCommandBuffer();
		void endBuildDeferredCommandBuffer();
		void drawFrame();

	private:
		//TODO help function ,abstract to image in future
		void createAttachment(VkFormat format, VkImageUsageFlagBits usage, FrameBufferAttachment *attachment);
		void createCommandBuffers();
		void freeCommandBuffers();

		//TODO 

		void recreateSwapChain();

		VtWindow &vtWindow;
		VtDevice &vtDevice;
		SimpleRenderSystem &renderSystem;
		std::unique_ptr<VtSwapChain> vtSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;
		VkCommandBuffer DefferedComandBuffer;
		std::unique_ptr<MRTShaderPass> mrtPass;
		std::unique_ptr<CompositeShaderPass> compositePass;
		
		VkSemaphore offscreenSemaphore = VK_NULL_HANDLE;

		VkDescriptorSet MRTdescriptorSet;

		uint32_t currentImageIndex;
		int currentFrameIndex{0};
		bool isFrameStarted{false};
		
	};
}