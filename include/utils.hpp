#pragma once
#include <vector>
#include "app.hpp"

const glm::vec3 WHITE{1.0f, 1.0f, 1.0f};

inline std::vector<Vertex> shapesToVertices(std::vector<std::vector<Vertex>> shapes)
{
    std::vector<Vertex> vertices{};
    for (auto &shape : shapes)
    {
        for (auto &v : shape)
        {
            vertices.push_back(v);
        }
    }
    return vertices;
}

inline std::vector<Vertex> createQuad(float x, float y, float w, float h)
{
    float wd2 = w / 2;
    float hd2 = h / 2;

    return {
        {{x - wd2, y - hd2, 0.0f}, WHITE},
        {{x + wd2, y - hd2, 0.0f}, WHITE},
        {{x - wd2, y + hd2, 0.0f}, WHITE},

        {{x + wd2, y + hd2, 0.0f}, WHITE},
        {{x - wd2, y + hd2, 0.0f}, WHITE},
        {{x + wd2, y - hd2, 0.0f}, WHITE},
    };
}