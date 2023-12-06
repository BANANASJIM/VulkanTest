#include "renderer.h"
#include <stdexcept>
#include <array>

namespace vt
{
    

    VtRenderer::VtRenderer(VtWindow &window, VtDevice &device, SimpleRenderSystem &_rendersystem) : vtWindow{window},vtDevice{device},renderSystem{_rendersystem}
    {
        recreateSwapChain();
        createCommandBuffers();
        createAttachments();
        buildPasses();
        createFramebuffer();
        createSampler();

    }

    VtRenderer::~VtRenderer()
    {
        freeCommandBuffers();
    }

    void VtRenderer::recreateSwapChain()
    {
        auto extent = vtWindow.getExtent();
        while (extent.width == 0 || extent.height == 0)
        {
            extent = vtWindow.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(vtDevice.device());
        if (vtSwapChain == nullptr)
        {
            vtSwapChain = std::make_unique<VtSwapChain>(vtDevice, extent);
        }
        else
        {
            std::shared_ptr<VtSwapChain> oldSwapChain = std::move(vtSwapChain);
            vtSwapChain = std::make_unique<VtSwapChain>(vtDevice, extent, oldSwapChain);

            if(!oldSwapChain ->compareSwapFormats(*vtSwapChain.get()))
            {
                throw std::runtime_error("Swap chain image(or depth) format has changed!");
            }

        }
    }

    void VtRenderer::createCommandBuffers()
    {
        commandBuffers.resize(VtSwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = vtDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(vtDevice.device(), &allocInfo, commandBuffers.data()) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocated command buffers!");
        }
    }

    void VtRenderer::freeCommandBuffers()
    {
        vkFreeCommandBuffers(vtDevice.device(), vtDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()),
                             commandBuffers.data());
        commandBuffers.clear();
    }
    
    
    void VtRenderer::drawFrame()
    {
        beginFrame();
        VkSubmitInfo submitInfo1 = submitInfo();
        		// Wait for swap chain presentation to finish
		submitInfo1.pWaitSemaphores = &vtSwapChain->semaphores.presentComplete;
		// Signal ready with offscreen semaphore
		submitInfo1.pSignalSemaphores = &offscreenSemaphore;

		// Submit work
		submitInfo1.commandBufferCount = 1;
		submitInfo1.pCommandBuffers = &DefferedComandBuffer;
		vkQueueSubmit(vtDevice.graphicsQueue(), 1, &submitInfo1, VK_NULL_HANDLE);

		// Scene rendering

		// Wait for offscreen semaphore
		submitInfo1.pWaitSemaphores = &offscreenSemaphore;
		// Signal ready with render complete semaphore
		submitInfo1.pSignalSemaphores = &vtSwapChain->semaphores.renderComplete;

		// Submit work
		submitInfo1.pCommandBuffers = &commandBuffers[currentFrameIndex];
		vkQueueSubmit(vtDevice.graphicsQueue(), 1, &submitInfo1, VK_NULL_HANDLE);

        endFrame();
    }


    void VtRenderer::beginFrame()
    {
        assert(!isFrameStarted && "Can't call beginFrame while already in progress");

        auto result = vtSwapChain->acquireNextImage(&currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return ;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        // isFrameStarted = true;

        // auto commandBuffer = getCurrentCommandBuffer();
        // VkCommandBufferBeginInfo beginInfo{};
        // beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        // if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        // {
        //     throw std::runtime_error("failed to begin recording command buffer!");
        // }
        // return commandBuffer;
    }

    void VtRenderer::endFrame()
    {
        // assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
        // auto commandBuffer = getCurrentCommandBuffer();
        // if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        // {
        //     throw std::runtime_error("failed to record command buffer!");
        // }
        auto result = vtSwapChain->submitCommandBuffers(&currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
            vtWindow.wasWindowResized())
        {
            vtWindow.resetWindowResizedFlag();
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1)% VtSwapChain::MAX_FRAMES_IN_FLIGHT;

        vkQueueWaitIdle(vtDevice.graphicsQueue());
        vkDeviceWaitIdle(vtDevice.device());
    }

    void VtRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vtSwapChain->getRenderPass();
        renderPassInfo.framebuffer = vtSwapChain->getFrameBuffer(currentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = vtSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<uint32_t>(vtSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<uint32_t>(vtSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, vtSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void VtRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");
        vkCmdEndRenderPass(commandBuffer);
    }
    
    void VtRenderer::buildPasses()
    {
        mrtPass	 = std::make_unique<MRTShaderPass>(vtDevice,&offScreenFrameBuf,"shaders/mrt.vert.spv","shaders/mrt.frag.spv");
		compositePass = std::make_unique<CompositeShaderPass>(vtDevice,&offScreenFrameBuf,"shaders/deferred.vert.spv","shaders/deferred.frag.spv",vtSwapChain->getRenderPass());
    }
    
    void VtRenderer::createSampler()
    {
        VkSamplerCreateInfo sampler{};
        sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler.maxAnisotropy = 1.0f;
        sampler.magFilter = VK_FILTER_NEAREST;
        sampler.minFilter = VK_FILTER_NEAREST;
        sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler.addressModeV = sampler.addressModeU;
        sampler.addressModeW = sampler.addressModeU;
        sampler.mipLodBias = 0.0f;
        sampler.maxAnisotropy = 1.0f;
        sampler.minLod = 0.0f;
        sampler.maxLod = 1.0f;
        sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        vkCreateSampler(vtDevice.device(), &sampler, nullptr, &offScreenFrameBuf.colorSampler);
    }
    
    void VtRenderer::createFramebuffer()
    {
        std::array<VkImageView, 4> attachments;
        attachments[0] = offScreenFrameBuf.position.view;
        attachments[1] = offScreenFrameBuf.normal.view;
        attachments[2] = offScreenFrameBuf.albedo.view;
        attachments[3] = offScreenFrameBuf.depth.view;

        VkFramebufferCreateInfo fbufCreateInfo{};
        fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbufCreateInfo.pNext = NULL;
        fbufCreateInfo.renderPass = mrtPass->getRenderPass();
        fbufCreateInfo.pAttachments = attachments.data();
        fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        fbufCreateInfo.width = vtSwapChain->getSwapChainExtent().width;
        fbufCreateInfo.height = vtSwapChain->getSwapChainExtent().height;
        fbufCreateInfo.layers = 1;
        vkCreateFramebuffer(vtDevice.device(), &fbufCreateInfo, nullptr, &offScreenFrameBuf.frameBuffer);
    }
    
    void VtRenderer::createAttachments()
    {		
        offScreenFrameBuf.width = vtSwapChain->getSwapChainExtent().width; // todo offscreen size
        offScreenFrameBuf.height = vtSwapChain->getSwapChainExtent().height;

        createAttachment(
            VK_FORMAT_R16G16B16A16_SFLOAT,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            &offScreenFrameBuf.position);

        // (World space) Normals
        createAttachment(
            VK_FORMAT_R16G16B16A16_SFLOAT,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            &offScreenFrameBuf.normal);

        // Albedo (color)
        createAttachment(
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            &offScreenFrameBuf.albedo);

        // Depth attachment
        createAttachment(
            vtDevice.findDepthFormat(),
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            &offScreenFrameBuf.depth);
    }
    
    // void VtRenderer::prepareUniformBuffers()
    // {
    //     uniformBuffers.offscreen = std::make_unique<VtBuffer>(
    //         vtDevice,
    //         sizeof(uboOffscreenVS),
    //         1,
    //         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    //         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    //     uniformBuffers.composition = std::make_unique<VtBuffer>(
    //         vtDevice,
    //         sizeof(uboOffscreenVS),
    //         1,
    //         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    //         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    //     // Map persistent
    //     uniformBuffers.offscreen->map();
    //     uniformBuffers.composition->map();
    // }
    
    void VtRenderer::setupDescriptorSet(VtDescriptorPool &descriptorPool)
    {
        std::vector<VkWriteDescriptorSet> writeDescriptorSets;

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = descriptorPool.getDescriptorPool();
		descriptorSetAllocateInfo.pSetLayouts = &compositePass->getSetLayout();
		descriptorSetAllocateInfo.descriptorSetCount = 1;

		// Image descriptors for the offscreen color attachments
		VkDescriptorImageInfo texDescriptorPosition{};
		texDescriptorPosition.sampler = offScreenFrameBuf.colorSampler;
		texDescriptorPosition.imageView = offScreenFrameBuf.position.view;
		texDescriptorPosition.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo texDescriptorNormal{};
		texDescriptorNormal.sampler = offScreenFrameBuf.colorSampler;
		texDescriptorNormal.imageView = offScreenFrameBuf.normal.view;
		texDescriptorNormal.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo texDescriptorAlbedo{};
		texDescriptorAlbedo.sampler = offScreenFrameBuf.colorSampler;
		texDescriptorAlbedo.imageView = offScreenFrameBuf.albedo.view;
		texDescriptorAlbedo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// Deferred composition
		vkAllocateDescriptorSets(vtDevice.device(), &descriptorSetAllocateInfo, &MRTdescriptorSet);
		
		writeDescriptorSets = {
			// Binding 1 : Position texture target
			writeDescriptorSet(MRTdescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &texDescriptorPosition),
			// Binding 2 : Normals texture target
			writeDescriptorSet(MRTdescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &texDescriptorNormal),
			// Binding 3 : Albedo texture target
			writeDescriptorSet(MRTdescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &texDescriptorAlbedo),
			// Binding 4 : Fragment shader uniform buffer
			writeDescriptorSet(MRTdescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, &renderSystem.uniformBuffers.composition->descriptorInfoForIndex(0)),
		};
		vkUpdateDescriptorSets(vtDevice.device(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

    }
    
    void VtRenderer::buildCommandBuffers()
    {
        
		VkCommandBufferBeginInfo cmdBufInfo = commandBufferBeginInfo();

		VkClearValue clearValues[2];
		clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo1 = renderPassBeginInfo();
		renderPassBeginInfo1.renderPass = vtSwapChain->getRenderPass();
		renderPassBeginInfo1.renderArea.offset.x = 0;
		renderPassBeginInfo1.renderArea.offset.y = 0;
		renderPassBeginInfo1.renderArea.extent.width = vtSwapChain->getSwapChainExtent().width;
		renderPassBeginInfo1.renderArea.extent.height = vtSwapChain->getSwapChainExtent().height;
		renderPassBeginInfo1.clearValueCount = 2;
		renderPassBeginInfo1.pClearValues = clearValues;

        for (int i = 0; i < commandBuffers.size(); i++)
        {
			
			renderPassBeginInfo1.framebuffer = vtSwapChain->getFrameBuffer(i);

			vkBeginCommandBuffer(commandBuffers[i], &cmdBufInfo);

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo1, VK_SUBPASS_CONTENTS_INLINE);
            float width,height;
            width = vtSwapChain->getSwapChainExtent().width;
            height = vtSwapChain->getSwapChainExtent().height;
			VkViewport viewport1 = viewport(width, height, 0.0f, 1.0f);
			vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport1);

			VkRect2D scissor = rect2D(width, height, 0, 0);
			vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);


			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, compositePass->getPipelineLayout(), 0, 1, &MRTdescriptorSet, 0, nullptr);

   			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, compositePass->getPipeline());
			// Final composition as full screen quad
			// Note: Also used for debug display if debugDisplayTarget > 0
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

			//drawUI(drawCmdBuffers[i]);

			vkCmdEndRenderPass(commandBuffers[i]);

			vkEndCommandBuffer(commandBuffers[i]);
		}
    }
    
    void VtRenderer::beginBuildDeferredCommandBuffer()
    {
		VkCommandBufferAllocateInfo cmdBufAllocateInfo = commandBufferAllocateInfo(vtDevice.getCommandPool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		vkAllocateCommandBuffers(vtDevice.device(), &cmdBufAllocateInfo, &DefferedComandBuffer);
        
        // Create a semaphore used to synchronize offscreen rendering and usage
        VkSemaphoreCreateInfo semaphoreCI = semaphoreCreateInfo();
        vkCreateSemaphore(vtDevice.device(), &semaphoreCI, nullptr, &offscreenSemaphore);
        VkCommandBufferBeginInfo cmdBufInfo = commandBufferBeginInfo();

        // Clear values for all attachments written in the fragment shader
        std::array<VkClearValue, 4> clearValues;
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 0.0f}};
        clearValues[1].color = {{0.0f, 0.0f, 0.0f, 0.0f}};
        clearValues[2].color = {{0.0f, 0.0f, 0.0f, 0.0f}};
        clearValues[3].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo renderPassBeginInfo1 = renderPassBeginInfo();
        renderPassBeginInfo1.renderPass = mrtPass->getRenderPass();
        renderPassBeginInfo1.framebuffer = offScreenFrameBuf.frameBuffer;
        renderPassBeginInfo1.renderArea.extent.width = offScreenFrameBuf.width;
        renderPassBeginInfo1.renderArea.extent.height = offScreenFrameBuf.height;
        renderPassBeginInfo1.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassBeginInfo1.pClearValues = clearValues.data();

        vkBeginCommandBuffer(DefferedComandBuffer, &cmdBufInfo);

        vkCmdBeginRenderPass(DefferedComandBuffer, &renderPassBeginInfo1, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewportinfo = viewport((float)offScreenFrameBuf.width, (float)offScreenFrameBuf.height, 0.0f, 1.0f);
        vkCmdSetViewport(DefferedComandBuffer, 0, 1, &viewportinfo);

        VkRect2D scissor = rect2D(offScreenFrameBuf.width, offScreenFrameBuf.height, 0, 0);
        vkCmdSetScissor(DefferedComandBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(DefferedComandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mrtPass->getPipeline());

        // vkCmdBindDescriptorSets(*DefferedComandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mrtPass->getPipelineLayout(), 0, 1, &off.model, 0, nullptr);
        // models.model.bindBuffers(offScreenCmdBuffer);
        // vkCmdDrawIndexed(*DefferedComandBuffer, models.model.indices.count, 3, 0, 0, 0);
        
        // vkCmdEndRenderPass(*DefferedComandBuffer);

        // vkEndCommandBuffer(*DefferedComandBuffer);
    }
    
    void VtRenderer::endBuildDeferredCommandBuffer()
    {
        vkCmdEndRenderPass(DefferedComandBuffer);
        vkEndCommandBuffer(DefferedComandBuffer);
    }
    
    
    void VtRenderer::createAttachment(VkFormat format, VkImageUsageFlagBits usage, FrameBufferAttachment *attachment)
    {
       
		VkImageAspectFlags aspectMask = 0;
		VkImageLayout imageLayout;

		attachment->format = format;

		if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
		{
			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (format >= VK_FORMAT_D16_UNORM_S8_UINT)
				aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		assert(aspectMask > 0);
		VkExtent2D swapChainExtent = vtSwapChain->getSwapChainExtent();

		VkImageCreateInfo image{};
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = format;
		image.extent.width = swapChainExtent.width;
		image.extent.height = swapChainExtent.height;
		image.extent.depth = 1;
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;

		vtDevice.createImageWithInfo(image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, attachment->image, attachment->mem);

		VkImageViewCreateInfo imageView{};
		imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageView.format = format;
		imageView.subresourceRange = {};
		imageView.subresourceRange.aspectMask = aspectMask;
		imageView.subresourceRange.baseMipLevel = 0;
		imageView.subresourceRange.levelCount = 1;
		imageView.subresourceRange.baseArrayLayer = 0;
		imageView.subresourceRange.layerCount = 1;
		imageView.image = attachment->image;

		vkCreateImageView(vtDevice.device(), &imageView, nullptr, &attachment->view);
    }

}