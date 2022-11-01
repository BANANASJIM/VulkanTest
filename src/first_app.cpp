#include "first_app.h"

#include "camera.h"
#include "keyboard_movement_controller.h"
#include "simple_render_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <chrono>
#include <array>

#define MAX_FRAME_TIME 1.f

namespace vt
{

	FirstApp::FirstApp()
	{
		loadGameObjects();
	}

	FirstApp::~FirstApp()
	{
	}
	void FirstApp::run()
	{
		SimpleRenderSystem simpleRenderSystem{vtDevice, vtRenderer.getSwapChainRenderPass()};
		VtCamera camera{};

		auto viewerObject = VtGameObject::createGameObject();
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
				vtRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
				vtRenderer.endSwapChainRenderPass(commandBuffer);
				vtRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(vtDevice.device());
	}

	std::unique_ptr<VtModel> createCubeModel(VtDevice &device, glm::vec3 offset)
	{
		VtModel::Builder modelBuilder{};
		modelBuilder.vertices = {
			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
		};
		for (auto &v : modelBuilder.vertices)
		{
			v.position += offset;
		}

		modelBuilder.indices = {0, 1, 2, 0, 3, 1, 4, 5, 6, 4, 7, 5, 8, 9, 10, 8, 11, 9,
								12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};

		return std::make_unique<VtModel>(device, modelBuilder);
	}

	void FirstApp::loadGameObjects()
	{
		std::shared_ptr<VtModel> vtModel = VtModel::createModelFromFile(vtDevice, "models/smooth_vase.obj");
		auto gameObj = VtGameObject::createGameObject();
		gameObj.model = vtModel;
		gameObj.transform.translation = {.0f, .0f, 2.5f};
		gameObj.transform.scale = glm::vec3{3.f};
		gameObjects.push_back(std::move(gameObj));
	}

}