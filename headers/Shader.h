#pragma once
#include <iostream>
#include <fstream>
#include <array>

#include "Transforms.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::string;
using std::to_string;



//---------------------DECLARATIONS-------------------------
class Shader {
private:
	unsigned int ID;
	string sname;

public:
	//default constructor. currently does nothing
	Shader() {
	}
	//simply returns the ID of the shader program bound to this shader object
	unsigned int getProgramID() {
		return this->ID;
	}
	//tells OpenGL to use the shader bound to this shader object.
	void use() {
		glUseProgram(ID);
	}
	//Loads named source files for a shader and generate the shader program, binding its ID to this shader objects shaderProgram variable.
	void loadShader(string name) {
		sname = name;
		//Read the vertex shader source code from disk
		string vertSourceString;
		ifstream vertReader("shaders/" + name + ".vert");
		char letter;

		if (!vertReader) {
			cout << "Error: could not open "<< name <<" vertex shader file!" << endl;
		}
		else {
			while (!vertReader.eof()) {
				vertReader.get(letter);
				vertSourceString = vertSourceString + letter;
			}
		}
		//closes the file and stores data for use
		vertReader.close();
		const char* vertSource = vertSourceString.c_str();

		//Read the fragment shader source code from disk
		string fragSourceString;
		ifstream fragReader("shaders/" + name + ".frag");
		if (!fragReader) {
			cout << "Error: could not open "<< name <<" fragment shader file!" << endl;
		}
		else {
			while (!fragReader.eof()) {
				fragReader.get(letter);
				fragSourceString = fragSourceString + letter;
			}
		}
		//closes the file and stores data for use
		fragReader.close();
		const char *fragSource = fragSourceString.c_str();


		//Create the shader ids
		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		//bind the source code and compile the shaders
		glShaderSource(vertexShader, 1, &vertSource, NULL);
		glCompileShader(vertexShader);
		glShaderSource(fragmentShader, 1, &fragSource, NULL);
		glCompileShader(fragmentShader);

		//check if vertex shader successfully compiled
		int successfulComp;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successfulComp);
		if (!successfulComp) {
			GLchar shaderCompilationErrorLog[512];
			glGetShaderInfoLog(vertexShader, 512, NULL, shaderCompilationErrorLog);
			cout << "ERROR: "<< name <<" vertex shader failed to compile!" << shaderCompilationErrorLog << std::endl;
		}
		//check if fragment shader successfully compiled
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &successfulComp);
		if (!successfulComp) {
			char fragmentShaderCompilationErrorLog[512];
			glGetShaderInfoLog(fragmentShader, 512, NULL, fragmentShaderCompilationErrorLog);
			cout << "ERROR: " << name << " fragment shader failed to compile!\n" << fragmentShaderCompilationErrorLog << std::endl;
		}


		//create the shader program
		ID = glCreateProgram();

		glAttachShader(ID, vertexShader);
		glAttachShader(ID, fragmentShader);
		glLinkProgram(ID);
		// check if the shader creation was successful
		glGetProgramiv(ID, GL_LINK_STATUS, &successfulComp);
		if (!successfulComp) {
			char shaderProgramLog[512];
			glGetProgramInfoLog(ID, 512, NULL, shaderProgramLog);
			cout << "Shader program creation failed! \n" << endl;
		}
		else {
			cout << "Successfully compiled and linked " << name << " shader." << endl;
		}
		//cleanup
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		return;

	}


	//matrix sets
	void setUniform4dv(const char* uniformName, double* matLoc) {
		this->use();
		unsigned int location = glGetUniformLocation(ID, uniformName);
		glUniformMatrix4dv(location, 1, GL_TRUE, matLoc);
	}

	//vec 3 uniform sets
	void setUniformV3(const char* uniformName, float* vec) {
		this->use();
		unsigned int location = glGetUniformLocation(ID, uniformName);
		glUniform3fv(location, 1, vec);
	}
	void setUniformV3(const char* uniformName, double* vec) {
		this->use();
		unsigned int location = glGetUniformLocation(ID, uniformName);
		glUniform3dv(location, 1, vec);
	}
	void setUniformV3(const char* uniformName, vec3f vec) {
		this->use();
		glUniform3f(glGetUniformLocation(ID, uniformName), vec[0], vec[1], vec[2]);
	}
	void setUniformV3(const char* uniformName, vec3 vec) {
		this->use();
		glUniform3d(glGetUniformLocation(ID, uniformName), vec[0], vec[1], vec[2]);
	}
	void setUniformV2(const char* uniformName, vec2f vec) {
		this->use();
		glUniform2f(glGetUniformLocation(ID, uniformName), vec[0], vec[1]);
	}
	void setUniformV2(const char* uniformName, vec2 vec) {
		this->use();
		glUniform2d(glGetUniformLocation(ID, uniformName), vec[0], vec[1]);
	}
	//single value sets
	void setUniform1f(const char* uniformName, float val) {
		this->use();
		glUniform1f(glGetUniformLocation(ID, uniformName), val);
	}
	void setUniform1i(string &uniformName, int arg) {
		this->use();
		glUniform1i(glGetUniformLocation(ID, uniformName.c_str()), arg);
	}
	void setUniform1i(const char* uniformName, int arg) {
		this->use();
		glUniform1i(glGetUniformLocation(ID, uniformName), arg);
	}
};
