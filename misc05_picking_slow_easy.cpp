#include <cstdio>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLFW_CDECL
#include <AntTweakBar.h>

#include "common/objloader.hpp"
#include "common/shader.hpp"
#include "tessshader.hpp"
#include "common/vboindexer.hpp"
#include "tga.h"
#include "utils.h"

const static int WIN_W = 600;  // Default Width
const static int WIN_H = 600;  // Default Height
GLFWwindow* window = nullptr;

std::string gMessage;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

const static GLuint NUM_CURVED_PN_TRIANGLES_PATCH_PTS = 3;

typedef struct Vertex {
    float Position[4];
    float Color[4];
    float Normal[3];

    void SetPosition(const float* coords) {
        Position[0] = coords[0];
        Position[1] = coords[1];
        Position[2] = coords[2];
        Position[3] = 1.0;
    }
    void SetColor(const float* color) {
        Color[0] = color[0];
        Color[1] = color[1];
        Color[2] = color[2];
        Color[3] = color[3];
    }
    void SetNormal(const float* coords) {
        Normal[0] = coords[0];
        Normal[1] = coords[1];
        Normal[2] = coords[2];
    }
};

const GLuint NumObjects =
5;  // ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
GLuint VertexArrayId[NumObjects] = { 0 };
GLuint VertexBufferId[NumObjects] = { 0 };
GLuint IndexBufferId[NumObjects] = { 0 };

GLsizei NumIndices[NumObjects] = { 0 };
size_t VertexBufferSize[NumObjects] = { 0 };
size_t IndexBufferSize[NumObjects] = { 0 };

Vertex* verticesFace;
GLushort* indicesFace;
glm::vec2* uvsFace;
GLuint bufFaceTexCoordsId;

enum ObjectIds {
    Axes,
    Grid,
    Face,
    FaceQuad,      // Press F key toggles show / hide
    FaceQuadGrid,  // Press F key toggles show / hide
};

struct {
    GLuint id;
    GLint idM, idMVP;
    GLint idEnableTexture;
    GLint idTex1;
} progBase;

struct {
    GLuint id{};
    GLint idM{}, idMVP{};
    GLint idTessellationLevel{};
    GLint idVP{};
    GLint idEnableTexture{};
    GLint idTex1{};
    // GLuint idLight, idLight1, idLight2;

    GLfloat tessellationLevel = 5.f;
} progFace;

long texFaceImgW, texFaceImgH;
GLuint bufFaceQuadTexCoordsId;
GLuint texFaceId = 0;

// Face Quad
const static GLfloat faceQuadW = 10.f;
const static GLfloat faceQuadH = 10.f;
const static int faceQuadSpacesW = 10;
const static int faceQuadSpacesH = 10;
// Face Quad Position
static glm::vec3 faceQuadTranslation(0.f, 0.f, 5.f);

bool showFaceWireframe = false;  // Press r key toggles show / hide
bool showFaceQuad = false;       // Press F key toggles show / hide
bool showFaceTexture = false;    // Press u key toggles show / hide

struct SatelliteCamera {
    // Don't forget to call updateViewMatrix() every time you modify these values
    // const GLfloat r = (GLfloat)sqrt(675);  // Sphere radius
    //  const GLfloat r = (GLfloat)sqrt(100);  // Sphere radius
    const GLfloat r = (GLfloat)sqrt(300);  // Sphere radius
    float pitch{};                         // degrees
    float yaw{};                           // degrees
};
static SatelliteCamera cam;

void updateViewMatrix() {
    float radPitch = glm::radians(cam.pitch);
    float radYaw = glm::radians(cam.yaw);

    float cameraX = cam.r * cos(radPitch) * cos(radYaw);
    float cameraZ = cam.r * cos(radPitch) * sin(radYaw);
    float cameraY = cam.r * sin(radPitch);

    glm::vec3 eye = glm::vec3(cameraX, cameraY, cameraZ);
    glm::vec3 center = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
    glm::vec3 dir = (center - eye);
    up = glm::cross(glm::cross(dir, up), dir);
    up = glm::normalize(up);

    gViewMatrix = glm::lookAt(eye, center, up);
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], int ObjectId) {
    GLenum ErrorCheckValue = glGetError();
    const size_t VertexSize = sizeof(Vertices[0]);
    const size_t RgbOffset = sizeof(Vertices[0].Position);
    const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;

    // Create Vertex Array Object
    glGenVertexArrays(1, &VertexArrayId[ObjectId]);  //
    glBindVertexArray(VertexArrayId[ObjectId]);      //

    // Create Buffer for vertex data
    glGenBuffers(1, &VertexBufferId[ObjectId]);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
    glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices,
        GL_STATIC_DRAW);

    // Create Buffer for indices
    if (Indices != nullptr) {
        glGenBuffers(1, &IndexBufferId[ObjectId]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices,
            GL_STATIC_DRAW);
    }

    // Assign vertex attributes
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize,
        (GLvoid*)RgbOffset);
    // glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, VertexSize,
    //                       (GLvoid*)Normaloffset);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VertexSize,
        (GLvoid*)Normaloffset);

    glEnableVertexAttribArray(0);  // position
    glEnableVertexAttribArray(1);  // color
    glEnableVertexAttribArray(2);  // normal

    // Disable our Vertex Buffer Object
    glBindVertexArray(0);

    ErrorCheckValue = glGetError();
    if (ErrorCheckValue != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Could not create a VBO: %s \n",
            gluErrorString(ErrorCheckValue));
    }
}

void loadObject(const char* file, glm::vec4 color, Vertex*& out_Vertices,
    GLushort*& out_Indices, glm::vec2*& out_Uvs, int ObjectId) {
    // Read our .obj file
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    bool res = loadOBJ(file, vertices, normals, uvs);

    std::vector<GLushort> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    indexVBO(vertices, normals, uvs, indices, indexed_vertices, indexed_normals,
        indexed_uvs);

    const size_t vertCount = indexed_vertices.size();
    const size_t idxCount = indices.size();

    // populate output arrays
    out_Vertices = new Vertex[vertCount];
    out_Uvs = new glm::vec2[vertCount];
    for (int i = 0; i < vertCount; i++) {
        out_Vertices[i].SetPosition(&indexed_vertices[i].x);
        out_Vertices[i].SetNormal(&indexed_normals[i].x);
        out_Vertices[i].SetColor(&color[0]);
        out_Uvs[i] = indexed_uvs[i];
    }
    out_Indices = new GLushort[idxCount];
    for (int i = 0; i < idxCount; i++) {
        out_Indices[i] = indices[i];
    }

    // set global variables!!
    NumIndices[ObjectId] = (GLsizei)idxCount;
    VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
    IndexBufferSize[ObjectId] = sizeof(GLushort) * idxCount;
}

void createObjects() {
    //-- COORDINATE AXES --//
    Vertex verticesAxes[] = {
        {{0.0, 0.0, 0.0, 1.0}, {1.0, 0.0, 0.0, 1.0}, {0.0, 0.0, 1.0}},
        {{5.0, 0.0, 0.0, 1.0}, {1.0, 0.0, 0.0, 1.0}, {0.0, 0.0, 1.0}},
        {{0.0, 0.0, 0.0, 1.0}, {0.0, 1.0, 0.0, 1.0}, {0.0, 0.0, 1.0}},
        {{0.0, 5.0, 0.0, 1.0}, {0.0, 1.0, 0.0, 1.0}, {0.0, 0.0, 1.0}},
        {{0.0, 0.0, 0.0, 1.0}, {0.0, 0.0, 1.0, 1.0}, {0.0, 0.0, 1.0}},
        {{0.0, 0.0, 5.0, 1.0}, {0.0, 0.0, 1.0, 1.0}, {0.0, 0.0, 1.0}},
    };
    VertexBufferSize[Axes] = sizeof(verticesAxes);
    NumIndices[Axes] = sizeof(verticesAxes) / sizeof(verticesAxes[0]);
    createVAOs(verticesAxes, nullptr, Axes);

    //-- GRID --//
    // ATTN: create your grid vertices here!
    const int rangeBeg = -5, rangeEnd = 5;
    const int numVerticesGrid = (rangeEnd - rangeBeg + 1) * 4;
    Vertex verticesGrid[numVerticesGrid];
    int k = 0;
    for (int i = rangeBeg; i <= rangeEnd; i++) {
        verticesGrid[4 * k] = {
            {float(i), 0, -5.0, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0} };
        verticesGrid[4 * k + 1] = {
            {float(i), 0, 5.0, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0} };
        verticesGrid[4 * k + 2] = {
            {-5, 0, float(i), 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0} };
        verticesGrid[4 * k + 3] = {
            {5, 0, float(i), 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0} };
        k++;
    }
    VertexBufferSize[Grid] = sizeof(verticesGrid);
    NumIndices[Grid] = sizeof(verticesGrid) / sizeof(verticesGrid[0]);
    createVAOs(verticesGrid, nullptr, Grid);

    loadObject("face.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), verticesFace,
        indicesFace, uvsFace, Face);  // test4
    createVAOs(verticesFace, indicesFace, Face);
    // assert(false);
    // UV
    glBindVertexArray(VertexArrayId[Face]);
    glGenBuffers(1, &bufFaceTexCoordsId);
    glBindBuffer(GL_ARRAY_BUFFER, bufFaceTexCoordsId);
    glBufferData(GL_ARRAY_BUFFER,
        (GLsizeiptr)((VertexBufferSize[Face] / sizeof(Vertex)) *
            (2 * sizeof(GLfloat))),
        uvsFace, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, (2 * sizeof(GLfloat)),
        (GLvoid*)0);
    glEnableVertexAttribArray(3);  // TexCoord
    glBindVertexArray(0);

    texFaceId = load_texture_TGA("face.tga", &texFaceImgW, &texFaceImgH,
        GL_REPEAT, GL_REPEAT);

    std::vector<Vertex> faceQuadVertices;
    std::vector<glm::vec2> faceQuadUvs;
    for (int i = 0; i <= faceQuadSpacesH; ++i) {
        for (int j = 0; j <= faceQuadSpacesW; ++j) {
            GLfloat x = (-1.f + 2.f * (GLfloat)j / (GLfloat)faceQuadSpacesW) / 2.f *
                faceQuadW;
            GLfloat y = (-1.f + 2.f * (GLfloat)i / (GLfloat)faceQuadSpacesH) / 2.f *
                faceQuadH;
            glm::vec4 pos(x, y, 0.f, 1.f);
            glm::vec4 color(1.0f, 0.f, 0.f, 1.0f);
            glm::vec3 normal(0.f, 0.f, 1.0f);

            faceQuadVertices.emplace_back();
            Vertex& vert = faceQuadVertices.back();
            vert.SetPosition(&pos[0]);
            vert.SetColor(&color[0]);
            vert.SetNormal(&normal[0]);

            GLfloat v = (GLfloat)i / (GLfloat)faceQuadSpacesH;
            GLfloat u = (GLfloat)j / (GLfloat)faceQuadSpacesW;
            faceQuadUvs.emplace_back(u, v);
        }
    }
    // printf("%u\n", faceQuadVertices.size());
    std::vector<GLushort> faceQuadGridIndices;  // GL_LINES
    std::vector<GLushort> faceQuadIndices;      // GL_TRIANGLES
    for (int i = 0; i <= faceQuadSpacesH; ++i) {
        GLushort idx1 = i * (faceQuadSpacesW + 1);
        GLushort idx2 = i * (faceQuadSpacesW + 1) + faceQuadSpacesW;
        faceQuadGridIndices.emplace_back(idx1);
        faceQuadGridIndices.emplace_back(idx2);
    }
    for (int j = 0; j <= faceQuadSpacesW; ++j) {
        GLushort idx1 = j;
        GLushort idx2 = faceQuadSpacesH * (faceQuadSpacesW + 1) + j;
        faceQuadGridIndices.emplace_back(idx1);
        faceQuadGridIndices.emplace_back(idx2);
    }
    for (int i = 0; i < faceQuadSpacesH; ++i) {
        for (int j = 0; j < faceQuadSpacesW; ++j) {
            GLushort idx1, idx2, idx3;
            idx1 = i * (faceQuadSpacesW + 1) + j;
            idx2 = i * (faceQuadSpacesW + 1) + j + 1;
            idx3 = (i + 1) * (faceQuadSpacesW + 1) + j;
            faceQuadIndices.emplace_back(idx1);
            faceQuadIndices.emplace_back(idx2);
            faceQuadIndices.emplace_back(idx3);
            idx1 = (i + 1) * (faceQuadSpacesW + 1) + j + 1;
            idx2 = (i + 1) * (faceQuadSpacesW + 1) + j;
            idx3 = i * (faceQuadSpacesW + 1) + j + 1;
            faceQuadIndices.emplace_back(idx1);
            faceQuadIndices.emplace_back(idx2);
            faceQuadIndices.emplace_back(idx3);
        }
    }
    // assert(false);

    VertexBufferSize[FaceQuadGrid] = faceQuadVertices.size() * sizeof(Vertex);
    NumIndices[FaceQuadGrid] = (GLsizei)faceQuadGridIndices.size();
    IndexBufferSize[FaceQuadGrid] = NumIndices[FaceQuadGrid] * sizeof(GLushort);
    createVAOs(faceQuadVertices.data(), faceQuadGridIndices.data(), FaceQuadGrid);

    VertexBufferSize[FaceQuad] = faceQuadVertices.size() * sizeof(Vertex);
    NumIndices[FaceQuad] = (GLsizei)faceQuadIndices.size();
    IndexBufferSize[FaceQuad] = NumIndices[FaceQuad] * sizeof(GLushort);
    createVAOs(faceQuadVertices.data(), faceQuadIndices.data(), FaceQuad);
    // UV
    glBindVertexArray(VertexArrayId[FaceQuad]);
    glGenBuffers(1, &bufFaceQuadTexCoordsId);
    glBindBuffer(GL_ARRAY_BUFFER, bufFaceQuadTexCoordsId);
    glBufferData(GL_ARRAY_BUFFER,
        (GLsizeiptr)(faceQuadUvs.size() * (2 * sizeof(GLfloat))),
        faceQuadUvs.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, (2 * sizeof(GLfloat)),
        (GLvoid*)0);
    glEnableVertexAttribArray(3);  // TexCoord
    glBindVertexArray(0);
}

void onKey(GLFWwindow* win, int key, int code, int action, int mods) {
    // bool isCaps = mods & GLFW_MOD_CAPS_LOCK;
    bool isShift = mods & GLFW_MOD_SHIFT;
    if (action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_R:
            cam.pitch = 35.f;
            cam.yaw = 45.f;
            updateViewMatrix();
            break;
        case GLFW_KEY_F:
            // if ((isCaps && !isShift) || (!isCaps && isShift)) {
            //   showFaceQuad = !showFaceQuad;
            // } else {
            //   showFaceWireframe = !showFaceWireframe;
            // }
            if (isShift) {
                showFaceQuad = !showFaceQuad;
            }
            else {
                showFaceWireframe = !showFaceWireframe;
            }
            break;
        case GLFW_KEY_U:
            showFaceTexture = !showFaceTexture;
            break;
        case GLFW_KEY_P:
            if (progFace.tessellationLevel == 1.f) {
                progFace.tessellationLevel = 5.f;
            }
            else {
                progFace.tessellationLevel = 1.f;
            }
            break;
        default:
            break;
        }
    }
}

void onMouseButton(GLFWwindow* win, int button, int action, int mods) {
    TwEventMouseButtonGLFW(button, action);
    // TwEventMouseButtonGLFW(win, button, action, mods);
}

int onInitWindow() {
    if (!glfwInit()) {  // Initialise GLFW
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // FOR MAC
#endif

  // Open a window and create its OpenGL context
    window =
        glfwCreateWindow(WIN_W, WIN_H, "Haolan Xu & Yujie Wang", NULL, NULL);
    if (!window) {
        fprintf(stderr,
            "Failed to open GLFW window. If you have an Intel GPU, they are "
            "not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true;  // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Initialize the GUI
    TwInit(TW_OPENGL_CORE, nullptr);
    TwWindowSize(WIN_W, WIN_H);
    TwBar* GUI = TwNewBar("Picking");
    TwSetParam(GUI, nullptr, "refresh", TW_PARAM_CSTRING, 1, "0.1");
    // TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage,
    // nullptr);
    TwAddVarRW(GUI, "tessellationLevel", TW_TYPE_FLOAT,
        &progFace.tessellationLevel,
        " label='Tess Level' min=1 max=9 step=1 keyIncr=[ keyDecr=] "
        "help='Tessellation Level' ");
    TwAddVarRW(GUI, "wireframe", TW_TYPE_BOOL8, &showFaceWireframe,
        " label='Wireframe' help='Toggle wireframe display mode.' ");
    TwAddVarRW(GUI, "showFaceTexture", TW_TYPE_BOOL8, &showFaceTexture,
        " label='Face Texture' ");
    TwAddVarRW(GUI, "showFaceQuad", TW_TYPE_BOOL8, &showFaceQuad,
        " label='Face Quad' ");

    // Set up inputs
    glfwSetCursorPos(window, WIN_W / 2., WIN_H / 2.);
    // glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
    glfwSetKeyCallback(window, onKey);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetCursorPosCallback(window,
        [](GLFWwindow* win, double xpos, double ypos) {
            TwEventMousePosGLFW((int)xpos, (int)ypos);
        });
    // glfwSetCursorPosCallback(window, (GLFWcursorposfun)TwEventMousePosGLFW);
    glfwSetWindowSizeCallback(window, [](GLFWwindow* win, int width, int height) {
        TwWindowSize(width, height);
        glViewport(0, 0, width, height);
        // gProjectionMatrix =
        //     glm::perspective(45.0f, (float)(width) / (float)(height), 0.1f,
        //     100.0f);
        gProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
        });

    return 0;
}

void onInitGL() {
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    // Projection matrix
    // 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    gProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    // Or, for an ortho camera (In world coordinates) :
    // gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f);

    cam.pitch = 35.f;
    cam.yaw = 45.f;
    updateViewMatrix();

    progBase.id = LoadShaders("base.vert", "base.frag");
    progBase.idMVP = glGetUniformLocation(progBase.id, "gMVP");
    progBase.idM = glGetUniformLocation(progBase.id, "gM");
    progBase.idEnableTexture =
        glGetUniformLocation(progBase.id, "gEnableTexture");
    progBase.idTex1 = glGetUniformLocation(progBase.id, "tex1");

    progFace.id =
        LoadTessShaders("face.tesc", "face.tese", "face.vert", "face.frag");
    progFace.idM = glGetUniformLocation(progFace.id, "gM");
    progFace.idMVP = glGetUniformLocation(progFace.id, "gMVP");
    progFace.idTessellationLevel =
        glGetUniformLocation(progFace.id, "gTessellationLevel");
    progFace.idVP = glGetUniformLocation(progFace.id, "gVP");
    progFace.idEnableTexture =
        glGetUniformLocation(progFace.id, "gEnableTexture");
    progFace.idTex1 = glGetUniformLocation(progFace.id, "tex1");

    createObjects();
}

void onRender() {
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
    // Re-clear the screen for real rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //  glEnable(GL_CULL_FACE);

    glUseProgram(progBase.id);
    {
        glm::mat4x4 matModel = glm::mat4(1.0);
        glm::mat4 matMVP = gProjectionMatrix * gViewMatrix * matModel;
        glUniformMatrix4fv(progBase.idM, 1, GL_FALSE, &matModel[0][0]);
        glUniformMatrix4fv(progBase.idMVP, 1, GL_FALSE, &matMVP[0][0]);
        glUniform1i(progFace.idEnableTexture, GL_FALSE);

        glBindVertexArray(VertexArrayId[Axes]);
        glDrawArrays(GL_LINES, 0, NumIndices[Axes]);

        glBindVertexArray(VertexArrayId[Grid]);
        glDrawArrays(GL_LINES, 0, NumIndices[Grid]);

        if (showFaceQuad) {
            glm::mat4x4 matModel =
                glm::translate(glm::mat4(1.0), faceQuadTranslation);
            glm::mat4 matMVP = gProjectionMatrix * gViewMatrix * matModel;
            glUniformMatrix4fv(progBase.idM, 1, GL_FALSE, &matModel[0][0]);
            glUniformMatrix4fv(progBase.idMVP, 1, GL_FALSE, &matMVP[0][0]);

            glBindVertexArray(VertexArrayId[FaceQuadGrid]);
            glDrawElements(GL_LINES, NumIndices[FaceQuadGrid], GL_UNSIGNED_SHORT,
                (void*)0);

            glUniform1i(progBase.idEnableTexture, GL_TRUE);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texFaceId);
            glUniform1i(progBase.idTex1, 0);

            glBindVertexArray(VertexArrayId[FaceQuad]);
            glDrawElements(GL_TRIANGLES, NumIndices[FaceQuad], GL_UNSIGNED_SHORT,
                (void*)0);
            glUniform1i(progBase.idEnableTexture, GL_FALSE);
        }
    }

    glUseProgram(progFace.id);
    {
        glm::mat4x4 matModel = glm::mat4(1.0);
        glm::mat4 matVP = gProjectionMatrix * gViewMatrix;
        glm::mat4 matMVP = matVP * matModel;
        glUniformMatrix4fv(progFace.idM, 1, GL_FALSE, &matModel[0][0]);
        glUniformMatrix4fv(progFace.idMVP, 1, GL_FALSE, &matMVP[0][0]);
        glUniform1i(progFace.idEnableTexture, GL_FALSE);
        glUniform1f(progFace.idTessellationLevel, progFace.tessellationLevel);
        glUniformMatrix4fv(progFace.idVP, 1, GL_FALSE, &matVP[0][0]);

        glBindVertexArray(VertexArrayId[Face]);
        glPatchParameteri(GL_PATCH_VERTICES, NUM_CURVED_PN_TRIANGLES_PATCH_PTS);

        if (showFaceWireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawElements(GL_PATCHES, NumIndices[Face], GL_UNSIGNED_SHORT, (void*)0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else {
            if (showFaceTexture) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texFaceId);
                glUniform1i(progFace.idTex1, 0);

                glUniform1i(progFace.idEnableTexture, GL_TRUE);
            }
            glDrawElements(GL_PATCHES, NumIndices[Face], GL_UNSIGNED_SHORT, (void*)0);
        }
    }

    glUseProgram(0);
}

void onExit() {
    delete[] verticesFace;
    delete[] indicesFace;
    delete[] uvsFace;

    for (int i = 0; i < NumObjects; i++) {
        glDeleteBuffers(1, &VertexBufferId[i]);
        glDeleteBuffers(1, &IndexBufferId[i]);
        glDeleteVertexArrays(1, &VertexArrayId[i]);
    }

    glDeleteBuffers(1, &bufFaceQuadTexCoordsId);
    glDeleteTextures(1, &texFaceId);

    glDeleteProgram(progBase.id);
    glDeleteProgram(progFace.id);

    // Memory leak!!!
    TwTerminate();

    glfwTerminate();
}

void pollKeyEvents() {
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        cam.yaw += 5.f;
        updateViewMatrix();
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        cam.yaw -= 5.f;
        updateViewMatrix();
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        if (cam.pitch < 90.f - 5.f) {
            cam.pitch += 5.f;
        }
        updateViewMatrix();
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        if (cam.pitch > -90.0f + 5.f) {
            cam.pitch -= 5.f;
        }
        updateViewMatrix();
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        faceQuadTranslation.y += 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        faceQuadTranslation.y -= 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        faceQuadTranslation.x -= 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        faceQuadTranslation.x += 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        faceQuadTranslation.z -= 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        faceQuadTranslation.z += 0.1f;
    }
}

int main(int argc, const char* argv[]) {
    int err = onInitWindow();
    if (err) return err;

    onInitGL();

    const int FPS = 60;
    const double SPF = 1.f / FPS;
    glfwWindowHint(GLFW_REFRESH_RATE, FPS);

    double timeLast = glfwGetTime();
    int nbFrames = 0;
    do {
        double timeCurr = glfwGetTime();
        ++nbFrames;
        if (timeCurr - timeLast >= 1.0) {
            printf("%f ms/frame\n", 1000.0 / double(nbFrames));
            nbFrames = 0;
            timeLast += 1.0;
        }
        // Limit FPS
        if ((timeCurr - timeLast) < SPF) {
            // It is recommended to use sleep()
            continue;
        }
        timeLast = timeCurr;

        pollKeyEvents();
        onRender();

        // Draw GUI
        TwDraw();
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);

    onExit();
    return 0;
}
