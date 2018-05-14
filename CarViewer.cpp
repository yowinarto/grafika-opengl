#define M_PI 3.14159265359
#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ImageLoader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <Shader.h>
#include <common/texture.hpp>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <algorithm>

struct Particle {
	glm::vec3 pos, speed;
	unsigned char code;
	unsigned char r, g, b, a; // Color
	float size, angle, weight;
	float life; // Remaining life of the particle. if <0 : dead and unused.
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};

const int MaxParticles = 20000;
Particle ParticlesContainer[MaxParticles];
int LastUsedParticle = 0;

// Finds a Particle in ParticlesContainer which isn't used yet.
// (i.e. life < 0);
int FindUnusedParticle() {

	for (int i = LastUsedParticle; i<MaxParticles; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = i;
			return i;
		}
	}

	for (int i = 0; i<LastUsedParticle; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = i;
			return i;
		}
	}

	return 0; // All particles are taken, override the first one
}

void SortParticles() {
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);
void DrawCar(Shader&);
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

/*
float vertices[] = {
	-0.5f, -0.5f, -0.5f, 0.5f, 0.0f,
	0.5f, -0.5f, -0.5f,  0.5f, 0.0f,
	0.5f,  0.5f, -0.5f,  0.5f, 0.0f,
	0.5f,  0.5f, -0.5f,  0.5f, 0.0f,
	-0.5f,  0.5f, -0.5f, 0.5f, 0.0f,
	-0.5f, -0.5f, -0.5f, 0.5f, 0.0f,

	-0.5f, -0.5f,  0.5f, 0.5f, 0.0f,
	0.5f, -0.5f,  0.5f,  0.5f, 0.0f,
	0.5f,  0.5f,  0.5f,  0.5f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.5f, 1.0f,
	-0.5f,  0.5f,  0.5f, 0.5f, 1.0f,
	-0.5f, -0.5f,  0.5f, 0.5f, 0.0f,

	-0.5f,  0.5f,  0.5f,  0.5f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.5f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.5f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.5f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.5f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.5f, 0.0f,

	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f, 0.5f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.5f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.5f, 0.0f,
	0.5f, -0.5f,  0.5f,  0.5f, 0.0f,
	-0.5f, -0.5f,  0.5f, 0.5f, 0.0f,
	-0.5f, -0.5f, -0.5f, 0.5f, 1.0f,

	-0.5f,  0.5f, -0.5f, 0.8f, 1.0f,
	0.5f,  0.5f, -0.5f,  0.8f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.8f, 0.0f,
	0.5f,  0.5f,  0.5f,  0.8f, 0.0f,
	-0.5f,  0.5f,  0.5f, 0.8f, 0.0f,
	-0.5f,  0.5f, -0.5f, 0.8f, 1.0f
};
*/
//---------------------------------------------------------------------------
const int na = 36;        // vertex grid size
const int nb = 18;
const int na3 = na * 3;     // line in grid size
const int nn = nb * na3;    // whole grid size
GLfloat sphere_pos[nn]; // vertex
GLfloat sphere_nor[nn]; // normal
						//GLfloat sphere_col[nn];   // color
GLuint  sphere_ix[na*(nb - 1) * 6];    // indices
GLuint sphere_vbo[4] = { -1,-1,-1,-1 };
GLuint sphere_vao[4] = { -1,-1,-1,-1 };

void sphere_init()
{
	// generate the sphere data
	GLfloat x, y, z, a, b, da, db, r = 3.5;
	int ia, ib, ix, iy;
	da = 2.0*M_PI / GLfloat(na);
	db = M_PI / GLfloat(nb - 1);
	// [Generate sphere point data]
	// spherical angles a,b covering whole sphere surface
	for (ix = 0, b = -0.5*M_PI, ib = 0; ib<nb; ib++, b += db)
		for (a = 0.0, ia = 0; ia<na; ia++, a += da, ix += 3)
		{
			// unit sphere
			x = cos(b)*cos(a);
			y = cos(b)*sin(a);
			z = sin(b);
			sphere_pos[ix + 0] = x * r;
			sphere_pos[ix + 1] = y * r;
			sphere_pos[ix + 2] = z * r;
			sphere_nor[ix + 0] = x;
			sphere_nor[ix + 1] = y;
			sphere_nor[ix + 2] = z;
		}
	// [Generate GL_TRIANGLE indices]
	for (ix = 0, iy = 0, ib = 1; ib<nb; ib++)
	{
		for (ia = 1; ia<na; ia++, iy++)
		{
			// first half of QUAD
			sphere_ix[ix] = iy;      ix++;
			sphere_ix[ix] = iy + 1;    ix++;
			sphere_ix[ix] = iy + na;   ix++;
			// second half of QUAD
			sphere_ix[ix] = iy + na;   ix++;
			sphere_ix[ix] = iy + 1;    ix++;
			sphere_ix[ix] = iy + na + 1; ix++;
		}
		// first half of QUAD
		sphere_ix[ix] = iy;       ix++;
		sphere_ix[ix] = iy + 1 - na;  ix++;
		sphere_ix[ix] = iy + na;    ix++;
		// second half of QUAD
		sphere_ix[ix] = iy + na;    ix++;
		sphere_ix[ix] = iy - na + 1;  ix++;
		sphere_ix[ix] = iy + 1;     ix++;
		iy++;
	}
	// [VAO/VBO stuff]
	GLuint i;
	glGenVertexArrays(4, sphere_vao);
	glGenBuffers(4, sphere_vbo);
	glBindVertexArray(sphere_vao[0]);
	i = 0; // vertex
	glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo[i]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_pos), sphere_pos, GL_STATIC_DRAW);
	glEnableVertexAttribArray(i);
	glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, 0, 0);
	i = 1; // indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_vbo[i]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_ix), sphere_ix, GL_STATIC_DRAW);
	glEnableVertexAttribArray(i);
	glVertexAttribPointer(i, 4, GL_UNSIGNED_INT, GL_FALSE, 0, 0);
	i = 2; // normal
	glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo[i]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_nor), sphere_nor, GL_STATIC_DRAW);
	glEnableVertexAttribArray(i);
	glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, 0, 0);
	/*
	i=3; // color
	glBindBuffer(GL_ARRAY_BUFFER,sphere_vbo[i]);
	glBufferData(GL_ARRAY_BUFFER,sizeof(sphere_col),sphere_col,GL_STATIC_DRAW);
	glEnableVertexAttribArray(i);
	glVertexAttribPointer(i,3,GL_FLOAT,GL_FALSE,0,0);
	*/
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
}
void sphere_exit()
{
	glDeleteVertexArrays(4, sphere_vao);
	glDeleteBuffers(4, sphere_vbo);
}
void sphere_draw(Shader& ourShader)
{
	//glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	ourShader.use();
	unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
	unsigned int viewLoc = glGetUniformLocation(ourShader.ID, "view");
	unsigned int projectionLoc = glGetUniformLocation(ourShader.ID, "projection");
	float radius = 12.0f;
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);

	glm::mat4 projection = glm::mat4(1.0f);;
	//model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
	view = glm::lookAt(cameraFront * radius, glm::vec3(0.0, 0.0, -0.5), cameraUp);
	projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	// retrieve the matrix uniform locations

	// pass them to the shaders (3 different ways)
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glm::vec3 spherePositions[] = {
		glm::vec3(1.6f, -1.25f, 2.1f),
		glm::vec3(-1.6f,-1.25f, -3.1f),
		glm::vec3(1.6f, -1.25f, -3.1f),
		glm::vec3(-1.6f,-1.25f, 2.1f)
	};
	glm::vec3 sphereScales = glm::vec3(0.185f, 0.185f, 0.1f);

	for (int i = 0; i < 4; i++) {
		float sphereRotation[] = {
			glm::radians(90.0f),
			glm::radians(-90.0f)
		};
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, spherePositions[i]);
		model = glm::rotate(model, sphereRotation[i%2], glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, (float)glfwGetTime()*5, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, sphereScales);
		ourShader.setMat4("model", model);

		glBindVertexArray(sphere_vao[0]);
		//  glDrawArrays(GL_POINTS,0,sizeof(sphere_pos)/sizeof(GLfloat));                   // POINTS ... no indices for debug
		glDrawElements(GL_TRIANGLES, sizeof(sphere_ix) / sizeof(GLuint), GL_UNSIGNED_INT, 0);    // indices (choose just one line not both !!!)
		glBindVertexArray(0);
	}
}

float vertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,  
	0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f,  1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f,  1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f,  1.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

	0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f
};
GLFWwindow* window;
int main() {
	

	if (!glfwInit())
		return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Car Viewer", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	if (glewInit() != GLEW_OK)
		std::cout << "Error GLEW Init" << std::endl;
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	sphere_init();
	Shader prog("car.vert", "car.frag");
	Shader prog2("car.vert", "ban.frag");
	Shader lightingShader("light.vert", "light.frag");
	Shader particle("Particle.vertexshader", "Particle.fragmentshader");

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	static GLfloat* g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	static GLubyte* g_particule_color_data = new GLubyte[MaxParticles * 4];

	for (int i = 0; i<MaxParticles; i++) {
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}

	static const GLfloat g_vertex_buffer_data[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f,
		0.5f,  0.5f, 0.0f,
	};

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	GLuint billboard_vertex_buffer;
	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// The VBO containing the positions and sizes of the particles
	GLuint particles_position_buffer;
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	GLuint particles_color_buffer;
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glVertexAttribPointer(
		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : positions of particles' centers
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		4,                                // size : x + y + z + size => 4
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// 3rd attribute buffer : particles' colors
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glVertexAttribPointer(
		2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		4,                                // size : r + g + b + a => 4
		GL_UNSIGNED_BYTE,                 // type
		GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint Texture = loadDDS("particle.DDS");
	ImageLoader texture1("container.jpg");
	ImageLoader texture2("awesomeface.jpg");
	ImageLoader particleTexture("particle.png");
	prog.use();
	prog.setInt("texture1", 0);
	prog.setInt("texture2", 1);
	prog2.use();
	prog2.setInt("texture1", 0);
	prog2.setInt("texture2", 0);
	lightingShader.use();
	lightingShader.setInt("texture1", 0);
	lightingShader.setInt("texture2", 0);
	lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
	lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos(2.5f, 1.5f, 1.0f);
	lightingShader.setVec3("lightPos", lightPos);
	unsigned int modelLoc = glGetUniformLocation(prog.ID, "model");
	unsigned int viewLoc = glGetUniformLocation(prog.ID, "view");
	unsigned int projectionLoc = glGetUniformLocation(prog.ID, "projection");
	// Vertex shader
	GLuint CameraRight_worldspace_ID = glGetUniformLocation(particle.ID, "CameraRight_worldspace");
	GLuint CameraUp_worldspace_ID = glGetUniformLocation(particle.ID, "CameraUp_worldspace");
	GLuint ViewProjMatrixID = glGetUniformLocation(particle.ID, "VP");

	// fragment shader
	GLuint TextureID = glGetUniformLocation(particle.ID, "myTextureSampler");
	double lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		glClearColor(0.1f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		double currentTime = glfwGetTime();
		double delta = currentTime - lastTime;
		lastTime = currentTime;

		
		//glActiveTexture(GL_TEXTURE1);
		//texture2.use();
		

		prog.use();

		float radius = 12.0f;
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		
		glm::mat4 projection = glm::mat4(1.0f);
		//model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
		view = glm::lookAt(cameraFront * radius, glm::vec3(0.0, 0.0, -0.5), cameraUp);
		projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		// retrieve the matrix uniform locations

		// pass them to the shaders (3 different ways)
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		//sprog.setMat4("projection", projection);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		texture1.use();

		//Light
		lightingShader.use();
		lightingShader.setVec3("viewPos", cameraFront);

		// render box
		sphere_draw(prog);
		lightingShader.use();
		glBindVertexArray(VAO);
		DrawCar(lightingShader);
		//Particle
		glm::vec3 CameraPosition(glm::inverse(view)[3]);
		glm::mat4 ViewProjectionMatrix = projection * view;
		int rainparticles = (int)(delta*1000.0);
		if (rainparticles > (int)(0.016f*1000.0))
			rainparticles = (int)(0.016f*1000.0);
		for (int i = 0; i<rainparticles; i++) {
			int particleIndex = FindUnusedParticle();
			ParticlesContainer[particleIndex].code = 0; // This particle will live 5 seconds.
			ParticlesContainer[particleIndex].life = 2.0f; // This particle will live 5 seconds.
			ParticlesContainer[particleIndex].pos = glm::vec3(rand() % 10 - 5.0f, 10.0f, rand() % 10 - 5.0f);

			float spread = 0.5f;
			glm::vec3 maindir = glm::vec3(0.0f, -10.0f, 0.0f);
			// Very bad way to generate a random direction; 
			// See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
			// combined with some user-controlled parameters (main direction, spread, etc)
			glm::vec3 randomdir = glm::vec3(
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f
			);
			ParticlesContainer[particleIndex].speed = maindir + randomdir * spread;

			int col = rand() % 25 + 150;
			// Very bad way to generate a random color
			ParticlesContainer[particleIndex].r = col;
			ParticlesContainer[particleIndex].g = col;
			ParticlesContainer[particleIndex].b = rand() % 30 + 180;
			ParticlesContainer[particleIndex].a = (rand() % 700) / 3;

			ParticlesContainer[particleIndex].size = (rand() % 50) / 2000.0f + 0.1f;

		}

		int newparticles = (int)(delta*5000.0);
		if (newparticles > (int)(0.016f*5000.0))
			newparticles = (int)(0.016f*5000.0);
		for (int i = 0; i<newparticles; i++) {
			int particleIndex = FindUnusedParticle();
			ParticlesContainer[particleIndex].code = 1; // This particle will live 5 seconds.
			ParticlesContainer[particleIndex].life = 2.0f; // This particle will live 5 seconds.
			ParticlesContainer[particleIndex].pos = glm::vec3(-1.0f, -1.0f, 4.0f);

			float spread = 0.5f;
			glm::vec3 maindir = glm::vec3(0.0f, 0.0f, 3.0f);
			glm::vec3 randomdir = glm::vec3(
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f
			);

			ParticlesContainer[particleIndex].speed = maindir + randomdir * spread;


			// Very bad way to generate a random color
			ParticlesContainer[particleIndex].r = 80;
			ParticlesContainer[particleIndex].g = 80;
			ParticlesContainer[particleIndex].b = 80;
			ParticlesContainer[particleIndex].a = (rand() % 512) / 3;

			ParticlesContainer[particleIndex].size = (rand() % 1000) / 2000.0f + 0.1f;

		}
		int ParticlesCount = 0;
		float smokeColor = 0;
		for (int i = 0; i<MaxParticles; i++) {

			Particle& p = ParticlesContainer[i]; // shortcut
			
			if (p.life > 0.0f) {

				// Decrease life
				p.life -= delta;
				if (p.life > 0.0f) {

					if (p.code == 1) {
						// Simulate simple physics : gravity only, no collisions
						p.speed += glm::vec3(0.0f, 9.81f, 0.0f) * (float)delta * 0.5f;
						//smokeColor += 0.1f;
						//p.r += smokeColor;
						//p.g += smokeColor;
						//p.b += smokeColor;
						p.a -= delta * 2.5f;
					} else
						p.speed += glm::vec3(0.0f, 0.0f, 0.0f) * (float)delta * 0.5f;
					p.pos += p.speed * (float)delta;
					p.cameradistance = glm::length2(p.pos - CameraPosition);
					//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

					// Fill the GPU buffer
					g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
					g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
					g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

					g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

					g_particule_color_data[4 * ParticlesCount + 0] = p.r;
					g_particule_color_data[4 * ParticlesCount + 1] = p.g;
					g_particule_color_data[4 * ParticlesCount + 2] = p.b;
					g_particule_color_data[4 * ParticlesCount + 3] = p.a;

				}
				else {
					// Particles that just died will be put at the end of the buffer in SortParticles();
					p.cameradistance = -1.0f;
				}

				ParticlesCount++;

			}
		}

		SortParticles();
		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		particle.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glUniform1i(TextureID, 0);
		glUniform3f(CameraRight_worldspace_ID, view[0][0], view[1][0], view[2][0]);
		glUniform3f(CameraUp_worldspace_ID, view[0][1], view[1][1], view[2][1]);
		glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);
		

		glBindVertexArray(VertexArrayID);
		// These functions are specific to glDrawArrays*Instanced*.
		// The first parameter is the attribute buffer we're talking about.
		// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
		// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
		glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
		glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
		glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

									 // Draw the particules !
									 // This draws many times a small triangle_strip (which looks like a quad).
									 // This is equivalent to :
									 // for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
									 // but faster.
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);



		//glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		fov = fov + 0.1f >= 60.0f ? 60.0f : fov + 0.1f;
	else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		fov = fov - 0.1f <= 1.0f ? 1.0f : fov - 0.1f;
		
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	//int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = -sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

void DrawCar(Shader& ourShader) {
	ourShader.use();
	unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
	unsigned int viewLoc = glGetUniformLocation(ourShader.ID, "view");
	unsigned int projectionLoc = glGetUniformLocation(ourShader.ID, "projection");
	float radius = 12.0f;
	glm::mat4 model1 = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);

	glm::mat4 projection = glm::mat4(1.0f);;
	//model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
	view = glm::lookAt(cameraFront * radius, glm::vec3(0.0, 0.0, -0.5), cameraUp);
	projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	// retrieve the matrix uniform locations

	// pass them to the shaders (3 different ways)
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));


	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.5f,  2.375f),
		glm::vec3(0.0f,  -1.0f, 3.5f),
		glm::vec3(0.0f,  -1.0f, 1.0f),
		glm::vec3(0.0f, 0, -1.0f),
		glm::vec3(0.0f, 0.0f, -3.8f),
		glm::vec3(0.0f, -1.0f, -4.5f),
		glm::vec3(0.0f,  -1.0f, -3.0f),
		glm::vec3(0.0f,  -1.0f, 2.1f),
		glm::vec3(0.0f, 0.725f, -2.72f)
	};
	glm::vec3 cubeScales[] = {
		glm::vec3(3.5f,  2.0f,  3.75f),
		glm::vec3(3.5f,  1.0f, 1.5f),
		glm::vec3(3.5f,  1.0f, 1.0f),
		glm::vec3(3.5f,  3.0f, 3.0f),
		glm::vec3(3.5f,  1.0f, 2.6f),
		glm::vec3(3.5f,  1.0f, 1.75f),
		glm::vec3(2.5f,  1.0f, 1.25f),
		glm::vec3(2.5f,  1.0f, 1.35f),
		glm::vec3(3.5f, 0.75f, 1.5f)
	};
	glEnable(GL_DEPTH);

	for (unsigned int i = 0; i < 8; i++)
	{
		// calculate the model matrix for each object and pass it to shader before drawing
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, cubePositions[i]);
		float angle = 20.0f * i;
		model = glm::scale(model, cubeScales[i]);
		//model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
		ourShader.setMat4("model", model);

		glDrawArrays(GL_TRIANGLES, 0, 36);

	}

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, cubePositions[8]);
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, cubeScales[8]);
	ourShader.setMat4("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}