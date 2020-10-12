#include "scenemultilight.h"

#include <sstream>
#include <iostream>
using std::endl;
using std::cerr;

#include <glm/gtc/matrix_transform.hpp>
#include <glutils.h>
using glm::vec3;
using glm::mat4;

SceneMultiLight::SceneMultiLight() : tPrev(0),
									plane(50.0f, 50.0f, 1, 1),
									teapot(14, glm::mat4(1.0f)),
									sphere(1.0f, 36, 18)
{ }


void SceneMultiLight::initScene()
{
    compileAndLinkShader();

    glEnable(GL_DEPTH_TEST);

    view = glm::lookAt(vec3(3.5f,3.5f,5.5f), vec3(0.0f,0.0f,0.0f), vec3(0.0f,1.0f,0.0f));
    projection = mat4(1.0f);

    float x, z;
    for( int i = 0; i < 5; i++ ) {
		std::stringstream name;
        name << "lights[" << i << "].Position";
        x = 2.0f * cosf((glm::two_pi<float>() / 5) * i);
        z = 2.0f * sinf((glm::two_pi<float>() / 5) * i);
        prog.setUniform(name.str().c_str(), view * glm::vec4(x, 1.2f, z + 1.0f, 1.0f) );
    }

    prog.setUniform("lights[0].L", vec3(0.0f,0.8f,0.8f) );
    prog.setUniform("lights[1].L", vec3(0.0f,0.0f,0.8f) );
    prog.setUniform("lights[2].L", vec3(0.8f,0.0f,0.0f) );
    prog.setUniform("lights[3].L", vec3(0.0f,0.8f,0.0f) );
    prog.setUniform("lights[4].L", vec3(0.8f,0.8f,0.8f) );

	prog.setUniform("lights[0].La", vec3(0.0f, 0.2f, 0.2f));
	prog.setUniform("lights[1].La", vec3(0.0f, 0.0f, 0.2f));
	prog.setUniform("lights[2].La", vec3(0.2f, 0.0f, 0.0f));
	prog.setUniform("lights[3].La", vec3(0.0f, 0.2f, 0.0f));
	prog.setUniform("lights[4].La", vec3(0.2f, 0.2f, 0.2f));
}

void SceneMultiLight::update( float t )
{

}

void SceneMultiLight::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    prog.setUniform("Material.Kd", 0.4f, 0.4f, 0.4f);
    prog.setUniform("Material.Ks", 0.9f, 0.9f, 0.9f);
    prog.setUniform("Material.Ka", 0.5f, 0.5f, 0.5f);
    prog.setUniform("Material.Shininess", 180.0f);

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
	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void SceneMultiLight::setMatrices()
{
    mat4 mv = view * model;
    prog.setUniform("ModelViewMatrix", mv);
    prog.setUniform("NormalMatrix",
                    glm::mat3( vec3(mv[0]), vec3(mv[1]), vec3(mv[2]) ));
    prog.setUniform("MVP", projection * mv);
}

void SceneMultiLight::resize(int w, int h)
{
    glViewport(0,0,w,h);
    width = w;
    height = h;
    projection = glm::perspective(glm::radians(70.0f), (float)w/h, 0.3f, 100.0f);
}

void SceneMultiLight::compileAndLinkShader()
{
	try {
		prog.compileShader("shader/multilight.vert.glsl");
		prog.compileShader("shader/multilight.frag.glsl");
    	prog.link();
    	prog.use();
    } catch(GLSLProgramException & e) {
 		cerr << e.what() << endl;
 		exit( EXIT_FAILURE );
    }
}
