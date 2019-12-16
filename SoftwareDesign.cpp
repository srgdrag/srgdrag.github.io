/*
 * Model.cpp
 *
 *  Created on: Nov 17, 2019
 *      Author: Josh
 */

#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// SOIL Image loader Inclusion
#include "SOIL2/SOIL2.h"

using namespace std;

#define WINDOW_TITLE "Modern OpenGL"

#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif

GLint objShaderProgram, lampShaderProgram, WindowWidth = 800, WindowHeight = 600;
GLuint VBO, ObjVAO, LampVAO, EBO, texture;
GLfloat degrees = glm::radians(-45.0f);
bool projSwap = false;

//Object position and scale
glm::vec3 objPosition(0.0f, 0.0f, 0.0f);
glm::vec3 objScale(2.0f);

//Object and light color
glm::vec3 objectColor(1.0f, 0.0f, 0.0f);
glm::vec3 lightColor(0.5f, 0.6f, 0.75f);

//Light position and scale
glm::vec3 lightPosition(0.5f, 0.5f, -3.0f);
glm::vec3 lightScale(0.7f);

GLfloat cameraSpeed = 0.05f; // Movement speed per frame

GLchar currentKey; // Will store key pressed

GLfloat lastMouseX = 400, lastMouseY = 300; // Locks mouse cursor at the center of the screen
GLfloat mouseXOffset, mouseYOffset, yaw = 0.0f, pitch = 0.0f; // mouse offset, yaw, and pitch variables
GLfloat sensitivity = 0.05f; // used for mouse/camera rotation sensitivity
bool mouseDetected = true; // Initially true when mouse movement is detected

//Global vector declarations
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f); // Initial camera position. placed 5 units in the Z
glm::vec3 CameraUpY = glm::vec3(0.0f, 1.0f, 0.0f); //Temporary y unit vector
glm::vec3 CameraForwardZ = glm::vec3(0.0f, 0.0f, -1.0f); // Temporary z unit vector
glm::vec3 front = glm::vec3(10.0f, 0.0f, 0.0f); // Temporary z unit vector for mouse

void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);
void UCreateCubeBuffers(void);
void UCreatePyramidBuffers(void);
void UKeyboard(unsigned char key, int x, int y);
void UKeyReleased(unsigned char key, int x, int y);
void UMouseMove(int x, int y);
void UGenerateTexture(void);
void SwapProjection(void);
void Menu(void);

const GLchar * objVertexShaderSource = GLSL(330,
		layout (location = 0) in vec3 position;
		layout (location = 1) in vec3 normal;
		layout (location = 2) in vec2 textureCoordinates;

		out vec2 mobileTextureCoordinate;
		out vec3 Normal; //For outgoing normals to fragment shader
		out vec3 FragmentPos; //For outgoing color / pixels to fragment shader

		//Global variables for the transform matrices
		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

void main(){
	gl_Position = projection * view * model * vec4(position, 1.0f);
	FragmentPos = vec3(model * vec4(position, 1.0f)); //Gets fragment/ pixel position in world space only
	Normal = mat3(transpose(inverse(model))) * normal; //get normal vectors in world space only and exclude normal translation properties
	mobileTextureCoordinate = vec2(textureCoordinates.x, 1.0f - textureCoordinates.y); //flips the texture horizontally
	}
);

const GLchar * objFragmentShaderSource = GLSL(330,
		in vec2 mobileTextureCoordinate;
		in vec3 Normal; //For incoming normals
		in vec3 FragmentPos; //For incoming fragment position

		out vec4 objColor; //For outgoing obj color to the gpu

		uniform sampler2D uTexture; // useful when working with multiple texutres
		uniform vec3 lightColor;
		uniform vec3 lightPos;
		uniform vec3 viewPosition;

	void main(){

		//Phong lighting model calculations to generate ambient, diffuse, and specular components

		//Calculate Ambient lighting
		float ambientStrength = 1.0f; // Set ambient or global lighting strength
		vec3 ambient = ambientStrength * lightColor; //Generate ambient light color

		//Calculate Diffuse lighting
		vec3 norm = normalize(Normal); //Normalize vectors to 1 unit
		vec3 lightDirection = normalize(lightPos - FragmentPos); // Calculate distance between light source and pixels
		float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
		vec3 diffuse = impact * lightColor; //Generate diffuse light color

		//Calculate Specular lighting
		float specularIntensity = 1.0f; //set specular light strength
		float highlightSize = 128.0f; // Set specular highlight size
		vec3 viewDir = normalize(viewPosition - FragmentPos); // Calcluate view direction
		vec3 reflectDir = reflect(-lightDirection, norm); // Calclutate reflection vector
		//Calcluate specular component
		float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
		vec3 specular = specularIntensity * specularComponent * lightColor;

		//Calculate phong result
		vec3 objectColor = texture(uTexture, mobileTextureCoordinate).xyz;
		vec3 phong = (ambient + diffuse + specular) * objectColor;
		objColor = vec4(phong, 1.0f);
	}
);

//Lamp Shader Source Code
const GLchar * lampVertexShaderSource = GLSL(330,

		layout (location = 0) in vec3 position; // VAP position for vertex  position data

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		void main()
		{
			gl_Position = projection * view * model * vec4(position, 1.0f);
		}
);

//Fragment Shader Source Code
const GLchar * lampFragmentShaderSource = GLSL(330,

		out vec4 color; // For outgoing lamp color to the gpu

		void main()
		{
			color = vec4(1.0f); // Set color to white (1.0f, 1.0f, 1.0f) with alpha 1.0
		}
);

int main(int argc, char* argv[])
{
	int choice = 0;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow(WINDOW_TITLE);

	glutReshapeFunc(UResizeWindow);

	glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
			{
				std::cout << "Failed to initialize GLEW" << std::endl;
				return -1;
			}

	UCreateShader();

	Menu();

	cin >> choice;
	switch (choice)
	{
		case 1:
			UCreateCubeBuffers();
			break;
		case 2:
			UCreatePyramidBuffers();
			break;
		case 3:
			UCreateBuffers();
			break;
		default:
			cout << "No Choice picked. Exiting program";
			return 0;
	}

	UGenerateTexture();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glutDisplayFunc(URenderGraphics);

	glutKeyboardFunc(UKeyboard);

	glutKeyboardUpFunc(UKeyReleased);

	glutPassiveMotionFunc(UMouseMove); //Detects mouse movement

	glutMainLoop();

	glDeleteVertexArrays(1, &ObjVAO);
	glDeleteVertexArrays(1, &LampVAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	return 0;

}

void UResizeWindow(int w, int h)
{
	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);
}

void URenderGraphics(void)
{
	glEnable(GL_DEPTH_TEST); // Enable z-depth

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clears the screen

	GLint modelLoc, viewLoc, projLoc, objectColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc;

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

	//Camera movement logic
	if(currentKey == 'w')
		cameraPosition += cameraSpeed * CameraForwardZ;

	if(currentKey == 's')
		cameraPosition -= cameraSpeed * CameraForwardZ;

	if(currentKey == 'a')
		cameraPosition -= glm::normalize(glm::cross(CameraForwardZ, CameraUpY)) * cameraSpeed;

	if(currentKey == 'd')
		cameraPosition += glm::normalize(glm::cross(CameraForwardZ, CameraUpY)) * cameraSpeed;

	CameraForwardZ = front; // Replaces camera forward vector with Radians normalizedd as a unit vector

	//Use the object shader and activate the vertex array
	glUseProgram(objShaderProgram);
	glBindVertexArray(ObjVAO);

	// Transforms the object
	model = glm::translate(model, objPosition); // Place the object at the center of the viewport
	model = glm::scale(model, objScale); //Increase the object size by a scale of 2

	// Transforms the camera
	view = glm::lookAt(cameraPosition - CameraForwardZ, cameraPosition, CameraUpY);

	if(projSwap == false)
	{
		//Creates a perspective projection
	projection = glm::perspective(45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f);
	}
	else{
		//Creates an orthographic projection
	projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
	}



	//Retrieves and passes transform matrices to the shader program
	modelLoc = glGetUniformLocation(objShaderProgram, "model");
	viewLoc = glGetUniformLocation(objShaderProgram, "view");
	projLoc = glGetUniformLocation(objShaderProgram, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//Reference matrix uniforms from the pyramid Shader program for the pyramid color, light color, light position, and camera position
	objectColorLoc = glGetUniformLocation(objShaderProgram, "objectColor");
	lightColorLoc = glGetUniformLocation(objShaderProgram, "lightColor");
	lightPositionLoc = glGetUniformLocation(objShaderProgram, "lightPos");
	viewPositionLoc = glGetUniformLocation(objShaderProgram, "viewPosition");

	//Pass color, light, and camera data to the pyramid shader program's corresponding uniforms
	glUniform3f(objectColorLoc, objectColor.r, objectColor.g, objectColor.b);
	glUniform3f(lightColorLoc, lightColor.r, lightColor.g, lightColor.b);
	glUniform3f(lightPositionLoc, lightPosition.x, lightPosition.y, lightPosition.z);
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	glutPostRedisplay();

	glBindTexture(GL_TEXTURE_2D, texture);

	glDrawElements(GL_TRIANGLES, 504, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0); // Deactivate the Vertex Array Object

	// use the lamp shader and activate the lamp vertex array object for rendering and transforming
	glUseProgram(lampShaderProgram);
	glBindVertexArray(LampVAO);

	//Transform the smaller pyramid used as a visual que for the light source
	model = glm::translate(model, lightPosition);
	model = glm::scale(model, lightScale);

	//Reference matrix uniforms from the Lamp Shader Program
	modelLoc = glGetUniformLocation(lampShaderProgram, "model");
	viewLoc = glGetUniformLocation(lampShaderProgram, "view");
	projLoc = glGetUniformLocation(lampShaderProgram, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//glDrawArrays(GL_TRIANGLES, 0, 18);

	glBindVertexArray(0);

	glutPostRedisplay();

	glutSwapBuffers(); // Flips the back buffer with the front buffer every frame.
}

void UCreateShader(void)
{
	//Vertex shader
	GLint objVertexShader = glCreateShader(GL_VERTEX_SHADER); //Creates the vertex shader
	glShaderSource(objVertexShader, 1, &objVertexShaderSource, NULL); // Attaches the Vertex  shader to the cource code
	glCompileShader(objVertexShader); // Compile the vertex shader

	// Fragment shader
	GLint objFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Creates the Fragment shader
	glShaderSource(objFragmentShader, 1, &objFragmentShaderSource, NULL); // retrieves the fragment shader source code
	glCompileShader(objFragmentShader); // Compile the fragment shader

	// Shader program
	objShaderProgram = glCreateProgram(); // Creates the Shader program and returns an id
	glAttachShader(objShaderProgram, objVertexShader); // Attach Vertex shader to the Shader program
	glAttachShader(objShaderProgram, objFragmentShader); // Attach Fragment shader to the Shader program
	glLinkProgram(objShaderProgram); //Link vertex and fragment shader to shader program

	// Delete the Vertex and Fragment shaders once linked
	glDeleteShader(objVertexShader);
	glDeleteShader(objFragmentShader);

	//Lamp Vertex shader
	GLint lampVertexShader = glCreateShader(GL_VERTEX_SHADER); //Creates the vertex shader
	glShaderSource(lampVertexShader, 1, &lampVertexShaderSource, NULL); // Attaches the Vertex  shader to the cource code
	glCompileShader(lampVertexShader); // Compile the vertex shader

	//lamp Fragment shader
	GLint lampFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Creates the Fragment shader
	glShaderSource(lampFragmentShader, 1, &lampFragmentShaderSource, NULL); // retrieves the fragment shader source code
	glCompileShader(lampFragmentShader); // Compile the fragment shader

	//lamp Shader program
	lampShaderProgram = glCreateProgram(); // Creates the Shader program and returns an id
	glAttachShader(lampShaderProgram, lampVertexShader); // Attach Vertex shader to the Shader program
	glAttachShader(lampShaderProgram, lampFragmentShader); // Attach Fragment shader to the Shader program
	glLinkProgram(lampShaderProgram); //Link vertex and fragment shader to shader program

	// Delete the lamp shaders once linked
	glDeleteShader(lampVertexShader);
	glDeleteShader(lampFragmentShader);
}

void UCreateBuffers()
{
	//Position and Color data
	GLfloat vertices[] = {
								// Vertex Positions		//Normals			//Textures
								//Bottom leg 1			//Negative Y Normals
								-0.5f, -0.5f, -0.5f,   -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
								-0.3f, -0.5f, -0.5f,   1.0f, -1.0f, -1.0f,	0.0f, 0.0f,
								-0.3f, -0.5f, -0.3f,   1.0f, -1.0f, 1.0f,	0.0f, 0.0f,
								-0.5f, -0.5f, -0.3f,   -1.0f, -1.0f, 1.0f,	0.0f, 0.0f,

								//Bottom leg 2			//Negative Y Normals
								-0.5f, -0.5f,  0.3f,   -1.0f, -1.0f, -1.0f, 	0.0f, 0.0f,
								-0.3f, -0.5f,  0.3f,  1.0f, -1.0f, -1.0f,	0.0f, 0.0f,
								-0.3f, -0.5f,  0.5f,   1.0f, -1.0f, 1.0f,	0.0f, 0.0f,
								-0.5f, -0.5f,  0.5f,   -1.0f, -1.0f, 1.0f,	0.0f, 0.0f,

								//Bottom leg 3			//Negative Y Normals
								 0.3f, -0.5f, -0.5f,   -1.0f, -1.0f, -1.0f, 	0.0f, 0.0f,
								 0.5f, -0.5f, -0.5f,   1.0f, -1.0f, -1.0f,	0.0f, 0.0f,
								 0.5f, -0.5f, -0.3f,   1.0f, -1.0f, 1.0f,	0.0f, 0.0f,
								 0.3f, -0.5f, -0.3f,   -1.0f, -1.0f, 1.0f,	0.0f, 0.0f,

								 //Bottom leg 4			//Negative Y Normals
								 0.3f, -0.5f,  0.3f,   -1.0f, -1.0f, -1.0f, 	0.0f, 0.0f,
								 0.5f, -0.5f,  0.3f,   1.0f, -1.0f, -1.0f,	0.0f, 0.0f,
								 0.5f, -0.5f,  0.5f,   1.0f, -1.0f, 1.0f,	0.0f, 0.0f,
								 0.3f, -0.5f,  0.5f,   -1.0f, -1.0f, 1.0f,	0.0f, 0.0f,

								 //Top leg 1
								-0.5f, -0.2f, -0.5f,   -1.0f, 1.0f, -1.0f, 	0.0f, 0.0f,
								-0.3f, -0.2f, -0.5f,   1.0f, 1.0f, -1.0f,	0.0f, 0.0f,
								-0.3f, -0.2f, -0.3f,   1.0f, 1.0f, 1.0f,	0.0f, 0.0f,
								-0.5f, -0.2f, -0.3f,   -1.0f, 1.0f, 1.0f,	0.0f, 0.0f,

								//Top leg 2
								-0.5f, -0.2f,  0.3f,   -1.0f, 1.0f, -1.0f, 	0.0f, 0.0f,
								-0.3f, -0.2f,  0.3f,   1.0f, 1.0f, -1.0f,	0.0f, 0.0f,
								-0.3f, -0.2f,  0.5f,   1.0f, 1.0f, 1.0f,	0.0f, 0.0f,
								-0.5f, -0.2f,  0.5f,   -1.0f, 1.0f, 1.0f,	0.0f, 0.0f,

								//Top leg 3
								 0.3f, -0.2f, -0.5f,   -1.0f, 1.0f, -1.0f, 	0.0f, 0.0f,
								 0.5f, -0.2f, -0.5f,   1.0f, 1.0f, -1.0f,	0.0f, 0.0f,
								 0.5f, -0.2f, -0.3f,   1.0f, 1.0f, 1.0f,	0.0f, 0.0f,
								 0.3f, -0.2f, -0.3f,   -1.0f, 1.0f, 1.0f,	0.0f, 0.0f,

								 //Top leg 4
								 0.3f, -0.2f,  0.3f,  -1.0f, 1.0f, -1.0f, 	0.0f, 0.0f,
								 0.5f, -0.2f,  0.3f,  1.0f, 1.0f, -1.0f,		0.0f, 0.0f,
								 0.5f, -0.2f,  0.5f,  1.0f, 1.0f, 1.0f,		0.0f, 0.0f,
								 0.3f, -0.2f,  0.5f,  -1.0f, 1.0f, 1.0f,		0.0f, 0.0f,

								 //Backrest bottom
								-0.5f, -0.2f, -0.5f,  -1.0f, -1.0f, -1.0f, 	0.0f, 0.0f,
								-0.3f, -0.2f, -0.5f,  1.0f, -1.0f, -1.0f,	1.0f, 0.0f,
								-0.3f, -0.2f,  0.5f,  1.0f, -1.0f, 1.0f,	1.0f, 1.0f,
								-0.5f, -0.2f,  0.5f,  -1.0f, -1.0f, 1.0f,	0.0f, 1.0f,

								//Backrest top
								-0.5f,  0.7f, -0.5f,  -1.0f, 1.0f, -1.0f, 	0.0f, 0.0f,
								-0.3f,  0.7f, -0.5f,  1.0f, 1.0f, -1.0f,	1.0f, 0.0f,
								-0.3f,  0.7f,  0.5f,  1.0f, 1.0f, 1.0f,		1.0f, 1.0f,
								-0.5f,  0.7f,  0.5f,  -1.0f, 1.0f, 1.0f,	0.0f, 1.0f,

								//Seat bottom
								-0.3f, -0.2f, -0.5f,  -1.0f, -1.0f, -1.0f, 	0.0f, 0.0f,
								 0.5f, -0.2f, -0.5f,  1.0f, -1.0f, -1.0f,	1.0f, 0.0f,
								 0.5f, -0.2f,  0.5f,  1.0f, -1.0f, 1.0f,	1.0f, 1.0f,
								-0.3f, -0.2f,  0.5f,  -1.0f, -1.0f, 1.0f,	0.0f, 1.0f,

								//Seat top
								-0.3f,  0.0f, -0.5f,  -1.0f, 1.0f, -1.0f, 	0.0f, 0.0f,
								 0.5f,  0.0f, -0.5f,  1.0f, 1.0f, -1.0f,	1.0f, 0.0f,
								 0.5f,  0.0f,  0.5f,  1.0f, 1.0f, 1.0f,		1.0f, 1.0f,
								-0.3f,  0.0f,  0.5f,  -1.0f, 1.0f, 1.0f,	0.0f, 1.0f

						};

	//Index data to share position data
	GLuint indices[] = {
								 0,  1,  2, //Leg 1
								 0,  3,  2,
								 0, 16, 17,
								 0,  1, 17,
								 0,  3, 19,
								 0, 16, 19,
								 2, 18, 19,
								 2,  3, 19,
								 2,  1, 17,
								 2, 18, 17,

								 4,  5,  6, //Leg 2
								 4,  7,  6,
								 4,  5, 21,
								 4, 20, 21,
								 4,  7, 23,
								 4, 20, 23,
								 6,  7, 23,
								 6, 22, 23,
								 6,  5, 21,
								 6, 22, 21,

								 8,  9, 10, //leg 3
								 8, 11, 10,
								 8,  9, 25,
								 8, 24, 25,
								 8, 11, 27,
								 8, 24, 27,
								10, 11, 27,
								10, 26, 27,
								10,  9, 25,
								10, 26, 25,

								12, 13, 14, //leg 4
								12, 15, 14,
								12, 13, 29,
								12, 28, 29,
								12, 15, 31,
								12, 28, 31,
								14, 15, 31,
								14, 30, 31,
								14, 13, 29,
								14, 30, 29,

								32, 33, 34, //Backrest
								32, 35, 34,
								32, 35, 39,
								32, 36, 39,
								32, 33, 37,
								32, 36, 37,
								34, 33, 37,
								34, 38, 37,
								34, 35, 39,
								34, 38, 39,
								36, 37, 38,
								36, 39, 38,

								40, 41, 42, // Seat
								40, 43, 42,
								40, 41, 45,
								40, 44, 45,
								40, 43, 47,
								40, 44, 47,
								42, 43, 47,
								42, 46, 47,
								42, 41, 45,
								42, 46, 45,
								44, 45, 46,
								44, 47, 46
						};

	//Generate buffer ids
	glGenVertexArrays(1, &ObjVAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	//Activate the Vertex Array Object before binding and setting any VBOs and VErtex Attribute Pointers
	glBindVertexArray(ObjVAO);

	//Activate the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Copy vertices to VBO

	//Activate the Element Buffer Object / Indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Copy indices to EBO

	//Set attribute pointer 0 to hold Position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); // Enables vertex attribute

	//Set attribute pointer 1 to hold Normal data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1); // Enables vertex attribute

	//Set attribute pointer 2 to hold Texture data
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Deactivates the VAO which is good practice

	//Generate buffer ids for lamp
	glGenVertexArrays(1, &LampVAO);

	//Activate the Vertex Array Object before binding and setting any VBOs and VErtex Attribute Pointers
	glBindVertexArray(LampVAO);

	//Activate the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Set attribute pointer 0 to hold Position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); // Enables vertex attribute
	glBindVertexArray(0); // Deactivates the VAO which is good practice

}

void UKeyboard(unsigned char key, GLint x, GLint y)
{
	switch(key){
				case 'w':
					currentKey = key;
					cout << "You pressed W!"<<endl;
					break;

				case 's':
					currentKey = key;
					cout << "You pressed S!"<<endl;
					break;

				case'a':
					currentKey = key;
					cout << "You pressed A!"<<endl;
					break;

				case 'd':
					currentKey = key;
					cout << "You pressed D!"<<endl;
					break;

				case 'o':
					currentKey = key;
					cout << "You pressed O!"<<endl;
					SwapProjection();
					break;

				default:
					cout <<"Press a key!"<<endl;
	}
}

void UKeyReleased(unsigned char key, GLint x, GLint y)
{
	cout << "Key released!"<<endl;
	currentKey = '0';
}

void UMouseMove (int x, int y)
{

	//Immediately replaces center locked coordinates with new mouse coordinates
	if(mouseDetected)
	{
		lastMouseX = x;
		lastMouseY = y;
		mouseDetected = false;
	}

	//Gets the direction the mouse was moved in x and y
	mouseXOffset = x - lastMouseX;
	mouseYOffset = lastMouseY - y; //inverted Y

	//Updates with new mouse coordinates
	lastMouseX = x;
	lastMouseY = y;

	//Applies sensitivity to mouse direction
	mouseXOffset *= sensitivity;
	mouseYOffset *= sensitivity;

	//Accumulates the yaw and pitch variables
	yaw += mouseXOffset;
	pitch += mouseYOffset;

	//Maintains a 90 degree pitch for gimbal lock
	if(pitch > 89.0f)
			pitch = 89.0f;

	if(pitch < -89.0f)
			pitch = -89.0f;

	//orbits around the center
	front.x = 10.0f * cos(yaw);
	front.y = 10.0f * sin(pitch);
	front.z = sin(yaw) * cos(pitch) * 10.0f;

}

void UGenerateTexture(){

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int width, height;

	unsigned char* image = SOIL_load_image("cloth.jpg", &width, &height, 0, SOIL_LOAD_RGB); // Loads texture file

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); //Unbind the texture
}

void SwapProjection(void){
	projSwap = !projSwap;
}

void Menu()
{
	// Displays menu for the user to choose from
	cout << "Choose a shape to render" << endl;
	cout << "1. Cube" << endl;
	cout << "2. Pyramid" << endl;
	cout << "3. Chair" << endl;
	cout << "Choice: ";
}

void UCreateCubeBuffers()
{
	//Position and Color data for the Cube
	GLfloat vertices[] = {
			// Vertex Positions		//Colors
			 0.5f,  0.5f, 0.0f,	  1.0f, 0.0f, 0.0f,	// Top Right Vertex 0
			 0.5f, -0.5f, 0.0f,	  0.0f, 1.0f, 0.0f,	// Bottom Right Vertex 1
			-0.5f, -0.5f, 0.0f,	  0.0f, 0.0f, 1.0f, // Bottom Left Vertex 2
			-0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 1.0f,	// Top Left Vertex 3

			 0.5f, -0.5f, -1.0f,  0.5f, 0.5f, 1.0f, // 4 br right
			 0.5f,  0.5f, -1.0f,  1.0f, 1.0f, 0.5f, // 5 tl right
			-0.5f,  0.5f, -1.0f,  0.2f, 0.2f, 0.5f, // 6 tl top
			-0.5f, -0.5f, -1.0f,  1.0f, 0.0f, 1.0f  // 7 bl back

						};
	// Index data to share position data
	GLuint indices[] = {
							0, 1, 3, //Triangle 1
							1, 2, 3, //Triangle 2
							0, 1, 4, //Triangle 3
							0, 4, 5, //Triangle 4
							0, 5, 6, //Triangle 5
							0, 3, 6, //Triangle 6
							4, 5, 6, //Triangle 7
							4, 6, 7, //Triangle 8
							2, 3, 6, //Triangle 9
							2, 6, 7, //Triangle 10
							1, 4, 7, //Triangle 11
							1, 2, 7  //Triangle 12
						};

	//Generate buffer ids
	glGenVertexArrays(1, &ObjVAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	//Activate the Vertex Array Object before binding and setting any VBOs and VErtex Attribute Pointers
	glBindVertexArray(ObjVAO);

	//Activate the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Copy vertices to VBO

	//Activate the Element Buffer Object / Indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Copy indices to EBO

	//Set attribute pointer 0 to hold Position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); // Enables vertex attribute

	//Set attribute pointer 1 to hold Color data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1); // Enables vertex attribute

	glBindVertexArray(0); // Deactivates the VAO which is good practice
}

void UCreatePyramidBuffers()
{
	//Position and Color data
		GLfloat vertices[] = {
				// Vertex Positions		//Colors
				 0.5f,  0.0f,  0.5f,   1.0f, 0.0f, 0.0f,	// Base Top Right Vertex 0
				 0.5f,  0.0f, -0.5f,   0.0f, 1.0f, 0.0f,	// Base Bottom Right Vertex 1
				-0.5f,  0.0f, -0.5f,   0.0f, 0.0f, 1.0f, 	// Base Bottom Left Vertex 2
				-0.5f,  0.0f,  0.5f,   1.0f, 0.0f, 1.0f,	// Base Top Left Vertex 3
				 0.0f,  0.5f,  0.0f,   0.5f, 0.5f, 1.0f, 	// Peak Vertex 4

							};
		// Index data to share position data
		GLuint indices[] = {
								0, 1, 3, //Triangle 1
								1, 2, 3, //Triangle 2
								0, 1, 4, //Triangle 3
								0, 3, 4, //Triangle 4
								1, 2, 4, //Triangle 5
								2, 3, 4  //Triangle 6

							};

		//Generate buffer ids
		glGenVertexArrays(1, &ObjVAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		//Activate the Vertex Array Object before binding and setting any VBOs and VErtex Attribute Pointers
		glBindVertexArray(ObjVAO);

		//Activate the VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Copy vertices to VBO

		//Activate the Element Buffer Object / Indices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Copy indices to EBO

		//Set attribute pointer 0 to hold Position data
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0); // Enables vertex attribute

		//Set attribute pointer 1 to hold Color data
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1); // Enables vertex attribute

		glBindVertexArray(0); // Deactivates the VAO which is good practice
}
