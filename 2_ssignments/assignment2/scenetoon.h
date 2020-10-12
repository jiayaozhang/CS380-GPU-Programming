#ifndef SCENETOON_H
#define SCENETOON_H

#include "scene.h"
#include "glslprogram.h"
#include "plane.h"
#include "teapot.h"
//----------------
#include "sphere.h"
//----------------

#include "cookbookogl.h"

#include <glm/glm.hpp>

class SceneToon : public Scene
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
    SceneToon();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENETOON_H
