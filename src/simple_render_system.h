#pragma once
#include "descriptors.h"
#include "camera.h"
#include "pipeline.h"
#include "device.h"
#include "game_object.h"
#include "frame_info.h"
//#include "material_system/shader_pass.h"
//#include "renderer.h"
//#include "vulkanInitializer.h"
#include "material_system/material_system.h"
#include <memory>
#include <vector>

namespace vt
{



	class SimpleRenderSystem
	{
	
	public:
		struct Light
		{
			glm::vec4 position;
			glm::vec3 color;
			float radius;
		};

		struct OffscreenUBO
		{
			glm::mat4 projection;
			glm::mat4 model;
			glm::mat4 view;
			// glm::vec4 instancePos[3];
		} uboOffscreenVS;

		struct CompositionUBO
		{
			Light lights[6];
			glm::vec4 viewPos;
			int debugDisplayTarget = 0;
		} uboComposition;

		struct 
		{
			std::unique_ptr<VtBuffer> offscreen;//TODO move to object
			std::unique_ptr<VtBuffer> composition;	// TODO move to camera and light
		} uniformBuffers;

		SimpleRenderSystem(VtDevice &device);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem &) = delete;
		SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

		void renderGameObjects(FrameInfo &frameInfo,
							   std::vector<VtGameObject> &gameObjects,VkCommandBuffer defferredComdBuf,VkPipelineLayout layout);
		
		//TODO 将实际的渲染剥离出来
		VtDevice& getDevice() const {return vtDevice;}
		//FrameBuffer& getOffScreenFrameBuf() {return offScreenFrameBuf;}
		
		void prepareUniformBuffers();
		//From camera and light
		void updateUniformBufferOffscreen(FrameInfo &frameInfo);
		void updateUniformBufferComposition(FrameInfo &frameInfo);

		// for model 
		void setupDescriptorSet(VtDescriptorPool &descriptorPool,std::vector<VtGameObject> &gameObjects,VkDescriptorSetLayout setLayout);
		void buildCommandBuffers();
		void buildDeferredCommandBuffer(VtGameObject &gameObject);

		//void createAttachment(VkFormat format, VkImageUsageFlagBits usage, FrameBufferAttachment *attachment);
	private:
		

		VtDevice &vtDevice;
	
		std::unique_ptr<ShaderEffect> effect;
		//TODO temp
		VkDescriptorSet descriptorSet;
		VkDescriptorSet modelDescriptorSet;

	};
}