#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <vector>

using std::cout;
using std::endl;
using std::string;

#if defined _WIN32 || defined _WIN64
#define fscanf fscanf_s
#endif

typedef std::array<float, 3> vec3f;
typedef std::array<float, 2> vec2f;

inline bool loadObj(string name, std::vector<std::array<float,8>>& vertex_out, std::vector<unsigned int>& indices_out){
	string path = "models/" + name + ".obj";
	FILE* file;
	#ifdef __APPLE__
	file = fopen(path.c_str(), "r");
	bool err = (file==NULL);
	#elif defined _WIN32 || defined _WIN64
	errno_t err = fopen(&file, path.c_str(), "r");
	#endif


	if (err) {
		cout << "Could not open file: " << path << endl;
		return false;
	}

	std::vector< unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<vec3f> tmp_verts;
	std::vector<vec3f> tmp_normals;
	std::vector<vec2f> tmp_uvs;


	while (1) {
		char lineHeader[128];
		#ifdef __APPLE__
		int res = fscanf(file, "%128s", lineHeader);
		#elif defined _WIN64 || defined _WIN32
		int res = fscanf(file, "%128s", lineHeader, (unsigned int)sizeof(lineHeader));
		#endif

		if (res == EOF)
			break;
		if (strcmp(lineHeader, "v")==0) {
			vec3f vertex;
			#ifdef __APPLE__
			fscanf(file, "%f %f %f\n", &vertex[0], &vertex[1], &vertex[2]);
			#elif defined _WIN32 || _WIN64
			fscanf(file, "%f %f %f\n", &vertex[0], &vertex[1], &vertex[2]);
			#endif
			tmp_verts.push_back(vertex);
		}
		else if (strcmp(lineHeader,"vn")==0){
			vec3f normal;
			fscanf(file, "%f %f %f\n", &normal[0], &normal[1], &normal[2]);
			tmp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader,"vt")==0) {
			vec2f uv;
			fscanf(file, "%f %f\n", &uv[0], &uv[1]);
			tmp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "f")==0) {

			string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%u/%u/%u %u/%u/%u %u/%u/%u\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				cout << "Model " + path + " could not be loaded. Are you sure it's triangulated?" <<matches<< " matches."<< endl;
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);

			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);

			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}

	std::vector<std::array<float, 8>> output;
	for (unsigned int i = 0; i < vertexIndices.size(); ++i) {
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int normalIndex = normalIndices[i];
		unsigned int uvIndex = uvIndices[i];
		vec3f vertex = tmp_verts[vertexIndex - 1];
		vec3f normal = tmp_normals[normalIndex - 1];
		vec2f uv = tmp_uvs[uvIndex - 1];

		output.push_back(std::array<float, 8>{ vertex[0],vertex[1],vertex[2], normal[0],normal[1],normal[2], uv[0], uv[1] });
		
	}
	indices_out = vertexIndices;
	vertex_out = output;

	vertexIndices.clear();
	output.clear();
	tmp_normals.clear();
	tmp_uvs.clear();
	tmp_verts.clear();
	vertexIndices.clear();
	uvIndices.clear();
	normalIndices.clear();

	fclose(file);

	cout << "Loaded model " << path << " using " << vertex_out.size() << " vertices." << endl;
	return true;
}
