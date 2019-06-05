#include "Camera.h"

void Camera::setSpeed(float speed) {
	this->moveSpeed = speed;
}

Camera::Camera(GLFWwindow * window, int width, int height, double julianTime) {
	this->winWidth = width;
	this->winHeight = height;
	this->curBody = solarSystem->Planets["earth"];
	this->aspect = 16.0 / 9.0;
	this->nearClip = 100;
	this->farClip = 10E8;
	this->fovy = 45.0;
	this->flare = new LensFlare(this);
	this->gui = new GUI(this);
	this->frameResize(width, height);

	
	this->gui->setCurParent(this->curBody);

	this->planetIndex = curBody->systemIndex;
	this->curMesh = curBody->bodyMesh;
	vec3 scale = this->curMesh->getScale();
	this->distance = scale[0] * 2.5 + this->nearClip;
	this->moveSpeed = this->distance / 2.0;

	string latStr = "Lat: " + to_string(degrees(this->inlat));
	this->gui->updateText("lat", latStr);
	string lonStr = "Lon: " + to_string(degrees(this->inlon));
	this->gui->updateText("lon", lonStr);
	string altStr = "Alt: " + to_string(this->distance);
	this->gui->updateText("alt", altStr);

	string simString = to_string(this->simSpeed);
	//simString = "Simspeed: " + simString.substr(0, 4) + "x";
	//this->gui->updateText("simspeed", simString);
	this->gui->updateText("simspeed", "Simspeed: Paused");
	printf("Camera created!\n");
}

vec3 Camera::getPosition() {
	return this->position;
}

mat4 Camera::getViewMat() {
	return this->viewMat;
}

void Camera::addPos(vec3 pos) {
	this->position[0] += pos[0];
	this->position[1] += pos[1];
	this->position[2] += pos[2];
}

void Camera::setPos(vec3 pos) {
	this->position = pos;
}

void Camera::lookAt(vec3 target) {
	this->shpFw = Normalize( this->getPosition() - target);

	this->shpLf = Normalize(Cross(this->globalUp, this->shpFw));
	this->shpUp = Normalize(Cross(this->shpFw, this->shpLf));
}

void Camera::updateViewMat() {
	//this->setPos(solarSystem->Planets[bodyName]->getPosition() + this->distance*vec3{ cos(inlon)*cos(inlat), sin(inlat), sin(inlon)*cos(inlat) });
	this->setPos(this->curMesh->getPos() + this->distance*vec3{ cos(inlat)*cos(inlon), cos(inlat)*sin(inlon), sin(inlat) });
	this->lookAt(this->curMesh->getPos());

	vec3 pos = this->getPosition();
	this->viewMat[0] = { shpFw[0],shpFw[1] ,shpFw[2], 0.0 };
	this->viewMat[1] = { shpLf[0],shpLf[1] ,shpLf[2], 0.0 };
	this->viewMat[2] = { shpUp[0],shpUp[1] ,shpUp[2], 0.0 };
	this->viewMat[3] = { 0.0, 0.0, 0.0, 1.0};

	this->viewMat = this->viewMat * TransMat(-1.0*pos);

}

void Camera::updateShaderUniforms() {
	solarSystem->setUniformV3("camPos", this->getPosPtr());
	this->updateViewMat();
	solarSystem->skybox->setPos(this->getPosition());
	solarSystem->setUniform4dv("view", this->getViewPtr());
	solarSystem->setUniformV3("camPos", this->getPosPtr());
	for(auto const& [key, planet] : solarSystem->Planets){
		float height = (len(this->position - planet->position));
		planet->bodyMat->matShader->setUniform1f("fCameraHeight2", pow(height,2));
		planet->bodyMat->matShader->setUniformV3("planPos", planet->position);
		planet->bodyMat->matShader->setUniformV3("camPos", this->position);
		if(planet->atmoMat!=NULL){
			planet->atmoMat->matShader->setUniform1f("fCameraHeight2", pow(height,2));
			planet->atmoMat->matShader->setUniformV3("planPos", planet->position);
			planet->atmoMat->matShader->setUniformV3("camPos", this->position);
		}
	}
}

void Camera::drawFlare(){
	this->flare->draw();
}

void Camera::frameResize(int width, int height) {
	this->winWidth = width;
	this->winHeight = height;
	this->aspect = (float)width / (float)height;
	this->projectionMat = ProjectionMat(radians(this->fovy), this->aspect, this->nearClip, this->farClip);
	this->craftProjMat = ProjectionMat(radians(this->fovy), this->aspect, 0.0001, 1000.0);
	this->orthoMat = OrthographicMat(0.0, width, 0.0, height, 0.0, 1.0);
	this->gui->resize(width, height);
	this->flare->updateOrtho(this->getOrthoPtr());
}

double * Camera::getProjPtr() {
	return &projectionMat[0][0];
}

mat4 Camera::getProjMat(){
	return this->projectionMat;
}

mat4 Camera::getOrthoMat(){
	return this->orthoMat;
}

double * Camera::getCraftProjPtr() {
	return &craftProjMat[0][0];
}

double * Camera::getOrthoPtr() {
	return &orthoMat[0][0];
}

double * Camera::getViewPtr() {
	return &viewMat[0][0];
}

double * Camera::getPosPtr() {
	return &position[0];
}

double Camera::getSimSpeed() {
	return this->simSpeed * this->isPlaying;
}

void Camera::setCurCraft(Spacecraft* craft){
	this->curCraft = craft;
}

//Takes mouse movement and translates it to latitude and longitude changes.

void Camera::mouse_callback(GLFWwindow * window, double xpos, double ypos) {

	if (mHeld) {
		this->inlon -= this->mouseSens*(xpos - this->xprev);
		this->inlat += this->mouseSens*(ypos - this->yprev);
	}

	if (inlat > radians(88.0))
		inlat = radians(88.0);
	else if (inlat < radians(-88.0))
		inlat = radians(-88.0);

	if (inlon > radians(360.0))
		inlon -= radians(360.0);
	else if (inlon < 0.0)
		inlon += radians(360.0);

	this->xprev = xpos;
	this->yprev = ypos;

	string latStr = "Lat: " + to_string(degrees(this->inlat));
	this->gui->updateText("lat", latStr);
	string lonStr = "Lon" + to_string(degrees(this->inlon));
	this->gui->updateText("lon", lonStr);
	
	glfwGetCursorPos(window, &this->gui->mousex, &this->gui->mousey);
	this->gui->hover();
}

//key press input

void Camera::key_callback(GLFWwindow * window, int key, int scancode, int action, int mods) {
	// ESCAPE
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS){
		this->gui->activeInput->removeChar();
	}


	// resetting GUI

	else if (this->curCraft != NULL){
		this->gui->setCurParent(this->curCraft);
	}
	string latStr = "Lat: " + to_string(degrees(this->inlat));
	this->gui->updateText("lat", latStr);
	string lonStr = "Lon" + to_string(degrees(this->inlon));
	this->gui->updateText("lon", lonStr);

}

//scroll callback

void Camera::scroll_callback(GLFWwindow * window, double xoffset, double yoffset) {
	this->moveSpeed = this->distance / 4.0;
	this->distance -= yoffset * this->moveSpeed;
	vec3 bodyScale = this->curMesh->getScale();
	if (this->distance < (bodyScale[0]*1.1))
		this->distance = (bodyScale[0]*1.1);
	if (this->distance > 5000000000.0f)
		this->distance = 5000000000.0f;
}

//mouse button callback

void Camera::mouse_button_callback(GLFWwindow * window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		mHeld = true;

	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		mHeld = false;
		this->gui->click();
	}

}

void Camera::pause(){
	this->isPlaying = 0;
	this->gui->updateText("simspeed", "Simspeed: Paused");
}

void Camera::play(){
	this->isPlaying = 1;
	string simString = to_string((int)this->simSpeed);
	simString = "Simspeed: " + simString + "x";
	this->gui->updateText("simspeed", simString);
}

void Camera::togglePause() {
	if (this->isPlaying == 1){
		this->pause();
	}
	else{
		this->play();
	}
}

void Camera::fastForward(){
	double speeds []= {1.0, 2.0, 5.0, 10.0, 25.0, 50.0, 100.0, 500.0, 1000.0, 2000.0, 5000.0, 10000.0, 1.0e5, 1.0e6};
	if (this->simSpeed < 0.0 || this->isPlaying==0){
		this->simSpeed = speeds[0];
		this->isPlaying = true;
	}else{
		for(int i = 0; i<(sizeof(speeds)/sizeof(*speeds)); ++i){
			if(i == (sizeof(speeds)/sizeof(*speeds)) - 1){
				this->simSpeed = speeds[0];
				this->isPlaying = true;
				break;
			} else if (abs(this->simSpeed - speeds[i]) < 0.1){
				this->simSpeed = speeds[i+1];
				this->isPlaying = true;
				break;
			}
		}
	}
	string simString = to_string((int)this->simSpeed);
	simString = "Simspeed: " + simString + "x";
	this->gui->updateText("simspeed", simString);
}
void Camera::rewind(){
	double speeds []= {-1.0, -2.0, -5.0, -10.0, -25.0, -50.0, -100.0, -500.0, -1000.0};
	if (this->simSpeed > 0.0 || this->isPlaying == 0){
		this->simSpeed = speeds[0];
		this->isPlaying = true;
	}else{
		for(int i = 0; i<(sizeof(speeds)/sizeof(*speeds)); ++i){
			if(i == (sizeof(speeds)/sizeof(*speeds)) - 1){
				this->simSpeed = speeds[0];
				this->isPlaying = true;
				break;
			} else if (abs(this->simSpeed - speeds[i]) < 0.1){
				this->simSpeed = speeds[i+1];
				this->isPlaying = true;
				break;
			}
		}
	}
	string simString = to_string((int)this->simSpeed);
	simString = "Simspeed: " + simString + "x";
	this->gui->updateText("simspeed", simString);
}

void Camera::selectBody(PlanetaryBody* body){
	this->curBody = body;
	this->curMesh = this->curBody->bodyMesh;
	this->curCraft = NULL;
	this->curMission = NULL;
	vec3 bodyScale = this->curMesh->getScale();
	this->distance = bodyScale[0] * 2.5 + this->nearClip;
	this->moveSpeed = this->distance / 2.0;
	this->inlat = radians(30.0);
	this->inlon = radians(240.0);
	// resetting GUI
	if (this->curBody != NULL){
		this->gui->setCurParent(this->curBody);
	}
	else if (this->curCraft != NULL){
		this->gui->setCurParent(this->curCraft);
	}
	string latStr = "Lat: " + to_string(degrees(this->inlat));
	this->gui->updateText("lat", latStr);
	string lonStr = "Lon" + to_string(degrees(this->inlon));
	this->gui->updateText("lon", lonStr);
}
void Camera::selectBody(Spacecraft* craft){
	this->curBody = NULL;
	this->curMission = NULL;
	this->curCraft = craft;
	this->curMesh = this->curCraft->bodyMesh;

	vec3 bodyScale = this->curMesh->getScale();
	this->distance = bodyScale[0] * 2.5 + this->nearClip;
	this->moveSpeed = this->distance / 2.0;
	this->inlat = radians(30.0);
	this->inlon = radians(240.0);
	// resetting GUI
	if (this->curBody != NULL){
		this->gui->setCurParent(this->curBody);
	}
	else if (this->curCraft != NULL){
		this->gui->setCurParent(this->curCraft);
	}
	string latStr = "Lat: " + to_string(degrees(this->inlat));
	this->gui->updateText("lat", latStr);
	string lonStr = "Lon" + to_string(degrees(this->inlon));
	this->gui->updateText("lon", lonStr);
}
void Camera::selectMission(Mission* mission){
	this->curMission=mission;
}
void Camera::selectStage(substage stg){
	this->curMesh = stg.obj;
	this->distance = this->curMesh->getScale()[0] * 2.5;
}

//#################################################################################

// LENS FLARE

LensFlare::LensFlare(Camera* cam){
    this->mainCam = cam;
    this->lfShader = new Shader();
    this->lfShader->loadShader("lensFlare");

		this->lfShader->setUniform1i("tex_0", 2);
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);


		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(std::array<float,4>), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);

		this->LoadTextures();
    }

void LensFlare::LoadTextures(){
	//texture slot 0 reserved for loading
	glActiveTexture(GL_TEXTURE0);
	//Not super sure. I think OpenGL flips textures or something.
	stbi_set_flip_vertically_on_load(true);

	//add texture directory
	string directory = "./textures/GUI/LF/";
	string filename;

	for (int i = 0; i < nrFlares; ++i){
		int width, height, channels;
		filename = directory + std::to_string(i) + ".png";
		//Create the texture object within OpenGL and add it to the dictionary
		//generates the texture and tells opengl to store the texture ID in the next available slot in the texture dictionary
		glGenTextures(1, &(this->texIds[i]));
		glBindTexture(GL_TEXTURE_2D, this->texIds[i]);
		// Sets the wrapping mode to standard repeat
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Sets the interpolation to nearest, with linear mipmap interpolation
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//use stb_image library to load the texture sent in by filename

		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

		if (data != NULL) {
			if (channels == 3) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			}
			else if (channels == 4) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			}
			else {
				cout << "ERROR: " + filename + " has an unknown file type. Possibly too many channels " << endl;
			}

			//glGenerateMipmap(GL_TEXTURE_2D);
			cout << "Loaded texture file " + filename << " with " << channels << " channels." << endl;
		}
		else {
			cout << "ERROR: Failed to load " + filename << endl;
		}

		stbi_image_free(data);

	}
}

void LensFlare::updateOrtho(double* ortho){
	this->lfShader->setUniform4dv("ortho", ortho);
}

bool LensFlare::isOccluded(){
	return false;
}

void LensFlare::draw(){
	this->lfShader->use();

	PlanetaryBody* sun = solarSystem->Planets["sun"];

	//successively transform the position of the suns near side in global coordinates to that of clip and screen coordinates
	vec4 sunPosGlob = {sun->position[0], sun->position[1], sun->position[2], 1.0};
	vec4 sunPosView = (mainCam->getViewMat()) * sunPosGlob;
	// sets the position to near side instead of center
	sunPosView[0] += sun->bodyMesh->getScale()[0]; 
	vec4 sunPosFrust = (mainCam->getProjMat()) * sunPosView;
	vec4 sunPosClip = sunPosFrust / sunPosFrust[3];
	//transform to window coordinates
	ivec2 sunPosWin = {(int) ((sunPosClip[0] + 1.0f)*mainCam->getWinWidth()/2.0), (int) ((sunPosClip[1] + 1.0f)*mainCam->getWinHeight()/2.0)};
	//get the depth value at the suns window coordinates
	GLfloat scale, sunPosDepthArr[4], sunPosDepth;
	glReadPixels(sunPosWin[0], sunPosWin[1], 2, 2, GL_DEPTH_COMPONENT, GL_FLOAT, sunPosDepthArr);
	//test if the expected value of the depth is greater than the drawn one. essentially tests if the sun has been drawn over
	sunPosDepth = std::max(std::max(sunPosDepthArr[0], sunPosDepthArr[1]), std::max(sunPosDepthArr[2], sunPosDepthArr[3]));
	if(((float)sunPosClip[2]/sunPosDepth) < 1.00 && (float)sunPosClip[2]/sunPosDepth > 0.95){
		scale = 1.0e4*sqrt(atan(sun->bodyMesh->getScale()[0]/len(mainCam->getPosition() - sun->position)));
	}else{ //scaling function here found with experimentation
		scale = 0.0;
	}
	vec2f coord = vec2f{(float)sunPosClip[0]+1.0f, (float)sunPosClip[1]+1.0f};
	// sets the position and scale parameters of the shader
	this->lfShader->setUniform1f("scale", scale);
	this->lfShader->setUniformV2("screenPos", coord);

	// actually draw the quads
	glActiveTexture(GL_TEXTURE0+2);
	glBindVertexArray(VAO);
	glDisable(GL_DEPTH_TEST);
	for(int i = 0; i < nrFlares; ++i){
		glBindTexture(GL_TEXTURE_2D, this->texIds[i]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
	}

