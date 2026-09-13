// Práctica 3: Modelado Geométrico y Cámara Sintética.
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <glew.h>
#include <glfw3.h>

// GLM
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/random.hpp>

// Clases de apoyo
#include "Mesh.h"
#include "Shader.h"
#include "Sphere.h"
#include "Window.h"
#include "Camera.h"

using std::vector;

// Variables Globales
const float toRadians = 3.14159265f / 180.0f;
const float PI = 3.14159265f;
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

Camera camera;
Window mainWindow;
vector<Mesh*> meshList;
vector<Shader> shaderList;

// Arreglo especial para las 5 caras de la Pirámide Multicolor
Mesh* piramideCaras[5];

static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
static const char* vShaderColor = "shaders/shadercolor.vert";
Sphere sp = Sphere(1.0, 20, 20);

// Prototipos de funciones
void RenderizarCohete(glm::mat4 modelBase, GLuint uniformModel, GLuint uniformColor);
void RenderizarUnion8Piramides(glm::mat4 modelBase, GLuint uniformModel, GLuint uniformColor);

void CrearCubo()
{
	unsigned int cubo_indices[] = {
		0, 1, 2,  2, 3, 0,
		1, 5, 6,  6, 2, 1,
		7, 6, 5,  5, 4, 7,
		4, 0, 3,  3, 7, 4,
		4, 5, 1,  1, 0, 4,
		3, 2, 6,  6, 7, 3
	};

	GLfloat cubo_vertices[] = {
		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f
	};
	Mesh* cubo = new Mesh();
	cubo->CreateMesh(cubo_vertices, cubo_indices, 24, 36);
	meshList.push_back(cubo); // ÍNDICE 0
}

void CrearPiramideTriangular()
{
	unsigned int indices_piramide_triangular[] = {
		0, 1, 2,
		1, 3, 2,
		3, 0, 2,
		1, 0, 3
	};
	GLfloat vertices_piramide_triangular[] = {
		-0.5f, -0.5f,  0.0f,
		 0.5f, -0.5f,  0.0f,
		 0.0f,  0.5f, -0.25f,
		 0.0f, -0.5f, -0.5f
	};
	Mesh* piramidet = new Mesh();
	piramidet->CreateMesh(vertices_piramide_triangular, indices_piramide_triangular, 12, 12);
	meshList.push_back(piramidet); // ÍNDICE 1
}

void CrearCilindro(int res, float R) {
	int n, i;
	GLfloat dt = 2 * PI / res, x, z, y = -0.5f;

	vector<GLfloat> vertices;
	vector<unsigned int> indices;

	for (n = 0; n <= res; n++) {
		x = (n != res) ? R * cos(n * dt) : R * cos(0.0f);
		z = (n != res) ? R * sin(n * dt) : R * sin(0.0f);
		for (i = 0; i < 6; i++) {
			switch (i) {
			case 0: vertices.push_back(x); break;
			case 1: vertices.push_back(y); break;
			case 2: vertices.push_back(z); break;
			case 3: vertices.push_back(x); break;
			case 4: vertices.push_back(0.5f); break;
			case 5: vertices.push_back(z); break;
			}
		}
	}
	for (n = 0; n <= res; n++) {
		x = R * cos(n * dt); z = R * sin(n * dt);
		vertices.push_back(x); vertices.push_back(-0.5f); vertices.push_back(z);
	}
	for (n = 0; n <= res; n++) {
		x = R * cos(n * dt); z = R * sin(n * dt);
		vertices.push_back(x); vertices.push_back(0.5f); vertices.push_back(z);
	}

	for (i = 0; i < (int)vertices.size(); i++) indices.push_back(i);

	Mesh* cilindro = new Mesh();
	cilindro->CreateMeshGeometry(vertices, indices, vertices.size(), indices.size());
	meshList.push_back(cilindro); // ÍNDICE 2
}

void CrearCono(int res, float R) {
	int n, i;
	GLfloat dt = 2 * PI / res, x, z, y = -0.5f;

	vector<GLfloat> vertices;
	vector<unsigned int> indices;

	vertices.push_back(0.0f); vertices.push_back(0.5f); vertices.push_back(0.0f);

	for (n = 0; n <= res; n++) {
		x = R * cos(n * dt); z = R * sin(n * dt);
		vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
	}
	vertices.push_back(R * cos(0.0f)); vertices.push_back(-0.5f); vertices.push_back(R * sin(0.0f));

	for (i = 0; i < res + 2; i++) indices.push_back(i);

	Mesh* cono = new Mesh();
	cono->CreateMeshGeometry(vertices, indices, vertices.size(), res + 2);
	meshList.push_back(cono); // ÍNDICE 3
}

void CrearPiramideCuadrangular()
{
	unsigned int piramidecuadrangular_indices[] = {
		0, 3, 4,
		3, 2, 4,
		2, 1, 4,
		1, 0, 4,
		0, 1, 2,
		0, 2, 3
	};
	GLfloat piramidecuadrangular_vertices[] = {
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		 0.0f,  0.5f,  0.0f
	};
	Mesh* piramidec = new Mesh();
	piramidec->CreateMesh(piramidecuadrangular_vertices, piramidecuadrangular_indices, 15, 18);
	meshList.push_back(piramidec); // ÍNDICE 4
}

void CrearPiramideEsquina()
{
	unsigned int indices[] = {
		0, 1, 2,
		0, 3, 1,
		0, 2, 3,
		1, 3, 2
	};

	GLfloat vertices[] = {
		 0.0f,  0.0f,  0.0f,
		-1.0f,  0.0f,  0.0f,
		 0.0f, -1.0f,  0.0f,
		 0.0f,  0.0f, -1.0f
	};

	Mesh* pEsquina = new Mesh();
	pEsquina->CreateMesh(vertices, indices, 12, 12);
	meshList.push_back(pEsquina); // ÍNDICE 5
}

// Inicializa 5 objetos Mesh independientes para cada cara de la pirámide
void CrearPiramideMulticolor()
{
	GLfloat v0[] = { 0.5f, -0.5f,  0.5f };
	GLfloat v1[] = { 0.5f, -0.5f, -0.5f };
	GLfloat v2[] = { -0.5f, -0.5f, -0.5f };
	GLfloat v3[] = { -0.5f, -0.5f,  0.5f };
	GLfloat v4[] = { 0.0f,  0.5f,  0.0f };

	unsigned int indTriangulos[] = { 0, 1, 2 };

	// 1. Cara Frontal
	GLfloat vert1[] = { v0[0],v0[1],v0[2], v3[0],v3[1],v3[2], v4[0],v4[1],v4[2] };
	piramideCaras[0] = new Mesh();
	piramideCaras[0]->CreateMesh(vert1, indTriangulos, 9, 3);

	// 2. Cara Izquierda
	GLfloat vert2[] = { v3[0],v3[1],v3[2], v2[0],v2[1],v2[2], v4[0],v4[1],v4[2] };
	piramideCaras[1] = new Mesh();
	piramideCaras[1]->CreateMesh(vert2, indTriangulos, 9, 3);

	// 3. Cara Trasera
	GLfloat vert3[] = { v2[0],v2[1],v2[2], v1[0],v1[1],v1[2], v4[0],v4[1],v4[2] };
	piramideCaras[2] = new Mesh();
	piramideCaras[2]->CreateMesh(vert3, indTriangulos, 9, 3);

	// 4. Cara Derecha
	GLfloat vert4[] = { v1[0],v1[1],v1[2], v0[0],v0[1],v0[2], v4[0],v4[1],v4[2] };
	piramideCaras[3] = new Mesh();
	piramideCaras[3]->CreateMesh(vert4, indTriangulos, 9, 3);

	// 5. Base Cuadrada
	GLfloat vertBase[] = { v0[0],v0[1],v0[2], v1[0],v1[1],v1[2], v2[0],v2[1],v2[2], v3[0],v3[1],v3[2] };
	unsigned int indBase[] = { 0, 1, 2, 0, 2, 3 };
	piramideCaras[4] = new Mesh();
	piramideCaras[4]->CreateMesh(vertBase, indBase, 12, 6);
}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);

	Shader* shader2 = new Shader();
	shader2->CreateFromFiles(vShaderColor, fShader);
	shaderList.push_back(*shader2);
}

// ------------------------------------------------------------------
// PUNTO 1: COHETE ESPACIAL
// ------------------------------------------------------------------
void RenderizarCohete(glm::mat4 modelBase, GLuint uniformModel, GLuint uniformColor)
{
	glm::mat4 model;
	glm::vec3 color;

	// 1. Fuselaje / Cuerpo Principal (Cilindro Blanco)
	model = glm::translate(modelBase, glm::vec3(-3.5f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.85f, 2.8f, 0.85f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	color = glm::vec3(0.9f, 0.9f, 0.95f);
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	meshList[2]->RenderMeshGeometry();

	// 2. Ojiva / Punta (Cono Rojo)
	model = glm::translate(modelBase, glm::vec3(-3.5f, 3.1f, 0.0f));
	model = glm::scale(model, glm::vec3(0.43f, 1.2f, 0.43f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	color = glm::vec3(0.9f, 0.15f, 0.15f);
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	meshList[3]->RenderMeshGeometry();

	// 3. Ventana Escotilla (Esfera Azul Cian)
	model = glm::translate(modelBase, glm::vec3(-3.5f, 1.8f, 0.86f));
	model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	color = glm::vec3(0.1f, 0.75f, 0.95f);
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	sp.render();

	// 4. Aletas de Estabilización (4 Pirámides a 90°)
	for (int i = 0; i < 4; i++) {
		model = glm::translate(modelBase, glm::vec3(-3.5f, -0.1f, 0.0f));
		model = glm::rotate(model, glm::radians(i * 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(0.6f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 1.2f, 0.25f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.2f, 0.35f, 0.85f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[4]->RenderMesh();
	}

	// 5. Motor Base (Cubo Gris Oscuro)
	model = glm::translate(modelBase, glm::vec3(-3.5f, -0.65f, 0.0f));
	model = glm::scale(model, glm::vec3(0.7f, 0.3f, 0.7f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	color = glm::vec3(0.2f, 0.2f, 0.25f);
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	meshList[0]->RenderMesh();

	// 6. Flama Propulsora (Cono Naranja Invertido)
	model = glm::translate(modelBase, glm::vec3(-3.5f, -1.05f, 0.0f));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.3f, 0.5f, 0.3f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	color = glm::vec3(1.0f, 0.45f, 0.0f);
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	meshList[3]->RenderMeshGeometry();
}

// ------------------------------------------------------------------
// PUNTO 2: FIGURA DE UNIÓN DE 8 PIRÁMIDES
// ------------------------------------------------------------------
void RenderizarUnion8Piramides(glm::mat4 modelBase, GLuint uniformModel, GLuint uniformColor)
{
	glm::vec3 rotaciones[8] = {
		glm::vec3(45.0f,   45.0f,   0.0f),
		glm::vec3(45.0f, -135.0f,   0.0f),
		glm::vec3(-45.0f,   45.0f,   0.0f),
		glm::vec3(-45.0f, -135.0f,   0.0f),
		glm::vec3(135.0f,   45.0f,   0.0f),
		glm::vec3(135.0f, -135.0f,   0.0f),
		glm::vec3(-135.0f,   45.0f,   0.0f),
		glm::vec3(-135.0f, -135.0f,   0.0f)
	};

	glm::vec3 coloresCaras[4] = {
		glm::vec3(1.0f, 0.0f, 0.0f), // Roja
		glm::vec3(0.0f, 1.0f, 0.0f), // Verde
		glm::vec3(1.0f, 1.0f, 0.0f), // Amarilla
		glm::vec3(1.0f, 0.0f, 1.0f)  // Magenta
	};
	glm::vec3 colorBaseAzul = glm::vec3(0.0f, 0.0f, 1.0f); // Base Azul

	for (int i = 0; i < 8; i++)
	{
		glm::mat4 modelPiramide = glm::translate(modelBase, glm::vec3(3.0f, 0.8f, 0.0f));

		modelPiramide = glm::rotate(modelPiramide, glm::radians(rotaciones[i].x), glm::vec3(1.0f, 0.0f, 0.0f));
		modelPiramide = glm::rotate(modelPiramide, glm::radians(rotaciones[i].y), glm::vec3(0.0f, 1.0f, 0.0f));
		modelPiramide = glm::rotate(modelPiramide, glm::radians(rotaciones[i].z), glm::vec3(0.0f, 0.0f, 1.0f));

		modelPiramide = glm::translate(modelPiramide, glm::vec3(0.0f, 0.45f, 0.0f));
		modelPiramide = glm::scale(modelPiramide, glm::vec3(0.85f, 0.85f, 0.85f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelPiramide));

		// (c + i) % 4 alterna la distribución de color para que los 4 colores aparezcan mezclados desde cualquier frente
		for (int c = 0; c < 4; c++) {
			int colorIdx = (c + i) % 4;
			glUniform3fv(uniformColor, 1, glm::value_ptr(coloresCaras[colorIdx]));
			piramideCaras[c]->RenderMesh();
		}

		// Base cuadrada azul
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorBaseAzul));
		piramideCaras[4]->RenderMesh();
	}
}

int main()
{
	mainWindow = Window(800, 600);
	mainWindow.Initialise();
	glEnable(GL_DEPTH_TEST);

	CrearCubo();                 // Mesh 0
	CrearPiramideTriangular();   // Mesh 1
	CrearCilindro(30, 1.0f);     // Mesh 2
	CrearCono(25, 2.0f);         // Mesh 3
	CrearPiramideCuadrangular(); // Mesh 4
	CrearPiramideEsquina();      // Mesh 5

	// Crear mallas independientes para las 5 caras de la pirámide multicolor
	CrearPiramideMulticolor();

	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 2.0f, 8.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -10.0f, 0.2f, 0.3f);

	GLuint uniformProjection = 0;
	GLuint uniformModel = 0;
	GLuint uniformView = 0;
	GLuint uniformColor = 0;

	glm::mat4 projection = glm::perspective(glm::radians(60.0f), mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

	sp.init();
	sp.load();

	glm::mat4 model(1.0f);
	glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);

	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderList[0].useShader();
		uniformModel = shaderList[0].getModelLocation();
		uniformProjection = shaderList[0].getProjectLocation();
		uniformView = shaderList[0].getViewLocation();
		uniformColor = shaderList[0].getColorLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

		// Piso
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -2.2f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 0.05f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.04f, 0.04f, 0.06f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();

		// Matriz base de rotación (Teclas E, R, T)
		glm::mat4 modelBase = glm::mat4(1.0f);
		modelBase = glm::rotate(modelBase, glm::radians(mainWindow.getrotax()), glm::vec3(1.0f, 0.0f, 0.0f));
		modelBase = glm::rotate(modelBase, glm::radians(mainWindow.getrotay()), glm::vec3(0.0f, 1.0f, 0.0f));
		modelBase = glm::rotate(modelBase, glm::radians(mainWindow.getrotaz()), glm::vec3(0.0f, 0.0f, 1.0f));

		// 1. DIBUJAR COHETE ESPACIAL
		RenderizarCohete(modelBase, uniformModel, uniformColor);

		// 2. DIBUJAR FIGURA DE UNIÓN DE 8 PIRÁMIDES (MULTICOLOR)
		RenderizarUnion8Piramides(modelBase, uniformModel, uniformColor);

		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}