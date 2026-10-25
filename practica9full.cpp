/*
Práctica 9: Animación
Ejercicio 9-1: Animación básica
*/
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
//para probar el importer
//#include<assimp/Importer.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"

//para iluminación
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
const float toRadians = 3.14159265f / 180.0f;

//variables para animación
bool avanza;

float toffsetflechau = 0.0f;
float toffsetflechav = 0.0f;
float toffsetnumerou = 0.0f;
float toffsetnumerov = 0.0f;
float toffsetnumerocambiau = 0.0;
float angulovaria = 0.0f;
float movCoche = 0.0f;
float movOffset = 0.01f;

// ---------------- Ejercicio 1: rover de ida y vuelta ----------------
// El piso mide 20 x 20 escalado 30 veces -> va de -300 a 300 en X y en Z
float limiteRover = 298.0f;           // orilla del piso menos la mitad del largo del rover
float radioRueda = 0.5f;              // radio de la llanta en el mundo (se ajusta con el modelo)
float girollanta = 0.0f;              // ángulo acumulado de las llantas (radianes)
float girollantaincremental = 0.0f;   // se calcula en el main: movOffset / radioRueda (sin derrapar)

// ---------------- Ejercicio 2: vuelo circular de Roger ----------------
float angVuelo = 0.0f;                // posición sobre el círculo (radianes), siempre crece
float velVuelo = 0.01f;               // radianes por cuadro
float radioVuelo = 20.0f;
float alturaVuelo = 8.0f;
glm::vec3 centroVuelo = glm::vec3(0.0f, 0.0f, 0.0f);

Window mainWindow;
std::vector<MeshModel*> meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;
Texture FlechaTexture;
Texture NumerosTexture;
Texture Numero1Texture;
Texture Numero2Texture;


//Rover jerarquizado (propio)
Model cuerpoModel, brazoModel;

//Separacion de las llantas del rover para animación
Model pataDelIzq, pataDelDer, pataMedIzq, pataMedDer, pataTraIzq, pataTraDer;
Model llantaDelIzq, llantaDelDer, llantaMedIzq, llantaMedDer, llantaTraIzq, llantaTraDer;

// centro de cada llanta (coordenadas del modelo, antes de la escala 0.4)
glm::vec3 cenDelIzq(5.913f, 1.832f, -3.105f);
glm::vec3 cenDelDer(5.913f, 1.832f, 3.105f);
glm::vec3 cenMedIzq(-0.747f, 1.539f, -4.124f);
glm::vec3 cenMedDer(-0.744f, 1.539f, 4.194f);
glm::vec3 cenTraIzq(-4.930f, 1.539f, -4.194f);
glm::vec3 cenTraDer(-4.930f, 1.539f, 4.194f);
glm::vec3 posRover = glm::vec3(0.0f, -1.5f, 6.0f);

//Avión de Roger (propio)
Model AvionCuerpo_M, AvionTimon_M, AvionElevonIzq_M, AvionElevonDer_M;
glm::vec3 posAvion = glm::vec3(0.0f, 8.0f, 20.0f);

//Pista y nave de la P9
Model Nave_M, Pista_M;
glm::vec3 centroPista = glm::vec3(0.0f, -1.98f, -55.0f);
float sNave = 0.0f;
float velNave = 0.4f;
Model SatCuerpo_M, SatPanelDer_M, SatPanelIzq_M, SatAntena_M;
glm::vec3 centroSat = glm::vec3(0.0f, 25.0f, 0.0f);
float tSat = 0.0f;
float velSat = 0.003f;
float angPaneles = 0.0f;
float angAntena = 0.0f;


Skybox skybox;

//materiales
Material Material_brillante;
Material Material_opaco;


//Sphere cabeza = Sphere(0.5, 20, 20);
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// luz direccional
DirectionalLight mainLight;
//para declarar varias luces de tipo pointlight
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";



//cálculo del promedio de las normales para sombreado de Phong
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}


void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
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
	unsigned int vegetacionIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	   4,5,6,
	   4,6,7
	};

	GLfloat vegetacionVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,


	};


	unsigned int flechaIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat flechaVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

	};

	unsigned int scoreIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat scoreVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

	};

	unsigned int numeroIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat numeroVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.67f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		0.25f, 0.67f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		0.25f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

	};

	MeshModel* obj1 = new MeshModel();
	obj1->CreateMeshModel(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	MeshModel* obj2 = new MeshModel();
	obj2->CreateMeshModel(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	MeshModel* obj3 = new MeshModel();
	obj3->CreateMeshModel(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);


	MeshModel* obj4 = new MeshModel();
	obj4->CreateMeshModel(vegetacionVertices, vegetacionIndices, 64, 12);
	meshList.push_back(obj4);

	MeshModel* obj5 = new MeshModel();
	obj5->CreateMeshModel(flechaVertices, flechaIndices, 32, 6);
	meshList.push_back(obj5);

	MeshModel* obj6 = new MeshModel();
	obj6->CreateMeshModel(scoreVertices, scoreIndices, 32, 6);
	meshList.push_back(obj6); // todos los números

	MeshModel* obj7 = new MeshModel();
	obj7->CreateMeshModel(numeroVertices, numeroIndices, 32, 6);
	meshList.push_back(obj7); // solo un número

}


void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

// Dibuja una pata del rover girando sobre su articulación con el chasis
void DibujaPata(Model& pata, Model& llanta, glm::mat4 padre, glm::vec3 pivote,
	float angulo, glm::vec3 centroLlanta, float giro, GLuint uniformModel)
{
	// pata: rota alrededor de su pivote
	padre = glm::translate(padre, pivote);
	padre = glm::rotate(padre, angulo * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
	padre = glm::translate(padre, -pivote);
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(padre));
	pata.RenderModel();

	// llanta: hereda la matriz de la pata y gira sobre su centro (eje Z)
	padre = glm::translate(padre, centroLlanta);
	padre = glm::rotate(padre, giro, glm::vec3(0.0f, 0.0f, 1.0f));
	padre = glm::translate(padre, -centroLlanta);
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(padre));
	llanta.RenderModel();
}

// ---------------- CIRCUITO DE CARRERAS ----------------

const float PI_F = 3.14159265f;
const float L_RECTA = 60.0f;
const float R_CURVA = 20.0f;
const float INCLINACION = 25.0f;    // alabeo máximo de la nave en las curvas (grados)
const float LONG_CIRCUITO = 2.0f * L_RECTA + 2.0f * PI_F * R_CURVA;

// rampa suave
float Rampa(float s, float ini, float subida, float plano, float H)
{
	if (s < ini) return 0.0f;
	s -= ini;
	if (s < subida) return H * 0.5f * (1.0f - cos(PI_F * s / subida));      // subida
	s -= subida;
	if (s < plano) return H;                                                 // parte alta
	s -= plano;
	if (s < subida) return H * 0.5f * (1.0f + cos(PI_F * s / subida));      // bajada
	return 0.0f;
}

// altura de la pista en el punto s
float AlturaCircuito(float s)
{
	s = fmod(s, LONG_CIRCUITO);
	if (s < 0.0f) s += LONG_CIRCUITO;
	if (s < L_RECTA)
		return Rampa(s, 8.0f, 12.0f, 20.0f, 6.0f);                  // recta 1: rampa alta
	s -= L_RECTA + PI_F * R_CURVA;
	if (s >= 0.0f && s < L_RECTA)
		return Rampa(s, 10.0f, 12.0f, 16.0f, 3.0f);                 // recta 2: rampa baja
	return 0.0f;                                                     // curvas: planas
}

// posición y orientación de la nave en el punto s del circuito
void PuntoCircuito(float s, glm::vec3& pos, float& yaw, float& pitch, float& roll)
{
	float x, z, dx, dz, a;      // (dx, dz) = dirección hacia donde avanza
	roll = 0.0f;
	if (s < L_RECTA)                                        // tramo 1: recta hacia +X
	{
		x = -30.0f + s;  z = -20.0f;
		dx = 1.0f;  dz = 0.0f;
	}
	else if (s < L_RECTA + PI_F * R_CURVA)                  // tramo 2: curva con centro en (30, 0)
	{
		a = (s - L_RECTA) / R_CURVA;                        // ángulo recorrido = arco / radio
		x = 30.0f + R_CURVA * sin(a);  z = -R_CURVA * cos(a);
		dx = cos(a);  dz = sin(a);
		roll = INCLINACION * sin(a);                        // 0 al entrar, máximo a la mitad, 0 al salir
	}
	else if (s < 2.0f * L_RECTA + PI_F * R_CURVA)           // tramo 3: recta hacia -X
	{
		x = 30.0f - (s - L_RECTA - PI_F * R_CURVA);  z = 20.0f;
		dx = -1.0f;  dz = 0.0f;
	}
	else                                                    // tramo 4: curva con centro en (-30, 0)
	{
		a = (s - 2.0f * L_RECTA - PI_F * R_CURVA) / R_CURVA;
		x = -30.0f - R_CURVA * sin(a);  z = R_CURVA * cos(a);
		dx = -cos(a);  dz = -sin(a);
		roll = INCLINACION * sin(a);
	}
	float pendiente = (AlturaCircuito(s + 0.05f) - AlturaCircuito(s - 0.05f)) / 0.1f;
	pos = glm::vec3(x, AlturaCircuito(s), z);
	yaw = atan2(dx, dz);            // la nariz de la nave mira a +Z
	pitch = -atan(pendiente);       // nariz arriba en subidas, abajo en bajadas
	roll = roll * toRadians;
}



int main()
{
	mainWindow = Window(1366, 768); 
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.5f, 0.5f);

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();
	AgaveTexture = Texture("Textures/Agave.tga");
	AgaveTexture.LoadTextureA();
	FlechaTexture = Texture("Textures/flechas.tga");
	FlechaTexture.LoadTextureA();
	NumerosTexture = Texture("Textures/numerosbase.tga");
	NumerosTexture.LoadTextureA();
	Numero1Texture = Texture("Textures/numero1.tga");
	Numero1Texture.LoadTextureA();
	Numero2Texture = Texture("Textures/numero2.tga");
	Numero2Texture.LoadTextureA();

	//Rover propio (cuerpo, brazo y 6 patas con rueda)
	cuerpoModel = Model();       cuerpoModel.LoadModel("Models/Cuerpo.obj");
	brazoModel = Model();        brazoModel.LoadModel("Models/brazo.obj");
	pataDelIzq = Model(); pataDelIzq.LoadModel("Models/pata_del_izq.obj");
	pataDelDer = Model(); pataDelDer.LoadModel("Models/pata_del_der.obj");
	pataMedIzq = Model(); pataMedIzq.LoadModel("Models/pata_med_izq.obj");
	pataMedDer = Model(); pataMedDer.LoadModel("Models/pata_med_der.obj");
	pataTraIzq = Model(); pataTraIzq.LoadModel("Models/pata_tra_izq.obj");
	pataTraDer = Model(); pataTraDer.LoadModel("Models/pata_tra_der.obj");

	llantaDelIzq = Model(); llantaDelIzq.LoadModel("Models/llanta_del_izq.obj");
	llantaDelDer = Model(); llantaDelDer.LoadModel("Models/llanta_del_der.obj");
	llantaMedIzq = Model(); llantaMedIzq.LoadModel("Models/llanta_med_izq.obj");
	llantaMedDer = Model(); llantaMedDer.LoadModel("Models/llanta_med_der.obj");
	llantaTraIzq = Model(); llantaTraIzq.LoadModel("Models/llanta_tra_izq.obj");
	llantaTraDer = Model(); llantaTraDer.LoadModel("Models/llanta_tra_der.obj");

	//Avión de Roger
	AvionCuerpo_M = Model();     AvionCuerpo_M.LoadModel("Models/roger_cuerpo.obj");
	AvionTimon_M = Model();      AvionTimon_M.LoadModel("Models/roger_timon.obj");
	AvionElevonIzq_M = Model();  AvionElevonIzq_M.LoadModel("Models/roger_elevon_izq.obj");
	AvionElevonDer_M = Model();  AvionElevonDer_M.LoadModel("Models/roger_elevon_der.obj");

	//Practica 9: nave y pista
	Nave_M = Model();   Nave_M.LoadModel("Models/a4o_aphelion.obj");
	Pista_M = Model();  Pista_M.LoadModel("Models/pista.obj");

	SatCuerpo_M = Model();    SatCuerpo_M.LoadModel("Models/sat_cuerpo.obj");
	SatPanelDer_M = Model();  SatPanelDer_M.LoadModel("Models/sat_panel_der.obj");
	SatPanelIzq_M = Model();  SatPanelIzq_M.LoadModel("Models/sat_panel_izq.obj");
	SatAntena_M = Model();    SatAntena_M.LoadModel("Models/sat_antena.obj");


	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);


	//luz direccional, sólo 1 y siempre debe de existir
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.3f, 0.3f,
		0.0f, 0.0f, -1.0f);
	//contador de luces puntuales
	unsigned int pointLightCount = 0;
	//Declaración de primer luz puntual
	pointLights[0] = PointLight(1.0f, 0.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 2.5f, 1.5f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;

	unsigned int spotLightCount = 0;


	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	// Ejercicio 1: el rover empieza en el centro y avanzando hacia +X (su frente)
	movCoche = 0.0f;
	movOffset = 0.3f;                              // unidades por cuadro
	avanza = true;
	girollantaincremental = movOffset / radioRueda; // ángulo = distancia / radio


	glm::vec3 lowerLight(0.0f, 0.0f, 0.0f);

	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::mat4 matRover(1.0);
	glm::mat4 matAvion(1.0);
	glm::mat4 matNave(1.0f), matSat(1.0f);
	glm::vec3 posNave(0.0f), posSat(0.0f);
	float yawNave = 0.0f, pitchNave = 0.0f, rollNave = 0.0f, yawSat = 0.0f;
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec2 toffset = glm::vec2(0.0f, 0.0f);

	// pivotes de las patas del rover (coordenadas del modelo, antes de la escala 0.4)
	glm::vec3 pivDelIzq(2.6f, 4.5f, -3.07f), pivDelDer(2.6f, 4.5f, 3.07f);
	glm::vec3 pivRockIzq(-2.85f, 3.57f, -3.04f), pivRockDer(-2.85f, 3.57f, 3.04f);

	// variables del ciclo
	GLfloat now = 0.0f;
	float rumbo = 0.0f;
	float rotacionPatas = 0.0f;
	glm::mat4 modelCuerpo(1.0f);
	glm::mat4 modelBrazo(1.0f);

	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		angulovaria += 0.5f * deltaTime;

		// EJERCICIO 1: ROVER DE IDA Y VUELTA 
		// bandera avanza: true -> va hacia +X, false -> regresa hacia -X

		if (avanza)
		{
			movCoche += movOffset * deltaTime;
			girollanta -= girollantaincremental * deltaTime;   // llantas giran hacia adelante
			if (movCoche >= limiteRover)                        // llegó a la orilla
			{
				movCoche = limiteRover;
				avanza = false;
			}
		}
		else
		{
			movCoche -= movOffset * deltaTime;
			girollanta += girollantaincremental * deltaTime;   // llantas giran hacia atrás
			if (movCoche <= -limiteRover)                       // llegó a la otra orilla
			{
				movCoche = -limiteRover;
				avanza = true;
			}
		}
		posRover = glm::vec3(movCoche, -1.5f, 6.0f);

		// EJERCICIO 2: VUELO CIRCULAR DE ROGER 
		// angVuelo solo aumenta, por eso Roger nunca vuela de reversa
		angVuelo += velVuelo * deltaTime;
		if (angVuelo > 2.0f * 3.14159265f)
			angVuelo -= 2.0f * 3.14159265f;
		posAvion = centroVuelo + glm::vec3(radioVuelo * sin(angVuelo), alturaVuelo, radioVuelo * cos(angVuelo));
		// la nariz de Roger mira a +Z; girando angVuelo + 90° apunta a la tangente del círculo
		rumbo = angVuelo + 90.0f * toRadians;


		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();
		uniformTextureOffset = shaderList[0].getOffsetLocation(); // para la textura con movimiento

		//información en el shader de intensidad especular y brillo
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);


		//información al shader de fuentes de iluminación
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);


		//Reinicializando variables cada ciclo de reloj
		model = glm::mat4(1.0);
		modelaux = glm::mat4(1.0);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		toffset = glm::vec2(0.0f, 0.0f);

		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		meshList[2]->RenderMeshModel();

		// ---------------- EJERCICIO 1: NAVE EN EL CIRCUITO ----------------
		sNave += velNave * deltaTime;               // solo aumenta: nunca va de reversa
		if (sNave >= LONG_CIRCUITO) sNave -= LONG_CIRCUITO;   // terminó la vuelta, empieza otra
		PuntoCircuito(sNave, posNave, yawNave, pitchNave, rollNave);
		posNave = centroPista + posNave + glm::vec3(0.0f, 1.15f, 0.0f);  // flota 1 unidad sobre la pista

		// ---------------- EJERCICIO 2: SATÉLITE ----------------
		tSat += velSat * deltaTime;
		if (tSat > 2.0f * PI_F) tSat -= 2.0f * PI_F;
		// trayectoria en forma de 8
		posSat = centroSat + glm::vec3(60.0f * sin(tSat), 5.0f * sin(tSat), 40.0f * sin(tSat) * cos(tSat));
		yawSat = atan2(60.0f * cos(tSat), 40.0f * cos(2.0f * tSat));   // mira hacia donde avanza
		angPaneles = 45.0f * sin(tSat * 6.0f);       // los paneles se mecen de -45° a 45°
		angAntena += 1.5f * deltaTime;               // la antena gira continuamente
		if (angAntena > 360.0f) angAntena -= 360.0f;


		// ---------------- ROVER (jerarquía) ----------------
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		// 1. PADRE: traslación animada del cuerpo
		matRover = glm::mat4(1.0);
		matRover = glm::translate(matRover, posRover);
		modelCuerpo = glm::scale(matRover, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelCuerpo));
		cuerpoModel.RenderModel();

		// 2. HIJO: brazo (con su ajuste de posición sobre el cuerpo)
		modelBrazo = glm::translate(modelCuerpo, glm::vec3(-7.0f, 0.5f, -0.8f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelBrazo));
		brazoModel.RenderModel();

		// 3. HIJOS: patas con rueda, cada una gira sobre su articulación con el chasis (R/F)
		rotacionPatas = mainWindow.getRotacionPatas();

		DibujaPata(pataDelIzq, llantaDelIzq, modelCuerpo, pivDelIzq, rotacionPatas, cenDelIzq, girollanta, uniformModel);
		DibujaPata(pataDelDer, llantaDelDer, modelCuerpo, pivDelDer, rotacionPatas, cenDelDer, girollanta, uniformModel);
		DibujaPata(pataMedIzq, llantaMedIzq, modelCuerpo, pivRockIzq, rotacionPatas, cenMedIzq, girollanta, uniformModel);
		DibujaPata(pataMedDer, llantaMedDer, modelCuerpo, pivRockDer, rotacionPatas, cenMedDer, girollanta, uniformModel);
		DibujaPata(pataTraIzq, llantaTraIzq, modelCuerpo, pivRockIzq, rotacionPatas, cenTraIzq, girollanta, uniformModel);
		DibujaPata(pataTraDer, llantaTraDer, modelCuerpo, pivRockDer, rotacionPatas, cenTraDer, girollanta, uniformModel);


		// ---------------- AVIÓN DE ROGER (jerarquía) ----------------
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);

		// 1. PADRE: posición sobre el círculo, rumbo tangente y alabeo hacia dentro de la curva
		matAvion = glm::mat4(1.0);
		matAvion = glm::translate(matAvion, posAvion);
		matAvion = glm::rotate(matAvion, rumbo, glm::vec3(0.0f, 1.0f, 0.0f));
		matAvion = glm::rotate(matAvion, -20.0f * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(matAvion, glm::vec3(0.6f, 0.6f, 0.6f));
		modelaux = model;
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		AvionCuerpo_M.RenderModel();

		// 2. HIJO: timón (N/M)
		model = modelaux;
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.65f));
		model = glm::rotate(model, mainWindow.getAngTimon() * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		AvionTimon_M.RenderModel();

		// 3. HIJOS: elevones (V/B)
		model = modelaux;
		model = glm::translate(model, glm::vec3(0.0f, -0.36f, -2.45f));
		model = glm::rotate(model, mainWindow.getAngElevon() * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		AvionElevonIzq_M.RenderModel();
		AvionElevonDer_M.RenderModel();

		// ---------------- PISTA ----------------
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		model = glm::translate(glm::mat4(1.0f), centroPista);
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Pista_M.RenderModel();

		// ---------------- NAVE APHELION ----------------
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		matNave = glm::mat4(1.0f);
		matNave = glm::translate(matNave, posNave);
		matNave = glm::rotate(matNave, yawNave, glm::vec3(0.0f, 1.0f, 0.0f));    // rumbo
		matNave = glm::rotate(matNave, pitchNave, glm::vec3(1.0f, 0.0f, 0.0f));  // rampas
		matNave = glm::rotate(matNave, rollNave, glm::vec3(0.0f, 0.0f, 1.0f));   // curvas
		model = glm::scale(matNave, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Nave_M.RenderModel();

		// ---------------- SATÉLITE (jerarquía) ----------------
		matSat = glm::mat4(1.0f);
		matSat = glm::translate(matSat, posSat);
		matSat = glm::rotate(matSat, yawSat, glm::vec3(0.0f, 1.0f, 0.0f));
		matSat = glm::scale(matSat, glm::vec3(1.5f, 1.5f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(matSat));
		SatCuerpo_M.RenderModel();

		// panel derecho: gira en X sobre su unión con el cuerpo
		model = glm::translate(matSat, glm::vec3(1.25f, 0.0f, 0.0f));
		model = glm::rotate(model, angPaneles * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(-1.25f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		SatPanelDer_M.RenderModel();

		// panel izquierdo
		model = glm::translate(matSat, glm::vec3(-1.25f, 0.0f, 0.0f));
		model = glm::rotate(model, angPaneles * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(1.25f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		SatPanelIzq_M.RenderModel();

		// antena: gira en Y sobre su base
		model = glm::translate(matSat, glm::vec3(0.0f, 1.4f, 0.0f));
		model = glm::rotate(model, angAntena * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(0.0f, -1.4f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		SatAntena_M.RenderModel();


		//Agave ¿qué sucede si lo renderizan antes del Rover?
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.5f, -3.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		AgaveTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMeshModel();

		//textura con movimiento
		//Importantes porque la variable uniform no podemos modificarla directamente
		toffsetflechau += 0.001;
		toffsetflechav = 0.000;
		//para que no se desborde la variable
		if (toffsetflechau > 1.0)
			toffsetflechau = 0.0;
		//if (toffsetv > 1.0)
		//	toffsetv = 0;
		//printf("\ntfosset %f \n", toffsetu);
		//pasar a la variable uniform el valor actualizado
		toffset = glm::vec2(toffsetflechau, toffsetflechav);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 0.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		FlechaTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMeshModel();

		//plano con todos los números
		toffsetnumerou = 0.0;
		toffsetnumerov = 0.0;
		toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-6.0f, 2.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		NumerosTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[5]->RenderMeshModel();

		//número 1
		//toffsetnumerou = 0.0;
		//toffsetnumerov = 0.0;
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-10.0f, 2.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		//glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		NumerosTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[6]->RenderMeshModel();

		for (int i = 1; i < 4; i++)
		{
			//números 2-4
			toffsetnumerou += 0.25;
			toffsetnumerov = 0.0;
			toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-10.0f - (i * 3.0), 2.0f, -6.0f));
			model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			NumerosTexture.UseTexture();
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[6]->RenderMeshModel();

		}

		for (int j = 1; j < 5; j++)
		{
			//números 5-8
			toffsetnumerou += 0.25;
			toffsetnumerov = -0.33;
			toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-7.0f - (j * 3.0), 5.0f, -6.0f));
			model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			NumerosTexture.UseTexture();
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[6]->RenderMeshModel();
		}


		//número cambiante 
		/*
		¿Cómo hacer para que sea a una velocidad visible?
		*/
		toffsetnumerocambiau += 0.25;
		if (toffsetnumerocambiau > 1.0)
			toffsetnumerocambiau = 0.0;
		toffsetnumerov = 0.0;
		toffset = glm::vec2(toffsetnumerocambiau, toffsetnumerov);
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-10.0f, 10.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		NumerosTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[6]->RenderMeshModel();

		//cambiar automáticamente entre textura número 1 y número 2
		toffsetnumerou = 0.0;
		toffsetnumerov = 0.0;
		toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-13.0f, 10.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Numero1Texture.UseTexture();
		//if
		//Numero1Texture.UseTexture();
		//Numero2Texture.UseTexture();

		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[5]->RenderMeshModel();

		glDisable(GL_BLEND);





		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}