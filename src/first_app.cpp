#include "first_app.h"

#include "camera.h"
#include "simple_render_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
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
		camera.setViewDirection(glm::vec3{0.f},glm::vec3(0.5f,0.f,1.f));
		while (!vtWindow.shouldClose())
		{
			glfwPollEvents();

			float aspect = vtRenderer.getAspectRatio();
			//camera.setOrthographicProjection(-aspect, aspect, -1.f, 1.f, -1.f, 1.f);
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f,100.f);
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
		std::vector<VtModel::Vertex> vertices{

			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

		};
		for (auto &v : vertices)
		{
			v.position += offset;
		}
		return std::make_unique<VtModel>(device, vertices);
	}

	void FirstApp::loadGameObjects()
	{
		std::shared_ptr<VtModel> vtModel = createCubeModel(vtDevice, {.0f, 0.f, 0.f});
		auto cube = VtGameObject::createGameObject();
		cube.model = vtModel;
		cube.transform.translation = {.0f, .0f, 2.5f};
		cube.transform.scale = {.5f, .5f, .5f};
		gameObjects.push_back(std::move(cube));
	}

}