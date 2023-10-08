// 3D Graphics and Animation - Pipeline Template
// Create a program and attach shaders.

#pragma once


#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
//#include <glm/glm.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;
//#include <vector>


 struct Pipe{
	public:
		GLuint program;										// Pipeline reference
		string vsName = ""s;								// name of file for vertex shader
		string fsName = ""s;								// name of file for fragment shader
		bool error = false;									// Flag any errors.
	private:

 };

class Pipeline{
	public:
		Pipeline();
	
		Pipe pipe;											// Save all information.

		void CreatePipeline();								// Create OpenGL Pipeline Program
		void LoadShaders(string vs, string fs);				// Load shaders using files		
		void ReloadShaders();								// Reload Shaders for debugging...			

		virtual ~Pipeline();
	protected:

	private:
		string ReadShader(string name);
		void CheckErrorShader(GLuint shader, string name);
		void CheckErrorLinking();
};