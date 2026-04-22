#include "thing.hpp"


Thing::Thing() { 
    vertices = { 
        {{-0.33f, 0.0f, -0.33f }, WHITE},
        {{0.33f, 0.0f, -0.33f }, WHITE},
        {{0.0f, 0.0f, 0.77f }, WHITE}
    };
    model = glm::mat4(1.0f);
    index = 0;
}

Thing::Thing(glm::vec3 color, glm::mat4 modelMatrix, int i) { 
    vertices = { 
        {{-0.33f, 0.0f, -0.33f }, color},
        {{0.33f, 0.0f, -0.33f }, color},
        {{0.0f, 0.0f, 0.77f }, color}
    };
    model = modelMatrix;
    index = i;
}

