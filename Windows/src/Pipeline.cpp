

#include "Pipeline.hpp"


Pipeline::Pipeline() {

}

void Pipeline::CreatePipeline(){

	cout << endl << "Creating new pipeline..." << endl;

	pipe.program = glCreateProgram();
}

void Pipeline::LoadShaders(string vsString, string fsString){
	pipe.vsName = vsString; pipe.fsName = fsString; 		// save filenames for reloading of shaders

	string vsText = ReadShader(vsString);
	auto vs_source = vsText.c_str();
	auto vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_source, NULL);
	glCompileShader(vs);
	CheckErrorShader(vs, vsString);
	glAttachShader(pipe.program, vs);

	string fsText = ReadShader(fsString);
	auto fs_source = fsText.c_str();
	auto fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_source, NULL);
	glCompileShader(fs);
	CheckErrorShader(fs, fsString);
	glAttachShader(pipe.program, fs);

	glLinkProgram(pipe.program);
	CheckErrorLinking();

	glUseProgram(pipe.program);

	if(!pipe.error) cout << "OK - Pipeline compiled.\n"; 
	else cout << "ERROR - Pipeline did not compile, please check for errors!\n" ;
}


string Pipeline::ReadShader(string name){
	auto shaderText = ""s;
	std::ifstream shaderFile(name);

	string vs_line;
	if (shaderFile.is_open()){

		while (getline(shaderFile, vs_line)){
			shaderText += vs_line;
			shaderText += '\n';
		}
		shaderFile.close();

		cout << "OK - Read shader file: " << name << "\n";
	}
	else{
		cout << "ERROR - Unable to open shader file: " << name << "\n";
		pipe.error = true;
	}
	return shaderText;
}

void Pipeline::CheckErrorShader(GLuint shader, string name){
	// Get log lenght
	GLint maxLength = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

	// Init a string for it
	vector<GLchar> errorLog(maxLength);

	if (maxLength > 1){
		// Get the log file
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		cout << "ERROR - Shader compilation error: " << errorLog.data() << "\n";

		pipe.error = true;
	}
	else{
		cout << "OK - Compiled Shader: " << name << "\n"; 
	}
}

void Pipeline::CheckErrorLinking(){
	GLint isLinked = 0;
	glGetProgramiv(pipe.program, GL_LINK_STATUS, &isLinked);
	
	if (isLinked == GL_FALSE) {
		GLint maxLength = 0;
		glGetProgramiv(pipe.program, GL_INFO_LOG_LENGTH, &maxLength);

		// Init a string for it
		vector<GLchar> errorLog(maxLength);

		// The maxLength includes the NULL character
		if (maxLength > 1){

			glGetProgramInfoLog(pipe.program, maxLength, &maxLength, &errorLog[0]);

			cout << "ERROR - Shaders linking error: " << errorLog.data() << "\n";

			pipe.error = true;
		}
	}

	return;

}

void Pipeline::ReloadShaders(){
	cout << "Hot-Reloading pipeline...\n";
	glDeleteProgram(pipe.program);
	pipe.error = false;
	pipe.program = glCreateProgram();
	LoadShaders(pipe.vsName, pipe.fsName);
}


Pipeline::~Pipeline(){
	
}


