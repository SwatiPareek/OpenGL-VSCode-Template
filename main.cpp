//
// COMP 371 Labs Framework
//
// Created by Nicolas Bergeron on 20/06/2019.
//
// Inspired by the following tutorials:
// - https://learnopengl.com/Getting-started/Hello-Window
// - https://learnopengl.com/Getting-started/Hello-Triangle

#include <iostream>
#include <vector>
#include <fstream>
#include <string>


#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
                        // initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices

// Global Variables
// ---------------------------------

// Primitive Scale
float GridUnit = 0.01f;

// Mouse Controls
double lastCursorPosX = -1.0f;
double lastCursorPosY = -1.0f;
bool isLeftButtonPressed = false;
bool isRightButtonPressed = false;
bool isMiddleButtonPressed = false;

// Camera
glm::vec3 cameraPosition;
glm::vec3 cameraLookAt;
glm::vec3 cameraUp;
glm::vec3 cameraRight;

// Model-View-Projection Matrices
glm::mat4 worldMatrix;
glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;

// Global identifiers
unsigned int shaderProgram;


// Create Geometry
// ---------------------------------

/* This struct holds the vao, vbo and ebo identifiers used to draw the geometry */
struct Geometry
{
	unsigned int vao;
	unsigned int vbo;
	unsigned int ebo;
	Geometry(unsigned int vao, unsigned int vbo, unsigned int ebo) : vao(0), vbo(0), ebo(0) {}
};

// Initialize Geometry Structures
Geometry Grid(0, 0, 0);
Geometry Cube(0, 0, 0);

void createGeometryGrid()
{
	// Generate 100x100 Grid
	glm::vec3 vertexArray[800];
	// Z Lines (200 Vertices)
	float xCoord = -(GridUnit*100/2);
	for (int x = 0; x < 200; x += 2)
	{
		if (x != 0)
			xCoord += GridUnit;

		vertexArray[x] = glm::vec3(xCoord, 0.0f, -(GridUnit * 100 / 2));
		vertexArray[x + 1] = glm::vec3(xCoord, 0.0f, (GridUnit * 100 / 2));
	}

	// X Lines (200 Vertices)
	float zCoord = -(GridUnit * 100 / 2);
	for (int z = 200; z < 400; z += 2)
	{
		if (z != 200)
			zCoord += GridUnit;

		vertexArray[z] = glm::vec3(-(GridUnit * 100 / 2), 0.0f, zCoord);
		vertexArray[z + 1] = glm::vec3((GridUnit * 100 / 2), 0.0f, zCoord);
	}

	// Create a vertex array
	glGenVertexArrays(1, &Grid.vao);
	glBindVertexArray(Grid.vao);


	// Upload Vertex Buffer to the GPU, keep a reference to it (vertexBufferObject)
	glGenBuffers(1, &Grid.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, Grid.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);

	glVertexAttribPointer(	0,                   // attribute 0 matches aPos in Vertex Shader
							3,                   // size
							GL_FLOAT,            // type
							GL_FALSE,            // normalized?
							sizeof(glm::vec3),	 // stride - each vertex contains 1 vec3 (position)
							(void*)0             // array buffer offset
	);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void createGeometryUnitCube()
{
	glm::vec3 vertexArray[] =
	{
		// Axis Polygon (X Default)
		glm::vec3(-(GridUnit/2), 0.0f, -(GridUnit / 2)),			// Bottom-left		0
		glm::vec3((GridUnit / 2), 0.0f, -(GridUnit / 2)),			// Bottom-right		1
		glm::vec3((GridUnit / 2), GridUnit, -(GridUnit / 2)),		// Top-right		2
		glm::vec3(-(GridUnit / 2), GridUnit, -(GridUnit / 2)),		// Top-left			3

		glm::vec3(-(GridUnit / 2), 0.0f, (GridUnit / 2)),			// Bottom-left		4
		glm::vec3((GridUnit / 2), 0.0f, (GridUnit / 2)),			// Bottom-right		5
		glm::vec3((GridUnit / 2), GridUnit, (GridUnit / 2)),		// Top-right		6
		glm::vec3(-(GridUnit / 2), GridUnit, (GridUnit / 2))		// Top-left			7
	};

	unsigned int elements[] =
	{
		// Back Face
		2, 1, 0,
		0, 3, 2,

		// Front Face
		4, 5, 6,
		6, 7, 4,

		// Top Face
		2, 3, 7,
		7, 6, 2,

		// Bottom Face
		1, 5, 4,
		4, 0, 1,

		// Right Face
		2, 6, 5,
		5, 1, 2,

		// Left Face
		3, 0, 4,
		4, 7, 3
	};

	// Create a vertex array
	glGenVertexArrays(1, &Cube.vao);
	glBindVertexArray(Cube.vao);


	// Upload Vertex Buffer Object
	glGenBuffers(1, &Cube.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, Cube.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);

	glVertexAttribPointer(	0,                   // attribute 0 matches aPos in Vertex Shader
							3,                   // size
							GL_FLOAT,            // type
							GL_FALSE,            // normalized?
							sizeof(glm::vec3),	 // stride - each vertex contains 1 vec3 (position)
							(void*)0             // array buffer offset
	);
	glEnableVertexAttribArray(0);

	// Upload Element Buffer Object
	glGenBuffers(1, &Cube.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Cube.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

// ---------------------------------

/* Read in a file as a string. Used for shaders */
std::string readFile(const char *filePath) {
	std::string content;
	std::ifstream fileStream(filePath, std::ios::in);

	if (!fileStream.is_open()) {
		std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
		return "";
	}

	std::string line = "";
	while (!fileStream.eof()) {
		std::getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;
}

/* Compiles and Links Shaders into a Shader Program, returning the program id*/
unsigned int createShaderProgram()
{
    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	std::string vertexShaderString = readFile("../../res/shaders/vertex0.vert");
	const char* vertexShaderSource = vertexShaderString.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // grid fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	std::string fragmentShaderString = readFile("../../res/shaders/fragment0.frag");
	const char* fragmentShaderSource = fragmentShaderString.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // check for linking errors
    glGetProgramiv(fragmentShader, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}



/* Sets the projection matrix uniform for the given shader program */
void setProjectionMatrix(int shaderProgram, glm::mat4 projectionMatrix)
{
	glUseProgram(shaderProgram);
	GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
}

/* Sets the view matrix uniform for the given shader program */
void setViewMatrix(int shaderProgram, glm::mat4 viewMatrix)
{
	glUseProgram(shaderProgram);
	GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
}

/* Sets the world matrix uniform for the given shader program */
void setWorldMatrix(int shaderProgram, glm::mat4 worldMatrix)
{
	glUseProgram(shaderProgram);
	GLuint worldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
}

void setTransformMatrix(int shaderProgram, glm::mat4 transformMatrix)
{
	glUseProgram(shaderProgram);
	GLuint transformMatrixLocation = glGetUniformLocation(shaderProgram, "transformMatrix");
	glUniformMatrix4fv(transformMatrixLocation, 1, GL_FALSE, &transformMatrix[0][0]);
}

void setFragmentColour(int shaderProgram, glm::vec4 fragmentColour)
{
	glUseProgram(shaderProgram);
	GLuint fragmentColourLocation = glGetUniformLocation(shaderProgram, "fragmentColour");
	glUniform4fv(fragmentColourLocation, 1, &fragmentColour[0]);
}

// Hierarchical Model Structure
// ---------------------------------
struct HierarchicalModel
{
	HierarchicalModel* Parent;
	std::vector<HierarchicalModel*> Children;
	glm::mat4 transform;
	glm::vec4 fragmentColour;
	glm::vec3 center;

	HierarchicalModel(HierarchicalModel* parent)
	{
		Parent = parent;
		transform = glm::mat4(1.0f);
		fragmentColour = glm::vec4(1.0f);
	}

	// Add a child to the current node
	void AddChild(HierarchicalModel* child)
	{
		Children.push_back(child);
	}

	// Transformations are applied recursively through the children
	void ApplyTransform(glm::mat4 t)
	{
		if (Children.size() > 0)
		{
			for (int i = 0; i < Children.size(); i++)
			{
				Children[i]->ApplyTransform(t);
			}
		}
		transform = t * transform;
	}

	// Get current node's transform
	glm::mat4 GetTransform()
	{
		return transform;
	}

	void SetFragmentColour(glm::vec4 colour)
	{
		fragmentColour = colour;
	}

	// Get current node's fragment colour
	glm::vec4 GetFragmentColour()
	{
		return fragmentColour;
	}

	// Draw Hierarchy
	void Draw(unsigned int shaderProgram, unsigned int renderMode)
	{
		if (Children.size() > 0)
		{
			glUseProgram(shaderProgram);
			glBindVertexArray(Cube.vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Cube.ebo);

			for (int i = 0; i < Children.size(); i++)
			{
				setTransformMatrix(shaderProgram, Children[i]->GetTransform());
				setFragmentColour(shaderProgram, Children[i]->GetFragmentColour());
				glDrawElements(renderMode, 36, GL_UNSIGNED_INT, nullptr);
			}
		}
	}
};

/* Callback function for mouse controls */
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
	double xPos, yPos;
	glfwGetCursorPos(window, &xPos, &yPos);
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		lastCursorPosY = yPos;
		lastCursorPosX = xPos;
		isLeftButtonPressed = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		return;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action== GLFW_PRESS)
	{
		lastCursorPosY = yPos;
		lastCursorPosX = xPos;
		isLeftButtonPressed = true;
		isRightButtonPressed = false;
		isMiddleButtonPressed = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		return;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		lastCursorPosY = yPos;
		lastCursorPosX = xPos;
		isRightButtonPressed = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		return;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		lastCursorPosY = yPos;
		lastCursorPosX = xPos;
		isLeftButtonPressed = false;
		isRightButtonPressed = true;
		isMiddleButtonPressed = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		return;
	}
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
	{
		lastCursorPosY = yPos;
		lastCursorPosX = xPos;
		isMiddleButtonPressed = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		return;
	}
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
	{
		lastCursorPosY = yPos;
		lastCursorPosX = xPos;
		isLeftButtonPressed = false;
		isRightButtonPressed = false;
		isMiddleButtonPressed = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		return;
	}
}

int main(int argc, char*argv[])
{
    // Initialize GLFW and OpenGL version
    glfwInit();
    
#if defined(PLATFORM_OSX)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    // On windows, we set OpenGL version to 2.1, to support more hardware
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif

    // Create Window and rendering context using GLFW, resolution is 800x600
    GLFWwindow* window = glfwCreateWindow(1024, 768, "Comp371 - Assignment 1 - Christian Galante", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

	// Initialize GLFW Input
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Compile and link shaders here ...
    shaderProgram = createShaderProgram();
    
    // Define and upload geometry to the GPU here ...
    createGeometryGrid();
	createGeometryUnitCube();

	// Initialize World, View and Projection Matrices

	worldMatrix = glm::mat4(1.0f);
	setWorldMatrix(shaderProgram, worldMatrix);


	float cameraSpeed = 0.75f;
	cameraPosition = glm::vec3(0.0f, 0.075f, 0.05f);
	cameraLookAt = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
	viewMatrix = lookAt(cameraPosition,  // eye
						cameraLookAt,  // center
						cameraUp); // up
	setViewMatrix(shaderProgram, viewMatrix);

	projectionMatrix = glm::perspective(	70.0f,// field of view in degrees
											1024.0f / 768.0f,  // aspect ratio
											0.01f, 10.0f);   // near and far (near > 0)
	setProjectionMatrix(shaderProgram, projectionMatrix);

	// Define transforms to create axes from unit cube geometry
	glm::mat4 transformXAxis(1.0f);
	transformXAxis = glm::translate(transformXAxis, glm::vec3(GridUnit, 0.0f, 0.0f));
	transformXAxis = glm::scale(transformXAxis, glm::vec3(2.0f, 0.025f, 0.025f));

	glm::mat4 transformZAxis(1.0f);
	transformZAxis = glm::translate(transformZAxis, glm::vec3(0.0f, 0.0f, GridUnit));
	transformZAxis = glm::rotate(transformZAxis, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	transformZAxis = glm::scale(transformZAxis, glm::vec3(2.0f, 0.025f, 0.025f));

	glm::mat4 transformYAxis(1.0f);
	transformYAxis = glm::translate(transformYAxis, glm::vec3(0.0f, GridUnit, 0.0f));
	transformYAxis = glm::rotate(transformYAxis, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	transformYAxis = glm::scale(transformYAxis, glm::vec3(2.0f, 0.025f, 0.025f));

	// Initialize Olaf Hierarchical Model
	glm::mat4 transform(1.0f);
	glm::vec3 olafPosition(0.0f, (GridUnit/4), 0.0f);
	glm::vec3 olafDirection(0.0f, 0.0f, 1.0f);
	float olafRotationSpeed = 2.0f;
	float olafMovementSpeed = 0.1f;
	float olafScaleIncrement = 0.0125f;
	// Olaf is the root of the hierarchy
	HierarchicalModel* Olaf = new HierarchicalModel(nullptr);
	// Add children to olaf and define each child's world transform
	// Olaf/Body
	HierarchicalModel* Olaf_Body = new HierarchicalModel(Olaf);
	transform = glm::translate(transform, glm::vec3(0.0f, GridUnit / 4, 0.0f));
	transform = glm::scale(transform, glm::vec3(1.5f, 2.0f, 2.0f));
	Olaf_Body->ApplyTransform(transform);
	Olaf_Body->SetFragmentColour(glm::vec4(0.75f, 0.75f, 0.75f, 1.0f));
	Olaf->AddChild(Olaf_Body);
	// Olaf/Head
	HierarchicalModel* Olaf_Head = new HierarchicalModel(Olaf);
	transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, (9*GridUnit/4), 0.0f));
	Olaf_Head->ApplyTransform(transform);
	Olaf->AddChild(Olaf_Head);
	// Olaf/Nose
	HierarchicalModel* Olaf_Nose = new HierarchicalModel(Olaf);
	transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, (10 * GridUnit / 4), (GridUnit/2)));
	transform = glm::scale(transform, glm::vec3(0.1f, 0.2f, 0.1f));
	Olaf_Nose->ApplyTransform(transform);
	Olaf_Nose->SetFragmentColour(glm::vec4(1.0f, 0.55f, 0.0f, 1.0f));
	Olaf->AddChild(Olaf_Nose);
	// Olaf/LHand
	HierarchicalModel* Olaf_LHand = new HierarchicalModel(Olaf);
	transform = glm::translate(glm::mat4(1.0f), glm::vec3((7 * GridUnit / 8), (8 * GridUnit / 4), GridUnit));
	transform = glm::scale(transform, glm::vec3(0.25f, 0.25f, 2.0f));
	Olaf_LHand->ApplyTransform(transform);
	Olaf->AddChild(Olaf_LHand);
	// Olaf/LHand
	HierarchicalModel* Olaf_RHand = new HierarchicalModel(Olaf);
	transform = glm::translate(glm::mat4(1.0f), glm::vec3(-(7 * GridUnit / 8), (8 * GridUnit / 4), GridUnit));
	transform = glm::scale(transform, glm::vec3(0.25f, 0.25f, 2.0f));
	Olaf_RHand->ApplyTransform(transform);
	Olaf->AddChild(Olaf_RHand);
	// Olaf/RLeg
	HierarchicalModel* Olaf_RLeg = new HierarchicalModel(Olaf);
	transform = glm::translate(glm::mat4(1.0f), glm::vec3((GridUnit / 2), 0.0f, 0.0f));
	transform = glm::scale(transform, glm::vec3(0.25f, 0.25f, 2.0f));
	Olaf_RLeg->ApplyTransform(transform);
	Olaf->AddChild(Olaf_RLeg);
	// Olaf/LLeg
	HierarchicalModel* Olaf_LLeg = new HierarchicalModel(Olaf);
	transform = glm::translate(glm::mat4(1.0f), glm::vec3(-(GridUnit / 2), 0.0f, 0.0f));
	transform = glm::scale(transform, glm::vec3(0.25f, 0.25f, 2.0f));
	Olaf_LLeg->ApplyTransform(transform);
	Olaf->AddChild(Olaf_LLeg);
	// Olaf/REye
	HierarchicalModel* Olaf_REye = new HierarchicalModel(Olaf);
	transform = glm::translate(glm::mat4(1.0f), glm::vec3(-(GridUnit / 4), (11 * GridUnit / 4), (GridUnit/2)));
	transform = glm::scale(transform, glm::vec3(0.1f, 0.1f, 0.1f));
	Olaf_REye->ApplyTransform(transform);
	Olaf_REye->SetFragmentColour(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	Olaf->AddChild(Olaf_REye);
	// Olaf/LEye
	HierarchicalModel* Olaf_LEye = new HierarchicalModel(Olaf);
	transform = glm::translate(glm::mat4(1.0f), glm::vec3((GridUnit / 4), (11 * GridUnit / 4), (GridUnit / 2)));
	transform = glm::scale(transform, glm::vec3(0.1f, 0.1f, 0.1f));
	Olaf_LEye->ApplyTransform(transform);
	Olaf_LEye->SetFragmentColour(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	Olaf->AddChild(Olaf_LEye);

	// Initialize Fragment Colour
	glm::vec4 fragmentColour(1.0f);
	setFragmentColour(shaderProgram, fragmentColour);


	// Frame calculation variables
	float lastFrameTime = glfwGetTime();

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// Default render mode is triangles
	unsigned int renderMode = GL_TRIANGLES;

    // Entering Main Loop
    while(!glfwWindowShouldClose(window))
    {
		// Frame time calculation
		float dt = glfwGetTime() - lastFrameTime;
		lastFrameTime += dt;

        // Each frame, reset color of each pixel to glClearColor
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Select Shader Program */
		glUseProgram(shaderProgram);

        /* Draw Geometry 
		-------------------------*/
		
		// Draw Grid
		glBindVertexArray(Grid.vao);
		glBindBuffer(GL_ARRAY_BUFFER, Grid.vbo);
		setTransformMatrix(shaderProgram, glm::mat4(1.0f)); // Grid is at Origin
		setFragmentColour(shaderProgram, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
		glDrawArrays(GL_LINES, 0, 400);

		// Bind unit cube
		glBindVertexArray(Cube.vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Cube.ebo);

		// Draw X Axis
		setTransformMatrix(shaderProgram, transformXAxis);
		setFragmentColour(shaderProgram, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

		// Draw Y Axis
		setTransformMatrix(shaderProgram, transformYAxis);
		setFragmentColour(shaderProgram, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

		// Draw Z Axis
		setTransformMatrix(shaderProgram, transformZAxis);
		setFragmentColour(shaderProgram, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

		//Draw Olaf
		Olaf->ApplyTransform(glm::mat4(1.0f));
		Olaf->Draw(shaderProgram, renderMode);

		// Handle Inputs
		if (glfwGetKey(window, GLFW_KEY_HOME) == GLFW_PRESS) // Re-initialize world position and orientation
		{
			worldMatrix = glm::mat4(1.0f);
			setWorldMatrix(shaderProgram, worldMatrix);

			float cameraSpeed = 0.75f;
			cameraPosition = glm::vec3(0.0f, 0.075f, 0.05f);
			cameraLookAt = glm::vec3(0.0f, 0.0f, 0.0f);
			cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
			cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
			viewMatrix = lookAt(cameraPosition,  // eye
				cameraLookAt,  // center
				cameraUp); // up
			setViewMatrix(shaderProgram, viewMatrix);

			projectionMatrix = glm::perspective(70.0f,// field of view in degrees
				1024.0f / 768.0f,  // aspect ratio
				0.01f, 10.0f);   // near and far (near > 0)
			setProjectionMatrix(shaderProgram, projectionMatrix);
		}
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) // Change render mode to points
		{
			renderMode = GL_POINTS;
		}
		if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) // Change render mode to line loop
		{
			renderMode = GL_LINE_LOOP;
		}
		if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) // Change render mode to triangles
		{
			renderMode = GL_TRIANGLES;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // move Olaf left
		{
			glm::vec3 translation = glm::vec3(-olafMovementSpeed*dt, 0.0f, 0.0f);
			olafPosition += translation;
			Olaf->ApplyTransform(glm::translate(glm::mat4(1.0f), translation));
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // move Olaf right
		{
			glm::vec3 translation = glm::vec3(olafMovementSpeed*dt, 0.0f, 0.0f);
			olafPosition += translation;
			Olaf->ApplyTransform(glm::translate(glm::mat4(1.0f), translation));
		}
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // move Olaf forward
		{
			glm::vec3 translation = olafDirection * olafMovementSpeed*dt;
			transform = glm::translate(glm::mat4(1.0f), translation);
			glm::vec4 temp = transform * glm::vec4(olafPosition.x, olafPosition.y, olafPosition.z, 1.0f);
			olafPosition = glm::vec3(temp.x, temp.y, temp.z);
			Olaf->ApplyTransform(transform);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // move Olaf backward
		{
			glm::vec3 translation = olafDirection * -olafMovementSpeed*dt;
			transform = glm::translate(glm::mat4(1.0f), translation);
			glm::vec4 temp = transform * glm::vec4(olafPosition.x, olafPosition.y, olafPosition.z, 1.0f);
			olafPosition = glm::vec3(temp.x, temp.y, temp.z);
			Olaf->ApplyTransform(transform);
		}
		if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) // scale Olaf up
		{
			float scaleIncrement = (1 + olafScaleIncrement);
			Olaf->ApplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-olafPosition.x, -olafPosition.y, -olafPosition.z)));
			Olaf->ApplyTransform(glm::scale(glm::mat4(1.0f), glm::vec3(scaleIncrement, scaleIncrement, scaleIncrement)));
			Olaf->ApplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(olafPosition.x, olafPosition.y, olafPosition.z)));
		}
		if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) // scale Olaf down
		{
			float scaleIncrement = (1 - olafScaleIncrement);
			Olaf->ApplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-olafPosition.x, -olafPosition.y, -olafPosition.z)));
			Olaf->ApplyTransform(glm::scale(glm::mat4(1.0f), glm::vec3(scaleIncrement, scaleIncrement, scaleIncrement)));
			Olaf->ApplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(olafPosition.x, olafPosition.y, olafPosition.z)));		
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) // rotate Olaf left
		{
			float rotationAngle = olafRotationSpeed * dt;
			Olaf->ApplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-olafPosition.x, -olafPosition.y, -olafPosition.z)));

			transform = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec4 temp = transform * glm::vec4(olafDirection.x, olafDirection.y, olafDirection.z, 1.0f);
			olafDirection = glm::vec3(temp.x, temp.y, temp.z);
			Olaf->ApplyTransform(transform);

			Olaf->ApplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(olafPosition.x, olafPosition.y, olafPosition.z)));
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) // rotate Olaf right
		{
			float rotationAngle = olafRotationSpeed * dt;
			Olaf->ApplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-olafPosition.x, -olafPosition.y, -olafPosition.z)));

			transform = glm::rotate(glm::mat4(1.0f), -rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec4 temp = transform * glm::vec4(olafDirection.x, olafDirection.y, olafDirection.z, 1.0f);
			olafDirection = glm::vec3(temp.x, temp.y, temp.z);
			Olaf->ApplyTransform(transform);

			Olaf->ApplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(olafPosition.x, olafPosition.y, olafPosition.z)));
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) // rotate world about y
		{
			float rotationAngle = cameraSpeed * dt;
			transform = glm::rotate(glm::mat4(1.0f), -rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			worldMatrix = transform * worldMatrix;
			setWorldMatrix(shaderProgram, worldMatrix);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) // rotate world about -y
		{
			float rotationAngle = cameraSpeed * dt;
			transform = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			worldMatrix = transform * worldMatrix;
			setWorldMatrix(shaderProgram, worldMatrix);
		}
		if (isLeftButtonPressed) // Zooming
		{
			double xPos, yPos;
			glfwGetCursorPos(window, &xPos, &yPos);

			if (lastCursorPosY != -1.0f)
			{
				double dy = yPos - lastCursorPosY;
				glm::vec3 translation = (cameraPosition - cameraLookAt) * (float)dy*(GridUnit/4);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation);
				glm::vec4 temp = transform * glm::vec4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f);
				cameraPosition = glm::vec3(temp.x, temp.y, temp.z);
				viewMatrix = lookAt(cameraPosition,  // eye
									cameraLookAt,  // center
									cameraUp); // up	
				setViewMatrix(shaderProgram, viewMatrix);
			}
			lastCursorPosY = yPos;
		}
		if (isRightButtonPressed) // Panning
		{
			double xPos, yPos;
			glfwGetCursorPos(window, &xPos, &yPos);

			if (lastCursorPosX != -1.0f)
			{
				double dx = xPos - lastCursorPosX;
				glm::vec3 translation = cameraRight * -(float)dx*(GridUnit / 100);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation);
				glm::vec4 temp = transform * glm::vec4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f);
				cameraPosition = glm::vec3(temp.x, temp.y, temp.z);
				temp = transform * glm::vec4(cameraLookAt.x, cameraLookAt.y, cameraLookAt.z, 1.0f);
				cameraLookAt = glm::vec3(temp.x, temp.y, temp.z);
				temp = transform * glm::vec4(cameraRight.x, cameraRight.y, cameraRight.z, 1.0f);
				cameraRight = glm::vec3(temp.x, temp.y, temp.z);
				viewMatrix = lookAt(cameraPosition,  // eye
									cameraLookAt,  // center
									cameraUp); // up	
				setViewMatrix(shaderProgram, viewMatrix);
			}
			lastCursorPosX = xPos;
		}
		if (isMiddleButtonPressed) //Tilting
		{
			double xPos, yPos;
			glfwGetCursorPos(window, &xPos, &yPos);

			if (lastCursorPosY != -1.0f)
			{
				double dy = yPos - lastCursorPosY;
				float distance = -(float)dy*(GridUnit / 20);
				transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, distance, 0.0f));
				glm::vec4 temp = transform * glm::vec4(cameraLookAt.x, cameraLookAt.y, cameraLookAt.z, 1.0f);
				cameraLookAt = glm::vec3(temp.x, temp.y, temp.z);
				viewMatrix = lookAt(cameraPosition,  // eye
									cameraLookAt,  // center
										cameraUp); // up	
				setViewMatrix(shaderProgram, viewMatrix);
			}
			lastCursorPosY = yPos;
		}

		// End Frame
		glfwSwapBuffers(window);
		glfwPollEvents();
    }
    
    // Shutdown GLFW
    glfwTerminate();
    
	return 0;
}
