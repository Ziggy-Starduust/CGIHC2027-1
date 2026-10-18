/*
Práctica 8: Iluminación 2 
*/
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
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

//para iluminación
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<MeshModel*> meshListModel; // recibe xyz uv nx ny nz
std::vector<Shader> shaderList;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;

// Añadir Texturas del dado y del holocron

Texture dadoTexture;

Texture holocronTexture;


//Rover jerarquizado
Model cuerpoModel, brazoModel;
Model ruedaDelIzqModel, ruedaDelDerModel;
Model ruedaMedIzqModel, ruedaMedDerModel;
Model ruedaTraIzqModel, ruedaTraDerModel;

// Para las llantas del rover al moverse
float rotacionPatas = 0.0f;glm::vec3 posRover = glm::vec3(1.0f, -0.5f, 6.0f);

//Añadir modelo del avión.
Model AvionCuerpo_M, AvionTimon_M, AvionElevonIzq_M, AvionElevonDer_M;
glm::vec3 posAvion = glm::vec3(0.0f, 5.0f, 6.0f);
float angTimon = 0.0f, angElevon = 0.0f;

// Ejercicio 1: colores del faro inferior del avión
glm::vec3 coloresFaro[6] = {
	glm::vec3(1.0f, 0.0f, 0.0f),   // rojo
	glm::vec3(1.0f, 1.0f, 0.0f),   // amarillo
	glm::vec3(0.0f, 1.0f, 0.0f),   // verde
	glm::vec3(0.0f, 1.0f, 1.0f),   // cian
	glm::vec3(0.0f, 0.0f, 1.0f),   // azul
	glm::vec3(1.0f, 0.0f, 1.0f)    // magenta
};

// Función para la luz del Holocrón
glm::vec3 posHolocron = glm::vec3(-7.0f, 7.0f, 6.0f);

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

// Nave Aphelion
Model Nave_M;
Texture plasmaTexture;
glm::vec3 posNave = glm::vec3(6.0f, 4.0f, 0.0f);
// faro inferior de Roger: uno por color, creados una sola vez
SpotLight faroAbajoColores[6];

// luces que SI se mandan al shader
PointLight lucesActivas[MAX_POINT_LIGHTS];
SpotLight spotActivas[MAX_SPOT_LIGHTS];

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";


//función de calculo de normales por promedio de vértices 
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
	
	MeshModel *obj1 = new MeshModel();
	obj1->CreateMeshModel(vertices, indices, 32, 12);
	meshListModel.push_back(obj1);

	MeshModel *obj2 = new MeshModel();
	obj2->CreateMeshModel(vertices, indices, 32, 12);
	meshListModel.push_back(obj2);

	MeshModel *obj3 = new MeshModel();
	obj3->CreateMeshModel(floorVertices, floorIndices, 32, 6);
	meshListModel.push_back(obj3);

	MeshModel *obj4 = new MeshModel();
	obj4->CreateMeshModel(vegetacionVertices, vegetacionIndices, 64, 12);
	meshListModel.push_back(obj4);

	calcAverageNormals(indices, 12, vertices, 32, 8, 5);

	calcAverageNormals(vegetacionIndices, 12, vegetacionVertices, 64, 8, 5);

}


void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}


void CrearDado() //sustituir con su propio dado, para que tenga la textura de la práctica 6
{
	unsigned int cubo_indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,

		// back
		8, 9, 10,
		10, 11, 8,

		// left
		12, 13, 14,
		14, 15, 12,
		// bottom
		16, 17, 18,
		18, 19, 16,
		// top
		20, 21, 22,
		22, 23, 20,

		// right
		4, 5, 6,
		6, 7, 4,

	};	GLfloat cubo_vertices[] = {
    //  x      y      z        u      v         nx     ny     nz
    // front  -> Rebel Alliance
    -0.5f, -0.5f,  0.5f,    0.00f, 0.75f,    0.0f,  0.0f, -1.0f,  //0
     0.5f, -0.5f,  0.5f,    0.25f, 0.75f,    0.0f,  0.0f, -1.0f,  //1
     0.5f,  0.5f,  0.5f,    0.25f, 1.00f,    0.0f,  0.0f, -1.0f,  //2
    -0.5f,  0.5f,  0.5f,    0.00f, 1.00f,    0.0f,  0.0f, -1.0f,  //3
    // right  -> Galactic Empire
     0.5f, -0.5f,  0.5f,    0.25f, 0.75f,   -1.0f,  0.0f,  0.0f,  //4
     0.5f, -0.5f, -0.5f,    0.50f, 0.75f,   -1.0f,  0.0f,  0.0f,  //5
     0.5f,  0.5f, -0.5f,    0.50f, 1.00f,   -1.0f,  0.0f,  0.0f,  //6
     0.5f,  0.5f,  0.5f,    0.25f, 1.00f,   -1.0f,  0.0f,  0.0f,  //7
    // back   -> Galactic Republic
    -0.5f, -0.5f, -0.5f,    0.75f, 0.75f,    0.0f,  0.0f,  1.0f,  //8
     0.5f, -0.5f, -0.5f,    0.50f, 0.75f,    0.0f,  0.0f,  1.0f,  //9
     0.5f,  0.5f, -0.5f,    0.50f, 1.00f,    0.0f,  0.0f,  1.0f,  //10
    -0.5f,  0.5f, -0.5f,    0.75f, 1.00f,    0.0f,  0.0f,  1.0f,  //11
    // left   -> Sith Empire
    -0.5f, -0.5f, -0.5f,    0.75f, 0.75f,    1.0f,  0.0f,  0.0f,  //12
    -0.5f, -0.5f,  0.5f,    1.00f, 0.75f,    1.0f,  0.0f,  0.0f,  //13
    -0.5f,  0.5f,  0.5f,    1.00f, 1.00f,    1.0f,  0.0f,  0.0f,  //14
    -0.5f,  0.5f, -0.5f,    0.75f, 1.00f,    1.0f,  0.0f,  0.0f,  //15
    // bottom -> Separatists
    -0.5f, -0.5f,  0.5f,    0.00f, 0.75f,    0.0f,  1.0f,  0.0f,  //16
     0.5f, -0.5f,  0.5f,    0.25f, 0.75f,    0.0f,  1.0f,  0.0f,  //17
     0.5f, -0.5f, -0.5f,    0.25f, 0.50f,    0.0f,  1.0f,  0.0f,  //18
    -0.5f, -0.5f, -0.5f,    0.00f, 0.50f,    0.0f,  1.0f,  0.0f,  //19
    // top    -> Boba Fett Crest
    -0.5f,  0.5f,  0.5f,    0.25f, 0.50f,    0.0f, -1.0f,  0.0f,  //20
     0.5f,  0.5f,  0.5f,    0.50f, 0.50f,    0.0f, -1.0f,  0.0f,  //21
     0.5f,  0.5f, -0.5f,    0.50f, 0.75f,    0.0f, -1.0f,  0.0f,  //22
    -0.5f,  0.5f, -0.5f,    0.25f, 0.75f,    0.0f, -1.0f,  0.0f,  //23
};

	MeshModel* dado = new MeshModel();
	dado->CreateMeshModel(cubo_vertices, cubo_indices, 192, 36);
	meshListModel.push_back(dado);

}


void CrearHolocron()
{
	// Holocron texturizado con Textures/holocron_logos.png: 28 triangulos con vertices propios.
	// Normales planas por cara, invertidas (apuntan hacia adentro) igual que el piso y el dado,
	// porque asi lo espera el shader de iluminacion del profe.
	unsigned int holocron_indices[] = {
		0, 1, 2,
		3, 4, 5,
		6, 7, 8,
		9, 10, 11,
		12, 13, 14,
		15, 16, 17,
		18, 19, 20,
		21, 22, 23,
		24, 25, 26,
		27, 28, 29,
		30, 31, 32,
		33, 34, 35,
		36, 37, 38,
		39, 40, 41,
		42, 43, 44,
		45, 46, 47,
		48, 49, 50,
		51, 52, 53,
		54, 55, 56,
		57, 58, 59,
		60, 61, 62,
		63, 64, 65,
		66, 67, 68,
		69, 70, 71,
		72, 73, 74,
		75, 76, 77,
		78, 79, 80,
		81, 82, 83,
	};

	GLfloat holocron_vertices[] = {
		//x            y            z            S        T         NX      NY      NZ
		// triangulo 0 -> cara Boba Fett
		 -2.453980f,  -0.063388f,  -2.477451f,	0.8820f, 0.4776f,	 0.529f,  0.589f,  0.611f,
		 -1.185055f,  -2.530499f,  -1.198261f,	0.8740f, 0.2763f,	 0.529f,  0.589f,  0.611f,
		 -2.517274f,  -2.579095f,   0.002115f,	0.9922f, 0.2724f,	 0.529f,  0.589f,  0.611f,
		// triangulo 1 -> cara Separatists
		 -2.453980f,  -0.063388f,  -2.477451f,	0.2584f, 0.6256f,	 0.998f, -0.040f,  0.038f,
		 -2.646786f,  -0.139047f,   2.493757f,	0.4916f, 0.6220f,	 0.998f, -0.040f,  0.038f,
		 -2.451372f,   2.421759f,   0.077538f,	0.3781f, 0.7422f,	 0.998f, -0.040f,  0.038f,
		// triangulo 2 -> cara First Order
		  2.521206f,   2.448241f,   0.063388f,	0.2578f, 0.4799f,	-0.566f, -0.599f,  0.566f,
		  2.534944f,  -0.061146f,  -2.580280f,	0.3823f, 0.2701f,	-0.566f, -0.599f,  0.566f,
		  0.049649f,   2.440128f,  -2.417217f,	0.4922f, 0.4792f,	-0.566f, -0.599f,  0.566f,
		// triangulo 3 -> cara Old Galactic Republic
		  0.049649f,   2.440128f,  -2.417217f,	0.7578f, 0.7276f,	 0.574f, -0.587f,  0.571f,
		 -2.453980f,  -0.063388f,  -2.477451f,	0.8721f, 0.5224f,	 0.574f, -0.587f,  0.571f,
		 -2.451372f,   2.421759f,   0.077538f,	0.9922f, 0.7261f,	 0.574f, -0.587f,  0.571f,
		// triangulo 4 -> cara Rebel Alliance
		 -0.140696f,  -2.601934f,   2.558561f,	0.1200f, 0.7631f,	 0.022f, -0.004f, -1.000f,
		  0.051152f,   2.399781f,   2.545043f,	0.1285f, 0.9869f,	 0.022f, -0.004f, -1.000f,
		 -2.646786f,  -0.139047f,   2.493757f,	0.0078f, 0.8733f,	 0.022f, -0.004f, -1.000f,
		// triangulo 5 -> cara New Republic
		 -2.451372f,   2.421759f,   0.077538f,	0.5078f, 0.7306f,	 0.565f, -0.589f, -0.578f,
		 -2.646786f,  -0.139047f,   2.493757f,	0.6111f, 0.5194f,	 0.565f, -0.589f, -0.578f,
		  0.051152f,   2.399781f,   2.545043f,	0.7422f, 0.7288f,	 0.565f, -0.589f, -0.578f,
		// triangulo 6 -> cara Rebels Phoenix
		  0.051152f,   2.399781f,   2.545043f,	0.0078f, 0.4749f,	-0.573f, -0.577f, -0.582f,
		  2.593003f,  -0.085222f,   2.505755f,	0.1309f, 0.2712f,	-0.573f, -0.577f, -0.582f,
		  2.521206f,   2.448241f,   0.063388f,	0.2422f, 0.4788f,	-0.573f, -0.577f, -0.582f,
		// triangulo 7 -> cara Mandalorian Crest
		 -2.646786f,  -0.139047f,   2.493757f,	0.6248f, 0.4774f,	 0.593f,  0.589f, -0.548f,
		 -1.328995f,  -2.578386f,   1.297838f,	0.6259f, 0.2746f,	 0.593f,  0.589f, -0.548f,
		 -0.140696f,  -2.601934f,   2.558561f,	0.7422f, 0.2726f,	 0.593f,  0.589f, -0.548f,
		// triangulo 8 -> cara Rebel Fist
		  2.593003f,  -0.085222f,   2.505755f,	0.1283f, 0.2238f,	-0.593f,  0.568f, -0.570f,
		  1.385028f,  -2.549476f,   1.308677f,	0.1299f, 0.0372f,	-0.593f,  0.568f, -0.570f,
		  2.632201f,  -2.553892f,   0.007226f,	0.2422f, 0.0369f,	-0.593f,  0.568f, -0.570f,
		// triangulo 9 -> cara The Cold Order
		  2.534944f,  -0.061146f,  -2.580280f,	0.3760f, 0.2289f,	-0.538f,  0.577f,  0.615f,
		  1.344182f,  -2.542571f,  -1.294387f,	0.3794f, 0.0268f,	-0.538f,  0.577f,  0.615f,
		  0.081432f,  -2.513204f,  -2.426005f,	0.4922f, 0.0292f,	-0.538f,  0.577f,  0.615f,
		// triangulo 10 -> cara Galactic Empire
		  0.081432f,  -2.513204f,  -2.426005f,	0.3734f, 0.7586f,	 0.064f, -0.001f,  0.998f,
		  0.049649f,   2.440128f,  -2.417217f,	0.3749f, 0.9914f,	 0.064f, -0.001f,  0.998f,
		  2.534944f,  -0.061146f,  -2.580280f,	0.2578f, 0.8738f,	 0.064f, -0.001f,  0.998f,
		// triangulo 11 -> cara Black Sun
		  1.344182f,  -2.542571f,  -1.294387f,	0.9336f, 0.8131f,	-0.011f,  1.000f,  0.003f,
		  1.385028f,  -2.549476f,   1.308677f,	0.9354f, 0.9315f,	-0.011f,  1.000f,  0.003f,
		 -1.328995f,  -2.578386f,   1.297838f,	0.8119f, 0.9311f,	-0.011f,  1.000f,  0.003f,
		// triangulo 12 -> cara SITH Empire
		  2.632201f,  -2.553892f,   0.007226f,	0.1229f, 0.5097f,	-1.000f, -0.022f,  0.006f,
		  2.521206f,   2.448241f,   0.063388f,	0.1204f, 0.7403f,	-1.000f, -0.022f,  0.006f,
		  2.593003f,  -0.085222f,   2.505755f,	0.0078f, 0.6235f,	-1.000f, -0.022f,  0.006f,
		// triangulo 13 -> cara Galactic Republic
		  0.049649f,   2.440128f,  -2.417217f,	0.6257f, 0.9919f,	 0.011f, -1.000f, -0.008f,
		  0.051152f,   2.399781f,   2.545043f,	0.6257f, 0.7581f,	 0.011f, -1.000f, -0.008f,
		  2.521206f,   2.448241f,   0.063388f,	0.7422f, 0.8750f,	 0.011f, -1.000f, -0.008f,
		// triangulo 14 -> cara Galactic Republic
		  0.049649f,   2.440128f,  -2.417217f,	0.6257f, 0.9919f,	-0.001f, -1.000f, -0.008f,
		 -2.451372f,   2.421759f,   0.077538f,	0.5078f, 0.8744f,	-0.001f, -1.000f, -0.008f,
		  0.051152f,   2.399781f,   2.545043f,	0.6257f, 0.7581f,	-0.001f, -1.000f, -0.008f,
		// triangulo 15 -> cara Galactic Empire
		  0.081432f,  -2.513204f,  -2.426005f,	0.3734f, 0.7586f,	-0.022f, -0.002f,  1.000f,
		 -2.453980f,  -0.063388f,  -2.477451f,	0.4922f, 0.8737f,	-0.022f, -0.002f,  1.000f,
		  0.049649f,   2.440128f,  -2.417217f,	0.3749f, 0.9914f,	-0.022f, -0.002f,  1.000f,
		// triangulo 16 -> cara SITH Empire
		  2.632201f,  -2.553892f,   0.007226f,	0.1229f, 0.5097f,	-1.000f, -0.022f,  0.016f,
		  2.534944f,  -0.061146f,  -2.580280f,	0.2422f, 0.6246f,	-1.000f, -0.022f,  0.016f,
		  2.521206f,   2.448241f,   0.063388f,	0.1204f, 0.7403f,	-1.000f, -0.022f,  0.016f,
		// triangulo 17 -> cara Rebel Alliance
		 -0.140696f,  -2.601934f,   2.558561f,	0.1200f, 0.7631f,	-0.017f, -0.002f, -1.000f,
		  2.593003f,  -0.085222f,   2.505755f,	0.2422f, 0.8757f,	-0.017f, -0.002f, -1.000f,
		  0.051152f,   2.399781f,   2.545043f,	0.1285f, 0.9869f,	-0.017f, -0.002f, -1.000f,
		// triangulo 18 -> cara Separatists
		 -2.453980f,  -0.063388f,  -2.477451f,	0.2584f, 0.6256f,	 0.999f,  0.013f,  0.039f,
		 -2.517274f,  -2.579095f,   0.002115f,	0.3747f, 0.5078f,	 0.999f,  0.013f,  0.039f,
		 -2.646786f,  -0.139047f,   2.493757f,	0.4916f, 0.6220f,	 0.999f,  0.013f,  0.039f,
		// triangulo 19 -> cara Boba Fett Crest
		 -2.453980f,  -0.063388f,  -2.477451f,	0.8820f, 0.4776f,	 0.558f,  0.590f,  0.584f,
		  0.081432f,  -2.513204f,  -2.426005f,	0.7578f, 0.2811f,	 0.558f,  0.590f,  0.584f,
		 -1.185055f,  -2.530499f,  -1.198261f,	0.8740f, 0.2763f,	 0.558f,  0.590f,  0.584f,
		// triangulo 20 -> cara Mandalorian Crest
		 -2.646786f,  -0.139047f,   2.493757f,	0.6248f, 0.4774f,	 0.595f,  0.589f, -0.546f,
		 -2.517274f,  -2.579095f,   0.002115f,	0.5078f, 0.2743f,	 0.595f,  0.589f, -0.546f,
		 -1.328995f,  -2.578386f,   1.297838f,	0.6259f, 0.2746f,	 0.595f,  0.589f, -0.546f,
		// triangulo 21 -> cara Rebel Fist
		  2.593003f,  -0.085222f,   2.505755f,	0.1283f, 0.2238f,	-0.534f,  0.567f, -0.628f,
		 -0.140696f,  -2.601934f,   2.558561f,	0.0078f, 0.0262f,	-0.534f,  0.567f, -0.628f,
		  1.385028f,  -2.549476f,   1.308677f,	0.1299f, 0.0372f,	-0.534f,  0.567f, -0.628f,
		// triangulo 22 -> cara The Cold Order
		  2.534944f,  -0.061146f,  -2.580280f,	0.3760f, 0.2289f,	-0.578f,  0.577f,  0.577f,
		  2.632201f,  -2.553892f,   0.007226f,	0.2578f, 0.0211f,	-0.578f,  0.577f,  0.577f,
		  1.344182f,  -2.542571f,  -1.294387f,	0.3794f, 0.0268f,	-0.578f,  0.577f,  0.577f,
		// triangulo 23 -> cara Black Sun
		 -1.328995f,  -2.578386f,   1.297838f,	0.8119f, 0.9311f,	-0.020f,  1.000f,  0.018f,
		 -2.517274f,  -2.579095f,   0.002115f,	0.7578f, 0.8721f,	-0.020f,  1.000f,  0.018f,
		 -1.185055f,  -2.530499f,  -1.198261f,	0.8185f, 0.8174f,	-0.020f,  1.000f,  0.018f,
		// triangulo 24 -> cara Black Sun
		 -1.185055f,  -2.530499f,  -1.198261f,	0.8185f, 0.8174f,	 0.006f,  1.000f,  0.020f,
		  0.081432f,  -2.513204f,  -2.426005f,	0.8761f, 0.7616f,	 0.006f,  1.000f,  0.020f,
		  1.344182f,  -2.542571f,  -1.294387f,	0.9336f, 0.8131f,	 0.006f,  1.000f,  0.020f,
		// triangulo 25 -> cara Black Sun
		  1.344182f,  -2.542571f,  -1.294387f,	0.9336f, 0.8131f,	 0.006f,  1.000f,  0.003f,
		  2.632201f,  -2.553892f,   0.007226f,	0.9922f, 0.8723f,	 0.006f,  1.000f,  0.003f,
		  1.385028f,  -2.549476f,   1.308677f,	0.9354f, 0.9315f,	 0.006f,  1.000f,  0.003f,
		// triangulo 26 -> cara Black Sun
		  1.385028f,  -2.549476f,   1.308677f,	0.9354f, 0.9315f,	-0.011f,  1.000f,  0.029f,
		 -0.140696f,  -2.601934f,   2.558561f,	0.8660f, 0.9884f,	-0.011f,  1.000f,  0.029f,
		 -1.328995f,  -2.578386f,   1.297838f,	0.8119f, 0.9311f,	-0.011f,  1.000f,  0.029f,
		// triangulo 27 -> cara Black Sun
		 -1.328995f,  -2.578386f,   1.297838f,	0.8119f, 0.9311f,	 0.006f,  1.000f,  0.019f,
		 -1.185055f,  -2.530499f,  -1.198261f,	0.8185f, 0.8174f,	 0.006f,  1.000f,  0.019f,
		  1.344182f,  -2.542571f,  -1.294387f,	0.9336f, 0.8131f,	 0.006f,  1.000f,  0.019f,
	};

	MeshModel* holocron = new MeshModel();
	holocron->CreateMeshModel(holocron_vertices, holocron_indices, 672, 84);
	meshListModel.push_back(holocron);
}

void CrearFuego()
{
	unsigned int fuego_indices[] = {
		0, 1, 2,    0, 2, 3,
		4, 5, 6,    4, 6, 7,
		8, 9, 10,   8, 10, 11,
		12, 13, 14, 12, 14, 15
	};
	GLfloat fuego_vertices[] = {
		//  x         y        z       u     v       nx    ny    nz
		// plano horizontal
		-0.5f,     0.0f,     0.0f,   0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
		 0.5f,     0.0f,     0.0f,   1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
		 0.5f,     0.0f,    -1.0f,   1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
		-0.5f,     0.0f,    -1.0f,   0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
		// plano vertical
		 0.0f,    -0.5f,     0.0f,   0.0f, 1.0f,  -1.0f,  0.0f, 0.0f,
		 0.0f,     0.5f,     0.0f,   1.0f, 1.0f,  -1.0f,  0.0f, 0.0f,
		 0.0f,     0.5f,    -1.0f,   1.0f, 0.0f,  -1.0f,  0.0f, 0.0f,
		 0.0f,    -0.5f,    -1.0f,   0.0f, 0.0f,  -1.0f,  0.0f, 0.0f,
		 // diagonal 1
		 -0.3536f, -0.3536f,  0.0f,   0.0f, 1.0f,   0.7f, -0.7f, 0.0f,
		  0.3536f,  0.3536f,  0.0f,   1.0f, 1.0f,   0.7f, -0.7f, 0.0f,
		  0.3536f,  0.3536f, -1.0f,   1.0f, 0.0f,   0.7f, -0.7f, 0.0f,
		 -0.3536f, -0.3536f, -1.0f,   0.0f, 0.0f,   0.7f, -0.7f, 0.0f,
		 // diagonal 2
		 -0.3536f,  0.3536f,  0.0f,   0.0f, 1.0f,  -0.7f, -0.7f, 0.0f,
		  0.3536f, -0.3536f,  0.0f,   1.0f, 1.0f,  -0.7f, -0.7f, 0.0f,
		  0.3536f, -0.3536f, -1.0f,   1.0f, 0.0f,  -0.7f, -0.7f, 0.0f,
		 -0.3536f,  0.3536f, -1.0f,   0.0f, 0.0f,  -0.7f, -0.7f, 0.0f,
	};
	MeshModel* fuego = new MeshModel();
	fuego->CreateMeshModel(fuego_vertices, fuego_indices, 128, 24);
	meshListModel.push_back(fuego);   // queda en meshListModel[6]
}




int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();
	CreateObjects();
	CrearDado();
	CrearHolocron();
	CrearFuego(); //Para fuego de la nave
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.5f);

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
	dadoTexture = Texture("Textures/dado_sw.png");
	dadoTexture.LoadTextureA();
	holocronTexture = Texture("Textures/holocron_logos.png");
	holocronTexture.LoadTexture();

	
	cuerpoModel = Model();
	cuerpoModel.LoadModel("Models/Cuerpo.obj");

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
	
	//Añadir modelo del avión
	AvionCuerpo_M = Model();     AvionCuerpo_M.LoadModel("Models/roger_cuerpo.obj");
	AvionTimon_M = Model();      AvionTimon_M.LoadModel("Models/roger_timon.obj");
	AvionElevonIzq_M = Model();  AvionElevonIzq_M.LoadModel("Models/roger_elevon_izq.obj");
	AvionElevonDer_M = Model();  AvionElevonDer_M.LoadModel("Models/roger_elevon_der.obj");

	//Añadir modelo de la nave Aphelion y textura del fuego
	Nave_M = Model();
	Nave_M.LoadModel("Models/a4o_aphelion.obj");
	plasmaTexture = Texture("Textures/plasma_azul.png");
	plasmaTexture.LoadTextureA();

	//Avion_M = Model();
	//Avion_M.LoadModel("Models/");

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

	//se crean mas luces puntuales y spotlight 

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	// nave: luz cian sobre la cabina
	pointLights[0] = PointLight(0.2f, 0.9f, 1.0f,
		0.0f, 1.5f,
		0.0f, 0.0f, 0.0f,
		1.0f, 0.15f, 0.05f);
	// motor 1 (izquierdo): azul
	pointLights[1] = PointLight(0.1f, 0.3f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 0.3f, 0.2f);
	// motor 2 (derecho): azul
	pointLights[2] = PointLight(0.1f, 0.3f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 0.3f, 0.2f);
	// faro delantero de la nave
	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.02f,
		18.0f);

	// ubicación del uniform de emisión (una sola vez)
	shaderList[0].UseShader();
	GLint programaActual;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programaActual);
	GLint uniformEmision = glGetUniformLocation(programaActual, "emision");

	// variables que usa el ciclo (se declaran una sola vez)
	glm::mat4 matNave(1.0f);
	glm::mat4 model(1.0f);
	glm::mat4 modelaux(1.0f);
	glm::vec3 color(1.0f, 1.0f, 1.0f);
	unsigned int numPoint = 0, numSpot = 0;
	
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

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

		//información en el shader de intensidad especular y brillo
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();
		
		glUniform3f(uniformEmision, 0.0f, 0.0f, 0.0f);

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		// ---------------- NAVE APHELION ----------------
		matNave = glm::mat4(1.0f);
		matNave = glm::translate(matNave, posNave);
		matNave = glm::scale(matNave, glm::vec3(0.4f, 0.4f, 0.4f));

		// las luces siguen a la nave (coordenadas del modelo -> mundo)
		pointLights[0].SetPos(glm::vec3(matNave * glm::vec4(0.0f, 2.2f, 0.5f, 1.0f)));      // cabina
		spotLights[0].SetFlash(glm::vec3(matNave * glm::vec4(0.0f, 0.4f, 3.3f, 1.0f)),      // punta
			glm::normalize(glm::vec3(matNave * glm::vec4(0.0f, -0.3f, 1.0f, 0.0f))));
		pointLights[1].SetPos(glm::vec3(matNave * glm::vec4(-0.76f, 0.58f, -3.8f, 1.0f)));  // motor 1
		pointLights[2].SetPos(glm::vec3(matNave * glm::vec4(0.76f, 0.58f, -3.8f, 1.0f)));  // motor 2

		// SOLO se mandan al shader las luces encendidas
		numPoint = 0;
		if (mainWindow.getLucesNave()) lucesActivas[numPoint++] = pointLights[0];
		if (mainWindow.getMotor1())    lucesActivas[numPoint++] = pointLights[1];
		if (mainWindow.getMotor2())    lucesActivas[numPoint++] = pointLights[2];

		numSpot = 0;
		if (mainWindow.getLucesNave()) spotActivas[numSpot++] = spotLights[0];

		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(lucesActivas, numPoint);
		shaderList[0].SetSpotLights(spotActivas, numSpot);

		glUniform3f(uniformEmision, 0.0f, 0.0f, 0.0f);

		model = glm::mat4(1.0);
		modelaux = glm::mat4(1.0);
		color = glm::vec3(1.0f, 1.0f, 1.0f);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[2]->RenderMeshModel();

		// NAVE APHELION
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(matNave));
		Nave_M.RenderModel();

		//Agave
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 1.0f, -4.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		AgaveTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[3]->RenderMeshModel();
		glDisable(GL_BLEND);

		// FUEGO DE LOS MOTORES: solo se dibuja si el motor está encendido
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);     // aditivo: lo negro no se ve, lo azul suma brillo
		glDepthMask(GL_FALSE);                 // la flama no tapa lo que está detrás
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		plasmaTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		glUniform3f(uniformEmision, 1.0f, 1.0f, 1.0f);   // brilla por sí sola

		if (mainWindow.getMotor1())
		{
			model = matNave;
			model = glm::translate(model, glm::vec3(-0.76f, 0.58f, -3.3f));   // tobera izquierda
			model = glm::scale(model, glm::vec3(0.6f, 0.6f, 1.8f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			meshListModel[6]->RenderMeshModel();
		}
		if (mainWindow.getMotor2())
		{
			model = matNave;
			model = glm::translate(model, glm::vec3(0.76f, 0.58f, -3.3f));    // tobera derecha
			model = glm::scale(model, glm::vec3(0.6f, 0.6f, 1.8f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			meshListModel[6]->RenderMeshModel();
		}

		glUniform3f(uniformEmision, 0.0f, 0.0f, 0.0f);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);

		glUseProgram(0);
		
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		AgaveTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[3]->RenderMeshModel();
		glDisable(GL_BLEND);

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
