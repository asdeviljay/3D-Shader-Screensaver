// 3DShaderScreensaver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#include "Thirdparty/glm/glm/glm.hpp"
#include "Thirdparty/glm/glm/gtc/matrix_transform.hpp"

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include "utility.hpp"

#ifndef GLSL
#define GLSL(version, A) "#version " #version "\n" #A
#endif

const unsigned int SCREEN_WIDTH = 640;
const unsigned int SCREEN_HEIGHT = 480;
int tri_fan_size = 0;

float x = 0.0f, y = 0.0f;

const char * vertexShaderCode = GLSL(120,

attribute vec4 position;
attribute vec4 color;

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

varying vec4 dstColor;

void main()
{
	dstColor = color;
	gl_Position = projection * view * transform * position;
}

);

const char * fragmentShaderCode = GLSL(120,

	varying vec4 dstColor;

void main()
{
	gl_FragColor = dstColor;
}

);

/*-----------------------------------------------------------------------------
*  FUNCION TO CHECK FOR SHADER COMPILER ERRORS
*-----------------------------------------------------------------------------*/
void compilerCheck(GLuint ID) {
	GLint comp;
	glGetShaderiv(ID, GL_COMPILE_STATUS, &comp);

	if (comp == GL_FALSE) {
		std::cout << "Shader Compilation FAILED" << std::endl;
		GLchar messages[256];
		glGetShaderInfoLog(ID, sizeof(messages), 0, &messages[0]);
		std::cout << messages;
	}
}

void linkCheck(GLuint ID) {
	GLint linkStatus, validateStatus;
	glGetProgramiv(ID, GL_LINK_STATUS, &linkStatus);

	if (linkStatus == GL_FALSE) {
		std::cout << "Shader Linking FAILED" << std::endl;
		GLchar messages[256];
		glGetProgramInfoLog(ID, sizeof(messages), 0, &messages[0]);
		std::cout << messages;
	}

	glValidateProgram(ID);
	glGetProgramiv(ID, GL_VALIDATE_STATUS, &validateStatus);

	std::cout << "Link: " << linkStatus << "  Validate: " << validateStatus << std::endl;
	if (linkStatus == GL_FALSE) {
		std::cout << "Shader Validation FAILED" << std::endl;
		GLchar messages[256];
		glGetProgramInfoLog(ID, sizeof(messages), 0, &messages[0]);
		std::cout << messages;
	}
}

/*----------------------------------------------------------------
*  Start Main Function
*-----------------------------------------------------------------*/
#include <vector>

struct Vertex
{
	Vertex() = default;
	Vertex(float t_x, float t_y, float t_z,
		float t_r, float t_g, float t_b)
	{
		position.x = t_x;
		position.y = t_y;
		position.z = t_z;

		color.r = t_r;
		color.g = t_g;
		color.b = t_b;
		color.a = 1.0f;
	}

	glm::vec3 position;
	glm::vec4 color;
};

std::vector<Vertex> model;

void initModel()
{
	float x = 0.0f, y = 0.0f;
	float r = 0.0f, g = 0.0f, b = 0.0f;
	float circum = 0.01745329252f;
	float radius = 0.2f;
	float k = 0.01666666666f;

	model.emplace_back(Vertex(
		x, y, 0.0f,
		r, g, b));
	tri_fan_size++;
	r = 1.0f;
	for (int i = 0; i < 360; i++) {
		if (i < 60) {
			b += k;
		}
		else if (i < 120) {
			r -= k;
		}
		else if (i < 180) {
			g += k;
		}
		else if (i < 240) {
			b -= k;
		}
		else if (i < 300) {
			r += k;
		}
		else if (i < 360) {
			g -= k;
		}
		model.emplace_back(Vertex(
			radius * sin(i * circum), radius * cos(i * circum), 0.0f,
			r, g, b));
		tri_fan_size++;
	}
	g -= k;
	model.emplace_back(Vertex(
		radius * sin(0.0f), radius * cos(0.0f), 0.0f,
		r, g, b));
	tri_fan_size++;
}

GLuint sID;
GLuint arrayID;
GLuint positionID;
GLuint colorID;
GLuint transformID;
GLuint viewID;
GLuint projectionID;

void initShaders()
{
	//open gl create a shader program
	sID = glCreateProgram();
	GLuint vID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fID = glCreateShader(GL_FRAGMENT_SHADER);

	// Load shader source code
	glShaderSource(vID, 1, &vertexShaderCode, nullptr);
	glShaderSource(fID, 1, &fragmentShaderCode, nullptr);

	// Compile
	glCompileShader(vID);
	glCompileShader(fID);

	compilerCheck(vID);
	compilerCheck(fID);

	glAttachShader(sID, vID);
	glAttachShader(sID, fID);

	glLinkProgram(sID);

	linkCheck(sID);

	glUseProgram(sID);

	positionID = glGetAttribLocation(sID, "position");
	colorID = glGetAttribLocation(sID, "color");
	transformID = glGetUniformLocation(sID, "transform");
	viewID = glGetUniformLocation(sID, "view");
	projectionID = glGetUniformLocation(sID, "projection");

	glUseProgram(0);
}

void createBuffer()
{
	GLuint bufferID;
	glGenVertexArrays(1, &arrayID);
	glBindVertexArray(arrayID);
	glGenBuffers(1, &bufferID);
	glBindBuffer(GL_ARRAY_BUFFER, bufferID);
	// send the vertex data to GPU memory
	glBufferData(GL_ARRAY_BUFFER, model.size() * sizeof(Vertex), model.data(), GL_STATIC_DRAW);

	// tell a shader program to get access to the data.
	glEnableVertexAttribArray(positionID);
	glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(colorID);
	glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(sizeof(glm::vec3)));

	// unbind vertex array
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void drawArray()
{
	glUseProgram(sID);
	glBindVertexArray(arrayID);

	glDrawArrays(GL_TRIANGLE_FAN, 0, tri_fan_size);

	glBindVertexArray(0);
	glUseProgram(0);
}

glm::mat4 transform;
glm::vec3 cameraPos;
glm::vec3 cameraFront;
glm::vec3 cameraUp;
glm::mat4 view;
glm::mat4 projection;

void initCamera() 
{
	projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void updateTranform()
{
	glUseProgram(sID);

	glUniformMatrix4fv(transformID, 1, GL_FALSE, &transform[0][0]);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);

	glUseProgram(0);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_W || key == GLFW_KEY_UP) {
		switch (action)
		{
		case GLFW_PRESS:
			y = y + 0.1f;
			break;
		case GLFW_REPEAT:
			y = y + 0.1f;
			break;
		case GLFW_RELEASE:
			break;
		default:
			break;
		}
	}
	if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN) {
		switch (action)
		{
		case GLFW_PRESS:
			y = y - 0.1f;
			break;
		case GLFW_REPEAT:
			y = y - 0.1f;
			break;
		case GLFW_RELEASE:
			break;
		default:
			break;
		}
	}
	if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT) {
		switch (action)
		{
		case GLFW_PRESS:
			x = x - 0.1f;
			break;
		case GLFW_REPEAT:
			x = x - 0.1f;
			break;
		case GLFW_RELEASE:
			break;
		default:
			break;
		}
	}
	if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) {
		switch (action)
		{
		case GLFW_PRESS:
			x = x + 0.1f;
			break;
		case GLFW_REPEAT:
			x = x + 0.1f;
			break;
		case GLFW_RELEASE:
			break;
		default:
			break;
		}
	}

	if (key == GLFW_KEY_ESCAPE) {
		switch (action)
		{
		case GLFW_PRESS:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
		case GLFW_REPEAT:
			break;
		case GLFW_RELEASE:
			break;
		default:
			break;
		}

	}
}

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	float posX = static_cast<float>(xpos);
	float posY = static_cast<float>(ypos);

	if (firstMouse)
	{
		lastX = posX;
		lastY = posY;
		firstMouse = false;
	}

	float xoffset = posX - lastX;
	float yoffset = lastY - posY;
	lastX = posX;
	lastY = posY;

	float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	REQUIRED(glfwInit());
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "3D Shader Screensaver", NULL, NULL);
	REQUIRED(window);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	GLenum glewError = glewInit();
	REQUIRED(glewError == GLEW_OK);
	if (glewError != GLEW_OK)
	{
		std::cout << " GLEW is not working. " << std::endl;
		return -1;
	}

	REQUIRED(GLEW_VERSION_2_1);
	if (!GLEW_VERSION_2_1)
	{
		std::cout << "OpenGL2.1 does not supported." << std::endl;
		return -1;
	}

	const GLubyte *glVersion = glGetString(GL_VERSION);
	std::cout << "Graphics driver: " << glVersion << std::endl;

	const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	std::cout << "GLSL version: " << glslVersion << std::endl;

	REQUIRED(GLEW_ARB_vertex_array_object);
	if (GLEW_ARB_vertex_array_object)
	{
		std::cout << "Vertex arrays is supported" << std::endl;
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	initCamera();
	initModel();
	initShaders();
	createBuffer();

	float angle = 0.0f;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

		drawArray();

		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		transform = glm::mat4(1.0f);
		transform = glm::translate(transform, glm::vec3(x, y, 0.0f));
		transform = glm::rotate(transform, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
		angle = (angle == 360.0f) ? 0.0f : angle + 5.0f;

		updateTranform();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}