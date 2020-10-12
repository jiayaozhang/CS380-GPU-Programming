#ifndef SCENESTRIPE_H
#define SCENESTRIPE_H

#include "scene.h"
#include "glslprogram.h"
#include "plane.h"
#include "teapot.h"
//----------------
#include "sphere.h"
//----------------

#include "cookbookogl.h"
#include <glm/glm.hpp>

class SceneStripe : public Scene
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
	SceneStripe();

	void initScene();
	void update(float t);
	void render();
	void resize(int, int);
};

#endif // SCENETOON_H
