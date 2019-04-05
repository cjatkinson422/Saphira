#pragma once

#define GL_SILENCE_DEPRECATION 
#include <array>
#include "Transforms.h"
#include "Planet.h"
#include "Window.h"
#include "Globals.h"
#include "GUI.h"
#include "stb_image.h"

class GUI;
class LensFlare;

class Camera {
private:
	mat4 viewMat = eye4();
	mat4 projectionMat;
	mat4 craftProjMat;
	mat4 orthoMat;

	vec3 position = { 0.0,0.0,0.0 };
	vec3 globalUp = { 0.0,0.0,1.0 };

	vec3 shpLf = { 0.0,1.0,0.0 };
	vec3 shpUp = { 0.0,0.0,1.0 };
	vec3 shpFw = { -1.0,0.0,0.0 };

	double fovy;
	double aspect;
	long double nearClip;
	long double farClip;

	bool mHeld = false;
	double inlat=radians(30.0), inlon=radians(240.0), xprev, yprev;
	double distance;

	LensFlare* flare;

	PlanetaryBody* curBody = NULL;
	Spacecraft* curCraft = NULL;
	SceneObject* curMesh = NULL;
	int planetIndex = 0;
	int moonIndex = 0;


	int winWidth;
	int winHeight;

	int isPlaying= 0;
	

public:
	GUI* gui;
	double simSpeed = 1.0;
	float moveSpeed = 5000.0f;
	void setSpeed(float speed);
	float mouseSens = 0.0005f;

	Camera(GLFWwindow* window, int width, int height, double julianTime);
	vec3 getPosition();
	mat4 getViewMat();
	void addPos(vec3 pos);
	void setPos(vec3 pos);


	void lookAt(vec3 target);
	void updateViewMat();
	void updateShaderUniforms();
	void drawFlare();

	void frameResize(int width, int height);
	double* getProjPtr();
	mat4 getProjMat();
	mat4 getOrthoMat();
	double* getCraftProjPtr();
	double* getOrthoPtr();
	double* getViewPtr();
	double* getPosPtr();

	PlanetaryBody* getCurBody(){
		return this->curBody;
	}
	
	double getSimSpeed();
	int getWinWidth(){
		return this->winWidth;
	}
	int getWinHeight(){
		return this->winHeight;
	}
	void setCurCraft(Spacecraft* craft);

	//Takes mouse movement and translates it to latitude and longitude changes.
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);

	//key press input
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

	//scroll callback
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	
	//mouse button callback
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

	// GUI CALLBACK FUNCTIONS -------------------------

	void pause();
	void play();
	void togglePause();
	void fastForward();
	void rewind();
	
	void selectBody(PlanetaryBody* body);
	void selectBody(Spacecraft* craft);

	// END GUI CALLBACK FUNCTIONS----------------------

	void resetCursor(GLFWwindow* window);

};

class LensFlare{
private:
	// the number of flare quads to draw along the line between the sun and the screen center
    int nrFlares = 1;
	unsigned int VAO, VBO, EBO;
	//array of the gl texture IDs for each flare quad to be drawn
    std::array<unsigned int, 8> texIds;
    Shader* lfShader;
    Camera* mainCam;

    // The quad raw data to be sent to the shader program for each flare elemenet
    GLfloat quadData[4][4] = {
        -0.5, -0.5, 0.0, 0.0, //1
         0.5, -0.5, 1.0, 0.0, //2
         0.5,  0.5, 1.0, 1.0, //3
		 -0.5,  0.5, 0.0, 1.0, //4

        //-0.5, -0.5, 0.0, 0.0, //1
        // 0.5,  0.5, 1.0, 1.0, //3  
    };
	unsigned int indexData[6] = {
		0,1,2,
		0,2,3
	};
public:
    LensFlare(Camera* cam);
    ~LensFlare(){
    }

	void updateOrtho(double* ortho);
    void LoadTextures();

	bool isOccluded();
	void draw();

};
