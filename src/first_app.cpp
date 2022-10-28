#include "first_app.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
namespace vt
{
	struct SimplePushConstantData
	{
		glm::mat2 transform{1.f};
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	FirstApp::FirstApp()
	{
		loadGameObjects();
		createPipelineLayout();
		recreateSwapChain();
		createCommandBuffers();
	}

	FirstApp::~FirstApp()
	{
		vkDestroyPipelineLayout(vtDevice.device(), pipelineLayout, nullptr);
	}
	void FirstApp::run()
	{
		while (!vtWindow.shouldClose())
		{
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(vtDevice.device());
	}

	void FirstApp::loadGameObjects()
		{
			std::vector<VtModel::Vertex> vertices{
				{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
				{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
				{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}} };
			auto lveModel = std::make_shared<VtModel>(vtDevice, vertices);

			auto triangle = VtGameObject::createGameObject();
			triangle.model = lveModel;
			triangle.color = { .1f, .8f, .1f };
			triangle.transform2d.translation.x = .2f;
			triangle.transform2d.scale = { 2.f, .5f };
			triangle.transform2d.rotation = .25f * glm::two_pi<float>();

			gameObjects.push_back(std::move(triangle));
		}
	

		void FirstApp::createPipelineLayout()
		{
			VkPushConstantRange pushConstantRange{};
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			pushConstantRange.offset = 0;
			pushConstantRange.size = sizeof(SimplePushConstantData);

			VkPipelineLayoutCreateInfo pipeLineLayoutInfo{};
			pipeLineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipeLineLayoutInfo.setLayoutCount = 0;
			pipeLineLayoutInfo.pSetLayouts = nullptr;
			pipeLineLayoutInfo.pushConstantRangeCount = 1;
			pipeLineLayoutInfo.pPushConstantRanges = &pushConstantRange;
			if (vkCreatePipelineLayout(vtDevice.device(), &pipeLineLayoutInfo, nullptr, &pipelineLayout) !=
				VK_SUCCESS)
			{
				throw std::runtime_error("failed to create pipeline layout!");
			}
		
		}

		void FirstApp::createPipeline()
		{
			assert(vtSwapChain != nullptr && "Cannnot create pipeline before swap chain");
			assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
			PipelineConfigInfo pipelineConfig{};
			VtPipeline::defaultPipelineConfigInfo(pipelineConfig);
			pipelineConfig.renderPass = vtSwapChain->getRenderPass();
			pipelineConfig.pipelineLayout = pipelineLayout;
			vtPipeline = std::make_unique<VtPipeline>(
				vtDevice,
				"shaders/simple_shader.vert.spv",
				"shaders/simple_shader.frag.spv",
				pipelineConfig);
		}

		void FirstApp::recreateSwapChain()
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
				vtSwapChain = std::make_unique<VtSwapChain>(vtDevice, extent, std::move(vtSwapChain));
				if (vtSwapChain->imageCount() != commandBuffers.size())
				{
					freeCommandBuffers();
					createCommandBuffers();
				}
			}
			createPipeline();
		}

		void FirstApp::createCommandBuffers()
		{
			commandBuffers.resize(vtSwapChain->imageCount());

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

		void FirstApp::freeCommandBuffers()
		{
			vkFreeCommandBuffers(vtDevice.device(), vtDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()),
								 commandBuffers.data());
			commandBuffers.clear();
		}

		void FirstApp::recordCommandBuffer(int imageIndex)
		{

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = vtSwapChain->getRenderPass();
			renderPassInfo.framebuffer = vtSwapChain->getFrameBuffer(imageIndex);

			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = vtSwapChain->getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
			clearValues[1].depthStencil = {1.0f, 0};
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<uint32_t>(vtSwapChain->getSwapChainExtent().width);
			viewport.height = static_cast<uint32_t>(vtSwapChain->getSwapChainExtent().height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			VkRect2D scissor{{0, 0}, vtSwapChain->getSwapChainExtent()};
			vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
			vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

			renderGameObjects(commandBuffers[imageIndex]);

			vkCmdEndRenderPass(commandBuffers[imageIndex]);
			if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to record command buffer!");
			}
		}

		void FirstApp::renderGameObjects(VkCommandBuffer commandBuffer)
		{
			int i = 0;
			for(auto& obj : gameObjects)
			{
				i += 1;
				obj.transform2d.rotation = 
					glm::mod<float>(obj.transform2d.rotation + 0.001f * i, 2.f * glm::pi<float>());
			}
			vtPipeline->bind(commandBuffer);

			for (auto &obj : gameObjects)
			{
				//obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.01f, glm::two_pi<float>());
				SimplePushConstantData push{};
				push.offset = obj.transform2d.translation;
				push.color = obj.color;
				push.transform = obj.transform2d.mat2();
				vkCmdPushConstants(
					commandBuffer,
					pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(SimplePushConstantData),
					&push);
				obj.model->bind(commandBuffer);
				obj.model->draw(commandBuffer);
			}
		}

		void FirstApp::drawFrame()
		{
			uint32_t imageIndex;
			auto result = vtSwapChain->acquireNextImage(&imageIndex);

			if (result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				recreateSwapChain();
				return;
			}
			if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			{
				throw std::runtime_error("failed to acquire swap chain image!");
			}
			recordCommandBuffer(imageIndex);
			result = vtSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
				vtWindow.wasWindowResized())
			{
				vtWindow.resetWindowResizedFlag();
				recreateSwapChain();
				return;
			}

			if (result != VK_SUCCESS)
			{
				throw std::runtime_error("failed to present swap chain image!");
			}
		}
	}