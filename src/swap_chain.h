#pragma once

#include "device.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace vt
{

  class VtSwapChain
  {
  public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

    struct
    {
      // Swap chain image presentation
      VkSemaphore presentComplete;
      // Command buffer submission and execution
      VkSemaphore renderComplete;
    } semaphores;

    VtSwapChain(VtDevice &deviceRef, VkExtent2D windowExtent);
    VtSwapChain(VtDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<VtSwapChain> previous);
    ~VtSwapChain();

    void setRenderPass(VkRenderPass &_renderPass) { renderPass = _renderPass;}

    VtSwapChain(const VtSwapChain &) = delete;
    VtSwapChain operator=(const VtSwapChain &) = delete;

    VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
    VkRenderPass getRenderPass() { return renderPass; }
    VkImageView getImageView(int index) { return swapChainImageViews[index]; }
    size_t imageCount() { return swapChainImages.size(); }
    VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
    VkExtent2D getSwapChainExtent() { return swapChainExtent; }
    uint32_t width() { return swapChainExtent.width; }
    uint32_t height() { return swapChainExtent.height; }
    VkSemaphore& getCurrentSemaphore(int currentImageIndex) {return imageAvailableSemaphores[currentImageIndex];}
    VkSemaphore& getRenderSemaphore(int currentImageIndex) {return renderFinishedSemaphores[currentImageIndex];}
    size_t& getCurrentFrame() {return currentFrame;}
    float extentAspectRatio()
    {
      return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
    }
    VkFormat findDepthFormat();

    VkResult acquireNextImage(uint32_t *imageIndex);
    VkResult submitCommandBuffers( uint32_t *imageIndex);

    bool compareSwapFormats(const VtSwapChain &swapChain) const
    {
      return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
             swapChain.swapChainImageFormat == swapChainImageFormat;
    }

  private:
    void init();
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    // Helper functions
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkFormat swapChainImageFormat;
    VkFormat swapChainDepthFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkRenderPass renderPass;

    std::vector<VkImage> depthImages;
    std::vector<VkDeviceMemory> depthImageMemorys;
    std::vector<VkImageView> depthImageViews;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VtDevice &device;
    VkExtent2D windowExtent;

    VkSwapchainKHR swapChain;
    std::shared_ptr<VtSwapChain> oldSwapChain;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;
  };

} // namespace lve
