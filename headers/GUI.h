#pragma once

#ifdef __APPLE__

#elif defined _WIN32 || defined _WIN64
#pragma comment(lib, "freetype.lib")
#endif
#define GL_SILENCE_DEPRECATION 
#include "ft2build.h"
#include <time.h>
#include <functional>
#include <array>
#include <unordered_map>
#include <iostream>
#include "cspice/SpiceUsr.h"
#include "Planet.h"
#include "Globals.h"
#include "Spacecraft.h"
#include "Camera.h"
#include "stb_image.h"
#include "Mission.h"


#include FT_FREETYPE_H  

#define GUI_MAIN 1
#define GUI_PLANETARY_INFORMATION 2
class Camera;
class GUI;
class SelectorButton;
using std::string;

template <class internT, class externT, class stateT>
struct codecvt : std::codecvt<internT,externT,stateT>
{ ~codecvt(){} };

typedef std::function<void ()> callbackPtr;

struct Character {
	GLuint     TextureID;  // ID handle of the glyph texture
	std::array<int, 2> Size;       // Size of glyph
	std::array<int, 2> Bearing;    // Offset from baseline to left/top of glyph
	GLuint     Advance;    // Offset to advance to next glyph
};

struct GUIText {
	bool draw = true;
	std::u32string textToDraw;
	vec2 offset;
	int location;
	int group = GUI_MAIN;
	int centering = GUI_TEXT_ANCHOR_LEFT;
	float scale;
	float depth;
	vec3 color;
	GUIText():textToDraw(),offset(),location(),group(),scale(),depth(),color() {};
	GUIText(std::u32string arg1, vec2 arg2, int arg3, int centering = GUI_TEXT_ANCHOR_LEFT, float scale = 0.5f, float depth = 0.5f, vec3 color = vec3{ 0.85,0.85,0.85 }) :textToDraw(arg1), offset(arg2), location(arg3), scale(scale), depth(depth), color(color), centering(centering) {}
};

// ---------------------------------------------

class Selector{
private:
	GUI* parentGUI;

	int nrButtons = 0;

	float spacing = 15.0f;
	float depth = 0.5f;

	vec2 offset;
	int location;
	float scale;
	std::u32string title;
	std::u32string activeButton = U"NULL";

	GLuint texIdOn, texIdOff;
	unsigned int VAO, VBO, EBO;

	void genGLArrays();
	vec2f getButtonStartCoords();
public:
	std::map<std::u32string, callbackPtr> buttons;


	Selector(GUI* parentGUI, std::u32string title, vec2 offset, int location, GLuint texIdOn, GLuint texIdOff, int winWidth, int winHeight, float scale = 1.0);

	bool checkClickButton(vec2 bb_ll, vec2 bb_ur, vec2 offset, double xpos, double ypos, int winWidth, int winHeight);
	bool checkClick(double xpos, double ypos, int winWidth, int winHeight);

	void addButton(std::u32string name, callbackPtr callbackFunc);
	
	void setDepth(float val){
		this->depth = val;
	}

	std::u32string getActiveElem();
	void setActiveElem(std::u32string);

	void draw();

};
class GUIButton {
private:
	//OpenGL variables
	unsigned int VAO, VBO, EBO;

	//Button variables
	vec2 bb_ll; // bounding box lower left, defined as an offset from the offset vector
	vec2 bb_ur; // bounding box upper right, defined as an offset from the offset vector
	vec2 offset; // the location of the center of the bounding box for the button, defined as an offset from the location
	int location; // which corner of the screen to define the offset from, ex. GUI_POS_LOWER_LEFT, or GUI_POS_MID_MID
	float scale; //scaling factor for this button (also scales with overall GUI scale)
	GLuint texId; //The OpenGL id number for the loaded texture for this button's texture
	
	float depth = 0.5f;

	callbackPtr clickCallback;
public:
	callbackPtr hoverCallback = [](){void();};
	callbackPtr hoverOffCallback = [](){void();};
	bool enabled = true;

	GUIButton(callbackPtr clickCallback, vec2 bb_ll, vec2 bb_ur, vec2 offset, int location, GLuint texId, int winWidth, int winHeight, float scale = 1.0) : clickCallback{clickCallback}, bb_ll{ bb_ll }, bb_ur{ bb_ur }, offset{ offset }, location{ location }, texId{ texId }, scale{ scale } {
		this->updateVertices(winWidth, winHeight);
	}
	~GUIButton();

	void updateOffset(vec2 newOffset, int winWidth, int winHeight);
	void updateVertices(int winWidth, int winHeight);
	void onClick();
	void onHover();
	void hoverOff();
	void draw();

	bool checkClick(double xpos, double ypos, int winWidth, int winHeight);
};

class TextInput {
private:
	//OpenGL variables
	unsigned int VAO, VBO, EBO;

	//Button variables
	vec2 bb_ll; // bounding box lower left, defined as an offset from the offset vector
	vec2 bb_ur; // bounding box upper right, defined as an offset from the offset vector
	vec2 offset; // the location of the center of the bounding box for the button, defined as an offset from the location
	int location; // which corner of the screen to define the offset from, ex. GUI_POS_LOWER_LEFT, or GUI_POS_MID_MID
	float scale; //scaling factor for this button (also scales with overall GUI scale)
	GUI* parentGUI;
	
	float depth = 0.58;

public:
	string textToDraw;
	
	TextInput(GUI* parentGUI, vec2 bb_ll, vec2 bb_ur, vec2 offset, int location, int winWidth, int winHeight, float scale = 1.0) : parentGUI{ parentGUI }, bb_ll{ bb_ll }, bb_ur{ bb_ur }, offset{ offset }, location{ location }, scale{ scale } {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		this->updateVertices(winWidth, winHeight);
	}
	~TextInput();
	void updateVertices(int winWidth, int winHeight);
	void onClick();
	void draw();
	void changeText(string text);
	void addChar(char toAdd);
	void setDepth(float val);
	void removeChar();
	bool checkClick(double xpos, double ypos, int winWidth, int winHeight);
};

class StaticBox {
private:
	//OpenGL variables
	unsigned int VAO, VBO, EBO;

	//Button variables
	vec2 bb_ll; // bounding box lower left, defined as an offset from the offset vector
	vec2 bb_ur; // bounding box upper right, defined as an offset from the offset vector
	vec2 offset; // the location of the center of the bounding box for the button, defined as an offset from the location
	int location; // which corner of the screen to define the offset from, ex. GUI_POS_LOWER_LEFT, or GUI_POS_MID_MID
	float scale; //scaling factor for this button (also scales with overall GUI scale)
	GUI* parentGUI;

	float depth = 0.59;
	
public:
	vec3 color = {0.12, 0.12, 0.15};
	StaticBox(GUI* parentGUI, vec2 bb_ll, vec2 bb_ur, vec2 offset, int location, int winWidth, int winHeight, float scale);
	void updateVertices(int winWidth, int winHeight);
	void setColor(vec3 col){
		this->color = col;
	}
	void setDepth(GLfloat depth);
	void draw();
};
// ---------------------------------------------

class GUI {
private:

	//Shader stuff
	std::map<GLuint, Character> Characters;
	GLuint textVAO, textVBO;


	PlanetaryBody* curBody = NULL;
	Spacecraft* curCraft = NULL;

	//maps
	std::map<string, GUIText> textMap;
	std::map<string, GUIButton*> buttonMap;
	std::map <string, int> textureMap;
	std::map<string, TextInput*> textInputs;
	std::map<string, StaticBox*> statBoxes;
	std::map<string, Selector*> selectorMap;
	
	//textures
	Texture textures[maxTextures];
	int numTextures = 0;
	GLuint loadTexture(string key, string filename);
	Texture* getTexturePtr(int index);

	typedef std::function<void ()> callbackPtr;


public:
	//window and scale vars
	int winWidth, winHeight;
	float guiScale = 1.0;

	double mousex, mousey;
	TextInput* activeInput;
	//Shaders
	Shader* textShader = NULL;
	Shader* guiShader = NULL;
	Shader* txtInputShader = NULL;
	Shader* selShader = NULL;
	Camera* parentCam = NULL;

	GUI(Camera* cam);

	//Initialization and shaders
	void loadCharacters(FT_Face face);
	void loadSymbols(FT_Face face);
	void initializeComponents();
	void RenderText(std::u32string text, GLfloat x, GLfloat y, GLfloat scale, vec3 color, float depth);
	void RenderText(string text, GLfloat x, GLfloat y, GLfloat scale, vec3 color, float depth);
	void setUniform4dv(const char * uniformName, double * uniformPtr);

	//Window stuff
	void resize(int width, int height);
	
	//state sets
	void setCurParent(PlanetaryBody* body);
	void setCurParent(Spacecraft* body);

	//
	void updateReadouts();

	// Text functions
	void createText(string key, GUIText textStruc);
	void updateText(string key, string text);
	void deleteText(string key);
	void text_callback(GLFWwindow* window, unsigned int codepoint);
	void createInput(string key, vec2 bb_ll, vec2 bb_ru, vec2 offset, int location, float scale = 1.0);
	void deleteInput(string key);
	GLfloat getStringWidth(std::u32string toDraw, float scale);
	GLfloat getStringHeight(std::u32string toDraw, float scale);
	GLfloat getStringHeight(string toDraw, float scale);


	// Button functions
	void createButton(std::function<void ()> callbackFunc, string key, string texPath, vec2 bb_ll, vec2 bb_ru, vec2 offset, int location, float scale = 1.0);
	void updateButton(string key);
	void deleteButton(string key);
	void createStaticBox(string key, vec2 bb_ll, vec2 bb_ur, vec2 offset, int location, float scale = 1.0);
	void deleteStaticBox(string key);

	void click();
	void hover();

	//misc
	PlanetaryBody* getCurBody(){
		return this->curBody;
	}

	void draw();
};

