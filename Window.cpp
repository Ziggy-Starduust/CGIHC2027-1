#include "Window.h"

Window::Window()
{
	width = 800;
	height = 600;
	for (size_t i = 0; i < 1024; i++)
	{
		keys[i] = 0;
	}
}
Window::Window(GLint windowWidth, GLint windowHeight)
{
	width = windowWidth;
	height = windowHeight;
	muevex = 2.0f;
	for (size_t i = 0; i < 1024; i++)
	{
		keys[i] = 0;
	}
}
int Window::Initialise()
{
	//Inicializacion de GLFW
	if (!glfwInit())
	{
		printf("Fallo inicializar GLFW");
		glfwTerminate();
		return 1;
	}
	//Asignando variables de GLFW y propiedades de ventana
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//para solo usar el core profile de OpenGL y no tener retrocompatibilidad
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//CREAR VENTANA
	mainWindow = glfwCreateWindow(width, height, "PracticaXX:Nombre de la practica", NULL, NULL);

	if (!mainWindow)
	{
		printf("Fallo en crearse la ventana con GLFW");
		glfwTerminate();
		return 1;
	}
	//Obtener tamano de Buffer
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	//asignar el contexto
	glfwMakeContextCurrent(mainWindow);

	//MANEJAR TECLADO y MOUSE
	createCallbacks();


	//permitir nuevas extensiones
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		printf("Fallo inicializacion de GLEW");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST); //HABILITAR BUFFER DE PROFUNDIDAD
							 // Asignar valores de la ventana y coordenadas
							 
							 //Asignar Viewport
	glViewport(0, 0, bufferWidth, bufferHeight);
	//Callback para detectar que se esta usando la ventana
	glfwSetWindowUserPointer(mainWindow, this);
}

void Window::createCallbacks()
{
	glfwSetKeyCallback(mainWindow, ManejaTeclado);
	glfwSetCursorPosCallback(mainWindow, ManejaMouse);
}
GLfloat Window::getXChange()
{
	GLfloat theChange = xChange;
	xChange = 0.0f;
	return theChange;
}

GLfloat Window::getYChange()
{
	GLfloat theChange = yChange;
	yChange = 0.0f;
	return theChange;
}




void Window::ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	// PRESS = al presionar, REPEAT = mientras se mantiene presionada
	bool presionada = (action == GLFW_PRESS || action == GLFW_REPEAT);

	// ---------------- AVION (ROGER) ----------------
	if (key == GLFW_KEY_I && presionada) theWindow->avionZ += 0.2f;   // adelante
	if (key == GLFW_KEY_K && presionada) theWindow->avionZ -= 0.2f;   // atras
	if (key == GLFW_KEY_U && presionada) theWindow->avionY += 0.2f;   // subir
	if (key == GLFW_KEY_J && presionada) theWindow->avionY -= 0.2f;   // bajar

	// timon: N / M, 
	if (key == GLFW_KEY_N && presionada && theWindow->angTimon < 30.0f)  theWindow->angTimon += 3.0f;
	if (key == GLFW_KEY_M && presionada && theWindow->angTimon > -30.0f) theWindow->angTimon -= 3.0f;

	// elevones: V / B, 
	if (key == GLFW_KEY_V && presionada && theWindow->angElevon < 25.0f)  theWindow->angElevon += 2.5f;
	if (key == GLFW_KEY_B && presionada && theWindow->angElevon > -25.0f) theWindow->angElevon -= 2.5f;

	// Ejercicio 1 (Practica 8): color del faro inferior, cambia una vez por pulsacion
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		theWindow->colorFaro = (theWindow->colorFaro + 1) % 6;
	}

	// Ejercicio 4: prender / apagar la luz del holocron
	if (key == GLFW_KEY_H && action == GLFW_PRESS)
	{
		theWindow->luzHolocron = !theWindow->luzHolocron;
	}

	// Ejercicio 5: prender / apagar la luz de la pinza del rover
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		theWindow->luzPinza = !theWindow->luzPinza;
	}

	// ---------------- ROVER ----------------
	if (key == GLFW_KEY_T && presionada) theWindow->roverX += 0.2f;   // adelante
	if (key == GLFW_KEY_G && presionada) theWindow->roverX -= 0.2f;   // atras

	// patas: R / F, limitado a +-45 grados
	if (key == GLFW_KEY_R && presionada && theWindow->rotacionPatas < 45.0f)  theWindow->rotacionPatas += 3.0f;
	if (key == GLFW_KEY_F && presionada && theWindow->rotacionPatas > -45.0f) theWindow->rotacionPatas -= 3.0f;

	// ---------------- NAVE ----------------
	if (key == GLFW_KEY_L && action == GLFW_PRESS) theWindow->lucesNave = !theWindow->lucesNave;
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) theWindow->motor1 = !theWindow->motor1;
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) theWindow->motor2 = !theWindow->motor2;


	// arreglo de teclas que usa la camara (W, A, S, D)
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			theWindow->keys[key] = true;
			//printf("se presiono la tecla %d'\n", key);
		}
		else if (action == GLFW_RELEASE)
		{
			theWindow->keys[key] = false;
			//printf("se solto la tecla %d'\n", key);
		}
	}
}

void Window::ManejaMouse(GLFWwindow* window, double xPos, double yPos)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (theWindow->mouseFirstMoved)
	{
		theWindow->lastX = xPos;
		theWindow->lastY = yPos;
		theWindow->mouseFirstMoved = false;
	}

	theWindow->xChange = xPos - theWindow->lastX;
	theWindow->yChange = theWindow->lastY - yPos;

	theWindow->lastX = xPos;
	theWindow->lastY = yPos;
}


Window::~Window()
{
	glfwDestroyWindow(mainWindow);
	glfwTerminate();

}
