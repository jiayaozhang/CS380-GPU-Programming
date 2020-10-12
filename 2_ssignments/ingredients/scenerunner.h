#include "cookbookogl.h"
#include "scene.h"
#include <GLFW/glfw3.h>
#include "glutils.h"
#include "camera.h"
#include "shadermodel.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <memory>


//void ProcessKeyboard(GLFWwindow* window, int width, int height);
//void Camera::mouse_callback(GLFWwindow* window, double xpos, double ypos);
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


class SceneRunner {
private:
	GLFWwindow* window;
	int fbw, fbh;
	bool debug;           // Set true to enable debug messages
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	void processInput(GLFWwindow *window)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, deltaTime);
	


		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			shadermodel.ProcessKeyboard(ShaderModel_FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			shadermodel.ProcessKeyboard(ShaderModel_BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			shadermodel.ProcessKeyboard(ShaderModel_LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			shadermodel.ProcessKeyboard(ShaderModel_RIGHT, deltaTime);
	}

public:
	Camera camera = Camera(glm::vec3(0.0f,0.0f,5.0f), glm::vec3(0.0f,1.0f,0.0f));
	ShaderModel shadermodel = ShaderModel();
	SceneRunner(const std::string& windowTitle, int width = WIN_WIDTH, int height = WIN_HEIGHT, int samples = 0) : debug(true) {
		// Initialize GLFW
		if (!glfwInit()) exit(EXIT_FAILURE);

#ifdef __APPLE__
		// Select OpenGL 4.1
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#else
		// Select OpenGL 4.6
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
		if (debug)
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		if (samples > 0) {
			glfwWindowHint(GLFW_SAMPLES, samples);
		}

		// Open the window
		window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, windowTitle.c_str(), NULL, NULL);
		if (!window) {
			std::cerr << "Unable to create OpenGL context." << std::endl;
			glfwTerminate();
			exit(EXIT_FAILURE);
		}
		glfwMakeContextCurrent(window);

		// Get framebuffer size
		glfwGetFramebufferSize(window, &fbw, &fbh);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		//glfwSetCursorPosCallback(window,mouse_callback);
		//glfwSetFramebufferSizeCallback(window, ProcessKeyboard);
		//glfwSetScrollCallback(window, scroll_callback);

		// Load the OpenGL functions.
		if (!gladLoadGL()) { exit(-1); }

		GLUtils::dumpGLInfo();

		// Initialization
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
#ifndef __APPLE__
		if (debug) {
			glDebugMessageCallback(GLUtils::debugCallback, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
			glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
				GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Start debugging");
		}
#endif
	}

	int run(std::unique_ptr<Scene> scene) {
		// Enter the main loop
		mainLoop(window, std::move(scene));

#ifndef __APPLE__
		if (debug)
			glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 1,
				GL_DEBUG_SEVERITY_NOTIFICATION, -1, "End debug");
#endif

		// Close window and terminate GLFW
		glfwTerminate();

		// Exit program
		return EXIT_SUCCESS;
	}

	static std::string parseCLArgs(int argc, char** argv, std::map<std::string, std::string>& sceneData) {
		if (argc < 2) {
			printHelpInfo(argv[0], sceneData);
			exit(EXIT_FAILURE);
		}

		std::string recipeName = argv[1];
		auto it = sceneData.find(recipeName);
		if (it == sceneData.end()) {
			printf("Unknown recipe: %s\n\n", recipeName.c_str());
			printHelpInfo(argv[0], sceneData);
			exit(EXIT_FAILURE);
		}

		return recipeName;
	}

private:
	static void printHelpInfo(const char* exeFile, std::map<std::string, std::string>& sceneData) {
		printf("Usage: %s recipe-name\n\n", exeFile);
		printf("Recipe names: \n");
		for (auto it : sceneData) {
			printf("  %11s : %s\n", it.first.c_str(), it.second.c_str());
		}
	}

	void mainLoop(GLFWwindow* window, std::unique_ptr<Scene> scene) {

		scene->setDimensions(fbw, fbh);
		scene->initScene();
		scene->resize(fbw, fbh);

		while (!glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			GLUtils::checkForOpenGLError(__FILE__, __LINE__);
			
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			processInput(window);   // key and mouse to control the camera location
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			scene->update(float(glfwGetTime()));
			scene->view = camera.GetViewMatrix();
			scene->render();
			glfwSwapBuffers(window);

			glfwPollEvents();
			int state = glfwGetKey(window, GLFW_KEY_SPACE);
			if (state == GLFW_PRESS)
				scene->animate(!scene->animating());
		}
	}
};

