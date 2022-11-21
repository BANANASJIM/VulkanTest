#pragma once

#include "camera.h"

#include <vulkan/vulkan.h>

namespace vt{
    struct FrameInfo{
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        VtCamera &camera;
        VkDescriptorSet globalDescriptorSet;
    };
}