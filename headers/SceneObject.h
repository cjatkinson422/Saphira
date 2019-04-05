#pragma once
#include <string>
#include "glad/glad.h"
#include "ObjLoad.h"
#include "Transforms.h"
#include "Shader.h"


class SceneObject {
protected:
	vec3 position = { 0.0,0.0,0.0 };
	vec3 scale = { 1.0,1.0,1.0 };

	mat4 attitude = eye4();

	mat4 model = eye4();

	unsigned int indexDataSize;
	unsigned int vertexDataSize;

public:
	unsigned int VAO, VBO, EBO;
	//Constructor and graphics stuff
	SceneObject(string filename) : SceneObject(filename, vec3{ 0.0,0.0,0.0 }) {
	}
	SceneObject(string filename, vec3 pos) {

		std::vector<std::array<float, 8>> vertexData;
		std::vector<unsigned int> indexData;
		loadObj(filename, vertexData, indexData);

		this->vertexDataSize = (unsigned int)sizeof(float) * 8 * (unsigned int)vertexData.size();
		this->indexDataSize = (unsigned int)sizeof(unsigned int)*(unsigned int)indexData.size();

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, this->vertexDataSize, vertexData.data(), GL_STATIC_DRAW);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indexDataSize, indexData.data(), GL_STATIC_DRAW);

		// position data
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(std::array<float, 8>), (void*)0);
		glEnableVertexAttribArray(0);

		//normal data
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(std::array<float, 8>), (void*)(3 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(1);
		// texture data
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(std::array<float, 8>), (void*)(6 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
		vertexData.clear();
		indexData.clear();
		this->setPos(pos);

	}
	~SceneObject() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}


	virtual void draw(Shader* shader) {
		shader->use();
		glBindVertexArray(VAO);
		shader->setUniform4dv("model", &model[0][0]);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_TRIANGLES, 0, (this->vertexDataSize)/(8*sizeof(float)));
		//glDrawElements(GL_TRIANGLES, int(this->indexDataSize / sizeof(unsigned int)), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	//Physical stuff
	void setPos(vec3 pos) {
		this->position = pos;
		this->updateModelMat();
	}
	void addPos(vec3 pos) {
		this->position += pos;
		this->updateModelMat();
	}
	void updateModelMat() {
		this->model = TransMat(position) * this->attitude * ScaleMat(scale);
	}
	mat4 getModelMat(){
		return this->model;
	}
	void rotate(double angle, vec3 axis) {
		this->attitude = RotMat(angle,axis) * this->attitude;
		this->updateModelMat();
	}
	void setAttitude(mat4 att){
		this->attitude = att;
		this->updateModelMat();
	}
	void setScale(vec3 s) {
		this->scale = s;
		this->updateModelMat();
	}
	vec3 getScale() {
		return this->scale;
	}
	void setScale(float s) {
		this->scale = vec3{ s,s,s };
		this->updateModelMat();
	}
	vec3 getPos() {
		return this->position;
	}
	double* getPosPtr() {
		return &this->position[0];
	}

};


class Skybox : public SceneObject {
public:
	Skybox() : Skybox("uvsphere", vec3{ 0.0,0.0,0.0 }) {
	}
	Skybox(string filename, vec3 pos):SceneObject(filename, pos){

	}
	~Skybox() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

	void draw(Shader* shader) {
		shader->use();
		glBindVertexArray(VAO);
		shader->setUniform4dv("model", &model[0][0]);
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, (this->vertexDataSize)/(8*sizeof(float)));
		//glDrawElements(GL_TRIANGLES, int(this->indexDataSize / sizeof(unsigned int)), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glEnable(GL_DEPTH_TEST);
		
		//glDepthMask(GL_TRUE);
	}
};