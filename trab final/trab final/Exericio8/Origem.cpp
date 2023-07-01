#include <iostream>
#include <string>
#include <assert.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include "Shader.h"
#include "Bezier.h"

using namespace std;

const GLuint WIDTH = 1000, HEIGHT = 1000;

bool rotateX = false;
bool rotateY = false;
bool rotateZ = false;
bool defaultMouse = true;

float lastX;
float lastY;
float sensitivity = 0.05;
float pitch = 0.0;
float yaw = -90.0;

int verticesSize = 0;

string objFile = "../models/SuzanneTriTextured.obj";
string mtlFile = "../materials/SuzanneTriTextured.mtl";
string curvesFile = "../animations/curves.txt";

glm::vec3 cameraPos = glm::vec3(0.0, 0.0, 3.0);
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0);
glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
int setupGeometry();
void readMaterialsFile(string filename, map<string, string>& properties);
float stofOrElse(string value, float def);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
int loadTexture(string path);
vector<glm::vec3> generateControlPoints(string filename);

struct Vertex {
	float x, y, z, r = 0.4f, g = 0.1f, b = 0.4f;
};

struct Texture {
	float s, t;
};

struct Normal {
	float x, y, z;
};

struct Face {
	Vertex vertices[3];
	Texture textures[3];
	Normal normals[3];
};

vector<string> splitString(const string& input, char delimiter) {
	vector<string> tokens;
	istringstream iss(input);
	string token;

	while (getline(iss, token, delimiter)) {
		tokens.push_back(token);
	}

	return tokens;
}

vector<float> parseObjToVertices(const string& filename) {
	ifstream file(filename);

	vector<Vertex> uniqueVertices;
	vector<Texture> uniqueTextures;
	vector<Normal> uniqueNormals;
	vector<Face> faces;

	vector<float> buffer;

	if (!file.is_open()) {
		cout << "Unable to open the file: " << filename << endl;
		return buffer;
	}

	string line;

	while (getline(file, line)) {
		vector<string> row = splitString(line, ' ');

		if (row.empty())
			continue;

		if (row[0] == "v") {
			float x = stof(row[1]);
			float y = stof(row[2]);
			float z = stof(row[3]);

			Vertex vertex;
			vertex.x = x;
			vertex.y = y;
			vertex.z = z;

			uniqueVertices.push_back(vertex);
		}

		if (row[0] == "vt") {
			float s = stof(row[1]);
			float t = stof(row[2]);

			Texture texture;
			texture.s = s;
			texture.t = t;

			uniqueTextures.push_back(texture);
		}

		if (row[0] == "vn") {
			float nx = stof(row[1]);
			float ny = stof(row[2]);
			float nz = stof(row[3]);

			Normal normal;
			normal.x = nx;
			normal.y = ny;
			normal.z = nz;

			uniqueNormals.push_back(normal);
		}

		if (row[0] == "f") {
			Face face;

			for (int i = 1; i <= 3; ++i) {
				vector<string> indices = splitString(row[i], '/');

				int vIndex = stoi(indices[0]) - 1;
				int tIndex = stoi(indices[1]) - 1;
				int nIndex = stoi(indices[2]) - 1;

				face.vertices[i - 1] = uniqueVertices[vIndex];
				face.textures[i - 1] = uniqueTextures[tIndex];
				face.normals[i - 1] = uniqueNormals[nIndex];
			}

			faces.push_back(face);
		}
	}

	file.close();

	for (const auto& face : faces) {
		for (int i = 0; i < 3; ++i) {
			buffer.push_back(face.vertices[i].x);
			buffer.push_back(face.vertices[i].y);
			buffer.push_back(face.vertices[i].z);

			buffer.push_back(face.vertices[i].r);
			buffer.push_back(face.vertices[i].g);
			buffer.push_back(face.vertices[i].b);

			buffer.push_back(face.textures[i].s);
			buffer.push_back(face.textures[i].t);

			buffer.push_back(face.normals[i].x);
			buffer.push_back(face.normals[i].y);
			buffer.push_back(face.normals[i].z);
		}
	}

	return buffer;
}


int main()
{
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "M6 - Trajetoria de Objetos - Felipe Tremarin", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	glfwSetCursorPos(window, WIDTH / 2, HEIGHT / 2);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
	}

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	Shader shader("../shaders/shaders.vs", "../shaders/shaders.fs");

	GLuint VAO = setupGeometry();

	glUseProgram(shader.ID);

	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	shader.setMat4("view", value_ptr(view));

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	shader.setMat4("projection", glm::value_ptr(projection));

	glm::mat4 model = glm::mat4(1);

	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	shader.setMat4("model", glm::value_ptr(model));

	map<string, string> properties;
	readMaterialsFile(mtlFile, properties);

	GLuint texID = loadTexture(properties["map_Kd"]);

	glUniform1i(glGetUniformLocation(shader.ID, "tex_buffer"), 0);

	shader.setFloat("ka", stofOrElse(properties["Ka"], 0));
	shader.setFloat("kd", stofOrElse(properties["Kd"], 1.5));
	shader.setFloat("ks", stofOrElse(properties["Ks"], 0));
	shader.setFloat("q", stofOrElse(properties["Ns"], 0));

	shader.setVec3("lightPos", -2.0f, 10.0f, 3.0f);
	shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);

	vector<glm::vec3> controlPoints = generateControlPoints(curvesFile);

	Bezier bezier;
	bezier.setControlPoints(controlPoints);
	bezier.setShader(&shader);
	bezier.generateCurve(1500);

	int nbCurvePoints = bezier.getNbCurvePoints();
	int i = 0;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat)glfwGetTime();

		model = glm::mat4(1);

		model = glm::translate(model, bezier.getPointOnCurve(i));

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

		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		shader.setMat4("view", glm::value_ptr(view));

		shader.setVec3("cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);

		model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));

		shader.setMat4("model", glm::value_ptr(model));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texID);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, verticesSize);

		glBindVertexArray(0);

		i = (i + 1) % nbCurvePoints;

		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);

	glfwTerminate();

	return 0;
}

void readMaterialsFile(string filename, map<string, string>& properties)
{
	ifstream file(filename);

	if (!file)
	{
		cout << "Unable to open the file: " << filename << endl;
		return;
	}

	string line;

	while (getline(file, line))
	{
		istringstream iss(line);
		vector<string> row(istream_iterator<string>{iss}, istream_iterator<string>{});

		if (row.empty())
		{
			continue;
		}

		properties[row[0]] = row[1];
	}

	file.close();
}

int loadTexture(string path)
{
	GLuint texID;

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3) //jpg, bmp
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else //png
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}


float stofOrElse(string value, float def)
{
	if (value.empty())
	{
		return def;
	}

	try
	{
		return stof(value);
	}
	catch (const exception& e)
	{
		cout << "Error converting string to float: " << e.what() << endl;
		return def;
	}
}



void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		rotateX = true;
		rotateY = false;
		rotateZ = false;
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

	float cameraSpeed = 0.01f;

	if (action == GLFW_REPEAT)
	{
		if (key == GLFW_KEY_W)
		{
			cameraPos += cameraFront * cameraSpeed;
		}

		if (key == GLFW_KEY_A)
		{
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		}

		if (key == GLFW_KEY_S)
		{
			cameraPos -= cameraFront * cameraSpeed;
		}

		if (key == GLFW_KEY_D)
		{
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		}
	}
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	static bool defaultMouse = true;
	static double lastX = xpos;
	static double lastY = ypos;

	if (defaultMouse)
	{
		lastX = xpos;
		lastY = ypos;
		defaultMouse = false;
		return;
	}

	double offsetX = xpos - lastX;
	double offsetY = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	offsetX *= sensitivity;
	offsetY *= sensitivity;

	pitch += offsetY;
	yaw += offsetX;

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


int setupGeometry()
{
	vector<float> vertices = parseObjToVertices(objFile);

	verticesSize = vertices.size();

	GLuint VBO, VAO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return VAO;
}


vector<glm::vec3> generateControlPoints(string filename)
{
	ifstream file(filename);
	vector<glm::vec3> points;

	if (file.is_open()) {
		string line;

		while (getline(file, line)) {
			vector<string> row = splitString(line, ',');

			if (row.empty()) {
				continue;
			}

			glm::vec3 point;
			point.x = stofOrElse(row[0], 0.0f);
			point.y = stofOrElse(row[1], 0.0f);
			point.z = stofOrElse(row[2], 0.0f);

			points.push_back(point);
		}

		file.close();
	}
	else {
		cout << "Unable to open the file: " << filename << endl;
	}

	return points;
}




