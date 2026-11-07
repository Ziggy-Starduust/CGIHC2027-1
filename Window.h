#pragma once
#include<stdio.h>
#include<glew.h>
#include<glfw3.h>

class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);
	int Initialise();
	GLfloat getBufferWidth() { return bufferWidth; }
	GLfloat getBufferHeight() { return bufferHeight; }
	GLfloat getXChange();
	GLfloat getYChange();
	GLfloat getmuevex() { return muevex; }
	bool getShouldClose() {
		return  glfwWindowShouldClose(mainWindow);
	}
	bool* getsKeys() { return keys; }
	void swapBuffers() { return glfwSwapBuffers(mainWindow); }

	// Practica 8: valores controlados con el teclado
	// Avion (Roger)
	GLfloat getAvionY() { return avionY; }
	GLfloat getAvionZ() { return avionZ; }
	GLfloat getAngTimon() { return angTimon; }
	GLfloat getAngElevon() { return angElevon; }
	int getColorFaro() { return colorFaro; }
	// Rover
	GLfloat getRoverX() { return roverX; }
	GLfloat getRotacionPatas() { return rotacionPatas; }

	// Para practica 8: nave
	bool getLucesNave() { return lucesNave; }
	bool getMotor1() { return motor1; }
	bool getMotor2() { return motor2; }

	// Ejercicios 4 y 5 (Practica 8): luces encendidas / apagadas
	bool getLuzHolocron() { return luzHolocron; }
	bool getLuzPinza() { return luzPinza; }

	// ---------- Practica 10: keyframes del cohete ----------
	// El main consulta con estos metodos; cada valor se "consume" al leerlo
	// para que una pulsacion cuente una sola vez.
	GLfloat tomarDeltaX() { GLfloat d = deltaKFx; deltaKFx = 0.0f; return d; }
	GLfloat tomarDeltaY() { GLfloat d = deltaKFy; deltaKFy = 0.0f; return d; }
	GLfloat tomarDeltaGiro() { GLfloat d = deltaKFgiro; deltaKFgiro = 0.0f; return d; }
	int tomarPieza() { int p = piezaKF; piezaKF = -1; return p; }
	bool tomarGuardarFrame() { bool b = kfGuardarFrame; kfGuardarFrame = false; return b; }
	bool tomarReproducir() { bool b = kfReproducir; kfReproducir = false; return b; }
	bool tomarGuardarArchivo() { bool b = kfGuardarArchivo; kfGuardarArchivo = false; return b; }
	bool tomarCargarArchivo() { bool b = kfCargarArchivo; kfCargarArchivo = false; return b; }
	bool tomarNuevaAnimacion() { bool b = kfNuevaAnimacion; kfNuevaAnimacion = false; return b; }

	~Window();

private:
	GLFWwindow* mainWindow;
	GLint width, height;
	bool keys[1024];
	GLint bufferWidth, bufferHeight;
	void createCallbacks();
	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	GLfloat muevex;
	bool mouseFirstMoved;

	// ---------- Practica 7 / 8: variables del teclado ----------
	GLfloat avionY = 5.0f;        // U / J : subir / bajar
	GLfloat avionZ = 6.0f;        // I / K : adelante / atras
	GLfloat angTimon = 0.0f;      // N / M : timon (+-30 grados)
	GLfloat angElevon = 0.0f;     // V / B : elevones (+-25 grados)
	int colorFaro = 0;            // C     : 0 rojo, 1 amarillo, 2 verde, 3 cian, 4 azul, 5 magenta
	GLfloat roverX = 1.0f;        // T / G : rover adelante / atras
	GLfloat rotacionPatas = 0.0f; // R / F : patas del rover (+-45 grados)
	bool lucesNave = true;   // L : luz puntual + spotlight de la nave
	bool motor1 = false;     // 1 : motor izquierdo (fuego + luz azul)
	bool motor2 = false;     // 2 : motor derecho  (fuego + luz azul)

	bool luzHolocron = true;      // H : prender / apagar luz del holocron
	bool luzPinza = true;         // P : prender / apagar luz de la pinza

	// ---------- Practica 10: keyframes del cohete ----------
	GLfloat deltaKFx = 0.0f;      // 1 / 2 : +X / -X (0.5 u)
	GLfloat deltaKFy = 0.0f;      // 3 / 4 : +Y / -Y (0.5 u)
	GLfloat deltaKFgiro = 0.0f;   // 5 / 6 : punta hacia +X / -X (10 grados)
	int piezaKF = -1;	//pieza elegida por teclado (-1 = ninguna)
	bool kfGuardarFrame = false;  // L       : guardar keyframe
	bool kfReproducir = false;    // ESPACIO : reproducir animacion
	bool kfGuardarArchivo = false;// G       : escribir/sobreescribir keyframescohete.txt
	bool kfCargarArchivo = false; // K       : leer keyframescohete.txt
	bool kfNuevaAnimacion = false;// N       : borrar frames en memoria

	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);

};