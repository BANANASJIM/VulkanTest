#include "window.h"
#include <stdexcept>

namespace vt {

	VtWindow::VtWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name }{

		initWindow();
	}

	VtWindow::~VtWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void VtWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window,this);
		glfwSetFramebufferSizeCallback(window,framebufferResizeCallback);
	}

	void VtWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface){
		if(glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS){
			throw std::runtime_error("failed to create window surface");
		}
	}
	
	void VtWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height)
	{
		auto vtWindow = reinterpret_cast<VtWindow *>(glfwGetWindowUserPointer(window));
		vtWindow->framebufferResized = true;
		vtWindow->width = width;
		vtWindow->height = height;
	}
}