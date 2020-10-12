#ifndef SCENEMULTILIGHT_H
#define SCENEMULTILIGHT_H

#include "scene.h"
#include "glslprogram.h"
#include "plane.h"
#include "sphere.h"
#include "teapot.h"

#include "cookbookogl.h"

#include <glm/glm.hpp>

class SceneMultiLight : public Scene
{
private:
    GLSLProgram prog;
	float tPrev;
    Plane plane;
	Teapot teapot;
	Sphere sphere;
	float angle;

    void setMatrices();
    void compileAndLinkShader();

public:
    SceneMultiLight();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEMULTILIGHT_H
