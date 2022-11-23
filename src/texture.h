#pragma once
#include <stdexcept>
#include <cassert>
#include <string>
#include <memory>
#include <stb_image.h>
#include "device.h"
#include "buffer.h"

namespace vt
{
    class texture
    {
    public:
        texture(VtDevice &vtDevice, const std::string &texturePath);
        ~texture();
        VkDescriptorImageInfo descriptorInfo() const;
        void createTextureImage(const std::string &texturePath = "textures/texture.jpg");
        void createTextureImageView();
        void createTextureSampler();
        VkImageView createImageView(VkImage image, VkFormat format);

    private:
        VtDevice &vtDevice;
        int texHeight{0};
        int texWidth{0};
        int texChannels{0};
        VkDeviceSize imageSize{0};
        VkImage textureImage = VK_NULL_HANDLE;
        VkSampler textureSampler = VK_NULL_HANDLE;
        VkImageView textureImageView = VK_NULL_HANDLE;
        VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
        
    };
}