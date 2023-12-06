#pragma once

#include "model.h"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>

namespace vt
{

  struct TransformComponent
  {
    glm::vec3 translation{}; // (position offset)
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::vec3 rotation{};
    // Matrix corresponds to translate * Ry * Rx * Rz * scale transform
    //  Rotation convention uses tait-bryan angles with axis order Y(1),X(2),Z(3)
    glm::mat4 mat4();
    glm::mat3 normalMatrix();
  };
  //TODO 需要有descriptor 以及 uniform buffer 以及 image view 和 sampler 
  // Build descriptor set
  
  class VtGameObject
  {
  public:
    using id_t = unsigned int;

    static VtGameObject createGameObject()
    {
      static id_t currentId = 0;
      return VtGameObject{currentId++};
    }

    VtGameObject(const VtGameObject &) = delete;
    VtGameObject &operator=(const VtGameObject &) = delete;
    VtGameObject(VtGameObject &&) = default;
    VtGameObject &operator=(VtGameObject &&) = default;

    id_t getId() { return id; }
    void render();

    std::shared_ptr<VtModel> model{};
    glm::vec3 color{};
    TransformComponent transform{};
    

  private:
    VtGameObject(id_t objId) : id{objId} {}

    id_t id;
  };
}