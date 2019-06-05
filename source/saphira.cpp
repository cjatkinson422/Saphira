#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <functional>

#pragma comment(lib, "Ws2_32.lib") 

#include "TexHandler.h"
#include "Shader.h"
#include "Window.h"
#include "Transforms.h"
#include "Camera.h"
#include "Material.h"
#include "Spacecraft.h"
#include "Planet.h"
#include "Globals.h"
#include "GUI.h"


#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#endif

// ----------------------------------------------------------------------------
// This makes relative paths work in C++ in Xcode by changing directory to the Resources folder inside the .app bundle


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void text_callback(GLFWwindow* window, unsigned int codepoint);

TexHandler* texHandler;
PlanetarySystem* solarSystem;
double timeJulian;

Window mainWindow = Window();
GLFWwindow* window = mainWindow.getWindow();


Camera* mainCamera;

int main() {

	// Changes the default directory search path to the app package root
	#ifdef __APPLE__    
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
    {
        // error!
    }
    CFRelease(resourcesURL);

    chdir(path);
    cout << "Current Path: " << path << endl;
	#endif

	//create the main texture handler. External global variable
	texHandler = new TexHandler();
	//create the solar system and then load the planets defined in function below
	solarSystem = new PlanetarySystem();
	solarSystem->loadPlanets();

	//set the start time to somewhere in the ephemeris table
	double startTime = 2459804.500000;
	//update the solar system to given start time
	solarSystem->curTime = startTime;
	solarSystem->updatePlanetPositions(startTime);
	solarSystem->genOrbitLines();

	//set the spacecraft initial data
	
	//create the craft to propagate
	mainCamera = new Camera(window, mainWindow.getWidth(), mainWindow.getHeight(), startTime);

	//propagate the craft through given time and error
	//after propagation, solar system is in wrong state, so reset to intial
	solarSystem->updatePlanetPositions(startTime);
	
	printf("Finished updating solar system!\n");

	//some graphics stuff
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPos(window, (float)mainWindow.getWidth() / 2.0, (float)mainWindow.getHeight() / 2.0);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCharCallback(window, text_callback);
	
	//Sending the projection matrix to the shaders
	solarSystem->setUniform4dv("projection", mainCamera->getProjPtr());

	//time handling variables
	double prevTimeSeconds = glfwGetTime();

	double dtimeSeconds;
	timeJulian = startTime;

	// Main Loop
	while (!glfwWindowShouldClose(window)){
		mainWindow.clear();

		//update time
		
		dtimeSeconds = glfwGetTime() - prevTimeSeconds;
		timeJulian += mainCamera->getSimSpeed()*secondsToJulianOffset(dtimeSeconds); // 10.0 is a simulation speed. can change to any arbitrary number. 1.0 is realtime
		prevTimeSeconds = glfwGetTime();
		//update the solar system to the new time
		solarSystem->updatePlanetPositions(timeJulian);
		time_t curTime = getUnixSFromJulian(timeJulian);
		char date[26];

		#ifdef __APPLE__
		struct tm *buf;
		buf = localtime(&curTime);
		strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", buf);
		#elif defined _WIN32 || defined _WIN64
		struct tm buf;
		localtime_s(&buf, &curTime);
		strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &buf);
		#endif
		
		string timeStr = string(date);
		mainCamera->gui->updateText("time", timeStr);
		mainCamera->gui->updateReadouts();
		//solarSystem->updateOrbitLines();

		mainCamera->updateShaderUniforms();
		
		//#### BEGIN DRAW #####
		//start with skybox since everything should be drawn over it.
		solarSystem->drawSkybox();
		
		solarSystem->draw();
		
		//end with GUI since it should be drawn over everything
		mainCamera->drawFlare();

		glClear(GL_DEPTH_BUFFER_BIT);

		if(mainCamera->getCurMission()!=NULL)
			mainCamera->getCurMission()->drawCraft();
		
		glClear(GL_DEPTH_BUFFER_BIT);
		mainCamera->gui->draw();
		//#### END DRAW ####
		
		//check events and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
		
	}

	// Exit the program
	glfwTerminate();
	return 0;
}

// Input Callbacks.  Cannot set member functions as callback in GLFW, so these are just mostly passthroughs
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	//glfwSetCursorPos(window, (float)width / 2.0, (float)height / 2.0);
	mainCamera->frameResize(width, height);
	solarSystem->setUniform4dv("projection", mainCamera->getProjPtr());
	for(auto const& [name, mis] : solarSystem->missions){
		mis->setUniform4dvCraft("projection", mainCamera->getCraftProjPtr());
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	mainCamera->mouse_callback(window, xpos, ypos);
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	mainCamera->mouse_button_callback(window, button, action, mods);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	mainCamera->key_callback(window, key, scancode, action, mods);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	mainCamera->scroll_callback(window, xoffset, yoffset);
}
void text_callback(GLFWwindow* window, unsigned int codepoint){
	mainCamera->gui->text_callback(window, codepoint);
}