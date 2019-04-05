#include "GUI.h"

//############################### SELECTOR ########################################
Selector::Selector(GUI* parentGUI, std::u32string title, vec2 offset, int location, GLuint texIdOn, GLuint texIdOff, int winWidth, int winHeight, float scale): parentGUI{parentGUI}, title{title}, offset{offset}, location{location}, texIdOn{texIdOn}, texIdOff{texIdOff}, scale{scale}{
	parentGUI->selShader->use();

	this->genGLArrays();

}

void Selector::genGLArrays(){

	float pos = 5.0*scale;
	
	GLfloat vertexData[4][4] = {
		-pos, -pos, 0.0, 0.0,
		pos, -pos, 1.0, 0.0,
		pos, pos, 1.0, 1.0,
		-pos, pos, 0.0, 1.0
	};
	unsigned int indexData[6] = {
		0,1,2,
		0,2,3
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);


	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(std::array<float,4>), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

bool Selector::checkClickButton(vec2 bb_ll, vec2 bb_ur, vec2 offset, double xpos, double ypos, int winWidth, int winHeight){
	float lx, rx, uy, ly;
	lx = bb_ll[0] * scale + offset[0];
	rx = bb_ur[0] * scale + offset[0];
	uy = bb_ur[1] * scale + offset[1];
	ly = bb_ll[1] * scale + offset[1];

	//adds to position based on location base
	switch (this->location)
	{
	case GUI_POS_BOTTOM_LEFT:
		break;
	case GUI_POS_BOTTOM_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		break;
	case GUI_POS_BOTTOM_RIGHT:
		lx += winWidth;
		rx += winWidth;
		break;
	case GUI_POS_MID_LEFT:
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_MID_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_MID_RIGHT:
		lx += winWidth;
		rx += winWidth;
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_UPPER_LEFT:
		ly += winHeight;
		uy += winHeight;
		break;
	case GUI_POS_UPPER_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		ly += winHeight;
		uy += winHeight;
		break;
	case GUI_POS_UPPER_RIGHT:
		lx += winWidth;
		rx += winWidth;
		ly += winHeight;
		uy += winHeight;
		break;
	}

	if (lx < xpos && xpos < rx && ly < ypos && ypos < uy) {
		return true;
	}
	else {
		return false;
	}
}

bool Selector::checkClick(double xpos, double ypos, int winWidth, int winHeight){
	bool val = false;
	int itr = 1;
	for(auto const& [name, callback]: this->buttons){
		if(checkClickButton(vec2{-5.0*scale, -5.0*scale}, vec2{5.0*scale, 5.0*scale}, vec2{offset[0], offset[1] - (itr)*spacing*scale}, xpos, ypos, winWidth, winHeight)){
			val = true;
			this->activeButton = name;
			callback();
			break;
		}
		itr++;
	}
	return val;
}

void Selector::addButton(std::u32string name, callbackPtr callbackFunc){
	this->buttons[name] = callbackFunc;
	this->parentGUI->selShader->setUniform1f("scale", scale);
	++nrButtons;
}

std::u32string Selector::getActiveElem(){
	return this->activeButton;
}

void Selector::setActiveElem(std::u32string elem){
	this->activeButton = elem;
}

void Selector::draw(){
	vec2f coord = getButtonStartCoords();

	this->parentGUI->RenderText(this->title, coord[0]*(float)parentGUI->winWidth/2.0, coord[1]*(float)parentGUI->winHeight/2.0, 0.3*scale, vec3{1.0,1.0,1.0}, this->depth );

	for(auto const& [name, func] : buttons){
		this->parentGUI->selShader->use();
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindVertexArray(VAO);
		glDisable(GL_DEPTH_TEST);
		if( name == activeButton)
			glBindTexture(GL_TEXTURE_2D, this->texIdOn);
		else
			glBindTexture(GL_TEXTURE_2D, this->texIdOff);
		coord[1] -= spacing*scale*2.0/ ((float)parentGUI->winHeight); //2.0 because NDC are -1 to 1
		this->parentGUI->selShader->setUniformV2("screenPos", coord);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		this->parentGUI->RenderText(name, coord[0]*(float)parentGUI->winWidth/2.0 + spacing*scale, coord[1]*(float)parentGUI->winHeight/2.0, 0.25*scale, vec3{1.0,1.0,1.0}, this->depth );
	}

	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
}

vec2f Selector::getButtonStartCoords(){
	float x, y;
	x = offset[0];
	y = offset[1];
	switch (this->location)
	{
	case GUI_POS_BOTTOM_LEFT:
		break;
	case GUI_POS_BOTTOM_MID:
		x += parentGUI->winWidth / 2.0;
		break;
	case GUI_POS_BOTTOM_RIGHT:
		x += parentGUI->winWidth;
		break;
	case GUI_POS_MID_LEFT:
		y += parentGUI->winHeight / 2.0;
		break;
	case GUI_POS_MID_MID:
		x += parentGUI->winWidth / 2.0;
		y += parentGUI->winHeight / 2.0;
		break;
	case GUI_POS_MID_RIGHT:
		x += parentGUI->winWidth;
		y += parentGUI->winHeight / 2.0;
		break;
	case GUI_POS_UPPER_LEFT:
		y += parentGUI->winHeight;
		break;
	case GUI_POS_UPPER_MID:
		x += parentGUI->winWidth / 2.0;
		y += parentGUI->winHeight;
		break;
	case GUI_POS_UPPER_RIGHT:
		x += parentGUI->winWidth;
		y += parentGUI->winHeight;
		break;
	}

	vec4 screenPos = vec4{x, y, -0.5, 1.0};
	screenPos = (parentGUI->parentCam->getOrthoMat()) * screenPos;
	screenPos = screenPos / screenPos[3];

	return vec2f{(float)screenPos[0]+1.0f, (float)screenPos[1]+1.0f};
}

//##############################  GUI BUTTON  ########################################
GUIButton::~GUIButton() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void GUIButton::updateOffset(vec2 newOffset, int winWidth, int winHeight){
	this->offset = newOffset;
	this->updateVertices(winWidth, winHeight);
}

void GUIButton::updateVertices(int winWidth, int winHeight) {
	//Convert window coordinates to NDC
	float lx, rx, uy, ly;
	lx = bb_ll[0] * scale + offset[0];
	rx = bb_ur[0] * scale + offset[0];
	uy = bb_ur[1] * scale + offset[1];
	ly = bb_ll[1] * scale + offset[1];

	//adds to position based on location base
	switch (this->location)
	{
	case GUI_POS_BOTTOM_LEFT:
		break;
	case GUI_POS_BOTTOM_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		break;
	case GUI_POS_BOTTOM_RIGHT:
		lx += winWidth;
		rx += winWidth;
		break;
	case GUI_POS_MID_LEFT:
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_MID_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_MID_RIGHT:
		lx += winWidth;
		rx += winWidth;
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_UPPER_LEFT:
		ly += winHeight;
		uy += winHeight;
		break;
	case GUI_POS_UPPER_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		ly += winHeight;
		uy += winHeight;
		break;
	case GUI_POS_UPPER_RIGHT:
		lx += winWidth;
		rx += winWidth;
		ly += winHeight;
		uy += winHeight;
		break;
	}

	rx = (2.0 * rx / (float)winWidth) - 1.0;
	lx = (2.0 * lx / (float)winWidth) - 1.0;
	ly = (2.0 * ly / (float)winHeight) - 1.0;
	uy = (2.0 * uy / (float)winHeight) - 1.0;

	float vertexData[] = {
		rx, uy, depth, 1.0, 1.0,
		rx, ly, depth, 1.0, 0.0,
		lx, ly, depth, 0.0, 0.0,
		lx, uy, depth, 0.0, 1.0
	};
	unsigned int indexData[] = {
		0,1,2,
		0,3,2
	};


	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO); // -------Bind VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

	// position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(std::array<float, 5>), (void*)0);
	glEnableVertexAttribArray(0);

	// texture data
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(std::array<float, 5>), (void*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0); // ----------Unbind VAO
}

void GUIButton::onClick() {
	clickCallback();
}
void GUIButton::onHover(){
	hoverCallback();
}
void GUIButton::hoverOff(){
	hoverOffCallback();
}
void GUIButton::draw() {
	glActiveTexture(GL_TEXTURE0 + 2);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, this->texId);
	glBindVertexArray(VAO);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
}

bool GUIButton::checkClick(double xpos, double ypos, int winWidth, int winHeight) {
	float lx, rx, uy, ly;
	lx = bb_ll[0] * scale + offset[0];
	rx = bb_ur[0] * scale + offset[0];
	uy = bb_ur[1] * scale + offset[1];
	ly = bb_ll[1] * scale + offset[1];

	//adds to position based on location base
	switch (this->location)
	{
	case GUI_POS_BOTTOM_LEFT:
		break;
	case GUI_POS_BOTTOM_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		break;
	case GUI_POS_BOTTOM_RIGHT:
		lx += winWidth;
		rx += winWidth;
		break;
	case GUI_POS_MID_LEFT:
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_MID_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_MID_RIGHT:
		lx += winWidth;
		rx += winWidth;
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_UPPER_LEFT:
		ly += winHeight;
		uy += winHeight;
		break;
	case GUI_POS_UPPER_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		ly += winHeight;
		uy += winHeight;
		break;
	case GUI_POS_UPPER_RIGHT:
		lx += winWidth;
		rx += winWidth;
		ly += winHeight;
		uy += winHeight;
		break;
	}

	if (lx < xpos && xpos < rx && ly < ypos && ypos < uy) {
		return true;
	}
	else {
		return false;
	}
}

//##################################  TEXT INPUT ###############################################
TextInput::~TextInput() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void TextInput::updateVertices(int winWidth, int winHeight) {
	//Convert window coordinates to NDC
	float lx, rx, uy, ly;
	lx = bb_ll[0] * scale + offset[0];
	rx = bb_ur[0] * scale + offset[0];
	uy = bb_ur[1] * scale + offset[1];
	ly = bb_ll[1] * scale + offset[1];

	//adds to position based on location base
	switch (this->location)
	{
	case GUI_POS_BOTTOM_LEFT:
		break;
	case GUI_POS_BOTTOM_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		break;
	case GUI_POS_BOTTOM_RIGHT:
		lx += winWidth;
		rx += winWidth;
		break;
	case GUI_POS_MID_LEFT:
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_MID_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_MID_RIGHT:
		lx += winWidth;
		rx += winWidth;
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_UPPER_LEFT:
		ly += winHeight;
		uy += winHeight;
		break;
	case GUI_POS_UPPER_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		ly += winHeight;
		uy += winHeight;
		break;
	case GUI_POS_UPPER_RIGHT:
		lx += winWidth;
		rx += winWidth;
		ly += winHeight;
		uy += winHeight;
		break;
	}

	rx = (2.0 * rx / (float)winWidth) - 1.0;
	lx = (2.0 * lx / (float)winWidth) - 1.0;
	ly = (2.0 * ly / (float)winHeight) - 1.0;
	uy = (2.0 * uy / (float)winHeight) - 1.0;

	float vertexData[] = {
		rx, uy, depth,
		rx, ly, depth,
		lx, ly, depth,
		lx, uy, depth
	};
	unsigned int indexData[] = {
		0,1,2,
		0,3,2,
		0,1,2,3,0
	};


	glBindVertexArray(VAO); // -------Bind VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

	// position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(std::array<float, 3>), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // ----------Unbind VAO
}
void TextInput::onClick() {
	parentGUI->activeInput = this;
}
void TextInput::draw() {
	glEnable(GL_DEPTH_TEST);
	this->parentGUI->txtInputShader->use();
	glBindVertexArray(VAO);
	
	this->parentGUI->txtInputShader->setUniformV3("color", vec3{0.1,0.1,0.1});
	this->parentGUI->txtInputShader->setUniform1f("depth", depth);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	vec3 col = (this->parentGUI->activeInput==this)? vec3{1.0,1.0,1.0} : vec3{0.3,0.3,1.0};
	this->parentGUI->txtInputShader->setUniformV3("color", col);
	this->parentGUI->txtInputShader->setUniform1f("depth", depth-0.01f);
	glDrawElements(GL_LINE_STRIP, 5, GL_UNSIGNED_INT, (void*)(6*sizeof(unsigned int)));

	glBindVertexArray(0);
	switch (this->location) {
		case GUI_POS_BOTTOM_LEFT:
			this->parentGUI->RenderText(this->textToDraw, this->offset[0] + (this->bb_ll[0] + 5)*scale, this->offset[1], 0.5*this->scale*this->parentGUI->guiScale, vec3{1.0,1.0,1.0}, this->depth-0.8);
			break;
		case GUI_POS_BOTTOM_RIGHT:
			this->parentGUI->RenderText(this->textToDraw, this->parentGUI->winWidth + this->offset[0] + (this->bb_ll[0] + 5)*scale, this->offset[1], 0.5*this->scale*this->parentGUI->guiScale, vec3{1.0,1.0,1.0}, this->depth-0.8);
			break;
		case GUI_POS_UPPER_LEFT:
			this->parentGUI->RenderText(this->textToDraw,  this->offset[0] + (this->bb_ll[0] + 5)*scale, this->parentGUI->winHeight + this->offset[1], 0.5*this->scale*this->parentGUI->guiScale, vec3{1.0,1.0,1.0}, this->depth-0.8);
			break;
		case GUI_POS_UPPER_RIGHT:
			this->parentGUI->RenderText(this->textToDraw,  this->parentGUI->winWidth + this->offset[0] + (this->bb_ll[0] + 5)*scale, this->parentGUI->winHeight + this->offset[1], 0.5*this->scale*this->parentGUI->guiScale, vec3{1.0,1.0,1.0}, this->depth-0.8);
			break;
		case GUI_POS_MID_MID:
			this->parentGUI->RenderText(this->textToDraw,  this->parentGUI->winWidth/2 + this->offset[0] + (this->bb_ll[0] + 5)*scale, this->parentGUI->winHeight/2 + this->offset[1], 0.5*this->scale*this->parentGUI->guiScale, vec3{1.0,1.0,1.0}, this->depth-0.8);
			break;
		}

}
bool TextInput::checkClick(double xpos, double ypos, int winWidth, int winHeight) {
	float lx, rx, uy, ly;
	lx = bb_ll[0] * scale + offset[0];
	rx = bb_ur[0] * scale + offset[0];
	uy = bb_ur[1] * scale + offset[1];
	ly = bb_ll[1] * scale + offset[1];

	//adds to position based on location base
	switch (this->location)
	{
	case GUI_POS_BOTTOM_LEFT:
		break;
	case GUI_POS_BOTTOM_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		break;
	case GUI_POS_BOTTOM_RIGHT:
		lx += winWidth;
		rx += winWidth;
		break;
	case GUI_POS_MID_LEFT:
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_MID_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_MID_RIGHT:
		lx += winWidth;
		rx += winWidth;
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_UPPER_LEFT:
		ly += winHeight;
		uy += winHeight;
		break;
	case GUI_POS_UPPER_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		ly += winHeight;
		uy += winHeight;
		break;
	case GUI_POS_UPPER_RIGHT:
		lx += winWidth;
		rx += winWidth;
		ly += winHeight;
		uy += winHeight;
		break;
	}

	if (lx < xpos && xpos < rx && ly < ypos && ypos < uy) {
		return true;
	}
	else {
		return false;
	}
}
void TextInput::changeText(string text){
	this->textToDraw = text;
}
void TextInput::addChar(char toAdd){
	this->textToDraw += toAdd;
}
void TextInput::removeChar(){
	this->textToDraw.pop_back();
}
void TextInput::setDepth(float val){
	this->depth = val;
}

// ################################## StaticBox #####################################################

StaticBox::StaticBox(GUI* parentGUI, vec2 bb_ll, vec2 bb_ur, vec2 offset, int location, int winWidth, int winHeight, float scale = 1.0) : parentGUI{ parentGUI }, bb_ll{ bb_ll }, bb_ur{ bb_ur }, offset{ offset }, location{ location }, scale{ scale } {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	this->updateVertices(winWidth, winHeight);
}
void StaticBox::updateVertices(int winWidth, int winHeight){
	//Convert window coordinates to NDC
	float lx, rx, uy, ly;
	lx = bb_ll[0] * scale + offset[0];
	rx = bb_ur[0] * scale + offset[0];
	uy = bb_ur[1] * scale + offset[1];
	ly = bb_ll[1] * scale + offset[1];

	//adds to position based on location base
	switch (this->location)
	{
	case GUI_POS_BOTTOM_LEFT:
		break;
	case GUI_POS_BOTTOM_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		break;
	case GUI_POS_BOTTOM_RIGHT:
		lx += winWidth;
		rx += winWidth;
		break;
	case GUI_POS_MID_LEFT:
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_MID_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_MID_RIGHT:
		lx += winWidth;
		rx += winWidth;
		ly += winHeight / 2.0;
		uy += winHeight / 2.0;
		break;
	case GUI_POS_UPPER_LEFT:
		ly += winHeight;
		uy += winHeight;
		break;
	case GUI_POS_UPPER_MID:
		lx += winWidth / 2.0;
		rx += winWidth / 2.0;
		ly += winHeight;
		uy += winHeight;
		break;
	case GUI_POS_UPPER_RIGHT:
		lx += winWidth;
		rx += winWidth;
		ly += winHeight;
		uy += winHeight;
		break;
	}

	rx = (2.0 * rx / (float)winWidth) - 1.0;
	lx = (2.0 * lx / (float)winWidth) - 1.0;
	ly = (2.0 * ly / (float)winHeight) - 1.0;
	uy = (2.0 * uy / (float)winHeight) - 1.0;

	GLfloat vertexData[] = {
		lx, ly, depth,
		rx, ly, depth,
		rx, uy, depth,
		lx, uy, depth
	};
	unsigned int indexData[] = {
		0,1,2,
		0,2,3
	};




	glBindVertexArray(VAO); // -------Bind VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

	// position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(std::array<GLfloat, 3>), (void*)0);
	glEnableVertexAttribArray(0);


	glBindVertexArray(0); // ----------Unbind VAO
}

void StaticBox::setDepth(GLfloat val){
	this->depth = val;
}

void StaticBox::draw(){
	this->parentGUI->txtInputShader->use();
	glBindVertexArray(VAO);
	glEnable(GL_DEPTH_TEST);
	this->parentGUI->txtInputShader->setUniform1f("depth", depth);
	this->parentGUI->txtInputShader->setUniformV3("color", this->color);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

}



// #################################   GUI   ###################################################


GLuint GUI::loadTexture(string key, string filename) {

	//texture slot 0 reserved for loading
	glActiveTexture(GL_TEXTURE0);
	//Not super sure. I think OpenGL flips textures or something.
	stbi_set_flip_vertically_on_load(true);

	//add texture directory
	this->textureMap[key] = this->numTextures;
	filename = "./textures/GUI/" + filename;
	this->textures[this->numTextures].path = filename;


	//Create the texture object within OpenGL and add it to the dictionary
	//generates the texture and tells opengl to store the texture ID in the next available slot in the texture dictionary
	glGenTextures(1, &(this->textures[this->numTextures].diffID));
	glBindTexture(GL_TEXTURE_2D, this->textures[this->numTextures].diffID);
	int texId = this->textures[this->numTextures].diffID;
	// Sets the wrapping mode to standard repeat
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Sets the interpolation to nearest, with linear mipmap interpolation
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//use stb_image library to load the texture sent in by filename

	unsigned char* data = stbi_load(filename.c_str(), &(textures[numTextures].width), &(textures[numTextures].height), &(textures[numTextures].channels), 0);

	if (data != NULL) {

		if (textures[numTextures].channels == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textures[numTextures].width, textures[numTextures].height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else if (textures[numTextures].channels == 4) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textures[numTextures].width, textures[numTextures].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else {
			cout << "ERROR: " + filename + " has an unknown file type. Possibly too many channels " << endl;
			return 0;
		}

		glGenerateMipmap(GL_TEXTURE_2D);
		cout << "Loaded texture file " + filename << " with " << textures[numTextures].channels << " channels." << endl;
	}
	else {
		cout << "ERROR: Failed to load " + filename << endl;
	}

	stbi_image_free(data);

	this->numTextures += 1;
	return texId;
}

Texture * GUI::getTexturePtr(int index) {
	if (textures[index].channels) {
		return &textures[index];
	}
	else {
		cout << "Could not find texture indexed as " << index << ". Returning index 0!" << endl;
		return &textures[0];
	}
}

GUI::GUI(Camera* cam) {
	this->parentCam = cam;

	this->guiShader = new Shader();
	this->guiShader->loadShader("GUI");

	this->txtInputShader = new Shader();
	this->txtInputShader->loadShader("txtInput");

	this->selShader = new Shader();
	this->selShader->loadShader("lensFlare");

	this->textShader = new Shader();
	this->textShader->loadShader("text");
	this->textShader->use();
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		printf("ERROR::FREETYPE: Could not init FreeType Library\n");

	FT_Face face;
	if (FT_New_Face(ft, "fonts/arial.otf", 0, &face))
		printf("ERROR::FREETYPE: Failed to load font\n");

	FT_Set_Pixel_Sizes(face, 0, 48);
	loadCharacters(face);

	FT_Face sym;
	if (FT_New_Face(ft, "fonts/Symbol.ttf", 0, &sym))
		printf("ERROR::FREETYPE: Failed to load font\n");

	FT_Set_Pixel_Sizes(face, 0, 48);
	loadSymbols(face);

	FT_Done_Face(face);
	FT_Done_Face(sym);
	FT_Done_FreeType(ft);

	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);
	glBindVertexArray(textVAO);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	this->winHeight = cam->getWinHeight();
	this->winWidth = cam->getWinWidth();

	this->initializeComponents();
}

//Initialization and shaders

void GUI::loadCharacters(FT_Face face) {
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

	for (GLuint c = 0; c < 128; c++)
	{
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			std::array<int, 2>{(int)face->glyph->bitmap.width, (int)face->glyph->bitmap.rows},
			std::array<int, 2>{(int)face->glyph->bitmap_left, (int)face->glyph->bitmap_top},
			static_cast<GLuint>(face->glyph->advance.x)
		};
		Characters.insert(std::pair<GLuint, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);

}

void GUI::loadSymbols(FT_Face face){
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

	std::vector<GLuint> symArray;
	symArray.push_back(0x03A9);
	symArray.push_back(0x03C9);
	symArray.push_back(0x03C5);
	symArray.push_back(0x1E8B);
	symArray.push_back(0x1E8F);
	symArray.push_back(0x017C);
	

	for (auto const& c : symArray)
	{
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			std::array<int, 2>{(int)face->glyph->bitmap.width, (int)face->glyph->bitmap.rows},
			std::array<int, 2>{(int)face->glyph->bitmap_left, (int)face->glyph->bitmap_top},
			static_cast<GLuint>(face->glyph->advance.x)
		};
		Characters.insert(std::pair<GLuint, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GUI::initializeComponents() {
	// #############################################################################
	// #############################################################################
	// #############################################################################
	// #############################################################################
	// #############################################################################
	// #############################################################################
	// ###################### ELEMENTS INITIALIZATION #############################

	// Texture Loading
	int texIdOn = loadTexture("selOn", "selectorPressed.png");
	int texIdOff = loadTexture("selOff", "selectorUnpressed.png");


	// ----------------------### Static Boxes ###----------------
	this->createStaticBox("mainRight", vec2{-250.0, 0.0}, vec2{0.0, 2160.0}, vec2{0.0, 0.0}, GUI_POS_BOTTOM_RIGHT);
	this->createStaticBox("mainLeft", vec2{0.0, 0.0}, vec2{250.0, 2160.0}, vec2{0.0, 0.0}, GUI_POS_BOTTOM_LEFT);
	
	//  -----------  Time   -----------

	vec2 timeButtonGroupOffset = vec2{125.0, -20.0};
	this->textMap["time"] = GUIText(U"TIME ERR", timeButtonGroupOffset + vec2{ 0.0, -60.0 }, GUI_POS_UPPER_LEFT, GUI_TEXT_ANCHOR_MID);
	this->textMap["simspeed"] = GUIText(U"SIM SPEED ERR", timeButtonGroupOffset + vec2{ 0.0,-30.0 }, GUI_POS_UPPER_LEFT, GUI_TEXT_ANCHOR_MID);

	this->createButton([this](){this->parentCam->togglePause();}, "pause", "pause.png", vec2{ -10.0,-10.0 }, vec2{ 10.0,10.0 }, timeButtonGroupOffset, GUI_POS_UPPER_LEFT, 2.0);
	this->createButton([this](){this->parentCam->fastForward();}, "fforward", "fforward.png", vec2{ 20.0,-10.0 }, vec2{ 40.0,10.0 }, timeButtonGroupOffset, GUI_POS_UPPER_LEFT, 2.0);
	this->createButton([this](){this->parentCam->rewind();}, "rewind", "rewind.png", vec2{ -40.0,-10.0 }, vec2{ -20.0,10.0 }, timeButtonGroupOffset, GUI_POS_UPPER_LEFT, 2.0);
	

	// --------------------------### Buttons ### --------------------
	
	// Info Panel Functionals
	std::function<void()> clearInfoPanel = [this](){
		this->textMap.erase("infoPanelName");
		this->textMap.erase("infoPanelMass");
		this->textMap.erase("infoPanelRadP");
		this->textMap.erase("infoPanelRadE");
		this->textMap.erase("infoPanelObla");
	};

	std::function<void(PlanetaryBody*)> planetSelectCall = [this, clearInfoPanel](PlanetaryBody* planet){
		std::wstring_convert<codecvt<char32_t,char,std::mbstate_t>,char32_t> convert32;	
		clearInfoPanel();
	SpiceInt randi; SpiceBoolean randb; SpiceInt id; SpiceDouble mu, j2; SpiceDouble radius[3];
		bodn2c_c(planet->name.c_str(), &id, &randb);
		bodvcd_c(id, "GM", 1, &randi, &mu); 
		bodvcd_c(id, "RADII",3,&randi, radius);
		//bodvcd_c(id, "J2", 1, &randi, &j2);
		this->textMap["infoPanelName"] = GUIText(convert32.from_bytes(capitalize(planet->name)), vec2{125.0, 270.0}, GUI_POS_MID_LEFT, GUI_TEXT_ANCHOR_MID, 0.5);
		this->textMap["infoPanelMass"] = GUIText(convert32.from_bytes("GM: "+std::to_string(mu)+" km3/s2"), vec2{15.0, 230.0}, GUI_POS_MID_LEFT, GUI_TEXT_ANCHOR_LEFT,0.25);
		this->textMap["infoPanelRadP"] = GUIText(convert32.from_bytes("Equatorial Radius: "+std::to_string(radius[0])+" km"), vec2{15.0, 210.0}, GUI_POS_MID_LEFT, GUI_TEXT_ANCHOR_LEFT,0.25);
		this->textMap["infoPanelRadE"] = GUIText(convert32.from_bytes("Polar Radius: "+std::to_string(radius[2])+" km"), vec2{15.0, 190.0}, GUI_POS_MID_LEFT, GUI_TEXT_ANCHOR_LEFT,0.25);
		//this->textMap["infoPanelObla"] = GUIText(convert32.from_bytes("J2: "+std::to_string(j2)), vec2{15.0, 170.0}, GUI_POS_MID_LEFT, GUI_TEXT_ANCHOR_LEFT,0.25);
	};
	planetSelectCall(this->parentCam->getCurBody());

	//--Planet Selection Buttons
	double bsl = 4.0*this->guiScale;
	for(auto const& [key, planet]: solarSystem->Planets){
		PlanetaryBody* p = planet;
		this->createButton([this, p, clearInfoPanel, planetSelectCall](){this->parentCam->selectBody(p);clearInfoPanel();planetSelectCall(p);}, planet->name+"Select",planet->name+"_button.png", vec2{ -10.0*(bsl/2.0), -10.0*(bsl/2.0) }, vec2{ 10.0*(bsl/2.0), 10.0*(bsl/2.0) }, vec2{ planet->systemIndex * 15.0 * bsl - solarSystem->nrPlanets*7.5*bsl, 10.0*bsl}, GUI_POS_BOTTOM_MID, 2.0);
		
		GUIButton* pbutt = this->buttonMap.at(planet->name+"Select");
		pbutt->hoverCallback = [this, p](){
			for (int i = 0; i < p->children.size(); ++i){
				this->buttonMap.at(p->children[i]->name+"Select")->enabled = true;
			}
		};
		pbutt->hoverOffCallback = [this, p](){
			for (int i = 0; i < p->children.size(); ++i){
				if(this->buttonMap.at(p->children[i]->name+"Select")->enabled)
					if(!this->buttonMap.at(p->children[i]->name+"Select")->checkClick(mousex, winHeight - mousey, winWidth, winHeight))
						this->buttonMap.at(p->children[i]->name+"Select")->enabled = false;
			}
		};

		for(int i = 0; i < planet->children.size(); ++i){
			PlanetaryBody* m = planet->children[i];
			this->createButton([this, m, clearInfoPanel, planetSelectCall](){this->parentCam->selectBody(m);clearInfoPanel();planetSelectCall(m);}, m->name+"Select",m->name+"_button.png", vec2{ -10.0*(bsl/2.0), -10.0*(bsl/2.0) }, vec2{ 10.0*(bsl/2.0), 10.0*(bsl/2.0) }, vec2{ planet->systemIndex * 15.0 * bsl - solarSystem->nrPlanets*7.5*bsl, 10.0*bsl + (i+1) * 15.0 * bsl}, GUI_POS_BOTTOM_MID, 1.5);
			this->buttonMap.at(m->name+"Select")->enabled = false;
			this->buttonMap.at(m->name+"Select")->hoverCallback = pbutt->hoverCallback;
		}
	}
	// ------- Functionals --------
	std::function<void(string)> scSelectFunc = [this, clearInfoPanel](string index){
		clearInfoPanel();
		this->parentCam->selectBody(solarSystem->spacecrafts[index]);
	};
	std::function<void(string)> scExitFunc = [this](string index){
		this->parentCam->selectBody(solarSystem->spacecrafts.at(index)->parentBody);

		solarSystem->spacecrafts.erase(index);
		
		this->deleteText("SCname_"+index);
		this->deleteButton("SCsettings_"+index);
		this->deleteButton("SCbutton_"+index);
		this->deleteButton("SCexit_"+index);

		string suffixB = "SCbutton_";
		string suffixI = "SCsettings_";
		string suffixE = "SCexit_";
		for(auto const& [key, button] : this->buttonMap){
			string name = key;
			if(key.rfind(suffixB,0)==0){
				name.erase(name.find(suffixB), suffixB.length());
				button->updateOffset(vec2{ -137.5, -20.0 - 50.0* (std::distance(solarSystem->spacecrafts.begin(), solarSystem->spacecrafts.find(name))+1)}, this->winWidth, this->winHeight);
				this->textMap.at("SCname_"+name).offset = vec2{ -170.0, -20.0 - 50.0* (std::distance(solarSystem->spacecrafts.begin(), solarSystem->spacecrafts.find(name))+1) };
			}else if(key.rfind(suffixI)==0){
				name.erase(name.find(suffixI), suffixI.length());
				button->updateOffset(vec2{ -37.5, 12.5 -20.0 - 50.0* (std::distance(solarSystem->spacecrafts.begin(), solarSystem->spacecrafts.find(name))+1)}, this->winWidth, this->winHeight);
			}else if(key.rfind(suffixE) == 0){
				name.erase(name.find(suffixE), suffixE.length());
				button->updateOffset(vec2{ -37.5, -12.5 -20.0 - 50.0* (std::distance(solarSystem->spacecrafts.begin(), solarSystem->spacecrafts.find(name))+1)}, this->winWidth, this->winHeight);
			}
		}

	};
	std::function<void(string)> scSettingsFunc = [this, texIdOn, texIdOff](string index){
		std::wstring_convert<codecvt<char32_t,char,std::mbstate_t>,char32_t> convert32;	

		this->createStaticBox("scInfo", vec2{-100.0, -150.0}, vec2{100.0,150.0}, vec2{0.0,0.0},GUI_POS_MID_MID);
		this->createButton([this](){
			this->deleteButton("scInfoExit");
			this->deleteStaticBox("scInfo");
			this->selectorMap.erase("scLinesParent");
		}, "scInfoExit", "exit.png", vec2{0.0,-25.0}, vec2{25.0, 0.0}, vec2{-100.0,150.0}, GUI_POS_MID_MID);


		this->selectorMap["scLinesParent"] = new Selector(this, U"Orbit Line Parent", vec2{-95.0,100.0}, GUI_POS_MID_MID, texIdOn, texIdOff, this->winWidth, this->winHeight);
		for(auto const& [key, planet] : solarSystem->Planets){
			PlanetaryBody* body = planet; // cannot caputure the const& planet from the for loop in a lambda
			this->selectorMap.at("scLinesParent")->addButton(convert32.from_bytes(planet->name.c_str()), [this, body, index](){solarSystem->spacecrafts[index]->updateOrbitLines(body);});
			for(auto const& moon : planet->children){
				body = moon;
				this->selectorMap.at("scLinesParent")->addButton(convert32.from_bytes(moon->name.c_str()), [this, body, index](){solarSystem->spacecrafts[index]->updateOrbitLines(body);});
			}

		}
		std::u32string key = convert32.from_bytes(solarSystem->spacecrafts[index]->parentBody->name.c_str());
		this->selectorMap.at("scLinesParent")->setActiveElem(key);


		

	};
	std::function<void()> createOEFunc = [this, scSelectFunc, scSettingsFunc, scExitFunc](){
		try{
			std::wstring_convert<codecvt<char32_t,char,std::mbstate_t>,char32_t> convert32;
			double a = std::stod(this->textInputs["ain"]->textToDraw);
			double e = std::stod(this->textInputs["ein"]->textToDraw);
			double i = radians(std::stod(this->textInputs["iin"]->textToDraw));
			double O = radians(std::stod(this->textInputs["Oin"]->textToDraw));
			double w = radians(std::stod(this->textInputs["win"]->textToDraw));
			double v = radians(std::stod(this->textInputs["nin"]->textToDraw));
			double duration = std::stod(this->textInputs["durin"]->textToDraw);
			string name = this->textInputs["namein"]->textToDraw;

			std::u32string pname32 = selectorMap.at("tempParent")->getActiveElem();
			string pname = convert32.to_bytes(pname32.c_str());

			std::u32string lpname32 = selectorMap.at("tempOrbParent")->getActiveElem();
			string lpname = convert32.to_bytes(lpname32.c_str());
			
			mat3 att;
			PlanetaryBody* bodyParent = NULL;
			PlanetaryBody* lineParent = NULL;
			try{
				bodyParent = solarSystem->Planets.at(pname);
			}
			catch(std::out_of_range){
				for(auto const& [key, body] : solarSystem->Planets){
					for(auto const& moon : body->children){
						if(pname == moon->name){
							bodyParent = moon;
							break;
						}
					}
				}
			}
			try{
				lineParent = solarSystem->Planets.at(lpname);
			}
			catch(std::out_of_range){
				for(auto const& [key, body] : solarSystem->Planets){
					for(auto const& moon : body->children){
						if(lpname == moon->name){
							lineParent = moon;
							break;
						}
					}
				}
			}


			switch(this->selectorMap.at("bfORin")->getActiveElem().c_str()[0]){
				case U'B':
					att = mat4reduce(getFrameTransformSpice(solarSystem->curTime, "IAU_"+bodyParent->name, "eclipj2000"));
					break;
				case U'J':
					att = mat4reduce(getFrameTransformSpice(solarSystem->curTime, "j2000", "eclipj2000"));
					break;
				case U'E': 
					att = eye3();
					break;
			}

			statevec rv = oe2rv(vec6{a,e,i,w,O,v}, bodyParent->mu);
			
			solarSystem->addSpacecraft(lineParent, name, bodyParent->getPosition(false)+ att*rv[0], bodyParent->getVelocity(false) + att*rv[1], duration, "craft.obj");
			solarSystem->setUniform4dv("projection", this->parentCam->getProjPtr());
			this->buttonMap.at("addSpacecraft")->enabled = true;

			this->createButton([this, name, scSelectFunc](){scSelectFunc(name);}, "SCbutton_"+name, "scButton.png", vec2{ -87.5,-25.0 }, vec2{ 87.5,25.0 }, vec2{0.0, 0.0}, GUI_POS_UPPER_RIGHT );
			this->createButton([this, name, scSettingsFunc](){scSettingsFunc(name);}, "SCsettings_"+name, "scSettings.png", vec2{-12.5, -12.5}, vec2{12.5, 12.5}, vec2{0.0,0.0},GUI_POS_UPPER_RIGHT);
			this->createButton([this, name, scExitFunc](){scExitFunc(name);}, "SCexit_"+name, "exit.png", vec2{-12.5, -12.5}, vec2{12.5, 12.5}, vec2{0.0,0.0},GUI_POS_UPPER_RIGHT);
			std::u32string name32 = convert32.from_bytes(name);
			this->textMap["SCname_"+name] = GUIText(name32, vec2{ -125.0, -120.0 }, GUI_POS_UPPER_RIGHT, GUI_TEXT_ANCHOR_LEFT, 0.5, 0.49, vec3{0.1,0.1,0.1});
			
			string suffixB = "SCbutton_";
			string suffixI = "SCsettings_";
			string suffixE = "SCexit_";
			for(auto const& [key, button] : this->buttonMap){
				string name = key;
				if(key.rfind(suffixB,0)==0){
					name.erase(name.find(suffixB), suffixB.length());
					button->updateOffset(vec2{ -137.5, -20.0 - 50.0* (std::distance(solarSystem->spacecrafts.begin(), solarSystem->spacecrafts.find(name))+1)}, this->winWidth, this->winHeight);
					this->textMap.at("SCname_"+name).offset = vec2{ -170.0, -20.0 - 50.0* (std::distance(solarSystem->spacecrafts.begin(), solarSystem->spacecrafts.find(name))+1) };
				}else if(key.rfind(suffixI)==0){
					name.erase(name.find(suffixI), suffixI.length());
					button->updateOffset(vec2{ -37.5, 12.5 -20.0 - 50.0* (std::distance(solarSystem->spacecrafts.begin(), solarSystem->spacecrafts.find(name))+1)}, this->winWidth, this->winHeight);
				}else if(key.rfind(suffixE) == 0){
					name.erase(name.find(suffixE), suffixE.length());
					button->updateOffset(vec2{ -37.5, -12.5 -20.0 - 50.0* (std::distance(solarSystem->spacecrafts.begin(), solarSystem->spacecrafts.find(name))+1)}, this->winWidth, this->winHeight);
				}
			}

			this->deleteButton("createOE");
			this->deleteButton("createRV");
			this->deleteText("tempCreate");
			this->textInputs.erase("namein");

			this->textInputs.erase("ain"); this->textInputs.erase("ein"); this->textInputs.erase("iin");
			this->textInputs.erase("Oin"); this->textInputs.erase("win"); this->textInputs.erase("nin");
			
			this->textInputs.erase("xin"); this->textInputs.erase("yin"); this->textInputs.erase("zin");
			this->textInputs.erase("dxin"); this->textInputs.erase("dyin"); this->textInputs.erase("dzin");

			this->deleteText("tempA"); this->deleteText("tempE"); this->deleteText("tempI");
			this->deleteText("tempO"); this->deleteText("tempW"); this->deleteText("tempN");

			this->deleteText("tempX"); this->deleteText("tempY"); this->deleteText("tempZ");
			this->deleteText("tempDX"); this->deleteText("tempDY"); this->deleteText("tempDZ");
			
			this->selectorMap.erase("oeORrv");
			this->selectorMap.erase("bfORin");
			this->selectorMap.erase("tempParent");
			this->selectorMap.erase("tempOrbParent");
			
			this->deleteText("tempManeuvers");
			this->deleteButton("addManeuver");
			this->deleteText("tempDur");
			this->deleteStaticBox("createSC");
			this->deleteText("tempSCName");

			this->textInputs.erase("durin");
			this->deleteText("create_ERR");
		}catch(std::invalid_argument){
			this->textMap["create_ERR"] = GUIText(U"Invalid character!", vec2{ -100.0, -190.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.25, 0.5, vec3{1.0, 0.2, 0.2});
		}

	};
	std::function<void()> createRVFunc = [this, scSelectFunc, scSettingsFunc, scExitFunc](){
		try{
			std::wstring_convert<codecvt<char32_t,char,std::mbstate_t>,char32_t> convert32;
			double x = std::stod(this->textInputs.at("xin")->textToDraw);
			double y = std::stod(this->textInputs.at("yin")->textToDraw);
			double z = std::stod(this->textInputs.at("zin")->textToDraw);
			double dx = std::stod(this->textInputs.at("dxin")->textToDraw);
			double dy = std::stod(this->textInputs.at("dyin")->textToDraw);
			double dz = std::stod(this->textInputs.at("dzin")->textToDraw);
			double duration = std::stod(this->textInputs["durin"]->textToDraw);
			string name = this->textInputs["namein"]->textToDraw;

			std::u32string pname32 = selectorMap.at("tempParent")->getActiveElem();
			string pname = convert32.to_bytes(pname32.c_str());

			std::u32string lpname32 = selectorMap.at("tempOrbParent")->getActiveElem();
			string lpname = convert32.to_bytes(lpname32.c_str());
			
			mat3 att;
			PlanetaryBody* bodyParent = NULL;
			PlanetaryBody* lineParent = NULL;
			try{
				bodyParent = solarSystem->Planets.at(pname);
			}
			catch(std::out_of_range){
				for(auto const& [key, body] : solarSystem->Planets){
					for(auto const& moon : body->children){
						if(pname == moon->name){
							bodyParent = moon;
							break;
						}
					}
				}
			}
			try{
				lineParent = solarSystem->Planets.at(lpname);
			}
			catch(std::out_of_range){
				for(auto const& [key, body] : solarSystem->Planets){
					for(auto const& moon : body->children){
						if(lpname == moon->name){
							lineParent = moon;
							break;
						}
					}
				}
			}


			switch(this->selectorMap.at("bfORin")->getActiveElem().c_str()[0]){
				case U'B':
					att = mat4reduce(getFrameTransformSpice(solarSystem->curTime, "IAU_"+bodyParent->name, "eclipj2000"));
					break;
				case U'J':
					att = mat4reduce(getFrameTransformSpice(solarSystem->curTime, "j2000", "eclipj2000"));
					break;
				case U'E': 
					att = eye3();
					break;
			}

			statevec rv = {vec3{x,y,z}, vec3{dx,dy,dz}};
			
			solarSystem->addSpacecraft(bodyParent, name, bodyParent->getPosition(false)+ att*rv[0], bodyParent->getVelocity(false) + att*rv[1], duration, "craft.obj");
			solarSystem->setUniform4dv("projection", this->parentCam->getProjPtr());
			this->buttonMap.at("addSpacecraft")->enabled = true;

			this->createButton([this, name, scSelectFunc](){scSelectFunc(name);}, "SCbutton_"+name, "scButton.png", vec2{ -87.5,-25.0 }, vec2{ 87.5,25.0 }, vec2{0.0, 0.0}, GUI_POS_UPPER_RIGHT );
			this->createButton([this, name, scSettingsFunc](){scSettingsFunc(name);}, "SCsettings_"+name, "scSettings.png", vec2{-12.5, -12.5}, vec2{12.5, 12.5}, vec2{0.0,0.0},GUI_POS_UPPER_RIGHT);
			this->createButton([this, name, scExitFunc](){scExitFunc(name);}, "SCexit_"+name, "exit.png", vec2{-12.5, -12.5}, vec2{12.5, 12.5}, vec2{0.0,0.0},GUI_POS_UPPER_RIGHT);
			std::u32string name32 = convert32.from_bytes(name);
			this->textMap["SCname_"+name] = GUIText(name32, vec2{ -125.0, -120.0 }, GUI_POS_UPPER_RIGHT, GUI_TEXT_ANCHOR_LEFT, 0.5, 0.49, vec3{0.1,0.1,0.1});
			
			string suffixB = "SCbutton_";
			string suffixI = "SCsettings_";
			string suffixE = "SCexit_";
			for(auto const& [key, button] : this->buttonMap){
				string name = key;
				if(key.rfind(suffixB,0)==0){
					name.erase(name.find(suffixB), suffixB.length());
					button->updateOffset(vec2{ -137.5, -20.0 - 50.0* (std::distance(solarSystem->spacecrafts.begin(), solarSystem->spacecrafts.find(name))+1)}, this->winWidth, this->winHeight);
					this->textMap.at("SCname_"+name).offset = vec2{ -170.0, -20.0 - 50.0* (std::distance(solarSystem->spacecrafts.begin(), solarSystem->spacecrafts.find(name))+1) };
				}else if(key.rfind(suffixI)==0){
					name.erase(name.find(suffixI), suffixI.length());
					button->updateOffset(vec2{ -37.5, 12.5 -20.0 - 50.0* (std::distance(solarSystem->spacecrafts.begin(), solarSystem->spacecrafts.find(name))+1)}, this->winWidth, this->winHeight);
				}else if(key.rfind(suffixE) == 0){
					name.erase(name.find(suffixE), suffixE.length());
					button->updateOffset(vec2{ -37.5, -12.5 -20.0 - 50.0* (std::distance(solarSystem->spacecrafts.begin(), solarSystem->spacecrafts.find(name))+1)}, this->winWidth, this->winHeight);
				}
			}

			this->deleteButton("createOE");
			this->deleteButton("createRV");
			this->deleteText("tempCreate");
			this->textInputs.erase("namein");

			this->textInputs.erase("ain"); this->textInputs.erase("ein"); this->textInputs.erase("iin");
			this->textInputs.erase("Oin"); this->textInputs.erase("win"); this->textInputs.erase("nin");
			
			this->textInputs.erase("xin"); this->textInputs.erase("yin"); this->textInputs.erase("zin");
			this->textInputs.erase("dxin"); this->textInputs.erase("dyin"); this->textInputs.erase("dzin");

			this->deleteText("tempA"); this->deleteText("tempE"); this->deleteText("tempI");
			this->deleteText("tempO"); this->deleteText("tempW"); this->deleteText("tempN");

			this->deleteText("tempX"); this->deleteText("tempY"); this->deleteText("tempZ");
			this->deleteText("tempDX"); this->deleteText("tempDY"); this->deleteText("tempDZ");
			
			this->selectorMap.erase("oeORrv");
			this->selectorMap.erase("bfORin");
			this->selectorMap.erase("tempParent");
			this->selectorMap.erase("tempOrbParent");
			
			this->deleteText("tempManeuvers");
			this->deleteButton("addManeuver");
			this->deleteText("tempDur");
			this->deleteStaticBox("createSC");
			this->deleteText("tempSCName");

			this->textInputs.erase("durin");
			this->deleteText("create_ERR");
		}catch(std::invalid_argument){
			this->textMap["create_ERR"] = GUIText(U"Invalid character!", vec2{ 100.0, -190.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.25, 0.5, vec3{1.0, 0.2, 0.2});
		}
	};
	std::function<void()> oeSelButton = [this, createOEFunc](){
		this->deleteInput("xin"); // x
		this->deleteInput("yin"); // y
		this->deleteInput("zin"); // z
		this->deleteInput("dxin"); // dx
		this->deleteInput("dyin"); // dy
		this->deleteInput("dzin"); // dz
		this->textMap.erase("tempX");
		this->textMap.erase("tempY");
		this->textMap.erase("tempZ");
		this->textMap.erase("tempDX");
		this->textMap.erase("tempDY");
		this->textMap.erase("tempDZ");

		this->createInput("ain", vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 100.0}, GUI_POS_MID_MID, 0.5); // a
		this->createInput("ein", vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 080.0}, GUI_POS_MID_MID, 0.5); // e
		this->createInput("iin", vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 060.0}, GUI_POS_MID_MID, 0.5); // i
		this->createInput("Oin", vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 040.0}, GUI_POS_MID_MID, 0.5); // Omega
		this->createInput("win", vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 020.0}, GUI_POS_MID_MID, 0.5); // omega
		this->createInput("nin", vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 000.0}, GUI_POS_MID_MID, 0.5); // nu
		this->textInputs["ain"]->textToDraw = "200000";
		this->textInputs["ein"]->textToDraw = "0.95";
		this->textInputs["iin"]->textToDraw = "20";
		this->textInputs["Oin"]->textToDraw = "82";
		this->textInputs["win"]->textToDraw = "180";
		this->textInputs["nin"]->textToDraw = "0";
		this->textMap["tempA"] = GUIText(U"a",      vec2{ -240.0, 100.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);
		this->textMap["tempE"] = GUIText(U"e",      vec2{ -240.0, 080.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);
		this->textMap["tempI"] = GUIText(U"i",      vec2{ -240.0, 060.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);
		this->textMap["tempO"] = GUIText(U"\u03A9", vec2{ -240.0, 040.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);
		this->textMap["tempW"] = GUIText(U"\u03C9", vec2{ -240.0, 020.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);
		this->textMap["tempN"] = GUIText(U"\u03c5", vec2{ -240.0, 000.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35); 

		this->createButton(createOEFunc, "createOE", "createOE.png", vec2{ -50.0,-12.5 }, vec2{ 50.0,12.5 }, vec2{0.0, -180.0}, GUI_POS_MID_MID, 1.0);
		this->deleteButton("createRV");
	};
	std::function<void()> rvSelButton = [this, createRVFunc](){
		this->deleteInput("ain"); // a
		this->deleteInput("ein"); // e
		this->deleteInput("iin"); // i
		this->deleteInput("Oin"); // Omega
		this->deleteInput("win"); // omega
		this->deleteInput("nin"); // nu
		this->textMap.erase("tempA");
		this->textMap.erase("tempE");
		this->textMap.erase("tempI");
		this->textMap.erase("tempO");
		this->textMap.erase("tempW");
		this->textMap.erase("tempN");

		this->createInput("xin",  vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 100.0}, GUI_POS_MID_MID, 0.5); // x
		this->createInput("yin",  vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 080.0}, GUI_POS_MID_MID, 0.5); // y
		this->createInput("zin",  vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 060.0}, GUI_POS_MID_MID, 0.5); // z
		this->createInput("dxin", vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 040.0}, GUI_POS_MID_MID, 0.5); // dx
		this->createInput("dyin", vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 020.0}, GUI_POS_MID_MID, 0.5); // dy
		this->createInput("dzin", vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 000.0}, GUI_POS_MID_MID, 0.5); // dz
		this->textInputs["xin"]->textToDraw = "7000";
		this->textInputs["yin"]->textToDraw = "0.0";
		this->textInputs["zin"]->textToDraw = "0.0";
		this->textInputs["dxin"]->textToDraw = "0.0";
		this->textInputs["dyin"]->textToDraw = "7.5";
		this->textInputs["dzin"]->textToDraw = "0.0";
		this->textMap["tempX"]  = GUIText(U"x",      vec2{ -240.0, 100.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);
		this->textMap["tempY"]  = GUIText(U"y",      vec2{ -240.0, 080.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);
		this->textMap["tempZ"]  = GUIText(U"z",      vec2{ -240.0, 060.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);
		this->textMap["tempDX"] = GUIText(U"\u1E8B", vec2{ -240.0, 040.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);
		this->textMap["tempDY"] = GUIText(U"\u1E8F", vec2{ -240.0, 020.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);
		this->textMap["tempDZ"] = GUIText(U"\u017C", vec2{ -240.0, 000.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);

		this->createButton(createRVFunc, "createRV", "createRV.png", vec2{ -50.0,-12.5 }, vec2{ 50.0,12.5 }, vec2{0.0, -180.0}, GUI_POS_MID_MID, 1.0);
		this->deleteButton("createOE");
	};


	
	
	//-----------BUTTON TO CREATE SPACECRAFT --------- (very recursive)
	this->createButton([this, createOEFunc, createRVFunc, oeSelButton, rvSelButton, texIdOn, texIdOff](){
		std::wstring_convert<codecvt<char32_t,char,std::mbstate_t>,char32_t> convert32;	

		this->buttonMap.at("addSpacecraft")->enabled = false;

		this->createStaticBox("createSC", vec2{-250.0, -200.0}, vec2{250.0, 300.0}, vec2{0.0,0.0}, GUI_POS_MID_MID);

		this->textMap["tempCreate"] = GUIText(U"Create a New Spacecraft", vec2{ 0.0, 290.0 }, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID);
		
		this->textMap["tempSCName"] = GUIText(U"Name", vec2{-135.0,260.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_RIGHT);
		this->createInput("namein", vec2{-120.0, -15.0}, vec2{120.0, 15.0}, vec2{ 0.0, 260.0}, GUI_POS_MID_MID);
		this->textInputs["namein"]->textToDraw = "Centaur";

		this->selectorMap["oeORrv"] = new Selector(this, U"Initial Conditions", vec2{-240.0, 220.0}, GUI_POS_MID_MID, texIdOn, texIdOff, winWidth, winHeight);
		this->selectorMap.at("oeORrv")->addButton(U"Orbital Elements", oeSelButton);
		this->selectorMap.at("oeORrv")->addButton(U"Position Velocity", rvSelButton);
		this->selectorMap.at("oeORrv")->setActiveElem(U"Orbital Elements");

		this->selectorMap["bfORin"] = new Selector(this, U"Reference Frame", vec2{-240.0, 170.0}, GUI_POS_MID_MID, texIdOn, texIdOff, winWidth, winHeight);
		this->selectorMap.at("bfORin")->addButton(U"Body Fixed", [](){void();});
		this->selectorMap.at("bfORin")->addButton(U"J2000", [](){void();});
		this->selectorMap.at("bfORin")->addButton(U"Ecliptic J2000", [](){void();});
		this->selectorMap.at("bfORin")->setActiveElem(U"Ecliptic J2000");

		this->createInput("ain", vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 100.0}, GUI_POS_MID_MID, 0.5); // a
		this->createInput("ein", vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 080.0}, GUI_POS_MID_MID, 0.5); // e
		this->createInput("iin", vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 060.0}, GUI_POS_MID_MID, 0.5); // i
		this->createInput("Oin", vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 040.0}, GUI_POS_MID_MID, 0.5); // Omega
		this->createInput("win", vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 020.0}, GUI_POS_MID_MID, 0.5); // omega
		this->createInput("nin", vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{ -180.0, 000.0}, GUI_POS_MID_MID, 0.5); // nu
		this->textInputs["ain"]->textToDraw = "200000";
		this->textInputs["ein"]->textToDraw = "0.96";
		this->textInputs["iin"]->textToDraw = "18";
		this->textInputs["Oin"]->textToDraw = "65";
		this->textInputs["win"]->textToDraw = "167";
		this->textInputs["nin"]->textToDraw = "0";
		this->textMap["tempA"] = GUIText(U"a",      vec2{ -240.0, 100.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);
		this->textMap["tempE"] = GUIText(U"e",      vec2{ -240.0, 080.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);
		this->textMap["tempI"] = GUIText(U"i",      vec2{ -240.0, 060.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);
		this->textMap["tempO"] = GUIText(U"\u03A9", vec2{ -240.0, 040.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);
		this->textMap["tempW"] = GUIText(U"\u03C9", vec2{ -240.0, 020.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);
		this->textMap["tempN"] = GUIText(U"\u03c5", vec2{ -240.0, 000.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.35);  

		this->selectorMap["tempParent"] = new Selector(this, U"Relative To:", vec2{-225.0,-60.0}, GUI_POS_MID_MID, texIdOn, texIdOff, this->winWidth, this->winHeight);
		this->selectorMap["tempOrbParent"] = new Selector(this, U"Orbit Lines Body", vec2{-115.0,225.0}, GUI_POS_MID_MID, texIdOn, texIdOff, this->winWidth, this->winHeight);
		for(auto const& [key, planet] : solarSystem->Planets){
			PlanetaryBody* pnet = planet; // cannot caputure the const& planet from the for loop in a lambda
			this->selectorMap.at("tempParent")->addButton(convert32.from_bytes(planet->name.c_str()), [](){void();});
			this->selectorMap.at("tempOrbParent")->addButton(convert32.from_bytes(planet->name.c_str()), [](){void();});
			for(auto const& moon : pnet->children){
				PlanetaryBody* mnet = moon;
				this->selectorMap.at("tempParent")->addButton(convert32.from_bytes(moon->name.c_str()), [](){void();});
				this->selectorMap.at("tempOrbParent")->addButton(convert32.from_bytes(moon->name.c_str()), [](){void();});
			}
		}

		this->selectorMap.at("tempParent")->setActiveElem(convert32.from_bytes(this->parentCam->getCurBody()->name));

		//FOR TESTING ONLY. DELETE
		this->selectorMap.at("tempOrbParent")->setActiveElem(U"moon"); 

		this->createInput("durin", vec2{-90.0, -15.0}, vec2{90.0, 15.0}, vec2{-180.0, -40.0}, GUI_POS_MID_MID, 0.5);
		this->textInputs["durin"]->textToDraw = "10";
		this->textMap["tempDur"] = GUIText(U"Duration of flight", vec2{-180.0, -20.0}, GUI_POS_MID_MID, GUI_TEXT_ANCHOR_MID, 0.3);

		this->createButton(createOEFunc, "createOE", "createOE.png", vec2{ -50.0,-12.5 }, vec2{ 50.0,12.5 }, vec2{0.0, -180.0}, GUI_POS_MID_MID, 1.0);
		

	}, "addSpacecraft", "add_spacecraft.png",vec2{ -50.0,-12.5 }, vec2{ 50.0,12.5 }, vec2{ -125.0, -20.0 }, GUI_POS_UPPER_RIGHT, 2.0 );
	

	// function layout:
	//createButton(callbackPtr callbackFunc, string key, string texPath, vec2 bb_ll, vec2 bb_ru, vec2 offset, int location, float scale) {
	
	// #############################################################################
	// ######################### END INITIALIZATION ################################
}

void GUI::RenderText(std::u32string text, GLfloat x, GLfloat y, GLfloat scale, vec3 color, float depth)
{
	GLfloat h = this->getStringHeight(text, scale);
	y-=h/2.0;

	// Activate corresponding render state
	this->textShader->use();
	this->textShader->setUniformV3("textColor", color);
	this->textShader->setUniform1i("text", 2);
	this->textShader->setUniform1f("depth", depth);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glBindVertexArray(textVAO);
	

	// Iterate through all characters
	std::string::const_iterator c;
	//for (c = text.begin(); c != text.end(); c++)

	for (char32_t c : text)
	{
		Character ch = Characters[(GLuint)c];

		GLfloat xpos = x + ch.Bearing[0] * scale;
		GLfloat ypos = y - (ch.Size[1] - ch.Bearing[1]) * scale;

		GLfloat w = ch.Size[0] * scale;
		GLfloat h = ch.Size[1] * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
		{ xpos,     ypos,       0.0, 1.0 },
		{ xpos + w, ypos,       1.0, 1.0 },

		{ xpos,     ypos + h,   0.0, 0.0 },
		{ xpos + w, ypos,       1.0, 1.0 },
		{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render glyph texture over quad
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_CULL_FACE);
}
void GUI::RenderText(string text, GLfloat x, GLfloat y, GLfloat scale, vec3 color, float depth)
{	
	GLfloat h = this->getStringHeight(text, scale);
	y-=h/2.0;

	// Activate corresponding render state
	this->textShader->use();
	this->textShader->setUniformV3("textColor", color);
	this->textShader->setUniform1i("text", 2);
	this->textShader->setUniform1f("depth", depth);

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glBindVertexArray(textVAO);
	

	// Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{

		Character ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing[0] * scale;
		GLfloat ypos = y - (ch.Size[1] - ch.Bearing[1]) * scale;

		GLfloat w = ch.Size[0] * scale;
		GLfloat h = ch.Size[1] * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
		{ xpos,     ypos + h,   0.0, 0.0 },
		{ xpos,     ypos,       0.0, 1.0 },
		{ xpos + w, ypos,       1.0, 1.0 },

		{ xpos,     ypos + h,   0.0, 0.0 },
		{ xpos + w, ypos,       1.0, 1.0 },
		{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render glyph texture over quad
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_CULL_FACE);
}

void GUI::setUniform4dv(const char * uniformName, double * uniformPtr) {
	this->textShader->setUniform4dv(uniformName, uniformPtr);
	this->guiShader->setUniform4dv(uniformName, uniformPtr);
}

//Window stuff

void GUI::resize(int width, int height) {
	this->winHeight = height;
	this->winWidth = width;

	for (auto const& x : this->buttonMap) {
		x.second->updateVertices(width, height);
	}
	for (auto const& [key, box] : this->statBoxes){
		box->updateVertices(width, height);
	}
	for (auto const& [key, input] : this->textInputs){
		input->updateVertices(width, height);
	}
	this->setUniform4dv("ortho", this->parentCam->getOrthoPtr());
}

//state sets

void GUI::setCurParent(PlanetaryBody * body) {
	this->curBody = body;
	this->curCraft = NULL;
	string name = this->curBody->name;
	name[0] = toupper(name[0]);

}


void GUI::setCurParent(Spacecraft * body) {
	this->curBody = NULL;
	this->curCraft = body;
	string name = this->curCraft->name;
}

// Text functions

void GUI::createText(string key, GUIText textStruc) {
	this->textMap[key] = textStruc;
}

void GUI::updateText(string key, string text) {
	std::wstring_convert<codecvt<char32_t,char,std::mbstate_t>,char32_t> convert32;
	std::u32string text32 = convert32.from_bytes(text);
	this->textMap[key].textToDraw = text32;
}

void GUI::deleteText(string key) {
	this->textMap.erase(key);
}
void GUI::text_callback(GLFWwindow* window, unsigned int codepoint){
	if(this->activeInput!=NULL){
		this->activeInput->addChar(codepoint);
	}
}
void GUI::createInput(string key, vec2 bb_ll, vec2 bb_ur, vec2 offset, int location, float scale){
	TextInput* input = new TextInput(this, bb_ll, bb_ur, offset, location, this->winWidth, this->winHeight, scale);
	this->textInputs[key] = input;
}

void GUI::deleteInput(string key){
	this->textInputs.erase(key);
}

GLfloat GUI::getStringWidth(std::u32string text, float scale = 1.0){
	GLfloat x = 0.0;
	std::u32string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];
		GLfloat xpos = x + ch.Bearing[0] * scale;
		GLfloat w = ch.Size[0] * scale;
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
	}
	return x;
}
GLfloat GUI::getStringHeight(std::u32string text, float scale = 1.0){
	auto c = text.begin();
	Character ch = Characters[*c];
	GLfloat h = ch.Size[1]*scale;
	return h;
}
GLfloat GUI::getStringHeight(string text, float scale = 1.0){
	auto c = text.begin();
	Character ch = Characters[*c];
	GLfloat h = ch.Size[1]*scale;
	return h;
}

// Button functions

void GUI::createButton(std::function<void ()> callbackFunc, string key, string texPath, vec2 bb_ll, vec2 bb_ru, vec2 offset, int location, float scale) {
	GLuint texId = this->loadTexture(key, texPath);
	this->buttonMap[key] = new GUIButton(callbackFunc, bb_ll, bb_ru, offset, location, texId, winWidth, winHeight, scale);
}

void GUI::updateButton(string key) {
	this->buttonMap.at(key)->updateVertices(winWidth, winHeight);
}

void GUI::deleteButton(string key) {
	this->buttonMap.erase(key);
}

void GUI::createStaticBox(string key, vec2 bb_ll, vec2 bb_ur, vec2 offset, int location, float scale){
	StaticBox* box = new StaticBox(this, bb_ll, bb_ur, offset, location, this->winWidth, this->winHeight, scale);
	this->statBoxes[key] = box;
}
void GUI::deleteStaticBox(string key){
	this->statBoxes.erase(key);
}

void GUI::click() {
	for (auto & [key, button] : this->buttonMap) {
		if (button->checkClick(mousex, winHeight - mousey, winWidth, winHeight)) {
			if(button->enabled){
				button->onClick();
				break;
			}
		}
	}
	for (auto const& [key, input] : this->textInputs){
		if (input->checkClick(mousex, winHeight-mousey, winWidth, winHeight))
			input->onClick();
	}
	for (auto const& [key, sel] : this->selectorMap){
		sel->checkClick(mousex, winHeight-mousey, winWidth, winHeight);
	}
}

void GUI::hover(){
	
	for (auto const& [key, button] : this->buttonMap) {
		if(button->enabled){
			if (button->checkClick(mousex, winHeight - mousey, winWidth, winHeight)){
				button->onHover();
			}
			else{
				button->hoverOff();
			}
		}
	}
}

void GUI::draw() {
	this->txtInputShader->use();
	for(auto const& [key, box] : this->statBoxes){
		box->draw();
	}

	this->txtInputShader->use();
	for(auto const& [key, input] : this->textInputs){
		input->draw();
	}

	this->guiShader->use();
	this->guiShader->setUniform1i("tex_0", 2);
	for (auto const& [key, button] : this->buttonMap) {
		if(button->enabled)
			button->draw();
	}

	this->selShader->use();
	this->selShader->setUniform4dv("ortho", this->parentCam->getOrthoPtr());
	this->selShader->setUniform1i("tex_0", 2);
	for(auto const& [key, selector] : this->selectorMap){
		selector->draw();
	}

	for (auto const& [key, item] : this->textMap) {
		if(item.draw){
			GLfloat x = 0.0;
			if (item.centering == GUI_TEXT_ANCHOR_MID)
				x = -getStringWidth(item.textToDraw, item.scale*this->guiScale)/2.0;
			if (item.centering == GUI_TEXT_ANCHOR_RIGHT)
				x = -getStringWidth(item.textToDraw, item.scale*this->guiScale);
			switch (item.location) {
			case GUI_POS_BOTTOM_LEFT:
				this->RenderText(item.textToDraw, item.offset[0] + x, item.offset[1], item.scale*this->guiScale, item.color, item.depth);
				break;
			case GUI_POS_BOTTOM_RIGHT:
				this->RenderText(item.textToDraw, this->winWidth + item.offset[0] + x, item.offset[1], item.scale*this->guiScale, item.color, item.depth);
				break;
			case GUI_POS_BOTTOM_MID:
				this->RenderText(item.textToDraw, this->winWidth/2.0 + item.offset[0] + x, item.offset[1], item.scale*this->guiScale, item.color, item.depth);
				break;
			case GUI_POS_UPPER_LEFT:
				this->RenderText(item.textToDraw, item.offset[0] + x, this->winHeight + item.offset[1], item.scale*this->guiScale, item.color, item.depth);
				break;
			case GUI_POS_UPPER_RIGHT:
				this->RenderText(item.textToDraw, this->winWidth + item.offset[0] + x, this->winHeight + item.offset[1], item.scale*this->guiScale, item.color, item.depth);
				break;
			case GUI_POS_UPPER_MID:
				this->RenderText(item.textToDraw, this->winWidth/2.0 + item.offset[0] + x, this->winHeight + item.offset[1], item.scale*this->guiScale, item.color, item.depth);
				break;
			case GUI_POS_MID_MID:
				this->RenderText(item.textToDraw, this->winWidth/2.0 + item.offset[0] + x, this->winHeight/2.0 + item.offset[1], item.scale*this->guiScale, item.color, item.depth);
				break;
			case GUI_POS_MID_LEFT:
				this->RenderText(item.textToDraw, item.offset[0] + x, this->winHeight/2.0 + item.offset[1], item.scale*this->guiScale, item.color, item.depth);
				break;
			case GUI_POS_MID_RIGHT:
				this->RenderText(item.textToDraw, this->winWidth + item.offset[0] + x, this->winHeight/2.0 + item.offset[1], item.scale*this->guiScale, item.color, item.depth);
				break;
			}
		}
	}



	
}


