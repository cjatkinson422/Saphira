#include "Material.h"
#include "Globals.h"

Material::Material(string shaderType, string name) {
	materialName = name;
	matShader = new Shader();
	matShader->loadShader(shaderType);

	if (name != "NULL") {
		addTexture(texHandler->textureMap[name]);
		setTexUniforms();
	}
}

void Material::addObj(SceneObject * argB) {
	objVec.push_back(argB);
	++numObjs;
}

void Material::addObj(Skybox * argB) {
	objVec.push_back(argB);
	++numObjs;
}

void Material::removeObj(SceneObject * argB) {
	auto it = std::find(objVec.begin(), objVec.end(), argB);
	if (it != objVec.end()) {
		objVec.erase(it);
	}
}

void Material::draw(bool cast_to_skybox) {
	matShader->use();

	if (numTextures > 0) {
		if (textures[0]->diffID != (GLuint)NULL) {
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, textures[0]->diffID);
		}
		if (textures[0]->specID != (GLuint)NULL) {
			glActiveTexture(GL_TEXTURE0 + 4);
			glBindTexture(GL_TEXTURE_2D, textures[0]->specID);
		}
		if (textures[0]->normID != (GLuint)NULL) {
			glActiveTexture(GL_TEXTURE0 + 6);
			glBindTexture(GL_TEXTURE_2D, textures[0]->normID);
		}
	}
	setTexUniforms();
	for (int i = 0; i < numObjs; ++i) {
		if (!cast_to_skybox) {
			objVec[i]->draw(matShader);
		}
		else {
			(dynamic_cast<Skybox*>(objVec[i]))->draw(matShader);

		}
	}
}

void Material::addTexture(int index) {
	if (this->numTextures < 16) {
		this->textures[numTextures] = texHandler->getTexturePtr(index);
		numTextures++;
	}
	else {
		cout << "Attempted to bind too many textures to material: " << this->materialName << ". Skipping." << endl;
		return;
	}

}

void Material::setTexUniforms() {
	if (numTextures == 0) { return; }
	for (int i = 0; i < numTextures; ++i) {
		string diffuseName = "tex_" + std::to_string(i) + "_diffuse";
		matShader->setUniform1i(diffuseName, 2);
		if (textures[0]->specID != (GLuint)NULL) {
			string specName = "tex_" + std::to_string(i) + "_spec";
			matShader->setUniform1i(specName, 4);
		}
		if (textures[0]->normID != (GLuint)NULL) {
			string normName = "tex_" + std::to_string(i) + "_norm";
			matShader->setUniform1i(normName, 6);
		}
	}

}
