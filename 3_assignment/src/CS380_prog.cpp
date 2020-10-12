// CS 380 - GPGPU Programming
// Programming Assignment #3

// system includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <math.h>
#include <sstream>
#include <algorithm> 
#include <array>
#include "texture.h"

// library includes
// include glm: http://glm.g-truc.net/
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLg_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform2.hpp>

#include "glad/glad.h" 
// include glfw library: http://www.glfw.org/
#include <GLFW/glfw3.h>

// framework-includes
#include "vbocube.h"
#include "vbomesh.h"
#include "vbosphere.h"
#include "glslprogram.h"

float alpha = 2.5f;
// for loading bmp files as textures
//#include "bmpreader.h"

// mesh option constants
static const unsigned int MESH_CUBE = 0;
static const unsigned int MESH_SPHERE = 1;
static const unsigned int MESH_OBJ = 2;

// switch between meshes
unsigned int  g_uMeshOption = MESH_CUBE;

// active shader
int g_iShaderOption;
int g_iImgOption = 1;
GLuint g_uFBOHandle = 0;
GLuint g_uImgVOAHandle;
GLuint RenderOption;
// pointer to the window that will be created by glfw
GLFWwindow* g_pWindow;

// window size
unsigned int g_uWindowWidth = 800;
unsigned int g_uWindowHeight = 600;
int width = g_uWindowWidth, height = g_uWindowHeight;
bool Pass2_open = false;
float g_fCameraZ = 30.f; 
// a simple sphere
VBOSphere* g_pSphere;

// a simple cube
VBOCube* g_pCube;

// a more complex mesh
VBOMesh* g_pMesh;

// glsl program
GLSLProgram* g_glslProgram;

// matrices for setting up camera
mat4 g_matModel;
mat4 g_matView;
mat4 g_matProjection;

// current rotation angle around the x axis
float g_fRotationAngleX;
// current rotation angle around the z axis
float g_fRotationAngleZ;
// current cursor position x 
double g_dCursorX;
// current cursor position y
double g_dCursorY;

GLuint texture_brick;
GLuint texture_cement;
GLuint texture_moss;

// glfw errors are reported to this callback
void errorCallback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		g_iShaderOption = 1;
		printf("Switched to Phong+Gouraud.\n");
	}
	else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		g_iShaderOption = 2;  // id for each fragmentshader
		printf("Switched to Phong+Phong.\n");
	}
	else if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		g_iShaderOption = 3;
		printf("Switched to Phong+Phong, Stripes.\n");
	}
	else if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		g_iShaderOption = 4;
		printf("Switched to Phong+Phong, Lattice.\n");
	}
	else if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
		g_iShaderOption = 5;
		RenderOption = 1;  //render one is brick
		printf("Switched to brick texture mapping.\n");
	}
	else if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
		g_iShaderOption = 6;
		RenderOption = 2;  // render two is cement
		printf("Switched to cement texture mapping.\n");
	}else if (key == GLFW_KEY_7 && action == GLFW_PRESS) {
		g_iShaderOption = 7;
		RenderOption = 3;  //render three is moss
		printf("Switched to moss texture mapping.\n");
	}else if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
	//	g_iShaderOption = 8;
		g_iImgOption = 1;
		printf("Switched to brightness image processing.\n");
	}
	else if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
	//	g_iShaderOption = 9;
		g_iImgOption = 2;
		printf("Switched to constrast image processing.\n");
	}
	else if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
	//	g_iShaderOption = 10;
		g_iImgOption = 3;
		printf("Switched to saturation image processing.\n");
	}
	else if (key == GLFW_KEY_F4 && action == GLFW_PRESS) {
	//	g_iShaderOption = 11;
		g_iImgOption = 4;
		printf("Switched to edge image processing.\n");
	}
	else if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
		g_iShaderOption = 12;
		g_iImgOption = 5;
		printf("Switched to blur image processing.\n");
	}
	else if (key == GLFW_KEY_F6 && action == GLFW_PRESS) {
		g_iShaderOption = 13;
		g_iImgOption = 6;
		printf("Switched to sharpening image processing.\n");
	}
	else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		Pass2_open = !Pass2_open;
	}
	else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		g_uMeshOption++;
		if (g_uMeshOption > 2) {
			g_uMeshOption = 0;
		}
	}
	else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		if (g_uMeshOption == 0) {
			g_uMeshOption = 2;
		}
		else {
			g_uMeshOption--;
		}
	}
}

void cameraZoom(GLFWwindow* window, double dx, double dy) {
	g_fCameraZ += (float)dy * 0.1f;
	g_fCameraZ = std::min(1000.f, std::max(3.f, g_fCameraZ));
	g_matView = glm::lookAt(vec3(0.0f, 0.0f, g_fCameraZ), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

}

void cursorPosCallback(GLFWwindow* window, double x, double y)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {

		g_fRotationAngleX += x - g_dCursorX;
		if (g_fRotationAngleX >= 360.0f) {
			//	rotAngleX = 0.0f;
		}
		g_dCursorX = x;

		g_fRotationAngleZ += y - g_dCursorY;
		if (g_fRotationAngleZ >= 360.0f) {
			//	rotAngleY = 0.0f;
		}
		g_dCursorY = y;
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
		cameraZoom(window, 0, y - g_dCursorY);
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS) {
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		g_dCursorX = (float)x;
		g_dCursorY = (float)y;
	}
}

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	g_uWindowWidth = width;
	g_uWindowHeight = height;
	g_matProjection = glm::perspective(70.0f, (float)g_uWindowWidth / (float)g_uWindowHeight, 1.0f, 1000.0f);
	glViewport(0, 0, g_uWindowWidth, g_uWindowHeight);
}

void pass1() {

	g_glslProgram->setUniform("Pass", 1);
	g_glslProgram->setUniform("ImgType", g_iImgOption);
	if(Pass2_open) glBindFramebuffer(GL_FRAMEBUFFER, g_uFBOHandle);
	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, RenderOption);
	// use your program
	g_glslProgram->use();

	// set uniforms
	g_matModel *= glm::rotate(g_fRotationAngleX, vec3(1.0f, 0.0f, 0.0f));
	g_matModel *= glm::rotate(g_fRotationAngleZ, vec3(0.0f, 0.0f, 1.0f));

	g_fRotationAngleX *= 0.1;
	g_fRotationAngleZ *= 0.1;

	if (abs(g_fRotationAngleX) < 0.05)
	{
		g_fRotationAngleX = 0;
	}

	if (abs(g_fRotationAngleZ) < 0.05)
	{
		g_fRotationAngleZ = 0;
	}


	mat4 mv = g_matView * g_matModel;
	g_glslProgram->setUniform("ModelViewMatrix", mv);
	g_glslProgram->setUniform("NormalMatrix", mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
	g_glslProgram->setUniform("MVP", g_matProjection * mv);

	vec4 worldLight = vec4(5.0f, 5.0f, 2.0f, 1.0f);
	g_glslProgram->setUniform("Material.Kd", 0.9f, 0.5f, 0.3f);
	g_glslProgram->setUniform("Light.Ld", 1.0f, 1.0f, 1.0f);
	g_glslProgram->setUniform("Light.Position", g_matView * worldLight);
	g_glslProgram->setUniform("Material.Ka", 0.9f, 0.5f, 0.3f);
	g_glslProgram->setUniform("Light.La", 0.4f, 0.4f, 0.4f);
	g_glslProgram->setUniform("Material.Ks", 0.8f, 0.8f, 0.8f);
	g_glslProgram->setUniform("Light.Ls", 1.0f, 1.0f, 1.0f);
	g_glslProgram->setUniform("Material.Shininess", 100.0f);

	g_glslProgram->setUniform("ShaderType", g_iShaderOption);
	g_glslProgram->setUniform("VarLightIntensity", vec3(0.5f, 0.5f, 0.5f));

	g_glslProgram->setUniform("StripeColor", vec3(0.5f, 0.5f, 0.0f));
	g_glslProgram->setUniform("BackColor", vec3(0.0f, 0.5f, 0.5f));
	g_glslProgram->setUniform("Width", 0.5f);
	g_glslProgram->setUniform("Fuzz", 0.1f);
	g_glslProgram->setUniform("Scale", 10.0f);
	g_glslProgram->setUniform("Threshold", vec2(0.13f, 0.13f));

	switch (g_uMeshOption)
	{
	case MESH_CUBE: g_pCube->render(); break;
	case MESH_SPHERE: g_pSphere->render(); break;
	case MESH_OBJ: g_pMesh->render(); break;
	}
}

void pass2() {

	if(Pass2_open)
	{ 
	g_glslProgram->setUniform("Pass", 2);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, RenderOption);

	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 mv = view * model;
	g_glslProgram->setUniform("ModelViewMatrix", mv);
	g_glslProgram->setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
	g_glslProgram->setUniform("MVP", projection * mv);

	// Render the full-screen quad
	glBindVertexArray(g_uImgVOAHandle);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	}
}


// render a frame
void renderFrame()
{
	pass1();
	glFlush();
	pass2();
}


// query GPU functionality we need for OpenGL, return false when not available
bool queryGPUCapabilitiesOpenGL()
{
	// OPTIONAL: Check for required OpenGL functionality
	return true;
}

// check for opengl error and report if any
void checkForOpenGLError()
{
	GLenum error = glGetError();
	if (error == GL_NO_ERROR) {
		return;
	}
	std::string errorString = "unknown";
	std::string errorDescription = "undefined";

	if (error == GL_INVALID_ENUM)
	{
		errorString = "GL_INVALID_ENUM";
		errorDescription = "An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.";
	}
	else if (error == GL_INVALID_VALUE)
	{
		errorString = "GL_INVALID_VALUE";
		errorDescription = "A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.";
	}
	else if (error == GL_INVALID_OPERATION)
	{
		errorString = "GL_INVALID_OPERATION";
		errorDescription = "The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.";
	}
	else if (error == GL_INVALID_FRAMEBUFFER_OPERATION)
	{
		errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
		errorDescription = "The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.";
	}
	else if (error == GL_OUT_OF_MEMORY)
	{
		errorString = "GL_OUT_OF_MEMORY";
		errorDescription = "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
	}
	else if (error == GL_STACK_UNDERFLOW)
	{
		errorString = "GL_STACK_UNDERFLOW";
		errorDescription = "An attempt has been made to perform an operation that would cause an internal stack to underflow.";
	}
	else if (error == GL_STACK_OVERFLOW)
	{
		errorString = "GL_STACK_OVERFLOW";
		errorDescription = "An attempt has been made to perform an operation that would cause an internal stack to overflow.";
	}

	fprintf(stdout, "Error %s: %s\n", errorString.c_str(), errorDescription.c_str());
}



// OpenGL error debugging callback
void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}


void setupFBO() {
	// Generate and bind the framebuffer
	glGenFramebuffers(1, &g_uFBOHandle);
	glBindFramebuffer(GL_FRAMEBUFFER, g_uFBOHandle);

	// Bind the texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderOption, 0);

	// Create the texture object
	GLuint depthBuf;
	glGenRenderbuffers(1, &depthBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, g_uWindowWidth, g_uWindowHeight);

	// Bind the depth buffer to the FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER, depthBuf);

	// Set the targets for the fragment output variables
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	// Unbind the framebuffer, and revert to default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setupScene()
{

	glEnable(GL_DEPTH_TEST);
	g_iShaderOption = 1;

	// Set up a camera. Hint: The glm library is your friend
	g_matModel = mat4(1.0f);
	g_fRotationAngleX = 0.0f;
	g_fRotationAngleZ = 0.0f;

	g_matView = glm::lookAt(vec3(0.0f, 0.0f, g_fCameraZ), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	g_matProjection = glm::perspective(70.0f, (float)g_uWindowWidth / (float)g_uWindowHeight, 1.0f, 1000.0f);
	vec4 worldLight = vec4(50.0f, 50.0f, 20.0f, 1.0f);

//	g_glslProgram->setUniform("Pass", 1);
	// Set up glsl program 
	g_glslProgram = new GLSLProgram();
	std::cout << "loading file vertexshader.vert" << std::endl;
	try {
		g_glslProgram->compileShader("./src/shaders/vertexshader.vert");
	}
	catch (GLSLProgramException e) {
		std::cout << "loading vertex shader failed." << std::endl;
		std::cout << e.what() << std::endl;

	}
	std::cout << "loading file fragmentshader.vert" << std::endl;
	try {
		g_glslProgram->compileShader("./src/shaders/fragmentshader.frag");
	}
	catch (GLSLProgramException e) {
		std::cout << "loading vertex shader failed." << std::endl;
		std::cout << e.what() << std::endl;
	}
	g_glslProgram->link();

	g_pCube = new VBOCube();

	g_pSphere = new VBOSphere(1, 100, 100);

	// also try loading other meshes
	std::cout << "loading file bs_ears.obj" << std::endl;
	g_pMesh = new VBOMesh("./../data/bs_ears.obj", true, true, true);

	// load textures
	// TODO: load files and create textures
	RenderOption= Texture::loadTexture("../data/textures/brick1.bmp");
	RenderOption = Texture::loadTexture("../data/textures/cement.bmp");
	RenderOption = Texture::loadTexture("../data/textures/moss.bmp");

	// Load brick texture file into channel 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, RenderOption);
	
	//question-----------
	// load to buffer
	setupFBO(); // frame buffer
	unsigned int handle[2];
	glGenBuffers(2, handle);
	// Array for full-screen quad
	GLfloat verts[] = {
			-1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
	};
	GLfloat tc[] = {
			0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
	};
	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);

	// Set up the vertex array object
	glGenVertexArrays(1, &g_uImgVOAHandle);
	glBindVertexArray(g_uImgVOAHandle);

	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);  // Vertex position

	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);  // Texture coordinates

	glBindVertexArray(0);

}

void printUsage() {
	fprintf(stdout, "================================ usage ================================\n");
	fprintf(stdout, "right/left arrow: switch model\n");
	fprintf(stdout, "left mouse button to rotate\n");
	fprintf(stdout, "right mouse button or mouse wheel: zoom\n");
	fprintf(stdout, "1,2,3,4,5: switch shading mode\n");
	fprintf(stdout, "1: Phong+Gouraud.\n");
	fprintf(stdout, "2: Phong+Phong.\n");
	fprintf(stdout, "3: Phong+Phong, Stripes.\n");
	fprintf(stdout, "4: Phong+Phong, Lattice.\n");
	fprintf(stdout, "5: Texture brick mapping.\n");
	fprintf(stdout, "6: Texture cement mapping.\n");
	fprintf(stdout, "7: Texture moss mapping.\n");
	fprintf(stdout, "F1: brightness image processing.\n");
	fprintf(stdout, "F2: constrast image processing.\n");
	fprintf(stdout, "F3: saturation image processing.\n");
	fprintf(stdout, "F4: blur image processing .\n");
	fprintf(stdout, "F5: edge image processing .\n");
	fprintf(stdout, "F6: sharpening image processing .\n");
	fprintf(stdout, "R: termination for multi-processing executions .\n");
	fprintf(stdout, "=======================================================================\n");
}

// init application 
bool initApplication(int argc, char** argv)
{
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(glDebugOutput, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	std::string version((const char*)glGetString(GL_VERSION));
	std::stringstream stream(version);
	unsigned major, minor;
	char dot;

	stream >> major >> dot >> minor;

	assert(dot == '.');
	if (major > 3 || (major == 2 && minor >= 0)) {
		std::cout << "OpenGL Version " << major << "." << minor << std::endl;
	}
	else {
		std::cout << "The minimum required OpenGL version is not supported on this machine. Supported is only " << major << "." << minor << std::endl;
		return false;
	}
	// set callbacks
	glfwSetKeyCallback(g_pWindow, keyCallback);
	glfwSetCursorPosCallback(g_pWindow, cursorPosCallback);
	glfwSetMouseButtonCallback(g_pWindow, mouseButtonCallback);
	glfwSetWindowSizeCallback(g_pWindow, windowResizeCallback);

	// default initialization
	glClearColor(0.2, 0.3, 0.4, 1.0);
	glEnable(GL_DEPTH_TEST);

	// viewport
	glViewport(0, 0, g_uWindowWidth, g_uWindowHeight);

	return true;
}

// entry point
int main(int argc, char** argv)
{
	// set glfw error callback
	glfwSetErrorCallback(errorCallback);

	// init glfw
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	// init glfw window 

	g_pWindow = glfwCreateWindow(g_uWindowWidth, g_uWindowHeight, "CS380 - OpenGL Image Processing", nullptr, nullptr);
	if (!g_pWindow)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// set GLFW callback functions 


	// make context current (once is sufficient)
	glfwMakeContextCurrent(g_pWindow);

	// get the frame buffer size
	int width, height;
	glfwGetFramebufferSize(g_pWindow, &width, &height);

	// init the OpenGL API (we need to do this once before any calls to the OpenGL API)
	gladLoadGL();

	// query OpenGL capabilities
	if (!queryGPUCapabilitiesOpenGL())
	{
		// quit in case capabilities are insufficient
		glfwTerminate();
		exit(EXIT_FAILURE);
	}


	// init our application
	if (!initApplication(argc, argv)) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// setting up our 3D scene
	setupScene();

	// start traversing the main loop
	// loop until the user closes the window 
	while (!glfwWindowShouldClose(g_pWindow))
	{
		// render one frame  
		renderFrame();

		// swap front and back buffers 
		glfwSwapBuffers(g_pWindow);

		// poll and process input events (keyboard, mouse, window, ...)
		glfwPollEvents();
	}

	glfwTerminate();
	return EXIT_SUCCESS;
}
