/*
Animacion por keyframes - Practica 10
Cohete Jupiter-C (con el satelite Explorer 1) separado en 4 piezas: etapa 0 (Redstone),
etapa 1 (seccion de instrumentos), etapa 2 (tub de etapas superiores) y capsula.
El cohete despega, se va separando en fases, las etapas 0 y 1 caen al piso y quedan
acostadas, y al final la capsula (unida a la etapa 2) queda orbitando. Cada keyframe guarda x, y y giro de las 4 piezas.
Los keyframes se crean POR TECLADO y se guardan/leen del archivo keyframescohete.txt.

La textura del skybox fue conseguida desde la pagina https://opengameart.org/content/elyvisions-skyboxes?page=1
y se edito en Gimp rotando 90 grados en sentido antihorario la imagen sp2_up.png para poder ver continuidad.
Fuentes:
	https://www.khronos.org/opengl/wiki/Keyframe_Animation
	http://what-when-how.com/wp-content/uploads/2012/07/tmpcd0074_thumb.png
*/
//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>
#include <fstream>   //para escribir/leer el archivo de keyframes

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

//para iluminacion
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
const float toRadians = 3.14159265f / 180.0f;

//variables para animacion
float movCoche;
float movOffset;
float rotllanta;
float rotllantaOffset;
bool avanza;
float toffsetflechau = 0.0f;
float toffsetflechav = 0.0f;
float toffsetnumerou = 0.0f;
float toffsetnumerov = 0.0f;
float toffsetnumerocambiau = 0.0;
float angulovaria = 0.0f;


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


//Cohete Jupiter-C separado en 4 piezas
#define NUM_PIEZAS 4
Model Cohete_M[NUM_PIEZAS];
const char* archivosCohete[NUM_PIEZAS] = {
	"Models/JupiterC_Etapa_0.obj",
	"Models/JupiterC_Etapa_1.obj",
	"Models/JupiterC_Etapa_2.obj",
	"Models/JupiterC_Capsula.obj" };
const char* nombresPiezas[NUM_PIEZAS + 1] = { "etapa 0", "etapa 1", "etapa 2", "capsula", "todo el cohete" };
//escala del modelo y pivote de cada pieza (centro de la pieza ya escalado)
const float escalaCohete = 0.15f;
const glm::vec3 pivotePieza[NUM_PIEZAS] = {
	glm::vec3(0.0024f, 3.5924f, 0.0f),
	glm::vec3(0.0069f, 8.1938f, 0.0f),
	glm::vec3(0.0f, 9.1888f, 0.0f),
	glm::vec3(0.0f, 10.0069f, 0.0f) };

Skybox skybox;

//materiales
Material Material_brillante;
Material Material_opaco;


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

//aplica lo que Window detecto del teclado a los keyframes
void actualizarKeyframes(void);
//dibuja una pieza del cohete con su keyframe actual
void dibujarPieza(int i, GLuint uniformModel);

//calculo del promedio de las normales para sombreado de Phong
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
	meshList.push_back(obj6);

	MeshModel* obj7 = new MeshModel();
	obj7->CreateMeshModel(numeroVertices, numeroIndices, 32, 6);
	meshList.push_back(obj7);
}


void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}


///////////////////////////////KEYFRAMES/////////////////////

//Posicion base del cohete (la base de la etapa 0 queda sobre el piso, que esta en y = -2)
float posXcohete = 0.0f, posYcohete = -2.0f, posZcohete = -5.0f;
//Variables que se animan: desplazamiento y giro de cada pieza
float movPieza_x[NUM_PIEZAS] = { 0.0f, 0.0f, 0.0f, 0.0f };
float movPieza_y[NUM_PIEZAS] = { 0.0f, 0.0f, 0.0f, 0.0f };
float giroPieza[NUM_PIEZAS] = { 0.0f, 0.0f, 0.0f, 0.0f };	//grados; positivo = la punta se inclina hacia +X
int piezaSeleccionada = NUM_PIEZAS;	//0-3 una pieza, 4 = todo el cohete

//Orbita de la capsula (junto con la etapa 2) al terminar la animacion
bool orbitando = false;
float anguloOrbita = 0.0f;
const float radioOrbita = 8.0f;
const float velocidadOrbita = 0.01f;
glm::vec3 centroOrbita = glm::vec3(0.0f);
glm::vec3 inicioOrbita = glm::vec3(0.0f);	//punto donde la capsula entra en orbita

//Los incrementos por tecla (0.5 u y 10 grados) se definen en Window.cpp

#define MAX_FRAMES 100
int i_max_steps = 45;	//pasos de interpolacion entre cada par de keyframes
int i_curr_steps = 0;
typedef struct _frame
{
	//Variables para GUARDAR Key Frames (una por pieza)
	float movPieza_x[NUM_PIEZAS];		//Posicion X
	float movPieza_y[NUM_PIEZAS];		//Posicion Y
	float giroPieza[NUM_PIEZAS];		//Giro sobre Z
	float movPieza_xInc[NUM_PIEZAS];	//Incremento X
	float movPieza_yInc[NUM_PIEZAS];	//Incremento Y
	float giroPiezaInc[NUM_PIEZAS];		//Incremento del giro
}FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 0;			//los frames se crean por teclado, se empieza en 0
bool play = false;
int playIndex = 0;

const char* ARCHIVO_KEYFRAMES = "keyframescohete.txt";

void imprimirPose(void)
{
	printf("Pose actual (%s seleccionado):\n", nombresPiezas[piezaSeleccionada]);
	for (int p = 0; p < NUM_PIEZAS; p++)
		printf("   %-8s x = %7.2f  y = %7.2f  giro = %6.1f\n", nombresPiezas[p], movPieza_x[p], movPieza_y[p], giroPieza[p]);
}

void imprimirFrames(void)
{
	printf("\n---- Keyframes en memoria (%d) ----\n", FrameIndex);
	for (int i = 0; i < FrameIndex; i++)
	{
		printf("Frame %d:", i);
		for (int p = 0; p < NUM_PIEZAS; p++)
			printf("  [%.2f, %.2f, %.1f]", KeyFrame[i].movPieza_x[p], KeyFrame[i].movPieza_y[p], KeyFrame[i].giroPieza[p]);
		printf("\n");
	}
	printf("(cada corchete es [x, y, giro] de etapa 0, etapa 1, etapa 2 y capsula)\n");
	printf("-----------------------------------\n");
}

void saveFrame(void) //tecla L
{
	if (FrameIndex >= MAX_FRAMES)
	{
		printf("\nYa no caben mas frames (maximo %d)\n", MAX_FRAMES);
		return;
	}
	for (int p = 0; p < NUM_PIEZAS; p++)
	{
		KeyFrame[FrameIndex].movPieza_x[p] = movPieza_x[p];
		KeyFrame[FrameIndex].movPieza_y[p] = movPieza_y[p];
		KeyFrame[FrameIndex].giroPieza[p] = giroPieza[p];
	}
	printf("\nFrame %d guardado\n", FrameIndex);
	FrameIndex++;
}

//Escribe (sobreescribe) todos los frames en keyframescohete.txt - tecla G
void guardarArchivoKeyframes(void)
{
	std::ofstream archivo(ARCHIVO_KEYFRAMES, std::ios::out | std::ios::trunc);
	if (!archivo.is_open())
	{
		printf("\nNo se pudo abrir %s para escribir\n", ARCHIVO_KEYFRAMES);
		return;
	}
	//Formato: primera linea = numero de frames; luego por frame "x y giro" de cada una de las 4 piezas
	archivo << FrameIndex << "\n";
	for (int i = 0; i < FrameIndex; i++)
	{
		for (int p = 0; p < NUM_PIEZAS; p++)
			archivo << KeyFrame[i].movPieza_x[p] << " " << KeyFrame[i].movPieza_y[p] << " " << KeyFrame[i].giroPieza[p] << (p < NUM_PIEZAS - 1 ? " " : "\n");
	}
	archivo.close();
	printf("\nSe escribieron %d frames en %s\n", FrameIndex, ARCHIVO_KEYFRAMES);
}

void resetElements(void) //regresa el cohete al frame 0
{
	for (int p = 0; p < NUM_PIEZAS; p++)
	{
		movPieza_x[p] = KeyFrame[0].movPieza_x[p];
		movPieza_y[p] = KeyFrame[0].movPieza_y[p];
		giroPieza[p] = KeyFrame[0].giroPieza[p];
	}
	orbitando = false;
}

//Lee los frames de keyframescohete.txt - tecla K
void cargarArchivoKeyframes(void)
{
	std::ifstream archivo(ARCHIVO_KEYFRAMES);
	if (!archivo.is_open())
	{
		printf("\nNo existe %s (guarda primero con G)\n", ARCHIVO_KEYFRAMES);
		return;
	}
	int n = 0;
	if (!(archivo >> n) || n < 0 || n > MAX_FRAMES)
	{
		printf("\n%s tiene un formato incorrecto\n", ARCHIVO_KEYFRAMES);
		return;
	}
	int leidos = 0;
	bool completo = true;
	for (int i = 0; i < n && completo; i++)
	{
		for (int p = 0; p < NUM_PIEZAS && completo; p++)
		{
			if (!(archivo >> KeyFrame[i].movPieza_x[p] >> KeyFrame[i].movPieza_y[p] >> KeyFrame[i].giroPieza[p]))
				completo = false;
		}
		if (completo)
			leidos++;
	}
	archivo.close();
	FrameIndex = leidos;
	play = false;
	if (FrameIndex > 0)
		resetElements();
	printf("\nSe leyeron %d frames de %s\n", leidos, ARCHIVO_KEYFRAMES);
	imprimirFrames();
}

void interpolation(void)
{
	for (int p = 0; p < NUM_PIEZAS; p++)
	{
		KeyFrame[playIndex].movPieza_xInc[p] = (KeyFrame[playIndex + 1].movPieza_x[p] - KeyFrame[playIndex].movPieza_x[p]) / i_max_steps;
		KeyFrame[playIndex].movPieza_yInc[p] = (KeyFrame[playIndex + 1].movPieza_y[p] - KeyFrame[playIndex].movPieza_y[p]) / i_max_steps;
		KeyFrame[playIndex].giroPiezaInc[p] = (KeyFrame[playIndex + 1].giroPieza[p] - KeyFrame[playIndex].giroPieza[p]) / i_max_steps;
	}
}

//Al terminar la animacion la capsula (con la etapa 2) entra en orbita: circulo horizontal
//que empieza justo donde quedo, avanzando hacia +X (hacia donde apunta su punta)
void iniciarOrbita(void)
{
	inicioOrbita = glm::vec3(posXcohete, posYcohete, posZcohete) + pivotePieza[3]
		+ glm::vec3(movPieza_x[3], movPieza_y[3], 0.0f);
	centroOrbita = inicioOrbita - glm::vec3(0.0f, 0.0f, radioOrbita);
	anguloOrbita = 0.0f;
	orbitando = true;
	printf("La capsula y la etapa 2 entraron en orbita\n");
}

void animate(void)
{
	if (play)
	{
		if (i_curr_steps >= i_max_steps) //fin de animacion entre frames
		{
			playIndex++;
			if (playIndex > FrameIndex - 2)	//fin de toda la animacion
			{
				printf("\nTermino la animacion\n");
				playIndex = 0;
				play = false;
				//se deja exactamente en el ultimo frame para que las piezas queden bien acostadas
				for (int p = 0; p < NUM_PIEZAS; p++)
				{
					movPieza_x[p] = KeyFrame[FrameIndex - 1].movPieza_x[p];
					movPieza_y[p] = KeyFrame[FrameIndex - 1].movPieza_y[p];
					giroPieza[p] = KeyFrame[FrameIndex - 1].giroPieza[p];
				}
				iniciarOrbita();
			}
			else
			{
				i_curr_steps = 0;
				interpolation();
			}
		}
		else
		{
			for (int p = 0; p < NUM_PIEZAS; p++)
			{
				movPieza_x[p] += KeyFrame[playIndex].movPieza_xInc[p];
				movPieza_y[p] += KeyFrame[playIndex].movPieza_yInc[p];
				giroPieza[p] += KeyFrame[playIndex].giroPiezaInc[p];
			}
			i_curr_steps++;
		}
	}
}

//Mueve la pieza seleccionada; si esta seleccionado todo el cohete, lo mueve y lo gira
//como un solo cuerpo alrededor del centro de la etapa 0
void moverSeleccion(float dx, float dy, float dg)
{
	if (piezaSeleccionada < NUM_PIEZAS)
	{
		movPieza_x[piezaSeleccionada] += dx;
		movPieza_y[piezaSeleccionada] += dy;
		giroPieza[piezaSeleccionada] += dg;
		return;
	}
	glm::vec2 centro = glm::vec2(pivotePieza[0].x + movPieza_x[0], pivotePieza[0].y + movPieza_y[0]);
	float c = cos(-dg * toRadians), s = sin(-dg * toRadians);
	for (int p = 0; p < NUM_PIEZAS; p++)
	{
		glm::vec2 r = glm::vec2(pivotePieza[p].x + movPieza_x[p], pivotePieza[p].y + movPieza_y[p]) - centro;
		glm::vec2 nuevo = centro + glm::vec2(c * r.x - s * r.y, s * r.x + c * r.y) + glm::vec2(dx, dy);
		movPieza_x[p] = nuevo.x - pivotePieza[p].x;
		movPieza_y[p] = nuevo.y - pivotePieza[p].y;
		giroPieza[p] += dg;
	}
}

///////////////* FIN KEYFRAMES*////////////////////////////



int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	//limita a la frecuencia del monitor para que la animacion vaya a velocidad constante
	glfwSwapInterval(1);

	//camara alejada para ver todo el despegue, las caidas y la orbita
	camera = Camera(glm::vec3(12.0f, 10.0f, 40.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 0.5f, 0.5f);

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


	//Cohete 
	//Jupiter-C: 4 piezas
	for (int p = 0; p < NUM_PIEZAS; p++)
	{
		Cohete_M[p] = Model();
		Cohete_M[p].LoadModel(archivosCohete[p]);
	}


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


	//luz direccional, solo 1 y siempre debe de existir
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.3f, 0.3f,
		0.0f, 0.0f, -1.0f);
	//contador de luces puntuales
	unsigned int pointLightCount = 0;
	//Declaracion de primer luz puntual
	pointLights[0] = PointLight(1.0f, 0.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 2.5f, 1.5f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;

	unsigned int spotLightCount = 0;
	//linterna
	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		5.0f);
	spotLightCount++;

	//luz fija
	spotLights[1] = SpotLight(0.0f, 0.0f, 1.0f,
		1.0f, 2.0f,
		5.0f, 10.0f, 0.0f,
		0.0f, -5.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		15.0f);
	spotLightCount++;


	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	movCoche = 0.0f;
	movOffset = 0.01f;
	rotllanta = 0.0f;
	rotllantaOffset = 10.0f;

	//Matrices y vectores declarados
	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec2 toffset = glm::vec2(0.0f, 0.0f);
	glm::vec3 lowerLight = glm::vec3(0.0f);
	GLfloat now = 0.0f;

	printf("\n===== Teclas de Keyframes =====\n");
	printf("7 8 9 0 : seleccionar etapa 0 / etapa 1 / etapa 2 / capsula\n");
	printf("T     : seleccionar todo el cohete (se mueve y gira como un solo cuerpo)\n");
	printf("1 / 2 : mover seleccion en +X / -X (0.5 u)\n");
	printf("3 / 4 : mover seleccion en +Y / -Y (0.5 u)\n");
	printf("5 / 6 : girar punta hacia +X / -X (10 grados)\n");
	printf("L     : guardar keyframe con la pose actual\n");
	printf("N     : nueva animacion (borra los frames en memoria)\n");
	printf("G     : escribir/sobreescribir keyframescohete.txt\n");
	printf("K     : leer keyframescohete.txt\n");
	printf("ESPACIO : reproducir animacion\n");
	printf("================================\n");


	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		angulovaria += 0.5f * deltaTime;

		if (movCoche < 30.0f)
		{
			movCoche -= movOffset * deltaTime;
		}
		rotllanta += rotllantaOffset * deltaTime;


		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		//-------Para Keyframes
		actualizarKeyframes();
		animate();
		if (orbitando)
			anguloOrbita += velocidadOrbita * deltaTime;

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
		uniformTextureOffset = shaderList[0].getOffsetLocation();

		//informacion en el shader de intensidad especular y brillo
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		// luz ligada a la camara de tipo flash
		lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

		//informacion al shader de fuentes de iluminacion
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);


		model = glm::mat4(1.0);
		modelaux = glm::mat4(1.0);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		toffset = glm::vec2(0.0f, 0.0f);

		//Piso
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


		//Cohete animado por keyframes (4 piezas)
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		dibujarPieza(0, uniformModel);
		dibujarPieza(1, uniformModel);
		dibujarPieza(2, uniformModel);
		dibujarPieza(3, uniformModel);


		//Agave
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.5f, -2.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		AgaveTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		//meshList[3]->RenderMeshModel();

		//textura con movimiento
		toffsetflechau += 0.001;
		toffsetflechav += 0.0;
		if (toffsetflechau > 1.0)
			toffsetflechau = 0.0;
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

		//plano con todos los numeros
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

		//numero 1
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-10.0f, 2.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		NumerosTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[6]->RenderMeshModel();

		for (int i = 1; i < 4; i++)
		{
			//numeros 2-4
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
			//numeros 5-8
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

		//numero cambiante
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

		//numero 1 / numero 2
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
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[5]->RenderMeshModel();

		glDisable(GL_BLEND);

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}


//Dibuja la pieza i: se coloca su centro (pivote) donde indica el keyframe y se gira sobre el.
//Si la capsula esta orbitando, ella y la etapa 2 se mueven juntas: el punto de entrada a la
//orbita recorre un circulo horizontal y ambas piezas giran con el como un solo cuerpo.
void dibujarPieza(int i, GLuint uniformModel)
{
	glm::mat4 model(1.0);
	if ((i == 2 || i == 3) && orbitando)
	{
		model = glm::translate(model, centroOrbita + radioOrbita * glm::vec3(sin(anguloOrbita), 0.0f, cos(anguloOrbita)));
		model = glm::rotate(model, anguloOrbita, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, -inicioOrbita);
	}
	model = glm::translate(model, glm::vec3(posXcohete, posYcohete, posZcohete) + pivotePieza[i]
		+ glm::vec3(movPieza_x[i], movPieza_y[i], 0.0f));
	//giro positivo inclina la punta hacia +X (rotacion horaria sobre Z vista de frente)
	model = glm::rotate(model, -giroPieza[i] * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, -pivotePieza[i]);
	model = glm::scale(model, glm::vec3(escalaCohete, escalaCohete, escalaCohete));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	Cohete_M[i].RenderModel();
}

//Aplica a los keyframes lo que Window detecto del teclado.
//La asignacion de teclas esta en Window.cpp; aqui solo se consulta.
void actualizarKeyframes(void)
{
	//Reproducir (ESPACIO)
	if (mainWindow.tomarReproducir())
	{
		if (!play && FrameIndex > 1)
		{
			resetElements();
			playIndex = 0;
			i_curr_steps = 0;
			interpolation();
			play = true;
			printf("\nReproduciendo animacion (%d frames)\n", FrameIndex);
		}
		else if (FrameIndex <= 1)
			printf("\nSe necesitan al menos 2 frames para reproducir\n");
	}

	//Archivo (G escribir, K leer)
	if (mainWindow.tomarGuardarArchivo())
		guardarArchivoKeyframes();
	if (mainWindow.tomarCargarArchivo())
		cargarArchivoKeyframes();

	//Lo pendiente del teclado se consume siempre para que no se acumule
	float dx = mainWindow.tomarDeltaX();
	float dy = mainWindow.tomarDeltaY();
	float dg = mainWindow.tomarDeltaGiro();
	bool guardar = mainWindow.tomarGuardarFrame();
	bool nueva = mainWindow.tomarNuevaAnimacion();
	int pieza = mainWindow.tomarPieza();

	//Mientras se reproduce no se edita la pose
	if (play)
		return;

	//Seleccion de pieza (7, 8, 9, 0 y T)
	if (pieza >= 0 && pieza <= NUM_PIEZAS)
	{
		piezaSeleccionada = pieza;
		printf("Seleccionado: %s\n", nombresPiezas[piezaSeleccionada]);
	}

	if (dx != 0.0f || dy != 0.0f || dg != 0.0f)
	{
		orbitando = false;
		moverSeleccion(dx, dy, dg);
		imprimirPose();
	}

	//Guardar keyframe (L)
	if (guardar)
	{
		saveFrame();
		imprimirFrames();
	}

	//Nueva animacion (N)
	if (nueva)
	{
		FrameIndex = 0;
		for (int p = 0; p < NUM_PIEZAS; p++)
		{
			movPieza_x[p] = 0.0f;
			movPieza_y[p] = 0.0f;
			giroPieza[p] = 0.0f;
		}
		orbitando = false;
		piezaSeleccionada = NUM_PIEZAS;
		printf("\nFrames borrados. Coloca el cohete y presiona L para el frame 0\n");
	}
}