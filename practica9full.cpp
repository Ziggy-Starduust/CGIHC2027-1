/*
Práctica 9: Animación
Ejercicio 9-2: Animación compleja
  1. El holocrón separa sus esquinas, las hace girar y las vuelve a juntar (máquina de estados)
  2. El cohete (Roger) despega de su plataforma con fuego en los motores, acelera según
	 y = 1/2 a t^2 y se pierde en el cielo; deja una estela de humo que desaparece a los 2 s
*/
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
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

//Ejercicio 9-2 (2): despegue del cohete
// fases: 0 en la plataforma, 1 encendido, 2 ascenso, 3 fuera de vista
glm::vec3 posPlataforma = glm::vec3(-16.0f, 2.1f, -6.0f);   // dónde está parado (ajústalo)
glm::vec3 posCohete = glm::vec3(0.0f);                       // posición actual
const float ESPERA_PLATAFORMA = 3.0f;  // segundos parado antes de encender
const float DUR_ENCENDIDO = 1.5f;      // segundos que tarda el fuego en crecer
const float ESPERA_FUERA = 2.0f;       // segundos fuera de vista antes de reaparecer
const float ACEL_COHETE = 3.0f;        // aceleración del ascenso (unidades/s^2)
const float ALTURA_MAX = 300.0f;       // a esta altura ya se perdió en el cielo
const float VEL_ROLL = 45.0f;          // giro sobre su propio eje al subir (grados/s)
const float ACEL_LATERAL = 6.0f;       // aceleración horizontal (unidades/s^2)
const float T_INCLINA = 3.0f;          // segundos de ascenso vertical antes de empezar a inclinarse
float desvCohete = 0.0f;               // desplazamiento horizontal (en X)
float inclCohete = 0.0f;               // inclinación respecto a la vertical (grados)
int   faseCohete = 0;
float tCohete = 0.0f;                  // tiempo que lleva la fase actual (segundos)
float altCohete = 0.0f;                // altura sobre la plataforma
float fuegoCohete = 0.0f;              // tamaño del fuego: 0 apagado, 1 a todo
float rollCohete = 0.0f;               // ángulo de giro sobre su eje (grados)

// ---------------- Ejercicio 9-2 (2): humo de los motores ----------------
const int MAX_HUMO = 400;             // 3 motores x (2 s / 0.02 s) = 300 partículas vivas como máximo
const float VIDA_HUMO = 2.0f;         // segundos que dura cada bocanada de humo
const float INTERVALO_HUMO = 0.02f;   // cada cuánto sale humo de los motores (segundos)
const int FRAMES_HUMO = 8;            // cuadros de la textura humo.png (se va desvaneciendo)

struct ParticulaHumo
{
	glm::vec3 pos;                    // posición en el mundo (se queda donde salió)
	float vida;                       // segundos que lleva viva
	bool activa;
};
ParticulaHumo humo[MAX_HUMO];
int sigHumo = 0;                      // siguiente casilla a usar (arreglo circular)
float acumHumo = 0.0f;                // tiempo acumulado desde la última emisión

// toberas de Roger en coordenadas del modelo (antes de la escala 0.6)
// AJUSTAR si el humo no sale exactamente de los motores
glm::vec3 motoresRoger[3] = {
	glm::vec3(0.0f,  0.35f, -2.9f),   // motor superior
	glm::vec3(-0.3f, -0.15f, -2.9f),  // motor inferior izquierdo
	glm::vec3(0.3f, -0.15f, -2.9f)    // motor inferior derecho
};

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
Texture HumoTexture;

//Avión de Roger (propio)
Model AvionCuerpo_M, AvionTimon_M, AvionElevonIzq_M, AvionElevonDer_M;

// ===== globales para P9-2=====
Model cuerpoHolo, esquinas[8];
const char* rutasEsq[8] = { "Esquina_PPP","Esquina_PPN","Esquina_PNP","Esquina_PNN",
						   "Esquina_NPP","Esquina_NPN","Esquina_NNP","Esquina_NNN" };
glm::vec3 dirEsq[8] = { {1,1,-1},{1,-1,-1},{1,1,1},{1,-1,1},
						{-1,1,-1},{-1,-1,-1},{-1,1,1},{-1,-1,1} };

glm::vec3 posHolocron = glm::vec3(6.0f, 0.0f, -3.0f);
// fases: 0 elevarse, 1 asomarse, 2 salir disparadas, 3 flotar, 4 regresar, 5 bajar
const float DUR_HOLO[6] = { 2.0f, 0.6f, 1.2f, 4.0f, 0.6f, 2.0f };  // segundos
const float SEP_ASOMA = 1.0f, SEP_MAX = 12.0f;
int   faseH = 0;
float tFase = 0.0f, sepH = 0.0f, altH = 0.0f, giroH = 0.0f, angEsq = 0.0f, angEsqIni = 0.0f;

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

	// cuadro del humo: plano XY que siempre mira a la cámara (billboard)
	// u va de 0 a 1/8 porque la textura trae 8 cuadros; el offset elige cuál se ve
	unsigned int humoIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat humoVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, -1.0f,
		0.5f, -0.5f, 0.0f,		0.125f, 0.0f,	0.0f, 0.0f, -1.0f,
		0.5f, 0.5f, 0.0f,		0.125f, 1.0f,	0.0f, 0.0f, -1.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, -1.0f,
	};

	MeshModel* obj8 = new MeshModel();
	obj8->CreateMeshModel(humoVertices, humoIndices, 32, 6);
	meshList.push_back(obj8); // meshList[7]: humo

	// cono para el fuego de los motores: base de radio 1 en z = 0 y punta en z = -1,
	// apunta hacia atrás (-Z) igual que las toberas de Roger
	const int LADOS_FUEGO = 16;
	GLfloat fuegoVertices[LADOS_FUEGO * 3 * 8];
	unsigned int fuegoIndices[LADOS_FUEGO * 3];
	float ang0, ang1;
	float px[3], py[3], pz[3], an[3];
	int k = 0;
	for (int s = 0; s < LADOS_FUEGO; s++)
	{
		ang0 = 2.0f * 3.14159265f * s / LADOS_FUEGO;
		ang1 = 2.0f * 3.14159265f * (s + 1) / LADOS_FUEGO;
		// cada lado es un triángulo: dos puntos de la base y la punta
		px[0] = cos(ang0); py[0] = sin(ang0); pz[0] = 0.0f;  an[0] = ang0;
		px[1] = cos(ang1); py[1] = sin(ang1); pz[1] = 0.0f;  an[1] = ang1;
		px[2] = 0.0f;      py[2] = 0.0f;      pz[2] = -1.0f; an[2] = (ang0 + ang1) / 2.0f;
		for (int v = 0; v < 3; v++)
		{
			fuegoVertices[k * 8 + 0] = px[v];
			fuegoVertices[k * 8 + 1] = py[v];
			fuegoVertices[k * 8 + 2] = pz[v];
			fuegoVertices[k * 8 + 3] = 0.5f;            // u
			fuegoVertices[k * 8 + 4] = v / 2.0f;        // v
			// normal del cono (cos, sin, -1)/raiz(2), guardada invertida como el resto de la práctica
			fuegoVertices[k * 8 + 5] = -cos(an[v]) * 0.7071f;
			fuegoVertices[k * 8 + 6] = -sin(an[v]) * 0.7071f;
			fuegoVertices[k * 8 + 7] = 0.7071f;
			fuegoIndices[k] = k;
			k++;
		}
	}

	MeshModel* obj9 = new MeshModel();
	obj9->CreateMeshModel(fuegoVertices, fuegoIndices, LADOS_FUEGO * 3 * 8, LADOS_FUEGO * 3);
	meshList.push_back(obj9); // meshList[8]: fuego de los motores

}


void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

// guarda una bocanada de humo en la siguiente casilla libre del arreglo
void EmitirHumo(glm::vec3 p)
{
	// pequeño desorden para que la estela no se vea como una línea perfecta
	p.x += ((rand() % 100) / 100.0f - 0.5f) * 0.15f;
	p.y += ((rand() % 100) / 100.0f - 0.5f) * 0.15f;
	p.z += ((rand() % 100) / 100.0f - 0.5f) * 0.15f;
	humo[sigHumo].pos = p;
	humo[sigHumo].vida = 0.0f;
	humo[sigHumo].activa = true;
	sigHumo = (sigHumo + 1) % MAX_HUMO;
}

float suave(float t) { return t * t * (3.0f - 2.0f * t); }        // ease in-out
float salida(float t) { return 1.0f - (1.0f - t) * (1.0f - t); }   // ease out

void animaHolocron(float dt)
{
	tFase += dt;
	float t = glm::min(tFase / DUR_HOLO[faseH], 1.0f);   // avance de la fase de 0 a 1
	giroH += (faseH == 3 ? 25.0f : 60.0f) * dt;         // giro general (grados/s)

	switch (faseH)
	{
	case 0:// se eleva desde su lugar
		altH = suave(t);
		sepH = 0.0f;
		angEsq = 0.0f;
		break;
	case 1:// las esquinas se asoman
		sepH = SEP_ASOMA * suave(t);
		break;
	case 2: // salen hasta SEP_MAX
		sepH = SEP_ASOMA + (SEP_MAX - SEP_ASOMA) * suave(t);
		break;
	case 3:// flotan girando
		angEsq += 40.0f * dt;
		break;
	case 4:// REGRESAN a su lugar
		sepH = SEP_MAX * (1.0f - suave(t));
		angEsq = angEsqIni * (1.0f - suave(t));
		break;
	case 5:// ya armado, baja a su lugar
		altH = 1.0f - suave(t);
		break;
	}

	if (tFase >= DUR_HOLO[faseH])                        // termina la fase: pasa a la siguiente
	{
		tFase = 0.0f;
		if (faseH == 3) angEsqIni = angEsq;              // guarda el giro para deshacerlo al regresar
		faseH = (faseH + 1) % 6;
	}
}

//COHETE (Ejercicio 9-2)
void animaCohete(float dt)
{
	tCohete += dt;
	switch (faseCohete)
	{
	case 0:// en la plataforma
		altCohete = 0.0f;
		desvCohete = 0.0f;
		inclCohete = 0.0f;
		fuegoCohete = 0.0f;
		rollCohete = 0.0f;
		if (tCohete >= ESPERA_PLATAFORMA) { faseCohete = 1; tCohete = 0.0f; }
		break;
	case 1:// encendido: el fuego crece, aún no se mueve
		fuegoCohete = suave(glm::min(tCohete / DUR_ENCENDIDO, 1.0f));
		if (tCohete >= DUR_ENCENDIDO) { faseCohete = 2; tCohete = 0.0f; }
		break;
	case 2:// ascenso: vertical y luego se curva
		fuegoCohete = 1.0f;
		altCohete = 0.5f * ACEL_COHETE * tCohete * tCohete;           // y = 1/2 a t^2
		if (tCohete > T_INCLINA)                                       // x = 1/2 b (t - t0)^2
			desvCohete = 0.5f * ACEL_LATERAL * (tCohete - T_INCLINA) * (tCohete - T_INCLINA);
		else
			desvCohete = 0.0f;
		// la nariz apunta en la dirección de la velocidad: vx = b (t - t0), vy = a t
		inclCohete = atan2(ACEL_LATERAL * glm::max(tCohete - T_INCLINA, 0.0f),
			ACEL_COHETE * tCohete) / toRadians;
		if (altCohete >= ALTURA_MAX) { faseCohete = 3; tCohete = 0.0f; }
		break;
	case 3:// se pierde en el cielo, no se dibuja
		fuegoCohete = 0.0f;
		if (tCohete >= ESPERA_FUERA) { faseCohete = 0; tCohete = 0.0f; }
		break;
	}
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
	HumoTexture = Texture("Textures/humo.png");      // 8 cuadros: el humo se va desvaneciendo
	HumoTexture.LoadTextureA();

	// todas las partículas empiezan apagadas
	for (int i = 0; i < MAX_HUMO; i++)
		humo[i].activa = false;

	//Holocrón: cuerpo + 8 esquinas
	cuerpoHolo = Model();  cuerpoHolo.LoadModel("Models/Holocron_Cuerpo.obj");
	for (int i = 0; i < 8; i++)
	{
		esquinas[i] = Model();
		esquinas[i].LoadModel(std::string("Models/") + rutasEsq[i] + ".obj");
	}

	//Avión de Roger
	AvionCuerpo_M = Model();     AvionCuerpo_M.LoadModel("Models/roger_cuerpo.obj");
	AvionTimon_M = Model();      AvionTimon_M.LoadModel("Models/roger_timon.obj");
	AvionElevonIzq_M = Model();  AvionElevonIzq_M.LoadModel("Models/roger_elevon_izq.obj");
	AvionElevonDer_M = Model();  AvionElevonDer_M.LoadModel("Models/roger_elevon_der.obj");

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

	unsigned int spotLightCount = 0;


	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	glm::vec3 lowerLight(0.0f, 0.0f, 0.0f);

	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::mat4 matAvion(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec2 toffset = glm::vec2(0.0f, 0.0f);

	// variables del ciclo
	GLfloat now = 0.0f;
	GLfloat dtSeg = 0.0f;                          // segundos reales entre cuadros (para el humo)
	float rotacionPatas = 0.0f;

	float largoFuego = 0.0f;



	// humo: vectores de la cámara para que los cuadros siempre la miren
	glm::mat4 vista(1.0f);
	glm::vec3 camDer(1.0f, 0.0f, 0.0f), camArriba(0.0f, 1.0f, 0.0f);
	float tVida = 0.0f, tamHumo = 0.0f;
	int frameHumo = 0;
	glm::mat4 modelCuerpo(1.0f);
	glm::mat4 modelBrazo(1.0f);

	// holocrón 9-2
	glm::mat4 matHolo(1.0f);
	glm::vec3 dEsq(0.0f), centroEsq(0.0f), vaiven(0.0f);
	float sentido = 0.0f, desfase = 0.0f;

	// contadores de los ciclos for dentro del while
	int i = 0, j = 0, m = 0;


	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		now = glfwGetTime();
		dtSeg = now - lastTime;                    // tiempo real del cuadro, en segundos
		if (dtSeg > 0.1f) dtSeg = 0.1f;            // evita un salto enorme en el primer cuadro
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		angulovaria += 0.5f * deltaTime;

		// EJERCICIO 9-2: HOLOCRÓN
		animaHolocron(glm::min(dtSeg, 1.0f / 30.0f));

		// EJERCICIO 9-2: COHETE
		animaCohete(glm::min(dtSeg, 1.0f / 30.0f));

		// EJERCICIO 9-2 (2): ENVEJECER EL HUMO
		for (i = 0; i < MAX_HUMO; i++)
		{
			if (!humo[i].activa) continue;
			humo[i].vida += dtSeg;
			humo[i].pos.y += 0.4f * dtSeg;         // el humo sube un poquito
			if (humo[i].vida >= VIDA_HUMO)         // a los 2 segundos desaparece
				humo[i].activa = false;
		}


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
		vista = camera.calculateViewMatrix();
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(vista));
		// derecha y arriba de la cámara (renglones de la matriz de vista) para el humo
		camDer = glm::vec3(vista[0][0], vista[1][0], vista[2][0]);
		camArriba = glm::vec3(vista[0][1], vista[1][1], vista[2][1]);
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

		//COHETE: ROGER DESPEGANDO (jerarquía)
		if (faseCohete != 3)                       // en la fase 3 ya se perdió en el cielo
		{
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);

			posCohete = posPlataforma + glm::vec3(0.0f, altCohete, desvCohete);

			matAvion = glm::translate(glm::mat4(1.0f), posCohete);
			matAvion = glm::rotate(matAvion, inclCohete * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));  // se inclina de frente
			matAvion = glm::rotate(matAvion, -90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));      // nariz hacia arriba
			model = glm::scale(matAvion, glm::vec3(1.2f, 1.2f, 1.2f));
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

			// 4. HIJOS: fuego de los 3 motores (dos conos por motor)
			if (fuegoCohete > 0.0f)
			{
				plainTexture.UseTexture();
				Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
				for (m = 0; m < 3; m++)
				{
					// el largo crece con el encendido y parpadea con un seno más un poco de azar
					largoFuego = fuegoCohete * (1.6f + 0.3f * sin(now * 40.0f + m * 2.0f)
						+ 0.2f * (rand() % 100) / 100.0f);

					// núcleo amarillo: ancho y corto, pegado a la tobera
					color = glm::vec3(3.0f, 2.6f, 1.0f);
					glUniform3fv(uniformColor, 1, glm::value_ptr(color));
					model = glm::translate(modelaux, motoresRoger[m]);
					model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.5f * largoFuego));
					glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
					meshList[8]->RenderMeshModel();

					// cola naranja: más delgada y larga, sobresale del núcleo
					color = glm::vec3(2.5f, 0.9f, 0.1f);
					glUniform3fv(uniformColor, 1, glm::value_ptr(color));
					model = glm::translate(modelaux, motoresRoger[m]);
					model = glm::scale(model, glm::vec3(0.11f, 0.11f, largoFuego));
					glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
					meshList[8]->RenderMeshModel();
				}
				color = glm::vec3(1.0f, 1.0f, 1.0f);
				glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			}

			// EMISIÓN DE HUMO: solo con los motores encendidos. La posición de cada tobera
			// se pasa al mundo con la matriz del cuerpo, así sale de los motores aunque gire
			if (fuegoCohete > 0.3f)
			{
				acumHumo += dtSeg;
				while (acumHumo >= INTERVALO_HUMO)
				{
					acumHumo -= INTERVALO_HUMO;
					for (m = 0; m < 3; m++)
						EmitirHumo(glm::vec3(modelaux * glm::vec4(motoresRoger[m] + glm::vec3(0.0f, 0.0f, -2.0f), 1.0f)));
				}
			}
			else
				acumHumo = 0.0f;
		}

		// ---------------- HOLOCRÓN (cuerpo + 8 esquinas) ----------------
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);

		matHolo = glm::translate(glm::mat4(1.0f),
			posHolocron + glm::vec3(0.0f, 1.5f * altH + 0.15f * altH * sin(now * 2.0f), 0.0f));
		matHolo = glm::rotate(matHolo, giroH * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		matHolo = glm::rotate(matHolo, 20.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		matHolo = glm::scale(matHolo, glm::vec3(0.1f, 0.1f, 0.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(matHolo));
		cuerpoHolo.RenderModel();

		for (i = 0; i < 8; i++)
		{
			dEsq = glm::normalize(dirEsq[i]);
			centroEsq = dirEsq[i] * 3.9f;
			sentido = (i % 2 == 0) ? 1.0f : -1.0f;
			desfase = i * 0.785f;
			vaiven = glm::vec3(sin(now * 1.3f + desfase), cos(now * 1.1f + desfase), sin(now * 0.9f + desfase))
				* 0.8f * (sepH / SEP_MAX);

			model = glm::translate(matHolo, dEsq * sepH + vaiven);   // sale en diagonal y se mece
			model = glm::translate(model, centroEsq);             // gira sobre su propio centro
			model = glm::rotate(model, angEsq * sentido * toRadians, dEsq);
			model = glm::translate(model, -centroEsq);
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			esquinas[i].RenderModel();
		}

		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		AgaveTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMeshModel();

		// ---------------- HUMO DE ROGER (se dibuja al final por ser transparente) ----------------
		glDepthMask(GL_FALSE);                     // el humo no tapa a otros cuadros de humo
		HumoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		color = glm::vec3(0.9f, 0.9f, 0.9f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		for (i = 0; i < MAX_HUMO; i++)
		{
			if (!humo[i].activa) continue;
			tVida = humo[i].vida / VIDA_HUMO;      // 0 al nacer, 1 al desaparecer

			// cuadro de la textura según la edad: cada vez más transparente
			frameHumo = (int)(tVida * FRAMES_HUMO);
			if (frameHumo >= FRAMES_HUMO) frameHumo = FRAMES_HUMO - 1;
			toffset = glm::vec2((float)frameHumo / FRAMES_HUMO, 0.0f);
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
			// la bocanada crece mientras se dispersa
			tamHumo = 0.8f + 3.5f * tVida;
			// billboard: ejes de la cámara como columnas, así el cuadro siempre la mira
			model = glm::mat4(glm::vec4(camDer * tamHumo, 0.0f),
				glm::vec4(camArriba * tamHumo, 0.0f),
				glm::vec4(glm::cross(camDer, camArriba), 0.0f),
				glm::vec4(humo[i].pos, 1.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			meshList[7]->RenderMeshModel();
		}
		glDepthMask(GL_TRUE);
		toffset = glm::vec2(0.0f, 0.0f);
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));

		glDisable(GL_BLEND);





		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}