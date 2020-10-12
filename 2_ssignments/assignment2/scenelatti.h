#pragma once
class scenelatti
{
};

#ifndef SCENEHOLE_H
#define SCENEHOLE_H

#include "scene.h"
#include "glslprogram.h"
#include "plane.h"
#include "teapot.h"
//----------------
#include "sphere.h"
//----------------

#include "cookbookogl.h"
#include <glm/glm.hpp>

class SceneLatti : public Scene
{
private:
	GLSLProgram prog;

	float tPrev;
	Plane plane;
	Teapot teapot;
	//----------------
	Sphere sphere;
	//----------------

	float angle;

	void setMatrices();
	void compileAndLinkShader();

public:
	SceneLatti();

	void initScene();
	void update(float t);
	void render();
	void resize(int, int);
};

#endif // SCENETOON_H

