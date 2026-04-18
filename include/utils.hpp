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