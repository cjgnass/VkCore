#pragma once
#include "utils.hpp"

const glm::vec3 WHITE{1.0f, 1.0f, 1.0f};
const glm::vec3 RED{1.0f, 0.0f, 0.0f};
const glm::vec3 GREEN{0.0f, 1.0f, 0.0f};
const glm::vec3 BLUE{0.0f, 0.0f, 1.0f};

struct GameObject {
  GameObject();
  GameObject(glm::vec3 color, glm::mat4 modelMatrix);
  std::vector<Vertex> vertices;
  glm::mat4 model;
  int index;
};

struct SquareObject : GameObject {
    SquareObject(); 
    SquareObject(glm::vec3 color, glm::mat4 modelMatrix);
    int index;
};
