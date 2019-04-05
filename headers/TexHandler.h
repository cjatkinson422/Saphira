#pragma once
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <map>



#define maxTextures 256

using std::cout;
using std::endl;
using std::string;
using std::to_string;

struct Texture
{
	GLuint diffID = (GLuint)NULL;
	GLuint specID = (GLuint)NULL;
	GLuint normID = (GLuint)NULL;
	string path;
	int width;
	int height;
	int channels;
};

class TexHandler {
private:
	int numTextures = 0;
	Texture textures[maxTextures];
public:
	std::map <string, int> textureMap;

	TexHandler();

	//takes a texture dictionary index and returns the GL assigned ID
	Texture* getTexturePtr(int index);

	//loads texture data to the GPU and stores its information in the texture dictionary
	void loadTexture(string filename, bool spec = false, bool norm = false);

	void deleteTexture(int index);

	int getNumTx();
};

