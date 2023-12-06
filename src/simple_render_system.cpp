#include "simple_render_system.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "material_system/pass/mrt_pass.h"
#include "material_system/pass/composite_pass.h"
#include <stdexcept>
#include <array>
namespace vt
{

	//TODO remove
	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{1.f};
		glm::mat4 normalMatrix{1.f};
	};

	SimpleRenderSystem::SimpleRenderSystem(VtDevice &device)
		: vtDevice{device}
	{
		prepareUniformBuffers();
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		//vkDestroyPipelineLayout(vtDevice.device(), pipelineLayout, nullptr);
	}

	void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo,
											   std::vector<VtGameObject> &gameObjects,VkCommandBuffer defferredComdBuf,VkPipelineLayout layout)
	{
		 updateUniformBufferOffscreen(frameInfo);
		 updateUniformBufferComposition(frameInfo);
		
		

        vkCmdBindDescriptorSets(defferredComdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &modelDescriptorSet, 0, nullptr);
     

		for (auto &obj : gameObjects)
		{
			obj.model->bind(defferredComdBuf);
			obj.model->draw(defferredComdBuf); 
		}
	}
	
	//TODO need to be moved to Shaderpass
	void SimpleRenderSystem::prepareUniformBuffers()
	{
		uniformBuffers.offscreen = std::make_unique<VtBuffer>(
			vtDevice,
			sizeof(uboOffscreenVS),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		uniformBuffers.composition = std::make_unique<VtBuffer>(
			vtDevice,
			sizeof(uboComposition),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


		// Map persistent
		uniformBuffers.offscreen->map();
		uniformBuffers.composition->map();

		//  Setup instanced model positions
		// uboOffscreenVS.instancePos[0] = glm::vec4(0.0f);
		// uboOffscreenVS.instancePos[1] = glm::vec4(-4.0f, 0.0, -4.0f, 0.0f);
		// uboOffscreenVS.instancePos[2] = glm::vec4(4.0f, 0.0, -4.0f, 0.0f);
		

	}
	
	void SimpleRenderSystem::updateUniformBufferOffscreen(FrameInfo &frameInfo)
	{
		//TODO 对于每个模型和相机，更新game object中的uniform buffer
		uboOffscreenVS.projection = frameInfo.camera.getProjection();
		uboOffscreenVS.view = frameInfo.camera.getView();
		uboOffscreenVS.model = glm::mat4(1.0f);
		uniformBuffers.offscreen->writeToBuffer(&uboOffscreenVS);
	}
	
	void SimpleRenderSystem::updateUniformBufferComposition(FrameInfo &frameInfo)
	{
		
		// White
		uboComposition.lights[0].position = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
		uboComposition.lights[0].color = glm::vec3(1.5f);
		uboComposition.lights[0].radius = 15.0f * 0.25f;
		// // Red
		// uboComposition.lights[1].position = glm::vec4(-2.0f, 0.0f, 0.0f, 0.0f);
		// uboComposition.lights[1].color = glm::vec3(1.0f, 0.0f, 0.0f);
		// uboComposition.lights[1].radius = 15.0f;
		// // Blue
		// uboComposition.lights[2].position = glm::vec4(2.0f, -1.0f, 0.0f, 0.0f);
		// uboComposition.lights[2].color = glm::vec3(0.0f, 0.0f, 2.5f);
		// uboComposition.lights[2].radius = 5.0f;
		// // Yellow
		// uboComposition.lights[3].position = glm::vec4(0.0f, -0.9f, 0.5f, 0.0f);
		// uboComposition.lights[3].color = glm::vec3(1.0f, 1.0f, 0.0f);
		// uboComposition.lights[3].radius = 2.0f;
		// // Green
		// uboComposition.lights[4].position = glm::vec4(0.0f, -0.5f, 0.0f, 0.0f);
		// uboComposition.lights[4].color = glm::vec3(0.0f, 1.0f, 0.2f);
		// uboComposition.lights[4].radius = 5.0f;
		// // Yellow
		// uboComposition.lights[5].position = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
		// uboComposition.lights[5].color = glm::vec3(1.0f, 0.7f, 0.3f);
		// uboComposition.lights[5].radius = 25.0f;

		// uboComposition.lights[0].position.x = sin(glm::radians(360.0f * timer)) * 5.0f;
		// uboComposition.lights[0].position.z = cos(glm::radians(360.0f * timer)) * 5.0f;

		// uboComposition.lights[1].position.x = -4.0f + sin(glm::radians(360.0f * timer) + 45.0f) * 2.0f;
		// uboComposition.lights[1].position.z =  0.0f + cos(glm::radians(360.0f * timer) + 45.0f) * 2.0f;

		// uboComposition.lights[2].position.x = 4.0f + sin(glm::radians(360.0f * timer)) * 2.0f;
		// uboComposition.lights[2].position.z = 0.0f + cos(glm::radians(360.0f * timer)) * 2.0f;

		// uboComposition.lights[4].position.x = 0.0f + sin(glm::radians(360.0f * timer + 90.0f)) * 5.0f;
		// uboComposition.lights[4].position.z = 0.0f - cos(glm::radians(360.0f * timer + 45.0f)) * 5.0f;

		// uboComposition.lights[5].position.x = 0.0f + sin(glm::radians(-360.0f * timer + 135.0f)) * 10.0f;
		// uboComposition.lights[5].position.z = 0.0f - cos(glm::radians(-360.0f * timer - 45.0f)) * 10.0f;

		// Current view position
		uboComposition.viewPos = glm::vec4(0.0f,0.0f,0.0f, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

		uboComposition.debugDisplayTarget = 0;

		uniformBuffers.composition->writeToBuffer(&uboComposition);
	}
	
	void SimpleRenderSystem::setupDescriptorSet(VtDescriptorPool &descriptorPool,std::vector<VtGameObject> &gameObjects,VkDescriptorSetLayout setLayout)
	{

		std::vector<VkWriteDescriptorSet> writeDescriptorSets;

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = descriptorPool.getDescriptorPool();
		descriptorSetAllocateInfo.pSetLayouts = &setLayout;
		descriptorSetAllocateInfo.descriptorSetCount = 1;

		// // Image descriptors for the offscreen color attachments
		// VkDescriptorImageInfo texDescriptorPosition{};
		// texDescriptorPosition.sampler = dynamic_cast<CompositeShaderPass *>(passes[1].get())->colorSampler;
		// texDescriptorPosition.imageView = offScreenFrameBuf.position.view;
		// texDescriptorPosition.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// VkDescriptorImageInfo texDescriptorNormal{};
		// texDescriptorNormal.sampler = dynamic_cast<CompositeShaderPass *>(passes[1].get())->colorSampler;
		// texDescriptorNormal.imageView = offScreenFrameBuf.normal.view;
		// texDescriptorNormal.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// VkDescriptorImageInfo texDescriptorAlbedo{};
		// texDescriptorAlbedo.sampler = dynamic_cast<CompositeShaderPass *>(passes[1].get())->colorSampler;
		// texDescriptorAlbedo.imageView = offScreenFrameBuf.albedo.view;
		// texDescriptorAlbedo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// // Deferred composition
		//vkAllocateDescriptorSets(vtDevice.device(), &descriptorSetAllocateInfo, &descriptorSet);
		
		// writeDescriptorSets = {
		// 	// Binding 1 : Position texture target
		// 	writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &texDescriptorPosition),
		// 	// Binding 2 : Normals texture target
		// 	writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &texDescriptorNormal),
		// 	// Binding 3 : Albedo texture target
		// 	writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &texDescriptorAlbedo),
		// 	// Binding 4 : Fragment shader uniform buffer
		// 	writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, &uniformBuffers.composition->descriptor),
		// };
		// vkUpdateDescriptorSets(vtDevice.device(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

		// Offscreen (scene)

	//TODO need to move to model
		// Model
		vkAllocateDescriptorSets(vtDevice.device(), &descriptorSetAllocateInfo, &modelDescriptorSet);
		writeDescriptorSets = {
			// Binding 0: Vertex shader uniform buffer
			writeDescriptorSet(modelDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers.offscreen->descriptorInfoForIndex(0))
			// Binding 1: Color map
			//writeDescriptorSet(modelDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textures.model.colorMap->descriptor),
			// Binding 2: Normal map
			//writeDescriptorSet(modelDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &textures.model.normalMap.descriptor)
			};
		vkUpdateDescriptorSets(vtDevice.device(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	}

	
	void SimpleRenderSystem::buildCommandBuffers()
	{
	// ShaderPass* pass = new ShaderPass();
	// pass->effect = effect;
	// pass->layout = effect->builtLayout;
	// pipelineLayout = effect->builtLayout;
	// assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
	// PipelineConfigInfo pipelineConfig{};
	// VtPipeline::defaultPipelineConfigInfo(pipelineConfig);
	// pipelineConfig.renderPass = renderPass;
	// pipelineConfig.pipelineLayout = effect->builtLayout;
	// effect->fill_stages(pipelineConfig.shaderStages);

		// 	//drawUI(drawCmdBuffers[i]);

		// 	vkCmdEndRenderPass(drawCmdBuffers[i]);

		// 	vkEndCommandBuffer(drawCmdBuffers[i]);
		// }
	}
	
	void SimpleRenderSystem::buildDeferredCommandBuffer(VtGameObject &gameObject)
	{
		
        
		// if (offScreenCmdBuffer == VK_NULL_HANDLE)
		// {
		// 	offScreenCmdBuffer = vtDevice.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY,vtDevice.getCommandPool(), false);
		// }

		// // Create a semaphore used to synchronize offscreen rendering and usage
		// VkSemaphoreCreateInfo semaphoreCreateIf = semaphoreCreateInfo();
		// vkCreateSemaphore(vtDevice.device(), &semaphoreCreateIf, nullptr, &offscreenSemaphore);

		// VkCommandBufferBeginInfo cmdBufInfo = commandBufferBeginInfo();

		// // Clear values for all attachments written in the fragment shader
		// std::array<VkClearValue,4> clearValues;
		// clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		// clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		// clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		// clearValues[3].depthStencil = { 1.0f, 0 };

		// VkRenderPassBeginInfo renderPassBeginInfo1 = renderPassBeginInfo();
		// renderPassBeginInfo1.renderPass = dynamic_cast<CompositeShaderPass *>(passes[1].get())->renderPass;
		// renderPassBeginInfo1.framebuffer = offScreenFrameBuf.frameBuffer;
		// renderPassBeginInfo1.renderArea.extent.width = offScreenFrameBuf.width;
		// renderPassBeginInfo1.renderArea.extent.height = offScreenFrameBuf.height;
		// renderPassBeginInfo1.clearValueCount = static_cast<uint32_t>(clearValues.size());
		// renderPassBeginInfo1.pClearValues = clearValues.data();

		// vkBeginCommandBuffer(offScreenCmdBuffer, &cmdBufInfo);

		// vkCmdBeginRenderPass(offScreenCmdBuffer, &renderPassBeginInfo1, VK_SUBPASS_CONTENTS_INLINE);

		// VkViewport viewport1 = viewport((float)offScreenFrameBuf.width, (float)offScreenFrameBuf.height, 0.0f, 1.0f);
		// vkCmdSetViewport(offScreenCmdBuffer, 0, 1, &viewport1);

		// VkRect2D scissor = rect2D(offScreenFrameBuf.width, offScreenFrameBuf.height, 0, 0);
		// vkCmdSetScissor(offScreenCmdBuffer, 0, 1, &scissor);

		// vkCmdBindPipeline(offScreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, dynamic_cast<CompositeShaderPass *>(passes[1].get())->vtPipeline->getGraphicPipeline());

		// // Instanced object
		// vkCmdBindDescriptorSets(offScreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &modelDescriptorSet, 0, nullptr);
		// gameObject.model->bind(offScreenCmdBuffer);
		// vkCmdDrawIndexed(offScreenCmdBuffer, gameObject.model->getVertexCount(), 1, 0, 0, 0);

		// vkCmdEndRenderPass(offScreenCmdBuffer);

		// vkEndCommandBuffer(offScreenCmdBuffer);
	}
	

}