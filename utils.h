#pragma once

#ifndef UTILS_H
#define UTILS_H

#include <glm/glm.hpp>
#include <vector>

bool loadOBJ(const char* path, std::vector<glm::vec3>& out_vertices,
             std::vector<glm::vec3>& out_normals,
             std::vector<glm::vec2>& out_uvs);

void indexVBO(std::vector<glm::vec3>& in_vertices,
              std::vector<glm::vec3>& in_normals,
              std::vector<glm::vec2>& in_uvs,

              std::vector<unsigned short>& out_indices,
              std::vector<glm::vec3>& out_vertices,
              std::vector<glm::vec3>& out_normals,
              std::vector<glm::vec2>& out_uvs);

#endif  // UTILS_H
