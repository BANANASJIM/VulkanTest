#pragma once

#include "model.h"

// std
#include <memory>

namespace vt {

struct Transform2dComponent {
  glm::vec2 translation{};  // (position offset)
  glm::vec2 scale{1.f, 1.f};
  float rotation;

  glm::mat2 mat2() {
    const float s = glm::sin(rotation);
    const float c = glm::cos(rotation);
    glm::mat2 rotMatrix{{c, s}, {-s, c}};

    glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
    return rotMatrix * scaleMat;
  }
};

class VtGameObject {
 public:
  using id_t = unsigned int;

  static VtGameObject createGameObject() {
    static id_t currentId = 0;
    return VtGameObject{currentId++};
  }

  VtGameObject(const VtGameObject &) = delete;
  VtGameObject &operator=(const VtGameObject &) = delete;
  VtGameObject(VtGameObject &&) = default;
  VtGameObject &operator=(VtGameObject &&) = default;

  id_t getId() { return id; }

  std::shared_ptr<VtModel> model{};
  glm::vec3 color{};
  Transform2dComponent transform2d{};

 private:
  VtGameObject(id_t objId) : id{objId} {}

  id_t id;
};
} 