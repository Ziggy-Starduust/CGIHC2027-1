/*Práctica 4: Modelado Jerárquico.
Se implementa el uso de matrices adicionales para almacenar información de transformaciones geométricas que se quiere
heredar entre diversas instancias para que estén unidas
Teclas de la F a la K para rotaciones de articulaciones
*/
#include <stdio.h>
#include <string.h>
#include<cmath>
#include<vector>
#include <glew.h>
#include <glfw3.h>
//glm
#include<glm.hpp>
#include<gtc\matrix_transform.hpp>
#include<gtc\type_ptr.hpp>
#include <gtc\random.hpp>
//clases para dar orden y limpieza al còdigo
#include"Mesh.h"
#include"Shader.h"
#include"Sphere.h"
#include"Window.h"
#include"Camera.h"
//tecla E: Rotar sobre el eje X
//tecla R: Rotar sobre el eje Y
//tecla T: Rotar sobre el eje Z


using std::vector;

//Dimensiones de la ventana
const float toRadians = 3.14159265f/180.0; //grados a radianes
const float PI = 3.14159265f;
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;
Camera camera;
Window mainWindow;
vector<Mesh*> meshList;
vector<Shader>shaderList;
//Vertex Shader
static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
Sphere sp = Sphere(1.0, 20, 20); //recibe radio, slices, stacks




void CrearCubo()
{
	unsigned int cubo_indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
	};

	GLfloat cubo_vertices[] = {
		// front
		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		// back
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f
	};
	Mesh* cubo = new Mesh();
	cubo->CreateMesh(cubo_vertices, cubo_indices, 24, 36);
	meshList.push_back(cubo);
}

// Pirámide triangular regular
void CrearPiramideTriangular()
{
	unsigned int indices_piramide_triangular[] = {
			0,1,2,
			1,3,2,
			3,0,2,
			1,0,3

	};
	GLfloat vertices_piramide_triangular[] = {
		-0.5f, -0.5f,0.0f,	//0
		0.5f,-0.5f,0.0f,	//1
		0.0f,0.5f, -0.25f,	//2
		0.0f,-0.5f,-0.5f,	//3

	};
	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices_piramide_triangular, indices_piramide_triangular, 12, 12);
	meshList.push_back(obj1);

}
/*
Crear cilindro y cono con arreglos dinámicos vector creados en el Semestre 2023 - 1 : por Sánchez Pérez Omar Alejandro
*/
void CrearCilindro(int res, float R) {

	//constantes utilizadas en los ciclos for
	int n, i;
	//cálculo del paso interno en la circunferencia y variables que almacenarán cada coordenada de cada vértice
	GLfloat dt = 2 * PI / res, x, z, y = -0.5f;

	vector<GLfloat> vertices;
	vector<unsigned int> indices;

	//ciclo for para crear los vértices de las paredes del cilindro
	for (n = 0; n <= (res); n++) {
		if (n != res) {
			x = R * cos((n)*dt);
			z = R * sin((n)*dt);
		}
		//caso para terminar el círculo
		else {
			x = R * cos((0)*dt);
			z = R * sin((0)*dt);
		}
		for (i = 0; i < 6; i++) {
			switch (i) {
			case 0:
				vertices.push_back(x);
				break;
			case 1:
				vertices.push_back(y);
				break;
			case 2:
				vertices.push_back(z);
				break;
			case 3:
				vertices.push_back(x);
				break;
			case 4:
				vertices.push_back(0.5);
				break;
			case 5:
				vertices.push_back(z);
				break;
			}
		}
	}

	//ciclo for para crear la circunferencia inferior
	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0:
				vertices.push_back(x);
				break;
			case 1:
				vertices.push_back(-0.5f);
				break;
			case 2:
				vertices.push_back(z);
				break;
			}
		}
	}

	//ciclo for para crear la circunferencia superior
	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0:
				vertices.push_back(x);
				break;
			case 1:
				vertices.push_back(0.5);
				break;
			case 2:
				vertices.push_back(z);
				break;
			}
		}
	}

	//Se generan los indices de los vértices
	for (i = 0; i < vertices.size(); i++) indices.push_back(i);

	//se genera el mesh del cilindro
	Mesh *cilindro = new Mesh();
	cilindro->CreateMeshGeometry(vertices, indices, vertices.size(), indices.size());
	meshList.push_back(cilindro);
}

//función para crear un cono
void CrearCono(int res,float R) {

	//constantes utilizadas en los ciclos for
	int n, i;
	//cálculo del paso interno en la circunferencia y variables que almacenarán cada coordenada de cada vértice
	GLfloat dt = 2 * PI / res, x, z, y = -0.5f;
	
	vector<GLfloat> vertices;
	vector<unsigned int> indices;

	//caso inicial para crear el cono
	vertices.push_back(0.0);
	vertices.push_back(0.5);
	vertices.push_back(0.0);
	
	//ciclo for para crear los vértices de la circunferencia del cono
	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0:
				vertices.push_back(x);
				break;
			case 1:
				vertices.push_back(y);
				break;
			case 2:
				vertices.push_back(z);
				break;
			}
		}
	}
	vertices.push_back(R * cos(0) * dt);
	vertices.push_back(-0.5);
	vertices.push_back(R * sin(0) * dt);


	for (i = 0; i < res+2; i++) indices.push_back(i);

	//se genera el mesh del cono
	Mesh *cono = new Mesh();
	cono->CreateMeshGeometry(vertices, indices, vertices.size(), res + 2);
	meshList.push_back(cono);
}

//función para crear pirámide cuadrangular unitaria
void CrearPiramideCuadrangular()
{
	vector<unsigned int> piramidecuadrangular_indices = {
		0,3,4,
		3,2,4,
		2,1,4,
		1,0,4,
		0,1,2,
		0,2,4

	};
	vector<GLfloat> piramidecuadrangular_vertices = {
		0.5f,-0.5f,0.5f,
		0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f,0.5f,
		0.0f,0.5f,0.0f,
	};
	Mesh *piramide = new Mesh();
	piramide->CreateMeshGeometry(piramidecuadrangular_vertices, piramidecuadrangular_indices, 15, 18);
	meshList.push_back(piramide);
}



void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);

}


int main()
{
	mainWindow = Window(800, 600);
	mainWindow.Initialise();
	//Cilindro y cono reciben resolución (slices, rebanadas) y Radio de circunferencia de la base y tapa

	CrearCubo();//índice 0 en MeshList
	CrearPiramideTriangular();//índice 1 en MeshList
	CrearCilindro(20, 1.0f);//índice 2 en MeshList
	CrearCono(25, 2.0f);//índice 3 en MeshList
	CrearPiramideCuadrangular();//índice 4 en MeshList
	CreateShaders();
	

	/*Cámara se usa el comando: glm::lookAt(vector de posición, vector de orientación, vector up));
	En la clase Camera se reciben 5 datos:
	glm::vec3 vector de posición,
	glm::vec3 vector up,
	GlFloat yaw rotación para girar hacia la derecha e izquierda
	GlFloat pitch rotación para inclinar hacia arriba y abajo
	GlFloat velocidad de desplazamiento,
	GlFloat velocidad de vuelta o de giro
	Se usa el Mouse y las teclas WASD y su posición inicial está en 0,0,1 y ve hacia 0,0,-1.
	*/

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.3f);

	
	GLuint uniformProjection = 0;
	GLuint uniformModel = 0;
	GLuint uniformView = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(glm::radians(60.0f)	,mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);
	//glm::mat4 projection = glm::ortho(-1, 1, -1, 1, 1, 10);
	
	//Loop mientras no se cierra la ventana
	sp.init(); //inicializar esfera
	sp.load();//enviar la esfera al shader

	glm::mat4 model(1.0);//Inicializar matriz de Modelo 4x4
	glm::mat4 modelaux(1.0);//Inicializar matriz de Modelo 4x4
	glm::mat4 modelaux2(1.0);

	glm::vec3 color = glm::vec3(0.0f,0.0f,0.0f); //inicializar Color para enviar a variable Uniform;

	float anguloHombro = 35.0f; //Permite la animación y el control interactivo
	float rotacionRuedas = 0.0f; // Ángulo acumulado de giro de las llantas

	while (!mainWindow.getShouldClose())
	{

		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;
		//Recibir eventos del usuario
		glfwPollEvents();
		//Cámara
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		//Limpiar la ventana
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Se agrega limpiar el buffer de profundidad
		shaderList[0].useShader();
		uniformModel = shaderList[0].getModelLocation();
		uniformProjection = shaderList[0].getProjectLocation();
		uniformView = shaderList[0].getViewLocation();
		uniformColor = shaderList[0].getColorLocation();

		// Recibir eventos del usuario
		glfwPollEvents();

		// Cámara
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// ==========================================
		// CONTROLES DEL BRAZO ARTICULADO (F y G)
		// ==========================================
		const float LIMITE_INFERIOR = -20.0f;
		const float LIMITE_SUPERIOR = 60.0f;

		if (mainWindow.getsKeys()[GLFW_KEY_F])
		{
			anguloHombro += 5.0f * deltaTime; // Incrementa ángulo al presionar F
		}
		if (mainWindow.getsKeys()[GLFW_KEY_G])
		{
			anguloHombro -= 5.0f * deltaTime; // Decrementa ángulo al presionar G
		}

		anguloHombro = glm::clamp(anguloHombro, LIMITE_INFERIOR, LIMITE_SUPERIOR);

		// Controles para hacer girar las ruedas
		if (mainWindow.getsKeys()[GLFW_KEY_W])
		{
			rotacionRuedas += 100.0f * deltaTime; // Girar hacia adelante
		}
		if (mainWindow.getsKeys()[GLFW_KEY_S])
		{
			rotacionRuedas -= 100.0f * deltaTime; // Girar hacia atrás
		}

		// Mantener el ángulo en el rango continuo de 0° a 360° (opcional)
		if (rotacionRuedas >= 360.0f) rotacionRuedas -= 360.0f;
		if (rotacionRuedas < 0.0f)   rotacionRuedas += 360.0f;

		//articulacion1 hasta articulación5 sólo son puntos de rotación o articulación, en este caso no dibujaremos esferas que los representen
		model = glm::mat4(1.0);
	
		model = glm::translate(model, glm::vec3(0.0f, 4.0f, -4.0f)); //NOS POSICIONAMOS EN EL CENTRO DEL OBJETO X AMARILLA 
		modelaux = model; //guardamos la matriz de modelo para que la base se mueva con el objeto
		// Creando la cabina del rover ¿CUÁNTAS UNIDADES MEDIRÁ EN Z?
		model = glm::translate(model, glm::vec3(1.0f, 2.0f, 0.0f));//PARA LLEGAR AL CENTRO DE LA CABINA a PARTIR DEL ORIGEN
		model = glm::scale(model, glm::vec3(8.0f, 4.0f, 6.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//la línea de proyección solo se manda una vez a menos que en tiempo de ejecución se programe cambio entre proyección ortogonal y perspectiva
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		color = glm::vec3(1.0f, 0.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color)); //para cambiar el color del objetos
		meshList[0]->RenderMesh(); //dibuja cubo y pirámide triangular
		//meshList[3]->RenderMeshGeometry(); //dibuja las figuras geométricas cilindro, cono, pirámide base cuadrangular
		//sp.render(); //dibuja esfera


		// BASE
		model = glm::mat4(1.0); //Si dejamos está linea, la base no estará unida al centro del objeto. se debe comentar para que la base se mueva con el objeto.
		/*En su lugar usamos la matriz auxiliar modelaux para que la base se mueva con el objeto.
		Lo que debemos de saber es: de las transformaciones geométricas que se aplican a la cabina y 
		cuales queremos que se apliquen a la base. En este caso, sólo queremos que se aplique la traslación del origen, no la rotación ni el escalado de la cabina. 
		Por lo tanto, debemos de guardar en modelaux sólo la traslación del origen y luego aplicarla a la base.
		*/
		model = modelaux;
		//NOS POSICIONAMOS EN EL CENTRO DE LA BASE
		model = glm::translate(model, glm::vec3(0.0f, -0.75f, 0.0f));
		modelaux = model; //guardamos la matriz de modelo para conectar los siguientes objetos a la base
		model = glm::scale(model, glm::vec3(10.0f, 1.5f, 8.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.0f, 1.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color)); //para cambiar el color del objetos
		meshList[0]->RenderMesh();
		
		model = modelaux;
		modelaux2 = model; //guardamos la matriz de modelo para conectar los siguientes objetos a la base
		/*
		A PARTIR DE ESTE PUNTO HAY QUE ACOMODAR VARIOS ELEMENTOS:
		LA BASE PARA LA LLANTA FRONTAL
		LA BASE PARA EL BRAZO
		LA BASE PARA LA LLANTA DE EN MEDIO
		LA BASE PARA LA LLANTA DE ATRÁS
		.........¿FALTA ALGO MÁS?
		CÓMO ESTAREMOS REGRESANDO A ESTE PUNTO EN ESPECÍFICO VARIAS VECES, DEBEMOS DE USAR UNA NUEVA MATRIZ AUXILIAR PARA CADA UNO DE LOS ELEMENTOS 
		QUE SE QUIERAN CONECTAR A LA BASE, YA QUE SI USAMOS LA MISMA MATRIZ AUXILIAR, SE SOBRESCRIBIRÁN LAS TRANSFORMACIONES GEOMÉTRICAS DE CADA ELEMENTO 
		Y NO SE PODRÁN CONECTAR CORRECTAMENTE.

		*/

		//BASE PARA LA LLANTA FRONTAL
		model = modelaux2;

		//BASE PARA EL BRAZO
		model = modelaux2; 


		//BASE PARA LA LLANTA DE EN MEDIO
		model = modelaux2;

		//BASE PARA LA LLANTA DE ATRÁS
		model = modelaux2;

		// 1. LLANTAS DELANTERAS

		// DELANTERA IZQUIERDA 
		// 
		// Matrix auxiliar para la pata izquierda
		glm::mat4 modelPata = modelaux2;
		modelPata = glm::translate(modelPata, glm::vec3(-5.5f, -0.2f, 3.8f)); // Origen en la esfera roja

		// 1. Esfera roja
		model = modelPata;
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 0.0f, 0.0f); // Rojo
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// Aplicar la inclinación al sistema de coordenadas de la pata
		modelPata = glm::rotate(modelPata, glm::radians(-35.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		// 2. Cilindro alargado e inclinado (Gris)
		model = modelPata;
		model = glm::translate(model, glm::vec3(0.0f, -1.25f, 0.0f)); // Desfase a la mitad de su altura
		model = glm::scale(model, glm::vec3(0.2f, 2.5f, 0.2f));       // Altura = 2.5f
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.6f, 0.6f, 0.6f); // Gris metálico
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();

		// Avanzar el sistema de coordenadas hasta la PUNTA EXACTA del cilindro (Largo = -2.5f en Y)
		modelPata = glm::translate(modelPata, glm::vec3(0.0f, -2.5f, 0.0f));

		// 3. Esfera amarilla en el codo inferior (Acoplada a la punta)
		model = modelPata;
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 0.0f); // Amarillo
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// 4. Poste vertical corto hacia la rueda
		model = modelPata;
		model = glm::translate(model, glm::vec3(0.0f, -0.3f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f, 0.6f, 0.15f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.6f, 0.6f, 0.6f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();

		// 5. Rueda
		model = modelPata;
		model = glm::translate(model, glm::vec3(0.0f, -1.3f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.2f, 0.2f, 0.2f); // Gris oscuro
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();


		// ------------------------------------------
		// DELANTERA DERECHA (Z = -3.8f)
		// ------------------------------------------
		// Matrix auxiliar para la pata derecha
		modelPata = modelaux2;
		modelPata = glm::translate(modelPata, glm::vec3(-5.5f, -0.2f, -3.8f));

		// 1. Esfera roja en el chasis
		model = modelPata;
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 0.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// Aplicar la inclinación
		modelPata = glm::rotate(modelPata, glm::radians(-35.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		// 2. Cilindro alargado e inclinado
		model = modelPata;
		model = glm::translate(model, glm::vec3(0.0f, -1.25f, 0.0f));
		model = glm::scale(model, glm::vec3(0.2f, 2.5f, 0.2f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.6f, 0.6f, 0.6f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();

		// Avanzar a la punta del cilindro
		modelPata = glm::translate(modelPata, glm::vec3(0.0f, -2.5f, 0.0f));

		// 3. Esfera amarilla en el codo inferior
		model = modelPata;
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// 4. Poste vertical corto hacia la rueda
		model = modelPata;
		model = glm::translate(model, glm::vec3(0.0f, -0.3f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f, 0.6f, 0.15f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.6f, 0.6f, 0.6f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();

		// 5. Rueda
		model = modelPata;
		model = glm::translate(model, glm::vec3(0.0f, -1.3f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.2f, 0.2f, 0.2f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();


		// 2. PATAS INTERMEDIAS Y TRASERAS 

		// ------------------------------------------
		// COSTADO IZQUIERDO
		// ------------------------------------------

		// --- LLANTA INTERMEDIA IZQUIERDA
		glm::mat4 modelPataInterIzq = modelaux2;
		modelPataInterIzq = glm::translate(modelPataInterIzq, glm::vec3(0.5f, -0.2f, 4.4f)); // Pivote en esfera azul

		// 1. Esfera azul de articulación
		model = modelPataInterIzq;
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.0f, 0.0f, 1.0f); 
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// Inclinación del eje local 
		modelPataInterIzq = glm::rotate(modelPataInterIzq, glm::radians(-35.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		// 2. Cilindro diagonal
		model = modelPataInterIzq;
		model = glm::translate(model, glm::vec3(0.0f, -1.1f, 0.0f));
		model = glm::scale(model, glm::vec3(0.2f, 2.2f, 0.2f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.6f, 0.6f, 0.6f); // Gris metálico
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();

		// Desplazar el origen a la PUNTA EXACTA del cilindro (-2.2f)
		modelPataInterIzq = glm::translate(modelPataInterIzq, glm::vec3(0.0f, -2.2f, 0.0f));

		// 3. Rueda Intermedia Izquierda en la punta
		model = modelPataInterIzq;
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotacionRuedas), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.2f, 0.2f, 0.2f); // Gris oscuro
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();


		// --- LLANTA TRASERA IZQUIERDA (X = 4.0f) ---
		glm::mat4 modelPataTrasIzq = modelaux2;
		modelPataTrasIzq = glm::translate(modelPataTrasIzq, glm::vec3(4.0f, -0.2f, 4.4f));

		// 1. Esfera azul
		model = modelPataTrasIzq;
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.0f, 0.0f, 1.0f); // Azul
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// Inclinación (+35.0f)
		modelPataTrasIzq = glm::rotate(modelPataTrasIzq, glm::radians(35.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		// 2. Cilindro diagonal
		model = modelPataTrasIzq;
		model = glm::translate(model, glm::vec3(0.0f, -1.1f, 0.0f));
		model = glm::scale(model, glm::vec3(0.2f, 2.2f, 0.2f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.6f, 0.6f, 0.6f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();

		// Desplazar el origen a la PUNTA EXACTA del cilindro (-2.2f)
		modelPataTrasIzq = glm::translate(modelPataTrasIzq, glm::vec3(0.0f, -2.2f, 0.0f));

		// 3. Rueda Trasera Izquierda en la punta
		model = modelPataTrasIzq;
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotacionRuedas), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.2f, 0.2f, 0.2f); // Gris oscuro
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();


		// ------------------------------------------
		// COSTADO DERECHO (Z = -5.0f)
		// ------------------------------------------

		// --- LLANTA INTERMEDIA DERECHA (X = 0.5f) ---
		glm::mat4 modelPataInterDer = modelaux2;
		modelPataInterDer = glm::translate(modelPataInterDer, glm::vec3(0.5f, -0.2f, -4.4f));

		// 1. Esfera azul
		model = modelPataInterDer;
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.0f, 0.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// Inclinación
		modelPataInterDer = glm::rotate(modelPataInterDer, glm::radians(-35.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		// 2. Cilindro diagonal
		model = modelPataInterDer;
		model = glm::translate(model, glm::vec3(0.0f, -1.1f, 0.0f));
		model = glm::scale(model, glm::vec3(0.2f, 2.2f, 0.2f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.6f, 0.6f, 0.6f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();

		// Desplazar a la punta (-2.2f)
		modelPataInterDer = glm::translate(modelPataInterDer, glm::vec3(0.0f, -2.2f, 0.0f));

		// 3. Rueda Intermedia Derecha en la punta
		model = modelPataInterDer;
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotacionRuedas), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.2f, 0.2f, 0.2f); // Gris oscuro
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();


		// --- LLANTA TRASERA DERECHA (X = 4.0f) ---
		glm::mat4 modelPataTrasDer = modelaux2;
		modelPataTrasDer = glm::translate(modelPataTrasDer, glm::vec3(4.0f, -0.2f, -4.4f));

		// 1. Esfera azul
		model = modelPataTrasDer;
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.0f, 0.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// Inclinación
		modelPataTrasDer = glm::rotate(modelPataTrasDer, glm::radians(35.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		// 2. Cilindro diagonal
		model = modelPataTrasDer;
		model = glm::translate(model, glm::vec3(0.0f, -1.1f, 0.0f));
		model = glm::scale(model, glm::vec3(0.2f, 2.2f, 0.2f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.6f, 0.6f, 0.6f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();

		// Desplazar a la punta (-2.2f)
		modelPataTrasDer = glm::translate(modelPataTrasDer, glm::vec3(0.0f, -2.2f, 0.0f));

		// 3. Rueda Trasera Derecha en la punta
		model = modelPataTrasDer;
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotacionRuedas), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.2f, 0.2f, 0.2f); // Gris oscuro
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();

		// ==========================================
		// BRAZO CON PINZA CON BASE ESFÉRICA
		// ==========================================
		glm::mat4 modelBrazo = modelaux2;

		// Posicionamos la articulación del brazo en el frente del chasis
		modelBrazo = glm::translate(modelBrazo, glm::vec3(-4.1f, 0.75f, 3.2f));

		// ------------------------------------------
		// ESFERA DE LA BASE DEL BRAZO
		// ------------------------------------------
		model = modelBrazo;
		model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f)); // Escalar la esfera
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.0f, 0.0f, 1.0f); // Azul (como en la imagen de referencia)
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render(); // Dibujar la esfera instanciada con sp.init() y sp.load()

		// ------------------------------------------
		// Primer cilindro del brazo
		// ------------------------------------------
		// Aplicar la rotación a partir del centro de la esfera
		modelBrazo = glm::rotate(modelBrazo, glm::radians(anguloHombro), glm::vec3(0.0f, 0.0f, 1.0f));

		model = modelBrazo;
		// Desplazar el cilindro hacia arriba para que nazca desde la esfera
		model = glm::translate(model, glm::vec3(0.0f, 2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 4.0f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.8f, 0.8f, 0.8f); // Gris claro
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry(); // Cilindro del brazo

		// ==========================================
		// ARTICULACIÓN DEL CODO (ESFERA)
		// ==========================================
		// Línea 536: Avanzar al extremo del primer cilindro
		modelBrazo = glm::translate(modelBrazo, glm::vec3(0.0f, 4.0f, 0.0f));

		// Renderizar la esfera del codo
		model = modelBrazo;
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f)); // Escalar la esfera
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.0f, 0.0f, 1.0f); // Azul
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render(); // Dibujar la esfera en el codo

		// ==========================================
		// 3. Segundo cilindro
		// ==========================================

		modelBrazo = glm::rotate(modelBrazo, glm::radians(80.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		model = modelBrazo;
		model = glm::translate(model, glm::vec3(0.0f, 1.75f, 0.0f));
		model = glm::scale(model, glm::vec3(0.35f, 3.5f, 0.35f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.8f, 0.8f, 0.8f); // Regresar al color gris
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();

		// 4. Pinza / Muñeca
		modelBrazo = glm::translate(modelBrazo, glm::vec3(0.0f, 3.5f, 0.0f)); // Extremo del segundo eslabón

		// Dedos de la pinza (Cilindros delgados)
		// Dedo 1
		model = modelBrazo;
		model = glm::translate(model, glm::vec3(-0.15f, 0.3f, 0.0f));
		model = glm::rotate(model, glm::radians(-15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.6f, 0.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.9f, 0.1f, 0.1f); // Rojo
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();

		// Dedo 2
		model = modelBrazo;
		model = glm::translate(model, glm::vec3(0.15f, 0.3f, 0.0f));
		model = glm::rotate(model, glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.6f, 0.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[2]->RenderMeshGeometry();





		glUseProgram(0);
		mainWindow.swapBuffers();
	}
	return 0;
}