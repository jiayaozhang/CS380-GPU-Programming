#include "scenepervertex.h"

#include <iostream>
using std::cerr;
using std::endl;

#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;
using glm::mat4;
using glm::vec4;

ScenePervertex::ScenePervertex() :
	sphere(1.0f, 36, 18),
	teapot(14, glm::mat4(1.0f)),
	tPrev(0),
	plane(50.0f, 50.0f, 1, 1)
{ }

void ScenePervertex::initScene()
{
	compileAndLinkShader();

	glEnable(GL_DEPTH_TEST);

	view = glm::lookAt(vec3(4.0f, 4.0f, 6.5f), vec3(0.0f, 0.75f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	projection = mat4(1.0f);

	angle = 0.0;


	prog.setUniform("Light.Ld", 1.0f, 1.0f, 1.0f);
	prog.setUniform("Light.La", 0.4f, 0.4f, 0.4f);
	prog.setUniform("Light.Ls", 1.0f, 1.0f, 1.0f);
}

void ScenePervertex::update(float t)
{
	float deltaT = t - tPrev;
	if (tPrev == 0.0f) deltaT = 0.0f;
	tPrev = t;

	angle += 0.25f * deltaT;
	if (angle > glm::two_pi<float>()) angle -= glm::two_pi<float>();
}

void ScenePervertex::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	vec4 lightPos = vec4(10.0f * cos(angle), 10.0f, 10.0f * sin(angle), 1.0f);
	prog.setUniform("Light.Position", view * lightPos);

	prog.setUniform("Material.Kd", 0.9f, 0.5f, 0.3f);
	prog.setUniform("Material.Ks", 0.95f, 0.95f, 0.95f);
	prog.setUniform("Material.Ka", 0.9f * 0.3f, 0.5f * 0.3f, 0.3f * 0.3f);
	prog.setUniform("Material.Shininess", 100.0f);

	model = mat4(1.0f);
	model = glm::translate(model, vec3(0.0f, 0.0f, -2.0f));
	model = glm::rotate(model, glm::radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
	setMatrices();
	teapot.render();

	prog.setUniform("Material.Kd", 0.9f, 0.5f, 0.3f);
	prog.setUniform("Material.Ks", 0.95f, 0.95f, 0.95f);
	prog.setUniform("Material.Ka", 0.9f * 0.3f, 0.5f * 0.3f, 0.3f * 0.3f);
	prog.setUniform("Material.Shininess", 100.0f);

	model = mat4(1.0f);
	model = glm::translate(model, vec3(-1.0f, 0.75f, 3.0f));
	model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
	setMatrices();
	sphere.render();

	prog.setUniform("Material.Kd", 0.7f, 0.7f, 0.7f);
	prog.setUniform("Material.Ks", 0.9f, 0.9f, 0.9f);
	prog.setUniform("Material.Ka", 0.2f, 0.2f, 0.2f);
	prog.setUniform("Material.Shininess", 180.0f);

	model = mat4(1.0f);
	setMatrices();
	plane.render();
}

void ScenePervertex::setMatrices()
{
	mat4 mv = view * model;
	prog.setUniform("ModelViewMatrix", mv);
	prog.setUniform("NormalMatrix",
		glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
	prog.setUniform("MVP", projection * mv);
}

void ScenePervertex::resize(int w, int h)
{
	glViewport(0, 0, w, h);
	width = w;
	height = h;
	projection = glm::perspective(glm::radians(60.0f), (float)w / h, 0.3f, 100.0f);
}

void ScenePervertex::compileAndLinkShader()
{
	try {
		prog.compileShader("shader/pervertex.vert.glsl");
		prog.compileShader("shader/pervertex.frag.glsl");
		prog.link();
		prog.use();
	}
	catch (GLSLProgramException & e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}
