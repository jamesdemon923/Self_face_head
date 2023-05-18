// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <stack>   
#include <sstream>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/texture.hpp>

#include <iostream>

#define M_PI 3.14159265358979323846

const int window_width = 600, window_height = 600;

typedef struct Vertex {
	float Position[4];
	float Color[4];
	float Normal[3];
	float Uv[2];
	void SetPosition(float* coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0;
	}
	void SetColor(float* color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
	}
	void SetNormal(float* coords) {
		Normal[0] = coords[0];
		Normal[1] = coords[1];
		Normal[2] = coords[2];
	}
	void SetUv(float* coords) {
		Uv[0] = coords[0];
		Uv[1] = coords[1];
	}
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void createVAOs(Vertex[], GLushort[], int);
void loadObject(char*, glm::vec4, Vertex*&, GLushort*&, int);
void createObjects(void);
void pickObject(void);
void renderScene(void);
void cleanup(void);
void moveCamera(void);
void resetProgram(void);
vec3 findClosestPoint(vec3, vec3, vec3, double);
bool rayPicking(vec3, vec3, vec3, vec3, double, double);
bool rayPickingPoints(Vertex*, vec3, vec3, unsigned int*, double*, double, int);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);


// GLOBAL VARIABLES
GLFWwindow* window;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex = -1;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;
GLuint textureProgramID;

const GLuint NumObjects = 100;	// ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
GLuint VertexArrayId[NumObjects];
GLuint VertexBufferId[NumObjects];
GLuint IndexBufferId[NumObjects];

vec3 startMousePos;
vec3 endMousePos;
glm::mat4 ModelMatrix;
unsigned int id;

bool keyUpAlreadyPressed = false;
bool keyDownAlreadyPressed = false;
bool keyLeftAlreadyPressed = false;
bool keyRightAlreadyPressed = false;
bool keyShiftAlreadyPressed = false;
bool cameraSelect = false;
bool moveCameraLeft = false;
bool moveCameraRight = false;
bool moveCameraUp = false;
bool moveCameraDown = false;
bool startReset = false;
bool isUpper = false;
bool showWireframe = false;
bool showHighPolyHead = false;
bool showTexture = false;
bool showTextureH = false;

GLfloat cameraAngleT = M_PI / 4;
GLfloat cameraAngleP = asin(1 / (sqrt(3)));
GLfloat cameraSphereR = sqrt(300);
Vertex* Head_Verts;
GLushort* Head_Idcs;
Vertex* Head_VertsH;
GLushort* Head_IdcsH;
Vertex* Head_Line_Verts;
GLushort* Head_Line_Idcs_temp;
GLushort* Head_Line_Idcs;
Vertex* Head_Line_VertsH;
GLushort* Head_Line_Idcs_tempH;
GLushort* Head_Line_IdcsH;
GLuint texture;
GLuint textureH;

// TL
size_t VertexBufferSize[NumObjects];
size_t IndexBufferSize[NumObjects];
size_t NumIdcs[NumObjects];
size_t VertsNum[NumObjects]; // store the vertCount
size_t NumVerts[NumObjects];

GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint ProjMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorID;
GLuint LightID;
GLuint textureID;
GLuint showTexID;

// Declare global objects
// TL
const size_t CoordVertsCount = 6;
const size_t GridVertsCount = 44;
//const size_t MeshCount = 121;

float pickingColor[10];
Vertex CoordVerts[CoordVertsCount];
Vertex Grid[GridVertsCount];
//Vertex MeshVerts[MeshCount];
//GLushort MeshIdcs[484];

int initWindow(void) {
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);	// FOR MAC

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "Haolan,Xu(3432-6768)", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width, window_height);
	TwBar* GUI = TwNewBar("Picking");
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
}

void initOpenGL(void) {
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Projection matrix : 45?Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	gProjectionMatrix = glm::perspective(45.0f, 1.0f / 1.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	gViewMatrix = glm::lookAt(glm::vec3(10.0, 10.0, 10.0f),	// eye
		glm::vec3(0.0, 0.0, 0.0),	// center
		glm::vec3(0.0, 1.0, 0.0));	// up

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ProjMatrixID = glGetUniformLocation(programID, "P");

	textureID = glGetUniformLocation(programID, "myTextureSampler");
	showTexID = glGetUniformLocation(programID, "showTex");

	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");

	for (int i = 0; i < 10; i++)
	{
		pickingColor[i] = i / 255.0f;
	}

	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// TL
	// Define objects
	createObjects();

	// ATTN: create VAOs for each of the newly created objects here:
	VertexBufferSize[0] = sizeof(CoordVerts);
	VertexBufferSize[1] = sizeof(Grid);
	//VertexBufferSize[6] = sizeof(MeshVerts);
	//IndexBufferSize[6] = sizeof(MeshIdcs);

	NumVerts[0] = CoordVertsCount;
	NumVerts[1] = GridVertsCount;
	//NumVerts[6] = MeshCount;

	createVAOs(CoordVerts, NULL, 0);
	createVAOs(Grid, NULL, 1);
	//createVAOs(MeshVerts, MeshIdcs, 6);
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], int ObjectId) {
	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;
	const size_t Uvoffset = sizeof(Vertices[0].Normal) + Normaloffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);
	glBindVertexArray(VertexArrayId[ObjectId]);

	// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Uvoffset); // TL

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal
	glEnableVertexAttribArray(3);	// uv

	// Disable our Vertex Buffer Object 
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
		);
	}
}

// Ensure your .obj files are in the correct format and properly loaded by looking at the following function
void loadObject(char* file, glm::vec4 color, Vertex*& out_Vertices, GLushort*& out_Indices, int ObjectId) {
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(file, vertices, uvs, normals);

	std::vector<GLushort> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	const size_t vertCount = indexed_vertices.size();
	const size_t idxCount = indices.size();

	// populate output arrays
	out_Vertices = new Vertex[vertCount];
	for (int i = 0; i < vertCount; i++) {
		out_Vertices[i].SetPosition(&indexed_vertices[i].x);
		out_Vertices[i].SetNormal(&indexed_normals[i].x);
		out_Vertices[i].SetColor(&color[0]);
		out_Vertices[i].SetUv(&indexed_uvs[i].x);
	}
	out_Indices = new GLushort[idxCount];
	for (int i = 0; i < idxCount; i++) {
		out_Indices[i] = indices[i];
	}

	// set global variables!!
	NumIdcs[ObjectId] = idxCount;
	VertsNum[ObjectId] = vertCount;
	VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
	IndexBufferSize[ObjectId] = sizeof(GLushort) * idxCount;
}

void createObjects(void) {
	//-- COORDINATE AXES --//
	CoordVerts[0] = { { 0.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[1] = { { 5.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[2] = { { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[3] = { { 0.0, 5.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[4] = { { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[5] = { { 0.0, 0.0, 5.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };

	//-- GRID --//

	// ATTN: Create your grid vertices here!
	int k1 = 0;
	for (int i = -5; i <= 5; i++) {
		Grid[4 * k1] = { {(float)i, 0.0, -5.0, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0} };
		Grid[4 * k1 + 1] = { {(float)i, 0.0, 5.0, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0} };
		Grid[4 * k1 + 2] = { {-5.0, 0.0, (float)i, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0} };
		Grid[4 * k1 + 3] = { {5.0, 0.0, (float)i, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0} };
		k1++;
	}

	// ATTN: Load your models here through .obj files -- example of how to do so is as shown
	// Vertex* Verts;
	// GLushort* Idcs;
	// loadObject("models/base.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, ObjectID);
	// createVAOs(Verts, Idcs, ObjectID);

	// Use triangle to draw the obj
	// The low poly model
	loadObject("Head_LowPolywithUVwithMa.obj", glm::vec4(0.5, 0.5, 0.5, 1.0), Head_Verts, Head_Idcs, 2);
	createVAOs(Head_Verts, Head_Idcs, 2);
	// The high poly model, I made it in the Blender
	loadObject("Head_HighPolywithUVwithMa.obj", glm::vec4(0.5, 0.5, 0.5, 1.0), Head_VertsH, Head_IdcsH, 3);
	createVAOs(Head_VertsH, Head_IdcsH, 3);

	// Use Line to draw the obj
	loadObject("Head_LowPolywithUVwithMa.obj", glm::vec4(0.5, 0.5, 0.5, 1.0), Head_Line_Verts, Head_Line_Idcs_temp, 4);
	// The indice for drawing the head in lines
	Head_Line_Idcs = new GLushort[2 * NumIdcs[4]];
	for (int i = 0; i < NumIdcs[4] / 3; i++) {
		Head_Line_Idcs[6 * i] = Head_Line_Idcs_temp[3 * i];
		Head_Line_Idcs[6 * i + 1] = Head_Line_Idcs_temp[3 * i + 1];
		Head_Line_Idcs[6 * i + 2] = Head_Line_Idcs_temp[3 * i + 1];
		Head_Line_Idcs[6 * i + 3] = Head_Line_Idcs_temp[3 * i + 2];
		Head_Line_Idcs[6 * i + 4] = Head_Line_Idcs_temp[3 * i + 2];
		Head_Line_Idcs[6 * i + 5] = Head_Line_Idcs_temp[3 * i];
	}
	NumIdcs[4] = 2 * NumIdcs[4];
	IndexBufferSize[4] = 2 * IndexBufferSize[4];
	createVAOs(Head_Line_Verts, Head_Line_Idcs, 4);

	loadObject("Head_HighPolywithUVwithMa.obj", glm::vec4(0.5, 0.5, 0.5, 1.0), Head_Line_VertsH, Head_Line_Idcs_tempH, 5);
	// The indice for drawing the head in lines
	Head_Line_IdcsH = new GLushort[2 * NumIdcs[5]];
	for (int i = 0; i < NumIdcs[5] / 3; i++) {
		Head_Line_IdcsH[6 * i] = Head_Line_Idcs_tempH[3 * i];
		Head_Line_IdcsH[6 * i + 1] = Head_Line_Idcs_tempH[3 * i + 1];
		Head_Line_IdcsH[6 * i + 2] = Head_Line_Idcs_tempH[3 * i + 1];
		Head_Line_IdcsH[6 * i + 3] = Head_Line_Idcs_tempH[3 * i + 2];
		Head_Line_IdcsH[6 * i + 4] = Head_Line_Idcs_tempH[3 * i + 2];
		Head_Line_IdcsH[6 * i + 5] = Head_Line_Idcs_tempH[3 * i];
	}
	NumIdcs[5] = 2 * NumIdcs[5];
	IndexBufferSize[5] = 2 * IndexBufferSize[5];
	createVAOs(Head_Line_VertsH, Head_Line_IdcsH, 5);

	texture = loadDDS("Head_TextureLP.dds");
	textureH = loadDDS("Head_TextureHP.dds");

	std::cout << "The number of Vertices of the coarse input mesh is " << VertsNum[4] << std::endl;

	//// The grid for texture.
	//int k2 = 0;
	//for (int i = -5; i <= 5; i++) {
	//	for (int j = 0; j <= 10; j++) {
	//		MeshVerts[11 * k2 + j] = { { float(i), float(j), 5.0, 1.0 },{ 1.0, 1.0, 1.0, 1.0 },{ 0.0, 0.0, 1.0 } };
	//	}
	//	k2++;
	//}

	//for (int i = 0; i < 121; i++) {
	//	if ((i + 1) % 11 != 0 && i < 110 && i != 120) {
	//		MeshIdcs[4 * i] = i;
	//		MeshIdcs[4 * i + 1] = i + 1;
	//	}
	//	else {
	//		MeshIdcs[4 * i] = i;
	//		MeshIdcs[4 * i + 1] = i;
	//	}
	//	if (i < 110) {
	//		MeshIdcs[4 * i + 2] = i;
	//		MeshIdcs[4 * i + 3] = i + 11;
	//	}
	//	else if (i != 120) {
	//		MeshIdcs[4 * i + 2] = i;
	//		MeshIdcs[4 * i + 3] = i + 1;
	//	}
	//}
}

vec3 findClosestPoint(vec3 rayStartPos, vec3 rayEndPos, vec3 pointPos, double* proj) {
	vec3 rayVector = rayEndPos - rayStartPos;
	double raySquared = glm::dot(rayVector, rayVector);
	vec3 projection = pointPos - rayStartPos;
	double projectionVal = glm::dot(projection, rayVector);
	*proj = projectionVal / raySquared;
	vec3 closestPoint = rayStartPos + glm::vec3(rayVector.x * (*proj), rayVector.y * (*proj), rayVector.z * (*proj));
	return closestPoint;
}

bool rayPicking(vec3 pointPos, vec3 startPos, vec3 endPos, vec3* closestPoint, double* proj, double epsilon) {
	*closestPoint = findClosestPoint(startPos, endPos, pointPos, proj);
	double len = glm::distance2(*closestPoint, pointPos);
	return len < epsilon;
}

bool rayPickingPoints(Vertex* vert, vec3 start, vec3 end, unsigned int* id, double* proj, double epsilon, int maxval) {
	unsigned int pointID = maxval;
	bool foundCollision = false;
	double minDistToStart = 10000000.0;
	double distance;
	vec3 point;
	for (unsigned int i = 0; i < maxval - 1; ++i) {
		vec3 pointPos = glm::vec3(vert[i].Position[0], vert[i].Position[1], vert[i].Position[2]);
		if (rayPicking(pointPos, start, end, &point, proj, epsilon)) {
			distance = glm::distance2(start, point);
			if (distance < minDistToStart)
			{
				minDistToStart = distance;
				pointID = i;
				foundCollision = true;
			}
		}
	}

	*id = pointID;
	return foundCollision;
}

void pickObject(void) {
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pickingProgramID);
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);

		// ATTN: DRAW YOUR PICKING SCENE HERE. REMEMBER TO SEND IN A DIFFERENT PICKING COLOR FOR EACH OBJECT BEFOREHAND

	}
	glUseProgram(0);
	// Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow ! 
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel, 
	// because the framebuffer is on the GPU.
	double xpos, ypos;
	double epsilon = 0.1;
	double proj;
	GLfloat zpos;
	GLint viewport[4];
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGetIntegerv(GL_VIEWPORT, viewport);
	mat4 nModelMatrix = gViewMatrix * ModelMatrix;

	startMousePos = glm::unProject(glm::vec3(xpos, window_height - ypos, 0.0f), nModelMatrix, gProjectionMatrix, vec4(viewport[0], viewport[1], viewport[2], viewport[3]));
	endMousePos = glm::unProject(glm::vec3(xpos, window_height - ypos, 1.0f), nModelMatrix, gProjectionMatrix, vec4(viewport[0], viewport[1], viewport[2], viewport[3]));
	bool found = rayPickingPoints(Head_Line_Verts, startMousePos, endMousePos, &id, &proj, epsilon, 1446);

	if (!showHighPolyHead) {

		gPickedIndex = int(data[0]);
		if (gPickedIndex == 255) {
			gMessage = "background";
		}

		if (id <= 1445) {
			std::ostringstream oss;
			oss << "point:" << id;
			gMessage = oss.str();
		}
	}
	else {
		gMessage = " ";
	}
	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
}

void renderScene(void) {
	//ATTN: DRAW YOUR SCENE HERE. MODIFY/ADAPT WHERE NECESSARY!

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	moveCamera();

	resetProgram();

	glUseProgram(programID);
	{
		glm::vec3 lightPos = glm::vec3(4, 4, 4);

		glm::mat4x4 ModelMatrix = glm::mat4(1.0);

		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform1i(showTexID, 0);
		// Draw CoordAxes
		glBindVertexArray(VertexArrayId[0]);
		glDrawArrays(GL_LINES, 0, NumVerts[0]);

		// Draw Grid
		glBindVertexArray(VertexArrayId[1]);
		glDrawArrays(GL_LINES, 0, NumVerts[1]);

		glBindVertexArray(0);
	}
	glUseProgram(0);

	glUseProgram(programID);
	{
		glm::vec3 lightPos = glm::vec3(4, 4, 4);

		glm::mat4x4 ModelMatrix = glm::mat4(1.0);

		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		// Draw Head
		if (!showHighPolyHead) {
			if (!showWireframe) {
				if (showTexture) {
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, texture);
					glUniform1i(textureID, 0);
					glUniform1i(showTexID, 1);
				}
				else {
					glUniform1i(showTexID, 0);
				}
				glBindVertexArray(VertexArrayId[2]);
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[2], Head_Verts);
				glDrawElements(GL_TRIANGLES, NumIdcs[2], GL_UNSIGNED_SHORT, (void*)0);
			}
			else {
				glBindVertexArray(VertexArrayId[4]);
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[4]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[4], Head_Line_Verts);
				glDrawElements(GL_LINES, NumIdcs[4], GL_UNSIGNED_SHORT, (void*)0);
			}
		}
		else {
			if (!showWireframe) {
				if (showTextureH) {
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, textureH);
					glUniform1i(textureID, 0);
					glUniform1i(showTexID, 1);
				}
				else {
					glUniform1i(showTexID, 0);
				}
				glBindVertexArray(VertexArrayId[3]);
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[3], Head_VertsH);
				glDrawElements(GL_TRIANGLES, NumIdcs[3], GL_UNSIGNED_SHORT, (void*)0);
			}
			else {
				glBindVertexArray(VertexArrayId[5]);
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[5]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[5], Head_Line_VertsH);
				glDrawElements(GL_LINES, NumIdcs[5], GL_UNSIGNED_SHORT, (void*)0);
			}
		}

		glBindVertexArray(0);
	}
	glUseProgram(0);
	// Draw GUI
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void cleanup(void) {
	// Cleanup VBO and shader
	for (int i = 0; i < NumObjects; i++) {
		glDeleteBuffers(1, &VertexBufferId[i]);
		glDeleteBuffers(1, &IndexBufferId[i]);
		glDeleteVertexArrays(1, &VertexArrayId[i]);
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

// Alternative way of triggering functions on keyboard events
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// ATTN: MODIFY AS APPROPRIATE
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_R:
			startReset = true;
			cameraSelect = false;
			break;
		case GLFW_KEY_CAPS_LOCK:
			if (isUpper) {
				isUpper = false;
			}
			else {
				isUpper = true;
			}
			break;
		case GLFW_KEY_C:
			if (cameraSelect) {
				cameraSelect = false;
			}
			else {
				cameraSelect = true;
			}
			break;
		case GLFW_KEY_F:
			if (isUpper) {
				showWireframe = false;
				//uppercase
				if (showTexture) {
					showTexture = false;
					printf("F is pressed, Low Poly Head texture is hidden!");
				}
				else {
					showTexture = true;
					printf("F is pressed, Low Poly Head texture is attached!");
				}
			}
			else {
				//lowercase
				if (showWireframe) {
					showWireframe = false;
				}
				else {
					showWireframe = true;
				}
				printf("f is pressed, Wire frame is shown.");
			}
			break;
		case GLFW_KEY_P:
			showWireframe = false;
			if (showHighPolyHead) {
				showHighPolyHead = false;
			}
			else {
				printf("This is High Poly Head!");
				showHighPolyHead = true;
			}
			break;
		case GLFW_KEY_U:
			showWireframe = false;
			if (showTextureH) {
				showTextureH = false;
				printf("u is pressed, High Poly Head texture is hidden!");
			}
			else {
				showTextureH = true;
				printf("u is pressed, High Poly Head texture is attached!");
			}
			break;
		default:
			break;
		}
	}
	else if (action == GLFW_RELEASE) {
		switch (key)
		{
		case GLFW_KEY_R:
			startReset = false;
			break;
		default:
			break;
		}
	}
}

// Alternative way of triggering functions on mouse click events
static void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickObject();
	}
}

void moveCamera(void) {
	if (moveCameraLeft) {
		cameraAngleT -= 0.01f;
	}

	if (moveCameraRight) {
		cameraAngleT += 0.01f;
	}

	if (moveCameraUp) {
		cameraAngleP -= 0.01f;
	}

	if (moveCameraDown) {
		cameraAngleP += 0.01f;
	}

	if (cameraSelect && (moveCameraLeft || moveCameraRight || moveCameraDown || moveCameraUp)) {
		float cameraX = cameraSphereR * cos(cameraAngleP) * sin(cameraAngleT);
		float cameraY = cameraSphereR * sin(cameraAngleP);
		float cameraZ = cameraSphereR * cos(cameraAngleP) * cos(cameraAngleT);
		gViewMatrix = glm::lookAt(glm::vec3(cameraX, cameraY, cameraZ),	// eye
			glm::vec3(0.0, 0.0, 0.0),	// center
			glm::vec3(0.0, 1.0, 0.0));	// up
	}
}

void resetProgram(void) {
	if (startReset) {
		// resetCamera
		cameraAngleT = M_PI / 4;
		cameraAngleP = asin(1 / (sqrt(3)));
		cameraSphereR = sqrt(300);
		float cameraX = cameraSphereR * cos(cameraAngleP) * sin(cameraAngleT);
		float cameraY = cameraSphereR * sin(cameraAngleP);
		float cameraZ = cameraSphereR * cos(cameraAngleP) * cos(cameraAngleT);
		gViewMatrix = glm::lookAt(glm::vec3(cameraX, cameraY, cameraZ),	// eye
			glm::vec3(0.0, 0.0, 0.0),	// center
			glm::vec3(0.0, 1.0, 0.0));	// up
		// clean the scene
	}
}

int main(void) {
	// TL
	// ATTN: Refer to https://learnopengl.com/Getting-started/Transformations, https://learnopengl.com/Getting-started/Coordinate-Systems,
	// and https://learnopengl.com/Getting-started/Camera to familiarize yourself with implementing the camera movement

	// ATTN (Project 3 only): Refer to https://learnopengl.com/Getting-started/Textures to familiarize yourself with mapping a texture
	// to a given mesh

	// Initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;

	// Initialize OpenGL pipeline
	initOpenGL();

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		int keyUpState = glfwGetKey(window, GLFW_KEY_UP);
		if (keyUpState == GLFW_PRESS)
		{
			keyUpAlreadyPressed = true;
			if (cameraSelect) {
				moveCameraUp = true;
			}
		}
		if (keyUpState == GLFW_RELEASE) {
			if (keyUpAlreadyPressed) {
				keyUpAlreadyPressed = false;
				moveCameraUp = false;
			}
		}

		int keyDownState = glfwGetKey(window, GLFW_KEY_DOWN);
		if (keyDownState == GLFW_PRESS)
		{
			keyDownAlreadyPressed = true;
			if (cameraSelect) {
				moveCameraDown = true;
			}
		}
		if (keyDownState == GLFW_RELEASE) {
			if (keyDownAlreadyPressed) {
				keyDownAlreadyPressed = false;
				moveCameraDown = false;
			}
		}

		int keyLeftState = glfwGetKey(window, GLFW_KEY_LEFT);
		if (keyLeftState == GLFW_PRESS)
		{
			keyLeftAlreadyPressed = true;
			if (cameraSelect) {
				moveCameraLeft = true;
			}
		}
		if (keyLeftState == GLFW_RELEASE) {
			if (keyLeftAlreadyPressed) {
				keyLeftAlreadyPressed = false;
				moveCameraLeft = false;
			}
		}

		int keyRightState = glfwGetKey(window, GLFW_KEY_RIGHT);
		if (keyRightState == GLFW_PRESS)
		{
			keyRightAlreadyPressed = true;
			if (cameraSelect) {
				moveCameraRight = true;
			}
		}
		if (keyRightState == GLFW_RELEASE) {
			if (keyRightAlreadyPressed) {
				keyRightAlreadyPressed = false;
				moveCameraRight = false;
			}
		}

		// DRAWING POINTS
		renderScene();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}