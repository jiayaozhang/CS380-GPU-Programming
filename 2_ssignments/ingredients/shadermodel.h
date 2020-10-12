#ifndef ShaderModel_H
#define ShaderModel_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

const float Trans_SENSITIVITY = 2.5f;
const float ROT_SENSITIVITY = 30.0f;
const float Scale_SENSITIVITY = 0.5f;

// Defines several possible options for ShaderModel movement. Used as abstraction to stay away from window-system specific input methods
enum ShaderModel_Movement {
	ShaderModel_FORWARD,
	ShaderModel_BACKWARD,
	ShaderModel_LEFT,
	ShaderModel_RIGHT,
	ShaderModel_UP,
	ShaderModel_DOWN
};

// An abstract ShaderModel class that processes input and calculates the corresponding translation, rotatation and scaling for use in OpenGL
class ShaderModel
{
public:
	// camera Attributes
	glm::vec3 Translation;
	glm::vec3 Rotation;
	float Scale;
	glm::mat4 model;

	// constructor with vectors
	ShaderModel(glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f), float scale = float(1.0f))
	{
		Translation = translation;
		Rotation = rotation;
		Scale = scale;
		model = glm::mat4(1.0f);
	}


	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(ShaderModel_Movement direction, float deltaTime)
	{
		float velocity = Trans_SENSITIVITY * deltaTime;
		        if (direction == ShaderModel_FORWARD)
		            Translation[2] += velocity;
		        if (direction == ShaderModel_BACKWARD)
		            Translation[2] -= velocity;
		        if (direction == ShaderModel_LEFT)
		            Translation[0] += velocity;
		        if (direction == ShaderModel_RIGHT)
		            Translation[0] -= velocity;
		        if (direction == ShaderModel_UP)
		            Translation[1] += velocity;
		        if (direction == ShaderModel_DOWN)
		            Translation[1] -= velocity;
		//if (direction == ShaderModel_FORWARD)
		//	Rotation[0] += velocity;
		//if (direction == ShaderModel_BACKWARD)
		//	Translation[0] -= velocity;
		//if (direction == ShaderModel_LEFT)
		//	Rotation[2] += velocity;
		//if (direction == ShaderModel_RIGHT)
		//	Translation[2] -= velocity;
		//if (direction == ShaderModel_UP)
		//	Translation[1] += velocity;
		//if (direction == ShaderModel_DOWN)
		//	Translation[1] -= velocity;
		updateShaderModelVectors();
	}

	//    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	//    void ProcessMouseClick(float xoffset, float yoffset, GLboolean constrainPitch = true)
	//    {
	//        xoffset *= MouseSensitivity;
	//        yoffset *= MouseSensitivity;
	//
	//        Yaw   += xoffset;
	//        Pitch += yoffset;
	//
	//        // make sure that when pitch is out of bounds, screen doesn't get flipped
	//        if (constrainPitch)
	//        {
	//            if (Pitch > 89.0f)
	//                Pitch = 89.0f;
	//            if (Pitch < -89.0f)
	//                Pitch = -89.0f;
	//        }
	//
	//        // update Front, Right and Up Vectors using the updated Euler angles
	//        updateCameraVectors();
	//    }

		// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	//    void ProcessMouseScroll(float yoffset)
	//    {
	//        Zoom -= (float)yoffset;
	//        if (Zoom < 1.0f)
	//            Zoom = 1.0f;
	//        if (Zoom > 45.0f)
	//            Zoom = 45.0f;
	//    }

private:
	// calculates the front vector from the Camera's (updated) Euler Angles
	void updateShaderModelVectors()
	{
		// calculate the new Front vector
		model = glm::mat4(1.0f); // initlized by an indentity matrix.
		model = glm::translate(model, Translation);
			model = glm::rotate(model, glm::radians(Rotation[0]), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, glm::radians(Rotation[1]), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
	}
};
#endif