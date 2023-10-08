// 3D Graphics and Animation - Main Template
// This uses Visual Studio Code - https://code.visualstudio.com/docs/cpp/introvideos-cpp
// Two versions available -  Win64 and Apple MacOS - please see notes
// Last changed September 2023

//#pragma comment(linker, "/NODEFAULTLIB:MSVCRT")

//#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h> // Extend OpenGL Specfication to version 4.5 for WIN64 and versions 4.1 for Apple (note: two different files).

#include <imgui/imgui.h>			  // Load GUI library - Dear ImGui - https://github.com/ocornut/imgui
#include <imgui/imgui_impl_glfw.h>	  // Platform ImGui using GLFW
#include <imgui/imgui_impl_opengl3.h> // Platform new OpenGL - aka better than 3.3 core version.

#include <GLFW/glfw3.h> // Add GLFW - library to launch a window and callback mouse and keyboard - https://www.glfw.org

#define GLM_ENABLE_EXPERIMENTAL	 // Enable all functions include non LTS
#include <glm/glm.hpp>			 // Add helper maths library - GLM 0.9.9.9 - https://github.com/g-truc/glm - for example variables vec3, mat and operators.
#include <glm/gtx/transform.hpp> // Help us with transforms
using namespace glm;

//#include <tinygltf/tiny_gltf.h> // Model loading library - tiny gltf - https://github.com/syoyo/tinygltf
//#include "src/stb_image.hpp" // Add library to load images for textures

//#include "src/Mesh.hpp" // Simplest mesh holder and OBJ loader - can update more - from https://github.com/BennyQBD/ModernOpenGLTutorial

#include "src/Pipeline.hpp"		// Setup pipeline and load shaders.
#include "src/Content.hpp"		// Setup content loader and drawing functions - https://github.com/KhronosGroup/glTF - https://github.com/syoyo/tinygltf 
#include "src/Debugger.hpp"		// Setup debugger functions.



// Main fuctions
void startup();
void update();
void render();
void ui();
void endProgram();

// HELPER FUNCTIONS OPENGL
void hintsGLFW();
//string readShader(string name);
//void checkErrorShader(GLuint shader);
inline void errorCallbackGLFW(int error, const char *description){cout << "Error GLFW: " << description << "\n";};
void debugGL();

void APIENTRY openGLDebugCallback(GLenum source,
								  GLenum type,
								  GLuint id,
								  GLenum severity,
								  GLsizei length,
								  const GLchar *message,
								  const GLvoid *userParam);
GLenum glCheckError_(const char *file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__)

// Setup all the message loop callbacks - do this before Dear ImGui
// Callback functions for the window and interface devices
void onResizeCallback(GLFWwindow *window, int w, int h);
void onKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void onMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
void onMouseMoveCallback(GLFWwindow *window, double x, double y);
void onMouseWheelCallback(GLFWwindow *window, double xoffset, double yoffset);

// VARIABLES
GLFWwindow *window; 								// Keep track of the window
auto windowWidth = 800;								// Window width					
auto windowHeight =800;								// Window height
auto running(true);							  		// Are we still running our main loop
mat4 projMatrix;							 		// Our Projection Matrix
vec3 cameraPosition = vec3(0.0f, 0.0f, 5.0f);		// Where is our camera
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);			// Camera front vector
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);				// Camera up vector
auto aspect = (float)windowWidth / (float)windowHeight;	// Window aspect ration
auto fovy = 45.0f;									// Field of view (y axis)
bool keyStatus[1024];								// Track key strokes
auto currentTime = 0.0f;							// Framerate
auto deltaTime = 0.0f;								// time passed
auto lastTime = 0.0f;								// Used to calculate Frame rate

Pipeline pipeline;									// Add one pipeline plus some shaders.
Content content;									// Add one content loader (+drawing).
Debugger debugger;									// Add one debugger to use for callbacks ( Win64 - openGLDebugCallback() ) or manual calls ( Apple - glCheckError() ) 

vec3 modelPosition;									// Model position
vec3 modelRotation;									// Model rotation


int main()
{
	cout << endl << "===" << endl << "3D Graphics and Animation - Running..." << endl;

	if (!glfwInit()) // Check if GLFW is working, if not then stop all
	{
		cout << "Could not initialise GLFW...";
		return -1;
	} 

	glfwSetErrorCallback(errorCallbackGLFW); // Setup a function callback to catch and display all GLFW errors.
	hintsGLFW();							 // Setup glfw with various hints.

	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor()); // Grab reference to monitor
	// windowWidth = mode->width; windowHeight = mode->height; //fullscreen
	// window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), glfwGetPrimaryMonitor(), NULL); // fullscreen

	// Create our Window
	const auto windowTitle = "My 3D Graphics and Animation OpenGL Application"s;
	window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), NULL, NULL);
	if (!window) // Test if Window or OpenGL context creation failed
	{
		cout << "Could not initialise GLFW...";
		glfwTerminate();
		return -1;
	} 

	glfwSetWindowPos(window, 10, 10); // Place it in top corner for easy debugging.
	glfwMakeContextCurrent(window);	  // making the OpenGL context current

	// GLAD: Load OpenGL function pointers - aka update specs to newest versions - plus test for errors.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD...";
		glfwMakeContextCurrent(NULL);
		glfwTerminate();
		return -1;
	}

	glfwSetWindowSizeCallback(window, onResizeCallback);	   // Set callback for resize
	glfwSetKeyCallback(window, onKeyCallback);				   // Set Callback for keys
	glfwSetMouseButtonCallback(window, onMouseButtonCallback); // Set callback for mouse click
	glfwSetCursorPosCallback(window, onMouseMoveCallback);	   // Set callback for mouse move
	glfwSetScrollCallback(window, onMouseWheelCallback);	   // Set callback for mouse wheel.
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);	// Set mouse cursor Fullscreen
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	// Set mouse cursor FPS fullscreen.

	// Setup Dear ImGui and add context	-	https://blog.conan.io/2019/06/26/An-introduction-to-the-Dear-ImGui-library.html
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO(); //(void)io;
								  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	ImGui::StyleColorsLight(); // ImGui::StyleColorsDark(); 		// Setup Dear ImGui style

	// Setup Platform/Renderer ImGui backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	const auto glsl_version = "#version 410";
	ImGui_ImplOpenGL3_Init(glsl_version);

	#if defined(__WIN64__)
		debugGL(); // Setup callback to catch openGL errors.	V4.2 or newer
	#elif(__APPLE__)
		glCheckError(); // Old way of checking for errors. Newest not implemented by Apple. Manually call function anywhere in code to check for errors.
	#endif

	glfwSwapInterval(1);			 // Ony render when synced (V SYNC) - https://www.tomsguide.com/features/what-is-vsync-and-should-you-turn-it-on-or-off
	glfwWindowHint(GLFW_SAMPLES, 2); // Multisampling - covered in lectures - https://www.khronos.org/opengl/wiki/Multisampling

	startup(); // Setup all necessary information for startup (aka. load texture, shaders, models, etc).

	cout << endl << "Starting main loop and rendering..." << endl;	

	do{											 // run until the window is closed
		auto currentTime = (float)glfwGetTime(); // retrieve timelapse
		deltaTime = currentTime - lastTime;		 // Calculate delta time
		lastTime = currentTime;					 // Save for next frame calculations.

		glfwPollEvents(); 						// poll callbacks

		update(); 								// update (physics, animation, structures, etc)
		render(); 								// call render function.
		ui();									// call function to render ui.

		#if defined(__APPLE__)
			glCheckError();				// Manually checking for errors for MacOS, Windows has a callback.
		#endif

		glfwSwapBuffers(window); 		// swap buffers (avoid flickering and tearing)

		running &= (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE); // exit if escape key pressed
		running &= (glfwWindowShouldClose(window) != GL_TRUE);
	} while (running);

	endProgram(); // Close and clean everything up...

	// cout << "\nPress any key to continue...\n";
	// cin.ignore(); cin.get(); // delay closing console to read debugging errors.

	return 0;
}


void hintsGLFW(){
	
	auto majorVersion = 3; auto minorVersion = 3; // define OpenGL version - at least 3.3 for bare basic NEW OpenGL

	#if defined(__WIN64__)	
		majorVersion = 4; minorVersion = 5;					// Recommended for Windows 4.5, but latest is 4.6 (not all drivers support 4.6 version).
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // Create context in debug mode - for debug message callback
	#elif(__APPLE__)
		majorVersion = 4; minorVersion = 1; 				// Max support for OpenGL in MacOS
	#endif

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 	// https://www.glfw.org/docs/3.3/window_guide.html
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, majorVersion); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minorVersion);
}

void endProgram()
{
	// Clean ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwMakeContextCurrent(NULL); 	// destroys window handler
	glfwTerminate();				// destroys all windows and releases resources.
}

void startup()
{
	// Output some debugging information
	cout << "VENDOR: " << (char *)glGetString(GL_VENDOR) << endl;
	cout << "VERSION: " << (char *)glGetString(GL_VERSION) << endl;
	cout << "RENDERER: " << (char *)glGetString(GL_RENDERER) << endl;	

	cout << endl << "Loading content..." << endl;	
	content.LoadGLTF("assets/dog.gltf");

	pipeline.CreatePipeline();
	pipeline.LoadShaders("shaders/vs_model.glsl", "shaders/fs_model.glsl");

	// Start from the centre
	modelPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	modelRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	// A few optimizations.
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Get the correct size in pixels - E.g. if retina display or monitor scaling
	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

	// Calculate proj_matrix for the first time.
	aspect = (float)windowWidth / (float)windowHeight;
	projMatrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
}

void update()
{
	if (keyStatus[GLFW_KEY_LEFT]) modelRotation.y += 0.05f;
	if (keyStatus[GLFW_KEY_RIGHT]) modelRotation.y -= 0.05f;
	if (keyStatus[GLFW_KEY_UP]) modelRotation.x += 0.05f;
	if (keyStatus[GLFW_KEY_DOWN]) modelRotation.x -= 0.05f;
	if (keyStatus[GLFW_KEY_W]) modelPosition.z += 0.10f;
	if (keyStatus[GLFW_KEY_S]) modelPosition.z -= 0.10f;

	if (keyStatus[GLFW_KEY_R]) pipeline.ReloadShaders();

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void render()
{
	glViewport(0, 0, windowWidth, windowHeight);

	// Clear colour buffer
	glm::vec4 grayLilac = glm::vec4(0.831f, 0.792f, 0.803f, 1.0f);
	glm::vec4 backgroundColor = grayLilac;
	glClearBufferfv(GL_COLOR, 0, &backgroundColor[0]);

	// Clear deep buffer
	static const GLfloat one = 1.0f;
	glClearBufferfv(GL_DEPTH, 0, &one);

	// Enable blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Use our shader programs
	glUseProgram(pipeline.pipe.program);

	// Setup camera
	glm::mat4 viewMatrix = glm::lookAt(cameraPosition,				 // eye
									   cameraPosition + cameraFront, // centre
									   cameraUp);					 // up

	// Do some translations, rotations and scaling
	// glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(modelPosition.x+rX, modelPosition.y+rY, modelPosition.z+rZ));
	glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(1.2f, 1.2f, 1.2f));

	glm::mat4 mv_matrix = viewMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(pipeline.pipe.program, "model_matrix"), 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(pipeline.pipe.program, "view_matrix"), 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(pipeline.pipe.program, "proj_matrix"), 1, GL_FALSE, &projMatrix[0][0]);

	glBindTexture(GL_TEXTURE_2D, content.texid);

	content.DrawModel(content.vaoAndEbos, content.model);
	
	#if defined(__APPLE__)
		glCheckError();
	#endif
}

void ui()
{
	ImGuiIO &io = ImGui::GetIO();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration; 
	window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	window_flags |= ImGuiWindowFlags_NoSavedSettings; 
	window_flags |= ImGuiWindowFlags_NoFocusOnAppearing; 
	window_flags |= ImGuiWindowFlags_NoNav;

	const auto PAD = 10.0f;
	const ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
	ImVec2 work_size = viewport->WorkSize;
	ImVec2 window_pos, window_pos_pivot;
	window_pos.x = work_pos.x + work_size.x - PAD;
	window_pos.y = work_pos.y + work_size.y - PAD;
	window_pos_pivot.x = 1.0f;
	window_pos_pivot.y = 1.0f;

	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	window_flags |= ImGuiWindowFlags_NoMove;

	ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
	bool *p_open = NULL;
	if (ImGui::Begin("Info", nullptr, window_flags)) {
		ImGui::Text("About: 3D Graphics and Animation 2023/24"); // ImGui::Separator();
		ImGui::Text("Performance: %.3fms/Frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Pipeline: %s", pipeline.pipe.error?"ERROR":"OK");
	}
	ImGui::End();

	// Rendering imgui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void onResizeCallback(GLFWwindow *window, int w, int h)
{
	windowWidth = w;
	windowHeight = h;

	// Get the correct size in pixels
	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

	if (windowWidth > 0 && windowHeight > 0)
	{ // Avoid issues when minimising window - it gives size of 0 which fails division.
		aspect = (float)w / (float)h;
		projMatrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
	}
}

void onKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
		keyStatus[key] = true;
	else if (action == GLFW_RELEASE)
		keyStatus[key] = false;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void onMouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
}

void onMouseMoveCallback(GLFWwindow *window, double x, double y)
{
	int mouseX = static_cast<int>(x);
	int mouseY = static_cast<int>(y);
}

void onMouseWheelCallback(GLFWwindow *window, double xoffset, double yoffset)
{
	int yoffsetInt = static_cast<int>(yoffset);
}

void APIENTRY openGLDebugCallback(GLenum source,
								  GLenum type,
								  GLuint id,
								  GLenum severity,
								  GLsizei length,
								  const GLchar *message,
								  const GLvoid *userParam)  // Debugger callback for Win64 - OpenGL versions 4.2 or better.
{
	debugger.OpenGLDebugCallback(source, type, id, severity, length, message, userParam);
}

void debugGL() // Debugger function for Win64 - OpenGL versions 4.2 or better.
{
	// Enable Opengl Debug
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback((GLDEBUGPROC)openGLDebugCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
}

GLenum glCheckError_(const char *file, int line) // Debugger manual function for Apple - OpenGL versions 3.3 to 4.1.
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) debugger.GlGetError(errorCode, file, line);

	return errorCode;
}
