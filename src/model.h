#pragma once
#include "device.h"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>


namespace vt {
    class VtModel{
        public:
        struct Vertex{
            glm::vec3 position;
            glm::vec3 color;
            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        };
        
        VtModel(VtDevice &device, const std::vector<Vertex>& vertices);
        ~VtModel();

        VtModel(const VtModel &) = delete;
        VtModel &operator=(const VtModel& ) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
        private:
        void createVertexBuffers(const std::vector<Vertex>& vertices);

        VtDevice &vtDevice;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
    };
}