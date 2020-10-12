#ifndef SCENEPERVERTEX_H
#define SCENEPERVERTEX_H

#include "scene.h"
#include "glslprogram.h"
#include "plane.h"
#include "teapot.h"
//----------------
#include "sphere.h"
//----------------

#include "cookbookogl.h"

#include <glm/glm.hpp>

class ScenePervertex : public Scene
{
private:
	GLSLProgram prog;

	float tPrev;
	Plane plane;
	Teapot teapot;
	//   Torus torus;
   //----------------
	Sphere sphere;
	//Cube cube;
//----------------

	float angle;

	void setMatrices();
	void compileAndLinkShader();

public:
	ScenePervertex();

	void initScene();
	void update(float t);
	void render();
	void resize(int, int);
};

#endif // SCENETOON_H

