#include "first_app.h"

#include "texture.h"
#include "camera.h"
#include "keyboard_movement_controller.h"
#include "simple_render_system.h"
#include "buffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <chrono>
#include <array>
#include <numeric>

#define MAX_FRAME_TIME 1.f

namespace vt
{
	struct GlobalUbo
	{
		glm::mat4 projectionView{1.f};
		glm::vec4 ambientLightColor{1.f,1.f,1.f,0.02f};
		glm::vec3 lightPosition{-1.f};
		alignas(16) glm::vec4 lightColor{1.f};
	};

	FirstApp::FirstApp()
	{
		globalPool = VtDescriptorPool::Builder(vtDevice)
						 .setMaxSets(VtSwapChain::MAX_FRAMES_IN_FLIGHT)
						 .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VtSwapChain::MAX_FRAMES_IN_FLIGHT)
						 .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VtSwapChain::MAX_FRAMES_IN_FLIGHT)
						 .build();
		loadGameObjects();
	}

	FirstApp::~FirstApp()
	{
	}
	void FirstApp::run()
	{
		//TODO Need more abstract
		std::vector<std::unique_ptr<texture>> textures(1);
		for (int i = 0; i < textures.size(); ++i)
		{
			textures[i] = std::make_unique<texture>(vtDevice, "F:/Dev/Vulkan Tutorial/VulkanTest/textures/texture.jpg");
		}
		std::vector<std::unique_ptr<VtBuffer>> uboBuffers(VtSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++)
		{
			uboBuffers[i] = std::make_unique<VtBuffer>(
				vtDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		auto globalSetLayout = VtDescriptorSetLayout::Builder(vtDevice)
								   .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
								   .addBinding(1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT)
								   .build();

		std::vector<VkDescriptorSet> globalDescriptorSets(VtSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0 ;i < globalDescriptorSets.size(); i++)
		{
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			auto imageInfo = textures[0]->descriptorInfo();
			VtDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0,&bufferInfo)
				.writeImage(1,&imageInfo)
				.build(globalDescriptorSets[i]);
		}
		SimpleRenderSystem simpleRenderSystem{vtDevice, vtRenderer.getSwapChainRenderPass(),globalSetLayout->getDescriptorSetLayout()};
		VtCamera camera{};

		auto viewerObject = VtGameObject::createGameObject();
		viewerObject.transform.translation.z = -2.5f;
		KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();
		while (!vtWindow.shouldClose())
		{
			glfwPollEvents();
			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			frameTime = glm::min(frameTime, MAX_FRAME_TIME);

			cameraController.moveInPlaneXZ(vtWindow.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			float aspect = vtRenderer.getAspectRatio();
			// camera.setOrthographicProjection(-aspect, aspect, -1.f, 1.f, -1.f, 1.f);
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
			if (auto commandBuffer = vtRenderer.beginFrame())
			{
				int frameIndex = vtRenderer.getFrameIndex();
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex]};

				// update
				GlobalUbo ubo{};
				ubo.projectionView = camera.getProjection() * camera.getView();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();
				// render
				vtRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
				vtRenderer.endSwapChainRenderPass(commandBuffer);
				vtRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(vtDevice.device());
	}


	void FirstApp::loadGameObjects()
	{
		std::shared_ptr<VtModel> vtModel = VtModel::createModelFromFile(vtDevice, "models/flat_vase.obj");
		auto gameObj = VtGameObject::createGameObject();
		gameObj.model = vtModel;
		gameObj.transform.translation = {.5f, 0.5f, 0.f};
		gameObj.transform.scale = glm::vec3{2.f};
		gameObjects.push_back(std::move(gameObj));
	}

}