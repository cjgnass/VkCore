#include "object.hpp"


GameObject::GameObject() { 
    vertices = { 
        {{-0.33f, 0.0f, -0.33f }, WHITE},
        {{0.33f, 0.0f, -0.33f }, WHITE},
        {{0.0f, 0.0f, 0.77f }, WHITE}
    };
    model = glm::mat4(1.0f);
}

GameObject::GameObject(glm::vec3 color, glm::mat4 modelMatrix) { 
    vertices = { 
        {{-0.33f, 0.0f, -0.33f }, color},
        {{0.33f, 0.0f, -0.33f }, color},
        {{0.0f, 0.0f, 0.77f }, color}
    };
    model = modelMatrix;
}

SquareObject::SquareObject() { 
    vertices = { 
        {{-0.5f, 0.0f, -0.5f }, WHITE},
        {{-0.5f, 0.0f, 0.5f }, WHITE},
        {{0.5f, 0.0f, -0.5f }, WHITE},
        {{-0.5f, 0.0f, 0.5f }, WHITE},
        {{0.5f, 0.0f, 0.5f }, WHITE},
        {{0.5f, 0.0f, -0.5f }, WHITE}
    };
    model = glm::mat4(1.0f);
}

SquareObject::SquareObject(glm::vec3 color, glm::mat4 modelMatrix) { 
    vertices = { 
        {{-0.5f, 0.0f, -0.5f }, color},
        {{-0.5f, 0.0f, 0.5f }, color},
        {{0.5f, 0.0f, -0.5f }, color},
        {{-0.5f, 0.0f, 0.5f }, color},
        {{0.5f, 0.0f, 0.5f }, color},
        {{0.5f, 0.0f, -0.5f }, color}
    };
    model = modelMatrix;
}

CubeObject::CubeObject() { 
    vertices = { 
        {{-0.5f, 0.5f, -0.5f }, WHITE},
        {{-0.5f, 0.5f, 0.5f }, WHITE},
        {{0.5f, 0.5f, -0.5f }, WHITE},
        {{-0.5f, 0.5f, 0.5f }, WHITE},
        {{0.5f, 0.5f, 0.5f }, WHITE},
        {{0.5f, 0.5f, -0.5f }, WHITE},

        {{-0.5f, -0.5f, -0.5f }, YELLOW},
        {{-0.5f, -0.5f, 0.5f }, YELLOW},
        {{0.5f, -0.5f, -0.5f }, YELLOW},
        {{-0.5f, -0.5f, 0.5f }, YELLOW},
        {{0.5f, -0.5f, 0.5f }, YELLOW},
        {{0.5f, -0.5f, -0.5f }, YELLOW}, 

        {{-0.5f, -0.5f, -0.5f }, GREEN},
        {{-0.5f, -0.5f, 0.5f }, GREEN},
        {{-0.5f, 0.5f, -0.5f }, GREEN},
        {{-0.5f, -0.5f, 0.5f }, GREEN},
        {{-0.5f, 0.5f, 0.5f }, GREEN},
        {{-0.5f, 0.5f, -0.5f }, GREEN},

        {{0.5f, -0.5f, -0.5f }, BLUE},
        {{0.5f, -0.5f, 0.5f }, BLUE},
        {{0.5f, 0.5f, -0.5f }, BLUE},
        {{0.5f, -0.5f, 0.5f }, BLUE},
        {{0.5f, 0.5f, 0.5f }, BLUE},
        {{0.5f, 0.5f, -0.5f }, BLUE},

        {{-0.5f, -0.5f , -0.5f}, RED},
        {{-0.5f, 0.5f , -0.5f}, RED},
        {{0.5f, -0.5f , -0.5f}, RED},
        {{-0.5f, 0.5f , -0.5f}, RED},
        {{0.5f, 0.5f , -0.5f}, RED},
        {{0.5f, -0.5f , -0.5f}, RED},

        {{-0.5f, -0.5f , 0.5f}, ORANGE},
        {{-0.5f, 0.5f , 0.5f}, ORANGE},
        {{0.5f, -0.5f , 0.5f}, ORANGE},
        {{-0.5f, 0.5f , 0.5f}, ORANGE},
        {{0.5f, 0.5f , 0.5f}, ORANGE},
        {{0.5f, -0.5f , 0.5f}, ORANGE},
    };
    model = glm::mat4(1.0f);
}

CubeObject::CubeObject(glm::vec3 color, glm::mat4 modelMatrix) { 
    vertices = { 
        {{-0.5f, 0.5f, -0.5f }, color},
        {{-0.5f, 0.5f, 0.5f }, color},
        {{0.5f, 0.5f, -0.5f }, color},
        {{-0.5f, 0.5f, 0.5f }, color},
        {{0.5f, 0.5f, 0.5f }, color},
        {{0.5f, 0.5f, -0.5f }, color},

        {{-0.5f, -0.5f, -0.5f }, color},
        {{-0.5f, -0.5f, 0.5f }, color},
        {{0.5f, -0.5f, -0.5f }, color},
        {{-0.5f, -0.5f, 0.5f }, color},
        {{0.5f, -0.5f, 0.5f }, color},
        {{0.5f, -0.5f, -0.5f }, color}, 

        {{-0.5f, -0.5f, -0.5f }, color},
        {{-0.5f, -0.5f, 0.5f }, color},
        {{-0.5f, 0.5f, -0.5f }, color},
        {{-0.5f, -0.5f, 0.5f }, color},
        {{-0.5f, 0.5f, 0.5f }, color},
        {{-0.5f, 0.5f, -0.5f }, color},

        {{0.5f, -0.5f, -0.5f }, color},
        {{0.5f, -0.5f, 0.5f }, color},
        {{0.5f, 0.5f, -0.5f }, color},
        {{0.5f, -0.5f, 0.5f }, color},
        {{0.5f, 0.5f, 0.5f }, color},
        {{0.5f, 0.5f, -0.5f }, color},

        // {{0.5f, -0.5f, -0.5f }, color},
        // {{0.5f, -0.5f, 0.5f }, color},
        // {{0.5f, 0.5f, -0.5f }, color},
        // {{0.5f, -0.5f, 0.5f }, color},
        // {{0.5f, 0.5f, 0.5f }, color},
        // {{0.5f, 0.5f, -0.5f }, color},

        {{-0.5f, -0.5f , 0.5f}, color},
        {{-0.5f, 0.5f , 0.5f}, color},
        {{0.5f, -0.5f , 0.5f}, color},
        {{-0.5f, 0.5f , 0.5f}, color},
        {{0.5f, 0.5f , 0.5f}, color},
        {{0.5f, -0.5f , 0.5f}, color},

        {{-0.5f, -0.5f , -0.5f}, color},
        {{-0.5f, 0.5f , -0.5f}, color},
        {{0.5f, -0.5f , -0.5f}, color},
        {{-0.5f, 0.5f , -0.5f}, color},
        {{0.5f, 0.5f , -0.5f}, color},
        {{0.5f, -0.5f , -0.5f}, color}

    };
    model = glm::mat4(1.0f);
}

