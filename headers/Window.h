#pragma once

#pragma comment(lib, "libglfw3.a")

using std::cout;
using std::endl;

#ifdef _WIN32 
#define platform "Windows"
#include <direct.h>
#define GetDir _getcwd
#endif
#ifdef __APPLE__
#define platform "Mac"
#include <unistd.h>
#define GetDir getcwd
#endif

inline void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

class Window {
private:
	//Variable declarations
	int win_width = 1600;
	int win_height = 800;
	GLFWwindow* window;

	//Method Declarations
	void initializeWin() {
		char buf[512];
		char* ans = GetDir(buf, 512);
		string path;
		if(ans!=NULL){
			path = ans;
		}


		if( !glfwInit() )
		{
    		fprintf( stderr, "Failed to initialize GLFW\n" );
		}
		glfwSetErrorCallback(error_callback);
		#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		#endif
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

		// Create GLFW context
		this->window = glfwCreateWindow(win_width, win_height, "Saphira", NULL, NULL);
		glfwSetWindowPos(this->window, 50, 50);
		// Check for problems with context initialization
		if (this->window == NULL) {
			cout << "Failed to create GLFW context" << endl;
			glfwTerminate();
			exit(-1);
		}
		// Make the window the current context
		glfwMakeContextCurrent(this->window);

		// Initialize GLAD
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			cout << "Failed to initialize GLAD" << endl;
			exit(-1);
		}

		// Set OpenGL viewport size
		glViewport(0, 0, win_width, win_height);
		// Set the window resize callback function in glfw

		// Get info about the gl context
		GLfloat lineWidth[2];
		glGetFloatv(GL_LINE_WIDTH_RANGE, lineWidth);
		printf("Implementation line width: min=%f, max=%f\n", (double)lineWidth[0], (double)lineWidth[1]);

		/*
		cout << "sizeof(float) = " << sizeof(float) << endl;
		cout << "sizeof(double) = " << sizeof(double) << endl;
		cout << "sizeof(GLfloat) = " << sizeof(GLfloat) << endl;
		cout << "sizeof(GLdouble) = " << sizeof(GLdouble) << endl;
		cout << "sizeof(long double) = " << sizeof(long double) << endl;
		*/
		
		//set the ColorClear Clear
		glClearColor(0.0,0.0,0.0,1.0);


	}
public:
	Window() {
		this->initializeWin();
		glEnable(GL_DEPTH_TEST);
	}
	void clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	//Getter Methods
	GLFWwindow* getWindow() {
		return this->window;
	}
	int getWidth() {
		return this->win_width;
	}
	int getHeight() {
		return this->win_height;
	}

	//Setter Method
};
