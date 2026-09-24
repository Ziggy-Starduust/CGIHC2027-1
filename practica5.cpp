/*Práctica 5: Optimización y Carga de Modelos*/
//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_m.h"
#include "Camera.h"
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<MeshColor*> meshListColor;
std::vector<MeshModel*> meshListModel;
std::vector<Shader> shaderList;
Camera camera;

// Lista de Modelos a importar
Model Rover_M;

// Partes del Rover por separado
Model cuerpoModel;
Model brazoModel;
Model ruedaDelIzqModel;
Model ruedaDelDerModel;
Model ruedaMedIzqModel;
Model ruedaMedDerModel;
Model ruedaTraIzqModel;
Model ruedaTraDerModel;


// Lista de Skybox a crear
Skybox skybox;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// Variables globales para la animación de rotación limitada a 45°
float rotacionPatas = 0.0f;

// Vertex Shader
static const char* vShader = "shaders/shader_m.vert";
// Fragment Shader
static const char* fShader = "shaders/shader_m.frag";

void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	MeshModel* obj1 = new MeshModel();
	obj1->CreateMeshModel(vertices, indices, 32, 12);
	meshListModel.push_back(obj1);

	MeshModel* obj2 = new MeshModel();
	obj2->CreateMeshModel(vertices, indices, 32, 12);
	meshListModel.push_back(obj2);

	MeshModel* obj3 = new MeshModel();
	obj3->CreateMeshModel(floorVertices, floorIndices, 32, 6);
	meshListModel.push_back(obj3);
}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

int main()
{
	mainWindow = Window(1366, 768);
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.5f, 7.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.3f);

	// Cargar modelos individuales
	cuerpoModel = Model();
	cuerpoModel.LoadModel("Models/cuerpo.obj");

	brazoModel = Model();
	brazoModel.LoadModel("Models/brazo.obj");

	ruedaDelIzqModel = Model();
	ruedaDelIzqModel.LoadModel("Models/rueda_del_izq.obj");

	ruedaDelDerModel = Model();
	ruedaDelDerModel.LoadModel("Models/rueda_del_der.obj");

	ruedaMedIzqModel = Model();
	ruedaMedIzqModel.LoadModel("Models/rueda_med_izq.obj");

	ruedaMedDerModel = Model();
	ruedaMedDerModel.LoadModel("Models/rueda_med_der.obj");

	ruedaTraIzqModel = Model();
	ruedaTraIzqModel.LoadModel("Models/rueda_tra_izq.obj");

	ruedaTraDerModel = Model();
	ruedaTraDerModel.LoadModel("Models/rueda_tra_der.obj");

	// Crear Skybox
	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0, uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

	// Loop principal
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// CONTROL DE ROTACIÓN CON TECLAS (Teclas R y F)
		bool* keys = mainWindow.getsKeys();
		if (keys[GLFW_KEY_R]) {
			rotacionPatas += 0.25f;
		}
		if (keys[GLFW_KEY_F]) {
			rotacionPatas -= 0.25f;
		}

		// Limitar rotación a máximo 45° y mínimo -45°
		if (rotacionPatas > 45.0f)  rotacionPatas = 45.0f;
		if (rotacionPatas < -45.0f) rotacionPatas = -45.0f;

		// Limpiar pantalla
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformColor = shaderList[0].getColorLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

		// ------------------ DIBUJO DEL PISO ------------------
		color = glm::vec3(0.5f, 0.5f, 0.5f);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshListModel[2]->RenderMeshModel();

		// ------------------ DIBUJO DEL ROVER (JERARQUÍA) ------------------
		color = glm::vec3(0.2f, 0.5f, 0.8f); // Color para el rover
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		// ------------------ DIBUJO DEL ROVER (JERARQUÍA Y POSICIONAMIENTO) ------------------

		// 1. MATRIZ PADRE: CUERPO (Posición general del Rover en el mundo)
		glm::mat4 modelCuerpo = glm::mat4(1.0f);
		modelCuerpo = glm::translate(modelCuerpo, glm::vec3(0.0f, -1.0f, -1.5f));

		// Dibujar Cuerpo en azul
		color = glm::vec3(0.2f, 0.4f, 0.8f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelCuerpo));
		cuerpoModel.RenderModel();

		// 2. MATRIZ HIJO: BRAZO 
		glm::mat4 modelBrazo = modelCuerpo; // Hereda la posición del cuerpo
		// AJUSTA ESTOS TRES VALORES hasta que el brazo embone sobre el cuerpo:
		modelBrazo = glm::translate(modelBrazo, glm::vec3(-7.0f, 0.5f, -0.8f));

		// Dibujar Brazo de otro color (por ejemplo, verde o rojo para diferenciarlo)
		color = glm::vec3(0.8f, 0.2f, 0.2f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelBrazo));
		brazoModel.RenderModel();

		// 3. MATRICES HIJOS: PATAS Y RUEDAS (Con rotación de 45°)
		color = glm::vec3(0.1f, 0.1f, 0.1f); // Color oscuro para las llantas/patas
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		// Rueda Delantera Izquierda
		glm::mat4 modelRuedaDelIzq = modelCuerpo;
		// Aplicar la rotación directamente sobre el eje adecuado (ej. Z o X)
		modelRuedaDelIzq = glm::rotate(modelRuedaDelIzq, glm::radians(rotacionPatas), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelRuedaDelIzq));
		ruedaDelIzqModel.RenderModel();

		// Rueda Delantera Derecha
		glm::mat4 modelRuedaDelDer = modelCuerpo;
		modelRuedaDelDer = glm::rotate(modelRuedaDelDer, glm::radians(rotacionPatas), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelRuedaDelDer));
		ruedaDelDerModel.RenderModel();

		// Repetir el mismo proceso con las demás ruedas (med_izq, med_der, tra_izq, tra_der)...

		// Rueda Media Izquierda
		glm::mat4 modelRuedaMedIzq = modelCuerpo;
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelRuedaMedIzq));
		ruedaMedIzqModel.RenderModel();

		// Rueda Media Derecha
		glm::mat4 modelRuedaMedDer = modelCuerpo;
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelRuedaMedDer));
		ruedaMedDerModel.RenderModel();

		// Rueda Trasera Izquierda
		glm::mat4 modelRuedaTraIzq = modelCuerpo;
		modelRuedaTraIzq = glm::translate(modelRuedaTraIzq, glm::vec3(-2.0f, 0.35f, 3.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelRuedaTraIzq));
		ruedaTraIzqModel.RenderModel();

		// Rueda Trasera Derecha
		glm::mat4 modelRuedaTraDer = modelCuerpo;
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelRuedaTraDer));
		ruedaTraDerModel.RenderModel();

		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}