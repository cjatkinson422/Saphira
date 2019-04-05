#pragma once
#include <vector>
#include "TexHandler.h"
#include "Shader.h"
#include "SceneObject.h"


using std::vector;

class Material {
private:	
	Texture* textures[16];

public:
	short numTextures = 0;
	int numObjs = 0;
	string materialName;
	vector<SceneObject*> objVec;

	float shininess = 64;
	float specStr = 1.0;
	Shader* matShader = NULL;

	Material(string shaderType, string name);

	void addObj(SceneObject* argB);
	void addObj(Skybox* argB);
	void removeObj(SceneObject* argB);
	
	void draw(bool cast_to_skybox = false);

	void addTexture(int index);

	void setTexUniforms();
};