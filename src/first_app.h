#pragma once

#include "descriptors.h"
#include "window.h"
#include "device.h"
#include "renderer.h"
#include "game_object.h"

#include <memory>
#include <vector>

namespace vt
{
	class FirstApp
	{

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp &) = delete;
		FirstApp &operator=(const FirstApp &) = delete;

		void run();

	private:
		void loadGameObjects();

		VtWindow vtWindow{WIDTH, HEIGHT, "Hello Vulkan"};
		VtDevice vtDevice{vtWindow};
		VtRenderer vtRenderer{vtWindow,vtDevice};

		std::unique_ptr<VtDescriptorPool> globalPool{};
		std::vector<VtGameObject> gameObjects;
	};
}