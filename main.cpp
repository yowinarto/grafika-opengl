/*#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <common/core/Engine.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Shader.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>

// Define some of the global variables we're using for this sample
GLuint program;
GLuint vao;

// This is the callback we'll be registering with GLFW for errors.
// It'll just print out the error to the STDERR stream.
void error_callback(int error, const char *description) {
  fputs(description, stderr);
}

// This is the callback we'll bse registering with GLFW for keyboard handling.
// The only thing we're doing here is setting up the window to close when we
// press ESC
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  } else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
    Engine::bgColor[0] = 0.5f;
    Engine::bgColor[1] = 0.3f;
    Engine::bgColor[2] =
        Engine::bgColor[2] > 1.0f ? 0.0f : Engine::bgColor[2] + 0.1;
  } else if (key == GLFW_KEY_B && action == GLFW_PRESS) {
    Engine::bgColor[0] = 0.0f;
    Engine::bgColor[1] = 0.0f;
    Engine::bgColor[2] = 0.0f;
  }
}

int main() {
	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	window = glfwCreateWindow(600, 600, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
		std::cout << "Error GLEW Init" << std::endl;
	cout << GL_VERSION << endl;
	float i = 0;
	
	Shader prog("vertDefaultShader.vert", "fragDefaultShader.frag");
	Shader circleProg("vertDefaultShader.vert", "fragCircleShader.frag");

	float vertices[] = {
		0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
		0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f
	};

	float vert[] = {
		-0.75f, -0.25f, 0.0f,
		-0.25f, -0.25f, 0.0f,
		-0.75f,  0.25f, 0.0f,
		-0.25f,  0.25f, 0.0f,
		0.75f, -0.25f, 0.0f,
		0.25f, -0.25f, 0.0f,
		0.75f,  0.25f, 0.0f,
		0.25f,  0.25f, 0.0f,
		
		0.85f, 0.0f, 0.0f,
		0.85f, 0.25f, 0.0f,
		-0.85f, 0.0f, 0.0f,
		-0.85f, 0.25f, 0.0f,
		
		0.4f, 0.25f, 0.0f,
		0.2f, 0.45f, 0.0f,
		-0.5f, 0.25f, 0.0f,
		-0.3f, 0.45f, 0.0f
	};

	unsigned int quadIndices[] = {
		2, 3, 0,
		0, 1, 3
	};

	unsigned int quadIndices2[] = {
		6, 7, 4,
		4, 5, 7
	};

	unsigned int carBody[] = {
		8, 9, 10,
		10, 11, 9
	};

	unsigned int carTop[] = {
		12, 13, 14,
		14, 15, 13
	};

	unsigned int quadVAO, quadVBO, quadEBO, circleVAO, circleVBO, circleEBO;
	unsigned int carBodyVAO, carBodyVBO, carBodyEBO, carTopVAO, carTopVBO, carTopEBO;
	//Generate all VAOs & VBOs
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glGenBuffers(1, &quadEBO);
	glGenVertexArrays(1, &circleVAO);
	glGenBuffers(1, &circleVBO);
	glGenBuffers(1, &circleEBO);

	glGenVertexArrays(1, &carBodyVAO);
	glGenBuffers(1, &carBodyVBO);
	glGenBuffers(1, &carBodyEBO);
	glGenVertexArrays(1, &carTopVAO);
	glGenBuffers(1, &carTopVBO);
	glGenBuffers(1, &carTopEBO);

	//Circle 1
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Circle 2
	glBindVertexArray(circleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices2), quadIndices2, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Car Top
	glBindVertexArray(carTopVAO);
	glBindBuffer(GL_ARRAY_BUFFER, carTopVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, carTopEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(carTop), carTop, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Car Body
	glBindVertexArray(carBodyVAO);
	glBindBuffer(GL_ARRAY_BUFFER, carBodyVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, carBodyEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(carBody), carBody, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//unsigned int colorLoc = glGetUniformLocation(prog.ID, "customColor");
	unsigned int transLoc = glGetUniformLocation(prog.ID, "transform");
	unsigned int transformLoc = glGetUniformLocation(circleProg.ID, "transform");
	unsigned int circlePosLoc = glGetUniformLocation(circleProg.ID, "customPos");
	unsigned int circleViewLoc = glGetUniformLocation(circleProg.ID, "viewDimensions");
	unsigned int circleOuterRadiusLoc = glGetUniformLocation(circleProg.ID, "outerRadius");

	int frame = 100000;
	int n = 0;
	clock_t begin = clock();

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.1f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		prog.use();
		glm::mat4 trans = glm::mat4(1.0f);
		glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(trans));
		glBindVertexArray(carTopVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(carBodyVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		circleProg.use();
		glm::mat4 circleTrans = glm::mat4(1.0f);
		float circleScale = cos(glfwGetTime());
		circleTrans = glm::translate(circleTrans, glm::vec3(-0.5f, 0.0f, 0.0f));
		circleTrans = glm::rotate(circleTrans, glm::radians((float)glfwGetTime()*90.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		circleTrans = glm::translate(circleTrans, glm::vec3(0.5f, 0.0f, 0.0f));
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(circleTrans));
		glUniform2f(circleViewLoc, 600.0f, 600.0f);
		glUniform1f(circleOuterRadiusLoc, 0.15f);

		glUniform3f(circlePosLoc, -0.5f, 0.0f, 0.0f);
		glBindVertexArray(quadVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glm::mat4 circle2Trans = glm::mat4(1.0f);
		circle2Trans = glm::translate(circle2Trans, glm::vec3(0.5f, 0.0f, 0.0f));
		circle2Trans = glm::rotate(circle2Trans, glm::radians((float)glfwGetTime()*90.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		circle2Trans = glm::translate(circle2Trans, glm::vec3(-0.5f, 0.0f, 0.0f));
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(circle2Trans));
		glUniform3f(circlePosLoc, 0.5f, 0.0f, 0.0f);
		glBindVertexArray(circleVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);


		glfwPollEvents();
		n++;
	}
	clock_t end = clock();
	cout << double(end - begin) / 1000;
	glfwTerminate();
	cin.get();
	return 0;
}*/