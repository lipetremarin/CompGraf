﻿
#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

int setupShader();
int setupGeometry();


const GLuint WIDTH = 1000, HEIGHT = 1000;

const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
"gl_Position = model * vec4(position, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"}\0";
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = finalColor;\n"
"}\n\0";

bool rotateX = false, rotateY = false, rotateZ = false;
bool translateX = false, translateY = false, translateZ = false;

float scaleFactor = 1.0f;

int main()
{
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Gabriel!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);


	GLuint shaderID = setupShader();

	GLuint VAO = setupGeometry();


	glUseProgram(shaderID);

	glm::mat4 model = glm::mat4(1);
	GLint modelLoc = glGetUniformLocation(shaderID, "model");
	
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));


	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat)glfwGetTime();

		model = glm::mat4(1);

		if (rotateX)
		{
			model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));

		}
		else if (rotateY)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

		}
		else if (rotateZ)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

		}

		if (translateX)
		{
			model = glm::translate(model, glm::vec3(sin(angle), 0.0f, 0.0f));
		}
		else if (translateY)
		{
			model = glm::translate(model, glm::vec3(0.0f, sin(angle), 0.0f));
		}
		else if (translateZ)
		{
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, sin(angle)));
		}

		glm::mat4 scaleMatrix = glm::mat4(1.0f);
		scaleMatrix[0][0] = scaleFactor;
		scaleMatrix[1][1] = scaleFactor;
		scaleMatrix[2][2] = scaleFactor;
		model = model * scaleMatrix;


		glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));
		
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36 * 2);


		glDrawArrays(GL_POINTS, 0, 36 * 2);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}
	
	glDeleteVertexArrays(1, &VAO);
	
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_KP_ADD && action == GLFW_PRESS)
	{
		scaleFactor += 0.1;
	}

	if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_PRESS)
	{
		scaleFactor -= 0.1;
	}

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		rotateX = true;
		rotateY = false;
		rotateZ = false;
		scaleFactor += 0.1;
	}

	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = true;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = true;
	}

	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		translateX = true;
		translateY = false;
		translateZ = false;
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		translateX = false;
		translateY = true;
		translateZ = false;
	}

	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		translateX = false;
		translateY = false;
		translateZ = true;
	}



}


int setupShader()
{
	
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

int setupGeometry()
{

	GLfloat vertices[] = {
		// Cube 1
	   // Front face
	   -0.25, -0.25, 0.25, 1.0, 0.0, 0.0,
	   0.25, -0.25, 0.25, 1.0, 0.0, 0.0,
	   0.25, 0.25, 0.25, 1.0, 0.0, 0.0,

	   0.25, 0.25, 0.25, 1.0, 0.0, 0.0,
	   -0.25, 0.25, 0.25, 1.0, 0.0, 0.0,
	   -0.25, -0.25, 0.25, 1.0, 0.0, 0.0,

	   // Back face
	   -0.25, -0.25, -0.25, 0.0, 1.0, 0.0,
	   -0.25, 0.25, -0.25, 0.0, 1.0, 0.0,
	   0.25, 0.25, -0.25, 0.0, 1.0, 0.0,

	   0.25, 0.25, -0.25, 0.0, 1.0, 0.0,
	   0.25, -0.25, -0.25, 0.0, 1.0, 0.0,
	   -0.25, -0.25, -0.25, 0.0, 1.0, 0.0,

	   // Left face
	   -0.25, 0.25, -0.25, 0.0, 0.0, 1.0,
	   -0.25, 0.25, 0.25, 0.0, 0.0, 1.0,
	   -0.25, -0.25, 0.25, 0.0, 0.0, 1.0,

	   -0.25, -0.25, 0.25, 0.0, 0.0, 1.0,
	   -0.25, -0.25, -0.25, 0.0, 0.0, 1.0,
	   -0.25, 0.25, -0.25, 0.0, 0.0, 1.0,

	   // Right face
	   0.25, 0.25, -0.25, 1.0, 1.0, 0.0,
	   0.25, 0.25, 0.25, 1.0, 1.0, 0.0,
	   0.25, -0.25, 0.25, 1.0, 1.0, 0.0,

	   0.25, -0.25, 0.25, 1.0, 1.0, 0.0,
	   0.25, -0.25, -0.25, 1.0, 1.0, 0.0,
	   0.25, 0.25, -0.25, 1.0, 1.0, 0.0,

	   // Top face
	   -0.25, 0.25, -0.25, 1.0, 0.0, 1.0,
	   -0.25, 0.25, 0.25, 1.0, 0.0, 1.0,
	   0.25, 0.25, 0.25, 1.0, 0.0, 1.0,

	   0.25, 0.25, 0.25, 1.0, 0.0, 1.0,
	   0.25, 0.25, -0.25, 1.0, 0.0, 1.0,
	   -0.25, 0.25, -0.25, 1.0, 0.0, 1.0,

	   // Bottom face
	   -0.25, -0.25, -0.25, 0.0, 1.0, 1.0,
	   0.25, -0.25, -0.25, 0.0, 1.0, 1.0,
	   0.25, -0.25, 0.25, 0.0, 1.0, 1.0,

	   0.25, -0.25, 0.25, 0.0, 1.0, 1.0,
	   -0.25, -0.25, 0.25, 0.0, 1.0, 1.0,
	   -0.25, -0.25, -0.25, 0.0, 1.0, 1.0,

	   // Cube 2
	   // Front face
	   -0.5, -0.5, -0.5, 1.0, 0.0, 0.0,
	   0.5, -0.5, -0.5, 1.0, 0.0, 0.0,
	   0.5, 0.5, -0.5, 1.0, 0.0, 0.0,

	   0.5, 0.5, -0.5, 1.0, 0.0, 0.0,
	   -0.5, 0.5, -0.5, 1.0, 0.0, 0.0,
	   -0.5, -0.5, -0.5, 1.0, 0.0, 0.0,

	   // Back face
	   -0.5, -0.5, -1.0, 0.0, 1.0, 0.0,
	   -0.5, 0.5, -1.0, 0.0, 1.0, 0.0,
	   0.5, 0.5, -1.0, 0.0, 1.0, 0.0,

	   0.5, 0.5, -1.0, 0.0, 1.0, 0.0,
	   0.5, -0.5, -1.0, 0.0, 1.0, 0.0,
	   -0.5, -0.5, -1.0, 0.0, 1.0, 0.0,

	   // Left face
	   -0.5, 0.5, -1.0, 0.0, 0.0, 1.0,
	   -0.5, 0.5, -0.5, 0.0, 0.0, 1.0,
	   -0.5, -0.5, -0.5, 0.0, 0.0, 1.0,

	   -0.5, -0.5, -0.5, 0.0, 0.0, 1.0,
	   -0.5, -0.5, -1.0, 0.0, 0.0, 1.0,
	   -0.5, 0.5, -1.0, 0.0, 0.0, 1.0,

	   
	   0.5, 0.5, -1.0, 1.0, 1.0, 0.0,
	   0.5, 0.5, -0.5, 1.0, 1.0, 0.0,
	   0.5, -0.5, -0.5, 1.0, 1.0, 0.0,

	   0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
	   0.5, -0.5, -1.0, 1.0, 1.0, 0.0,
	   0.5, 0.5, -1.0, 1.0, 1.0, 0.0,

	   // Top face
	   -0.5, 0.5, -1.0, 1.0, 0.0, 1.0,
	   -0.5, 0.5, -0.5, 1.0, 0.0, 1.0,
	   0.5, 0.5, -0.5, 1.0, 0.0, 1.0,

	   0.5, 0.5, -0.5, 1.0, 0.0, 1.0,
	   0.5, 0.5, -1.0, 1.0, 0.0, 1.0,
	   -0.5, 0.5, -1.0, 1.0, 0.0, 1.0,

	   // Bottom face
	   -0.5, -0.5, -1.0, 0.0, 1.0, 1.0,
	   0.5, -0.5, -1.0, 0.0, 1.0, 1.0,
	   0.5, -0.5, -0.5, 0.0, 1.0, 1.0,

	   0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
	   -0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
	   -0.5, -0.5, -1.0, 0.0, 1.0, 1.0,
	};



	GLuint VBO, VAO;

	
	glGenBuffers(1, &VBO);

	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	
	glGenVertexArrays(1, &VAO);

	
	glBindVertexArray(VAO);

	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);



	
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	
	glBindVertexArray(0);

	return VAO;
}

