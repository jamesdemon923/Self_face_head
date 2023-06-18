#include "utils.h"

#include <map>

bool loadOBJ(const char* path, std::vector<glm::vec3>& out_vertices,
             std::vector<glm::vec3>& out_normals,
             std::vector<glm::vec2>& out_uvs) {
  printf("Loading OBJ file %s...\n", path);

  std::vector<unsigned int> vertexIndices, normalIndices, uvIndices;
  std::vector<glm::vec3> temp_vertices;
  std::vector<glm::vec3> temp_normals;
  std::vector<glm::vec2> temp_uvs;

  FILE* file = fopen(path, "r");
  if (file == NULL) {
    printf(
        "Impossible to open the file ! Are you in the right path ? See "
        "Tutorial 1 for details\n");
    getchar();
    return false;
  }

  int matches = 0;
  bool succeed = true;

  char line[128];
  char head[8];

  while (!feof(file)) {
    size_t size = sizeof(line);
    memset(line, 0, sizeof(line));

    succeed = true;

    if (fgets(line, sizeof(line), file) == NULL) continue;

    if (strncmp(line, "v ", 2) == 0) {
      glm::vec3 vertex;
      matches = sscanf(line + 2, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
      if (matches != 3)
        succeed = false;
      else
        temp_vertices.push_back(vertex);
    } else if (strncmp(line, "vn", 2) == 0) {
      glm::vec3 normal;
      matches = sscanf(line + 2, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
      if (matches != 3)
        succeed = false;
      else
        temp_normals.push_back(normal);
    } else if (strncmp(line, "vt", 2) == 0) {
      glm::vec2 uv;
      matches = sscanf(line + 2, "%f %f\n", &uv.x, &uv.y);
      if (matches != 2)
        succeed = false;
      else
        temp_uvs.push_back(uv);
    } else if (strncmp(line, "f ", 2) == 0) {
      unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];

      matches = sscanf(line + 2, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
                       &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                       &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                       &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
      if (matches != 9) {
        matches = sscanf(line + 2, "%d//%d %d//%d %d//%d\n", &vertexIndex[0],
                         &normalIndex[0], &vertexIndex[1], &normalIndex[1],
                         &vertexIndex[2], &normalIndex[2]);
        if (matches != 6) {
          succeed = false;
          printf(
              "ERROR: NO NORMALS PRESENT IN FILE! YOU NEED NORMALS FOR "
              "LIGHTING CALCULATIONS!\n");
          printf(
              "File can't be read by our simple parser :-( Try exporting with "
              "other options. See the definition of the loadOBJ fuction.\n");

          // printf("%s\n", line);

          return false;
        }
      }

      vertexIndices.push_back(vertexIndex[0]);
      vertexIndices.push_back(vertexIndex[1]);
      vertexIndices.push_back(vertexIndex[2]);
      normalIndices.push_back(normalIndex[0]);
      normalIndices.push_back(normalIndex[1]);
      normalIndices.push_back(normalIndex[2]);
      uvIndices.push_back(uvIndex[0]);
      uvIndices.push_back(uvIndex[1]);
      uvIndices.push_back(uvIndex[2]);
    }

    if (!succeed) {
      return false;
    }
  }

  // For each vertex of each triangle
  for (unsigned int i = 0; i < vertexIndices.size(); i++) {
    // Get the indices of its attributes
    unsigned int vertexIndex = vertexIndices[i];
    unsigned int normalIndex = normalIndices[i];
    unsigned int uvIndex = uvIndices[i];

    // Get the attributes thanks to the index
    glm::vec3 vertex = temp_vertices[vertexIndex - 1];
    glm::vec3 normal = temp_normals[normalIndex - 1];
    glm::vec2 uv = temp_uvs[uvIndex - 1];

    // Put the attributes in buffers
    out_vertices.push_back(vertex);
    out_normals.push_back(normal);
    out_uvs.push_back(uv);
  }

  return true;
}

struct PackedVertex {
  glm::vec3 position;
  glm::vec3 normal;
  bool operator<(const PackedVertex that) const {
    return memcmp((void*)this, (void*)&that, sizeof(PackedVertex)) > 0;
  };
};

static bool getSimilarVertexIndex_fast(
    PackedVertex& packed,
    std::map<PackedVertex, unsigned short>& VertexToOutIndex,
    unsigned short& result) {
  auto it = VertexToOutIndex.find(packed);
  if (it == VertexToOutIndex.end()) {
    return false;
  } else {
    result = it->second;
    return true;
  }
}

void indexVBO(std::vector<glm::vec3>& in_vertices,
              std::vector<glm::vec3>& in_normals,
              std::vector<glm::vec2>& in_uvs,

              std::vector<unsigned short>& out_indices,
              std::vector<glm::vec3>& out_vertices,
              std::vector<glm::vec3>& out_normals,
              std::vector<glm::vec2>& out_uvs) {
  std::map<PackedVertex, unsigned short> VertexToOutIndex;

  // For each input vertex
  for (unsigned int i = 0; i < in_vertices.size(); i++) {
    PackedVertex packed = {in_vertices[i], in_normals[i]};

    // Try to find a similar vertex in out_XXXX
    unsigned short index;
    bool found = getSimilarVertexIndex_fast(packed, VertexToOutIndex, index);

    if (found) {  // A similar vertex is already in the VBO, use it instead !
      out_indices.push_back(index);
    } else {  // If not, it needs to be added in the output data.
      out_vertices.push_back(in_vertices[i]);
      out_normals.push_back(in_normals[i]);
      out_uvs.push_back(in_uvs[i]);
      unsigned short newindex = (unsigned short)out_vertices.size() - 1;
      out_indices.push_back(newindex);
      VertexToOutIndex[packed] = newindex;
    }
  }
}
