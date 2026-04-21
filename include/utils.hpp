#pragma once
#include <vector>
#include "app.hpp"


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

inline std::vector<Vertex> createQuad(float x, float y, float w, float h, glm::vec3 color)
{
    float wd2 = w / 2;
    float hd2 = h / 2;

    return {
        {{x + wd2, 0.0f, y - hd2}, color}, 
        {{x - wd2, 0.0f, y - hd2}, color},
        {{x - wd2, 0.0f, y + hd2}, color},

        {{x - wd2, 0.0f, y + hd2}, color},
        {{x + wd2, 0.0f, y + hd2}, color},
        {{x + wd2, 0.0f, y - hd2}, color},
    };
}
