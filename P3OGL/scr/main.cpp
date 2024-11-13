#include "BOX.h"
#include "auxiliar.h"


#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

using namespace std;
using namespace glm;

//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
mat4 proj = mat4(1.0f);
mat4 view = mat4(1.0f);
mat4 model = mat4(1.0f);


//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////

// Variables globales.
unsigned int vshader;
unsigned int fshader;
unsigned int program;

//VAO
unsigned int vao;

//VBOs que forman parte del objeto
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

// Texturas
unsigned int colorTexId;
unsigned int emiTexId;

// Texturas uniformes
int uColorTex;
int uEmiTex;

// Variables uniformes
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;

// Atributos
int inPos;
int inColor;
int inNormal;
int inTexCoord;


//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////
//!!Por implementar

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShader(const char *vname, const char *fname);
void initObj();
void destroy();


//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
//!!Por implementar
unsigned int loadTex(const char *fileName);


int main(int argc, char** argv)
{
	locale::global(locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();
	initShader("../shaders_P3/shader.v1.vert", "../shaders_P3/shader.v1.frag");
	initObj();

	// Se añade el bucle de eventos del glut.
	glutMainLoop();

	destroy();

	return 0;
}
	
//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv) {
	// Inicializamos el contexto, definiendo la versión de OpenGL y el tipo de perfil a utilizar.
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	// Creamos la ventana indicando el tipo de buffer, el formato de color y el uso de buffer de profundidad.
	// Se indica el tamaño de la ventana, su posición y se le da el nombre.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Prácticas OGL");

	// Inicializamos las extensiones. En caso de error, se muestra por consola el string del error.
	// Posteriormente, se indica la versión de OpenGL que soporta el dispositivo.
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cout << "Error: " << glewGetErrorString(err) << endl;
		exit(-1);
	}
	const GLubyte* oglVersion = glGetString(GL_VERSION);
	cout << "This system supports OpenGL Version: " << oglVersion << endl;

	// Se indican que funciones se encargarán de los distintos eventos.
	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);

}

void initOGL() {
	// Activa el test de profundidad y establece el color de fondo.
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	// Indica la orientación de la cara front, configura la etapa de rasterizado y activa el culling.
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	// Define la matriz de vista y la matriz de proyección. 
	// No tienen que ver con OpenGL.
	proj = perspective(radians(60.0f), 1.0f, 0.1f, 50.0f);
	view = mat4(1.0f);
	view[3].z = -6;

}

void destroy() {
	// Liberamos los recursos utilizados por el programa.
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);

	// Liberamos los recursos del VAO y VBOs.
	if (inPos != -1) glDeleteBuffers(1, &posVBO);
	if (inColor != -1) glDeleteBuffers(1, &colorVBO);
	if (inNormal != -1) glDeleteBuffers(1, &normalVBO);
	if (inTexCoord != -1) glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &triangleIndexVBO);
	glDeleteVertexArrays(1, &vao);

	// Liberamos los recursos de las texturas de color y emisiva.
	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);

}

void initShader(const char *vname, const char *fname) {
	// Creamos un shader de vértices y otro de fragmentos.
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	// Enlazamos los dos shaders en un mismo programa.
	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);

	// Comprobamos los errores de la fase de enlazado.
	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteProgram(program);
		program = 0;
		exit(-1);
	}

	// Asignamos identificadores a los atributos del programa, antes de enlazarlo.
	glBindAttribLocation(program, 0, "inPos");
	glBindAttribLocation(program, 1, "inColor");
	glBindAttribLocation(program, 2, "inNormal");
	glBindAttribLocation(program, 3, "inTexCoord");

	// Creamos los identificadores de las variables uniformes.
	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");

	/* int borrar = glGetUniformLocation(program, "borrar");
	 * int borrar = glGetAttribLocation(program, "inTexCoord");
	 * int borrar = glGetAttribLocation(program, "inPos");
	 */

	// Creamos los identificadores de los atributos.
	inPos = glGetAttribLocation(program, "inPos");
	inColor = glGetAttribLocation(program, "inColor");
	inNormal = glGetAttribLocation(program, "inNormal");
	inTexCoord = glGetAttribLocation(program, "inTexCoord");

	// Este código activa y desactiva el programa.
	glUseProgram(program);

	// Creamos los identificadores de las texturas de color y emisiva.
	uColorTex = glGetUniformLocation(program, "colorTex");
	uEmiTex = glGetUniformLocation(program, "emiTex");


}

void initObj() {
	// Se crea y activa el VAO en el que se almacenará la configuración del objeto.
	// VBO (atributos, index), VAO.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &posVBO);
	glGenBuffers(1, &colorVBO);
	glGenBuffers(1, &normalVBO);
	glGenBuffers(1, &texCoordVBO);

	/*
	 * También se puede utilizar buff como único buffer.
	 * unsigned int buff;
	 * glGenBuffers(1, &buff);
	 * 
	 * glBindBuffer(GL_ARRAY_BUFFER, buff);
	 * glBufferData(GL_ARRAY_BUFFER, (cubeNVertex * (3+3+3+2)) * sizeof(float) * 
	 * cubeNTriangleIndex * 3 * sizeof (unsigned int), NULL, GL_STATIC_DRAW);
	 * glBufferSubData(GL_ARRAY_BUFFER, 3 * cubeNVertex * sizeof(float),
	 * 3 * cubeNVertex * sizeof(float), cubeVertexColor);
	 */

	// Creamos y configuramos todos los atributos de la malla.
	if (inPos != -1)
	{
		// glGenBuffers(1, &posVBO);
		glBindBuffer(GL_ARRAY_BUFFER, posVBO); // Activación como buffer de atributo.
		glBufferData(GL_ARRAY_BUFFER, // Objeto para el que se reserva espacio.
			cubeNVertex * sizeof(float) * 3, // Tamaño en bytes.
			cubeVertexPos, // Puntero a vértices de la maya.
			GL_STATIC_DRAW); // Para pintar, no se modificada una vez esté subido.

		// glBufferSubData(GL_ARRAY_BUFFER, 0, cubeNVertex * 3 * sizeof(float), cubeVertexPos);

		glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}
	if (inColor != -1)
	{
		// glGenBuffers(1, &colorVBO);
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
			cubeVertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor);
	}
	if (inNormal != -1)
	{
		// glGenBuffers(1, &normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
			cubeVertexNormal, GL_STATIC_DRAW);
		glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inNormal);
	}
	if (inTexCoord != -1)
	{
		// glGenBuffers(1, &texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 2,
			cubeVertexTexCoord, GL_STATIC_DRAW);
		glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inTexCoord);
	}

	// Creamos la lista de índices.
	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		cubeNTriangleIndex * sizeof(unsigned int) * 3, cubeTriangleIndex,
		GL_STATIC_DRAW);

	// Inicializamos la matriz model de nuestro objeto.
	model = mat4(1.0f);

	// Creamos dos texturas, una del color y otra emisiva.
	colorTexId = loadTex("../img/color2.png");
	emiTexId = loadTex("../img/emissive.png");
}

GLuint loadShader(const char *fileName, GLenum type) {
	// Añade el código de carga y compilación de un shader genérico
	unsigned int fileLen;
	char* source = loadStringFromFile(fileName, fileLen);

	// Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1,
		(const GLchar**)&source, (const GLint*)&fileLen);
	glCompileShader(shader);
	delete[] source;

	// Comprobamos que se compiló bien
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		// Calculamos una cadena de error
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteShader(shader);
		exit(-1);

		// Devuelve el ID del shader compilado.
		return shader;
	}

	return shader;

}

unsigned int loadTex(const char *fileName) {
	// Cargamos y configuramos una textura genérica.
	// Carga la textura almacenada en el fichero indicado.
	unsigned char* map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);
	if (!map)
	{
		std::cout << "Error cargando el fichero: "
			<< fileName << endl;
		exit(-1);
	}

	// Crea una textura, la activa y la sube a la tarjeta gráfica.
	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);

	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)map);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);
	glTexSubImage2D(GL_TEXTURE_2D, 0, // Level
		0,0, // Offset
		w,h, // Tamaño de datos que subo
		GL_RGBA, GL_UNSIGNED_BYTE, // Dimensiones y tipo de dato
		(GLvoid*)map);

	// Libera la memoria de la CPU.
	delete[] map;

	// Crea los mipmaps asociados a la textura.
	glGenerateMipmap(GL_TEXTURE_2D);

	// Configuramos el modo de acceso.
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

	// Devuelve el identificador de la textura.
	return texId;

}

void renderFunc() {
	// Limpia el buffer de color y el buffer de profundidad antes de cada renderizado.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// En caso de ser varios objetos, se utilizan bucles.
	// Calcula y sube las matrices requeridas por el shader de vértices.
	mat4 modelView = view * model;
	mat4 modelViewProj = proj * view * model;
	mat4 normal = transpose(inverse(modelView));

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	// Activa el VAO con la configuración del objeto y pinta la lista de triángulos.
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3,
		GL_UNSIGNED_INT, (void*)0);
	
	// Se añade la función que intercambia los buffers del doble buffer.
	glutSwapBuffers();

	// Activamos las texturas y las enlazamos con el programa activo.
	if (uColorTex != -1)
	{
		glActiveTexture(GL_TEXTURE0); // Texture Unit 0
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(uColorTex, 0); // Se sube a pos 0
	}
	if (uEmiTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1); // Texture Unit 1
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex, 1); // Se sube a pos 1
	}

}

void resizeFunc(int width, int height) {
	// Ajusta el viewport.
	glViewport(0, 0, width, height);

	// Planifica un evento de renderizado.
	glutPostRedisplay();
}

void idleFunc(){
	// Modificamos la matriz model.
	model = mat4(1.0f);
	static float angle = 0.0f;
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.01f;
	model = rotate(model, angle, vec3(1.0f, 1.0f, 0.0f));

	// Lanzamos un evento de renderizado.
	glutPostRedisplay();

}

void keyboardFunc(unsigned char key, int x, int y){}
void mouseFunc(int button, int state, int x, int y){}









