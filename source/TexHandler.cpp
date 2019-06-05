#include "TexHandler.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

TexHandler::TexHandler() {
	int texUnits, fragUnits;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &texUnits);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &fragUnits);
	cout << "Maximum fragment textures: " << fragUnits << endl;
	cout << "Maximum total textures: " << texUnits << endl << endl;
}

//takes a texture dictionary index and returns the GL assigned ID

Texture* TexHandler::getTexturePtr(int index) {
	if (textures[index].channels) {
		return &textures[index];
	}
	else {
		cout << "Could not find texture indexed as " << index << ". Returning index 0!" << endl;
		return &textures[0];
	}
}

//loads texture data to the GPU and stores its information in the texture dictionary

void TexHandler::loadTexture(string filename, bool spec, bool norm) {
	//texture slot 0 reserved for loading
	glActiveTexture(GL_TEXTURE0);
	//Not super sure. I think OpenGL flips textures or something.
	stbi_set_flip_vertically_on_load(true);
	//Limit the number of textures loaded to maxTextures
	if (this->numTextures >= maxTextures) {
		cout << "ERROR: Attempted to load too many textures. " << endl;
		return;
	}
	//add texture directory
	this->textureMap[filename] = this->numTextures;
	filename = "textures/" + filename;
	this->textures[this->numTextures].path = filename;

	string diffPath = (filename + "_diffuse.png");


	//Create the texture object within OpenGL and add it to the dictionary
	//generates the texture and tells opengl to store the texture ID in the next available slot in the texture dictionary
	glGenTextures(1, &(this->textures[this->numTextures].diffID));
	glBindTexture(GL_TEXTURE_2D, this->textures[this->numTextures].diffID);

	// Sets the wrapping mode to standard repeat
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	// Sets the interpolation to nearest, with linear mipmap interpolation
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//use stb_image library to load the texture sent in by filename

	unsigned char* data = stbi_load(diffPath.c_str(), &(textures[numTextures].width), &(textures[numTextures].height), &(textures[numTextures].channels), 0);

	if (data) {

		if (textures[numTextures].channels == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textures[numTextures].width, textures[numTextures].height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else if (textures[numTextures].channels == 4) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textures[numTextures].width, textures[numTextures].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else {
			cout << "ERROR: " + diffPath + " has an unknown file type. " << endl;
			return;
		}

		glGenerateMipmap(GL_TEXTURE_2D);
		cout << "Loaded texture file " + diffPath << " with " << textures[numTextures].channels << " channels." << endl;
	}
	else {
		cout << "ERROR: Failed to load " + diffPath << endl;
	}



	stbi_image_free(data);

	if (spec) {
		string specPath = (filename + "_specular.png");

		glGenTextures(1, &(this->textures[this->numTextures].specID));
		glBindTexture(GL_TEXTURE_2D, this->textures[this->numTextures].specID);

		// Sets the wrapping mode to standard repeat
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Sets the interpolation to nearest, with linear mipmap interpolation
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//use stb_image library to load the texture sent in by filename

		int width, height, channels;

		unsigned char* data = stbi_load(specPath.c_str(), &width, &height, &channels, 0);

		if (data) {

			if (channels == 3) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			}
			else if (channels == 4) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			}
			else {
				cout << "ERROR: " + specPath + " has an unknown file type. " << endl;
				return;
			}

			glGenerateMipmap(GL_TEXTURE_2D);
			cout << "Loaded texture file " + specPath << " with " << channels << " channels." << endl;
		}
		else {
			cout << "ERROR: Failed to load " + specPath;
		}



		stbi_image_free(data);
	}
	if (norm) {
		string normPath = (filename + "_normal.png");


		glGenTextures(1, &(this->textures[this->numTextures].normID));
		glBindTexture(GL_TEXTURE_2D, this->textures[this->numTextures].normID);

		// Sets the wrapping mode to standard repeat
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Sets the interpolation to nearest, with linear mipmap interpolation
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//use stb_image library to load the texture sent in by filename

		int width, height, channels;

		unsigned char* data = stbi_load(normPath.c_str(), &width, &height, &channels, 0);

		if (data) {

			if (channels == 3) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			}
			else if (channels == 4) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			}
			else {
				cout << "ERROR: " + normPath + " has an unknown file type. " << endl;
				return;
			}

			glGenerateMipmap(GL_TEXTURE_2D);
			cout << "Loaded texture file " + normPath << " with " << channels << " channels." << endl;
		}
		else {
			cout << "ERROR: Failed to load " + normPath;
		}

		stbi_image_free(data);
	}


	this->numTextures += 1;
	return;
}

void TexHandler::deleteTexture(int index) {
	glDeleteTextures(1, &(textures[index].diffID));
	glDeleteTextures(1, &(textures[index].specID));
	glDeleteTextures(1, &(textures[index].normID));
	textures[index].channels = 0;
	textures[index].width = 0;
	textures[index].height = 0;
	textures[index].diffID = 0;
	textures[index].specID = 0;
	textures[index].normID = 0;
	textures[index].path = "";
}

int TexHandler::getNumTx() {
	return this->numTextures;
}
