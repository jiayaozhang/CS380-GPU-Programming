
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <cassert>
#include <cmath>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

// framework includes
#include "vbocube.h"
#include "vbomesh.h"

//#include "cylinder.h"
#include "sphere.h"
#include "teapot.h"
#include "camera.h"
#include "model.h"

// window size
const unsigned int gWindowWidth = 800;
const unsigned int gWindowHeight = 600;
int width = 800, height = 600;

unsigned int VBO, cubeVAO;
// a simple cube
VBOCube* m_pCube;
// a more complex mesh
VBOMesh* m_pMesh;
// sphere;
Sphere* sphere;
Teapot* teapot;

Shader obj1_shader;
Shader obj2_shader;
//Shader lightsource_shader;
// camera
Camera camera;
Model model;
// lighting
//glm::vec4 worldLight = glm::vec4(5.0f,5.0f,2.0f,1.0f);
glm::vec4 worldLight = glm::vec4(0.0f, 0.0f, 2.0f, 1.0f);

//GLSLProgram obj1_shader;
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = gWindowWidth / 2.0f;
float lastY = gWindowHeight / 2.0f;
bool firstMouse = true;

// call backs
void framebuffer_size_callback(GLFWwindow* window, int width, int height); // window size
void processInput(GLFWwindow* window); // key call back
void glfwErrorCallback(int error, const char* description);// glfw error callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
	const GLchar* message, const void* userParam); // OpenGL error debugging callback
bool queryGPUCapabilitiesOpenGL();// query GPU functionality we need for OpenGL, return false when not available
bool queryGPUCapabilitiesCUDA(); // query GPU functionality we need for CUDA, return false when not available
bool initApplication(int argc, char** argv);


// TODO:
// 1. understand shader/.vs

// one time initialization to setup the 3D scene
void setupScene()
{
	// TODO: Set up a camera. Hint: The glm library is your friend
	camera = Camera(vec3(0.0f, 0.0f, 10.0f), vec3(0.0f, 1.0f, 0.0f));
	model = Model(); // set up the rotation, translation for objects.

	// TODO: Set up glsl program (at least vertex and fragment shaders).
	// load (complie and link)
//    ResourceManager::LoadShader("shader/gouraud.vs", "shader/gouraud.fs", nullptr, "cube");
	// phong shading
	LoadShader("shader/phong.vs", "shader/phong.fs", nullptr, "obj1");
	LoadShader("shader/gouraud.vs", "shader/gouraud.fs", nullptr, "obj2");
	obj1_shader = GetShader("obj1");
	obj2_shader = GetShader("obj2");
	// lightsource_shader= ResourceManager::GetShader("lightsource");
	obj1_shader.Use();

	// init scene
	obj1_shader.setUniform("Light.Position", worldLight);
	obj1_shader.setUniform("Light.La", 0.4f, 0.4f, 0.4f);
	obj1_shader.setUniform("Light.Ld", 1.0f, 1.0f, 1.0f);
	obj1_shader.setUniform("Light.Ls", 1.0f, 1.0f, 1.0f);
	obj1_shader.setUniform("Material.Ka", 0.9f, 0.5f, 0.3f);
	obj1_shader.setUniform("Material.Kd", 0.9f, 0.5f, 0.3f);
	obj1_shader.setUniform("Material.Ks", 0.8f, 0.8f, 0.8f);
	obj1_shader.setUniform("Material.Shininess", 100.0f);

	// init objects in the scene
	m_pCube = new VBOCube();
	//    m_pMesh = new VBOMesh('data/bs_ears.obj');
	sphere = new Sphere(1, 120, 120);
	teapot = new Teapot(13, glm::mat4(1.0f));


}


/* TODO: read some background about the framework:

The renderFrame function is called every time we want to render our 3D scene.
Typically we want to render a new frame as a reaction to user input (e.g., mouse dragging the camera), or because we have some animation running in our scene.
We typically aim for 10-120 frames per second (fps) depending on the application (10fps is considered interactive for high demand visualization frameworks, 20fps is usually perceived as fluid, 30fps is for computationally highly demanding gaming, 60fps is the target for gaming, ~120fps is the target for VR).
From these fps-requirements it follows that your renderFrame method is very performance critical.
It will be called multiple times per second and needs to do all computations necessary to render the scene.
-> Only compute what you really have to compute in this function (but also not less).

Rendering one frame typically includes:
- updating buffers and variables in reaction to the time that has passed. There are typically three reasons to update something in your scene: 1. animation, 2. physics simulation, 3. user interaction (e.g., camera, render mode, application logic).
- clearing the frame buffer (we typically erase everything we drew the last frame)
- rendering each object in the scene with (a specific) shader program
*/

// render a frame
void renderFrame()
{
	// clear frame buffer
	// If you have depth testing enabled you should also clear the depth buffer before each frame using GL_DEPTH_BUFFER_BIT; otherwise you¡¯re stuck with the depth values from last frame:
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// for camera move
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// render code goes here
	// TODO: use your glsl programs here
	obj1_shader.Use();
	// view/projection transformations
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.3f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();

	// world transformation. translate, rotate and then scale.
//    glm::mat4 model = glm::mat4(1.0f); // initlized by an indentity matrix.
//    model = glm::translate(model, vec3(0.0,0.0,0.0));
//    model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f,0.0f,0.0f));
	glm::mat4 objectmodel = model.model;
	glm::mat4 mv = view * objectmodel;
	obj1_shader.setUniform("ModelViewMatrix", mv);
	obj1_shader.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
	obj1_shader.setUniform("MVP", projection * mv);

	teapot->render();
}

// entry point
int main(int argc, char** argv)
{
	// set glfw error callback
	glfwSetErrorCallback(glfwErrorCallback);

	// init glfw
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// init glfw window
	GLFWwindow* window;
	window = glfwCreateWindow(gWindowWidth, gWindowHeight, "CS380 - OpenGL and GLSL Shaders", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// make context current (once is sufficient)
	glfwMakeContextCurrent(window);

	// set GLFW callback functions
	// TODO: implement and register your callbacks for user interaction
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// get the frame buffer size
	glfwGetFramebufferSize(window, &width, &height);

	// init the OpenGL API (we need to do this once before any calls to the OpenGL API)
	if (!gladLoadGL()) { exit(-1); }

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

	// TODO: setting up our 3D scene
	setupScene();

	// start traversing the main loop
	// loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{
		// input
		processInput(window);   // key and mouse to control the camera location

		// render one frame
		renderFrame();

		// swap front and back buffers
		glfwSwapBuffers(window);
		// poll and process input events (keyboard, mouse, window, ...)
		glfwPollEvents();
	}

	glfwTerminate();
	return EXIT_SUCCESS;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		model.ProcessKeyboard(Model_FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		model.ProcessKeyboard(Model_BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		model.ProcessKeyboard(Model_LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		model.ProcessKeyboard(Model_RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void glfwErrorCallback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam) // OpenGL error debugging callback
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;


	std::cout << std::endl;
}

// query
// query GPU functionality we need for OpenGL, return false when not available
bool queryGPUCapabilitiesOpenGL()
{
	// =============================================================================
	// query and print (to console) OpenGL version and extensions:
	// - query and print GL vendor, renderer, and version using glGetString()
	printf("Q1: query and print (to console) OpenGL version and extensions:\n");
	printf("OpenGL vendor: %s\n", glGetString(GL_VENDOR));
	printf("OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
	// printf("OpenGL extensions: %s\n", glGetString(GL_EXTENSIONS));
	printf("\n=============================================================\n");

	// =============================================================================
	printf("Q2: query and print GPU OpenGL limits (using glGet(), glGetInteger() etc.):\n");
	GLint result;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &result);
	printf("maximum number of vertex shader attributes: %d\n", result);

	glGetIntegerv(GL_MAX_VARYING_FLOATS, &result);
	printf("maximum number of varying floats: %d\n", result);

	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &result);
	printf("number of texture image units in fragment shader: %d\n", result);

	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &result);
	printf("number of texture image units in vertex shader: %d\n", result);

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);
	printf("number of 2D texture size: %d\n", result);

	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &result);
	printf("number of 3D texture size: %d\n", result);

	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &result);
	printf("number of draw buffers: %d\n", result);
	printf("\n=============================================================\n");
	return true;
}


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

	// default initialization
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);;
	glEnable(GL_DEPTH_TEST);    // Enable !!!

	// viewport
	glViewport(0, 0, gWindowWidth, gWindowHeight);
	return true;
}