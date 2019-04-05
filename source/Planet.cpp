#include "Planet.h"
#include "Globals.h"

#if defined _WIN32 || defined _WIN64
#define fscanf fscanf_s
#endif

double SOLAR_MU;

int intcnt = 0;

PlanetaryBody::PlanetaryBody(string name, string idCode, string shaderType, bool specular, bool normal, bool rings, bool atmosphere, PlanetaryBody* parentBody) {
	this->parentBody = parentBody;
	this->name = name;
	this->idCode = idCode;
	loadEphemeris(name);

	texHandler->loadTexture(name, specular, normal);
	bodyMat = new Material(shaderType, name);
	bodyMesh = new SceneObject("uvsphere");
	

	if (atmosphere) {
		this->atmoMat = new Material("atmosphere", "NULL");
		this->atmoMesh = new SceneObject("atmosphere");
	}

	if (rings) {
		texHandler->loadTexture(name + "_rings");
		this->ringMesh = new SceneObject("rings");
		this->ringMat = new Material("ring", name + "_rings");
	}

	if (this->bodyMesh != NULL)
		this->bodyMat->addObj(this->bodyMesh);
	if (this->atmoMesh != NULL)
		this->atmoMat->addObj(this->atmoMesh);
	if (this->ringMesh != NULL)
		this->ringMat->addObj(this->ringMesh);


	// SCALE SETTING
	if (properties["radius"]) {
		if (bodyMesh != NULL) {
			bodyMesh->setScale(properties["radius"]);
			cout << "Setting " << name << " to scale of " << properties["radius"] << endl;
		}
		if (atmoMesh != NULL)
			atmoMesh->setScale(properties["radius"]*1.025);
		if (ringMesh != NULL)
			ringMesh->setScale(properties["radius"]);
	}
	else if (properties["equ_radius"] && properties["pol_radius"]) {
		bodyMesh->setScale(vec3{ properties["equ_radius"],properties["equ_radius"],properties["pol_radius"] });
		if (atmoMesh != NULL)
			atmoMesh->setScale(1.025*vec3{ properties["equ_radius"],1.025*properties["equ_radius"],properties["pol_radius"]*1.025 });
		if (ringMesh != NULL)
			ringMesh->setScale(properties["equ_radius"]);
		cout << "Setting " << name << " to scale of " << properties["pol_radius"] << " polar, and " << properties["equ_radius"] << " equatorial" << endl;;
	}
	else {
		cout << "ERROR: Failed to find scale data for " << name << endl;
	}

	// MASS SETTING

	if (properties["GM"]) {
		this->mu = properties["GM"];
		printf("MU FOR ");
		printf("%s",name.c_str());
		printf(" = %lg \n", mu);
	}
	if (name == "sun")
		SOLAR_MU = properties["GM"];

	float sunStr = 5.0;
	float sunLin = 5e-9;
	float sunQuad = 0.0;


	if (bodyMat != NULL) {
		bodyMat->matShader->setUniformV3("sun.color", vec3f{ 1.0f,1.0f,0.95f });
		bodyMat->matShader->setUniformV3("sun.position", vec3{ 0.0,0.0,0.0 });
		bodyMat->matShader->setUniform1f("sun.attn_str", sunStr);
		bodyMat->matShader->setUniform1f("sun.attn_lin", sunLin);
		bodyMat->matShader->setUniform1f("sun.attn_quad", sunQuad);

		
	}
	if (ringMat != NULL) {
		ringMat->matShader->setUniformV3("sun.color", vec3f{ 1.0f,1.0f,0.95f });
		ringMat->matShader->setUniformV3("sun.position", vec3{ 0.0,0.0,0.0 });
		ringMat->matShader->setUniform1f("sun.attn_str", sunStr);
		ringMat->matShader->setUniform1f("sun.attn_lin", sunLin);
		ringMat->matShader->setUniform1f("sun.attn_quad", sunQuad);
	}
	if (atmoMat != NULL) {
		float innerRadius = this->bodyMesh->getScale()[0];
		float outerRadius = this->atmoMesh->getScale()[0];
		float atmoHeight = outerRadius-innerRadius;
		atmoMat->matShader->setUniform1f("fInnerRadius", innerRadius);
		atmoMat->matShader->setUniform1f("fOuterRadius", outerRadius );
		atmoMat->matShader->setUniform1f("fOuterRadius2", pow((outerRadius),2));
		bodyMat->matShader->setUniform1f("fInnerRadius", innerRadius);
		bodyMat->matShader->setUniform1f("fOuterRadius", outerRadius );
		bodyMat->matShader->setUniform1f("fOuterRadius2", pow((outerRadius),2));
	}
	
}	

void PlanetaryBody::loadEphemeris(string body) {
	string path = "Ephemeris/" + body + ".epm";
	FILE* file;

	#ifdef __APPLE__
	file = fopen(path.c_str(), "r");
	bool err = (file==NULL);
	#elif defined _WIN32 || defined _WIN64
	errno_t err = fopen_s(&file, path.c_str(), "r");
	#endif

	if (err) {
		cout << "Could not open file: " << path << endl;
		return;
	}
	cout << "Beginning ephemeris read of " << path << endl;
	while (1) {
		char key[64];
		key[63] = 0;

		#ifdef __APPLE__ 
		int res = fscanf(file, "%63s", key);
		#elif defined _WIN64 || defined _WIN32
		int res = fscanf(file, "%63s", key, 63);
		#endif

		if (res == EOF)
			break;
		if (strcmp(key, "$$SOE") == 0) {
			while (res != EOF) {
				double JDTDB;
				int year;
				char month[] = "NLL";
				int day;
				int hour;
				int min;
				float second;
				vec3 position;
				vec3 velocity;
				//res = fscanf_s(file, "%lf, A.D. %d-%3c-%d %d:%d:%f, %lf, %lf, %lf, %lf, %lf, %lf,\n", &JDTDB, &year, month, &day, &hour, &min, &second, &position[0], &position[1], &position[2], &velocity[0], &velocity[1], &velocity[2]);
				res = fscanf(file, "%lf, A.D. %d-%3c-", &JDTDB, &year, month);
				res = fscanf(file, "%d %d:%d:%f, %lg, %lg, %lg, %lg, %lg, %lg,\n", &day, &hour, &min, &second, &position[0], &position[1], &position[2], &velocity[0], &velocity[1], &velocity[2]);
				bodyState state;
				state.JDTDB = JDTDB;
				state.position = position;
				state.velocity = velocity;
				this->EphemerisData.push_back(state);
			}
			break;
		}
		else {
			double value;
			int res = fscanf(file, "%lg", &value);
			if(res==EOF){
				printf("ERROR: reached end of %s too early", path.c_str());
			}
			properties[key] = value;
		}
	}
	fclose(file);
	cout << "Successfully loaded ephemeris data for " << body << ", " << EphemerisData.size() << " states found." << endl;
}

void PlanetaryBody::setPosition(vec3 pos) {
	if(this->parentBody != NULL)
		pos += parentBody->getPosition();

	this->position = pos;
	if (bodyMesh != NULL)
		this->bodyMesh->setPos(pos);
	if (ringMesh != NULL)
		this->ringMesh->setPos(pos);
	if (atmoMesh != NULL)
		this->atmoMesh->setPos(pos);
}

void PlanetaryBody::setVelocity(vec3 vel) {
	if(this->parentBody != NULL)
		vel += this->parentBody->getVelocity();
	this->velocity = vel;
}

mat4 PlanetaryBody::getAttitude(){
	return this->attitude;
}

mat4 PlanetaryBody::getAttitude(double julian){
	SpiceDouble data[6][6];
	double time = (julian - 2451545.0)*86400;
	string capname = "IAU_"+this->name;
	for(auto& x: capname)
        x = toupper(x);
	sxform_c(capname.c_str(), "eclipj2000", time, data);
	mat4 mat = eye4();
	mat[0] = {data[0][0],data[0][1],data[0][2],0.0};
	mat[1] = {data[1][0],data[1][1],data[1][2],0.0};
	mat[2] = {data[2][0],data[2][1],data[2][2],0.0};
	mat[3] = {0.0, 0.0, 0.0, 1.0};
	return mat;
}

void PlanetaryBody::updateAttitude(double julian){
	SpiceDouble data[6][6];
	double time = (julian - 2451545.0)*86400;
	string capname = "IAU_"+this->name;
	for(auto& x: capname)
        x = toupper(x);
	sxform_c(capname.c_str(), "eclipj2000", time, data);
	mat4 mat = eye4();
	mat[0] = {data[0][0],data[0][1],data[0][2],0.0};
	mat[1] = {data[1][0],data[1][1],data[1][2],0.0};
	mat[2] = {data[2][0],data[2][1],data[2][2],0.0};
	mat[3] = {0.0, 0.0, 0.0, 1.0};
	this->attitude = mat;
	if (bodyMesh != NULL)
		this->bodyMesh->setAttitude(mat);
	if (ringMesh != NULL)
		this->ringMesh->setAttitude(mat);
	if (atmoMesh != NULL)
		this->atmoMesh->setAttitude(mat);
}

vec3 PlanetaryBody::calculateAbsoluteAccelerationFromPosition(double julian, vec3 pos){
	double unitConv = 86400.0*86400.0; // seconds/day
	statevec pstate = this->getStateAtTime(julian);
	vec3 dir = pstate[0] - pos;
	double r = len(dir);
	vec3 acceleration = this->mu*unitConv*dir / (r*r*r);
	return acceleration;
}

vec3 PlanetaryBody::getPosition(bool relative) { // relative defaults to true
	if (this->parentBody != NULL && relative)
		return this->position - parentBody->getPosition();
	else
		return this->position;
}

vec3 PlanetaryBody::getVelocity(bool relative) { // relative defaults to true
	if (this->parentBody != NULL && relative)
		return this->velocity - parentBody->getVelocity();
	else
		return this->velocity;
}

void PlanetaryBody::interpToTime(double julian){
	statevec state = this->getStateAtTime(julian);
	this->setPosition(state[0]);
	this->setVelocity(state[1]);
}

statevec PlanetaryBody::getStateAtTime(double julian){
	double time = (julian - 2451545.0)*86400;
	SpiceDouble cstate[6];
	double lt;
	spkezr_c(this->idCode.c_str(), time, "eclipj2000", "NONE", "0", cstate, &lt);
	statevec state = {vec3{cstate[0], cstate[1], cstate[2]}, vec3{cstate[3], cstate[4], cstate[5]}};
	
	return state;
}

void PlanetaryBody::genOrbitLines() {
	this->lineMat = new Material("line", "NULL");
	this->lineMat->matShader->use();

	glGenVertexArrays(1, &this->lineVAO);
	glGenBuffers(1, &this->lineVBO);
}

void PlanetaryBody::updateOrbitLines()
{
	unsigned int numPts = 360*4 ;
	std::vector<double> lineData;
	double mu;
	vec3 color;
	if (this->parentBody != NULL){
		mu = this->parentBody->properties["GM"];
		color = {0.7, 0.4, 0.8};
	}
	else{
		mu = SOLAR_MU;
		color = {0.0, 0.8, 0.8};
	}
	vec6 oe = rv2oe(this->getPosition(),this->getVelocity(), mu); //fix gets
	for (double nu = 0; nu <= 360.0; nu += (360.0 / numPts)) {
		oe[5] = radians(nu);
		std::array<vec3, 2> rv = oe2rv(oe, mu);
		lineData.push_back(rv[0][0]);
		lineData.push_back(rv[0][1]);
		lineData.push_back(rv[0][2]);
		lineData.push_back(color[0]);
		lineData.push_back(color[1]);
		lineData.push_back(color[2]);

	}

	lineDataSize = lineData.size() / 6;


	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, lineDataSize * 6 * sizeof(double), lineData.data(), GL_STATIC_DRAW);
	glVertexAttribLPointer(0, 3, GL_DOUBLE, sizeof(double) * 6, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribLPointer(1, 3, GL_DOUBLE, sizeof(double) * 6, (void*)(3 * sizeof(double)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

void PlanetaryBody::genAxisLines() {
	glGenVertexArrays(1, &axisVAO);
	glGenBuffers(1, &axisVBO);
	
	vec3 scale = this->bodyMesh->getScale();
	std::vector<GLdouble> lineData = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
									scale[0]*2.0, 0.0, 0.0, 1.0, 0.0, 0.0,
					 				0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
									0.0, scale[1]*2.0, 0.0, 0.0, 1.0, 0.0,
									0.0, 0.0, 0.0, 0.0, 0.0, 1.0,
									0.0, 0.0, scale[2]*2.0, 0.0, 0.0, 1.0};

	glBindVertexArray(axisVAO);
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
	glBufferData(GL_ARRAY_BUFFER, (unsigned int)lineData.size() * sizeof(double), lineData.data(), GL_STATIC_DRAW);
	glVertexAttribLPointer(0, 3, GL_DOUBLE, sizeof(GLdouble) * 6, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribLPointer(1, 3, GL_DOUBLE, sizeof(GLdouble) * 6, (void*)(3 * sizeof(GLdouble)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

void PlanetaryBody::drawLines() {
	if (lineMat != NULL) {
		lineMat->matShader->use();

		// Draw orbit lines
		mat4 modelMat = (this->parentBody == NULL)? eye4(): TransMat(parentBody->getPosition());
		lineMat->matShader->setUniform4dv("model", &modelMat[0][0]);
		glBindVertexArray(lineVAO);
		glDrawArrays(GL_LINE_STRIP, 0, lineDataSize);

		// Draw attitude lines
		modelMat = TransMat(this->position)*this->getAttitude();
		lineMat->matShader->setUniform4dv("model", &modelMat[0][0]);
		glBindVertexArray(axisVAO);
		glDrawArrays(GL_LINES, 0, 6);
		glBindVertexArray(0);
	}
}

void PlanetaryBody::setUniform4dv(const char * uniformName, double * uniformPtr) {
	if (bodyMat != NULL){
		bodyMat->matShader->setUniform4dv(uniformName, uniformPtr);
	}
	if (atmoMat != NULL){
		atmoMat->matShader->setUniform4dv(uniformName, uniformPtr);
	}
	if (ringMat != NULL){
		ringMat->matShader->setUniform4dv(uniformName, uniformPtr);
	}
	if (lineMat != NULL){
		lineMat->matShader->setUniform4dv(uniformName, uniformPtr);
	}
	for (int i = 0; i < this->children.size(); ++i) {
		if (children[i]->bodyMat != NULL)
			children[i]->bodyMat->matShader->setUniform4dv(uniformName, uniformPtr);
		if (children[i]->lineMat != NULL)
			children[i]->lineMat->matShader->setUniform4dv(uniformName, uniformPtr);
		if (children[i]->atmoMat != NULL)
			children[i]->atmoMat->matShader->setUniform4dv(uniformName, uniformPtr);
		if (children[i]->ringMat != NULL)
			children[i]->ringMat->matShader->setUniform4dv(uniformName, uniformPtr);
	}

}

void PlanetaryBody::setUniformV3(const char * uniformName, float* vec) {
	if (bodyMat != NULL)
		bodyMat->matShader->setUniformV3(uniformName, vec);
	if (atmoMat != NULL)
		atmoMat->matShader->setUniformV3(uniformName, vec);
	if (ringMat != NULL)
		ringMat->matShader->setUniformV3(uniformName, vec);
	for (int i = 0; i < this->children.size(); ++i) {
		if (children[i]->bodyMat != NULL)
			children[i]->bodyMat->matShader->setUniformV3(uniformName, vec);
		if (children[i]->lineMat != NULL)
			children[i]->lineMat->matShader->setUniformV3(uniformName, vec);
		if (children[i]->atmoMat != NULL)
			children[i]->atmoMat->matShader->setUniformV3(uniformName, vec);
		if (children[i]->ringMat != NULL)
			children[i]->ringMat->matShader->setUniformV3(uniformName, vec);
	}
}

void PlanetaryBody::setUniformV3(const char * uniformName, double * vec) {
	if (bodyMat != NULL)
		bodyMat->matShader->setUniformV3(uniformName, vec);
	if (atmoMat != NULL)
		atmoMat->matShader->setUniformV3(uniformName, vec);
	if (ringMat != NULL)
		ringMat->matShader->setUniformV3(uniformName, vec);
	for (int i = 0; i < this->children.size(); ++i) {
		if (children[i]->bodyMat != NULL)
			children[i]->bodyMat->matShader->setUniformV3(uniformName, vec);
		if (children[i]->lineMat != NULL)
			children[i]->lineMat->matShader->setUniformV3(uniformName, vec);
		if (children[i]->atmoMat != NULL)
			children[i]->atmoMat->matShader->setUniformV3(uniformName, vec);
		if (children[i]->ringMat != NULL)
			children[i]->ringMat->matShader->setUniformV3(uniformName, vec);
	}
}

void PlanetaryBody::setUniformV3(const char * uniformName, vec3f vec) {
	if (bodyMat != NULL)
		bodyMat->matShader->setUniformV3(uniformName, vec);
	if (atmoMat != NULL)
		atmoMat->matShader->setUniformV3(uniformName, vec);
	if (ringMat != NULL)
		ringMat->matShader->setUniformV3(uniformName, vec);
	for (int i = 0; i < this->children.size(); ++i) {
		if (children[i]->bodyMat != NULL)
			children[i]->bodyMat->matShader->setUniformV3(uniformName, vec);
		if (children[i]->lineMat != NULL)
			children[i]->lineMat->matShader->setUniformV3(uniformName, vec);
		if (children[i]->atmoMat != NULL)
			children[i]->atmoMat->matShader->setUniformV3(uniformName, vec);
		if (children[i]->ringMat != NULL)
			children[i]->ringMat->matShader->setUniformV3(uniformName, vec);
	}
}

void PlanetaryBody::setUniformV3(const char * uniformName, vec3 vec) {
	if (bodyMat != NULL)
		bodyMat->matShader->setUniformV3(uniformName, vec);
	if (atmoMat != NULL)
		atmoMat->matShader->setUniformV3(uniformName, vec);
	if (ringMat != NULL)
		ringMat->matShader->setUniformV3(uniformName, vec);
	for (int i = 0; i < this->children.size(); ++i) {
		if (children[i]->bodyMat != NULL)
			children[i]->bodyMat->matShader->setUniformV3(uniformName, vec);
		if (children[i]->lineMat != NULL)
			children[i]->lineMat->matShader->setUniformV3(uniformName, vec);
		if (children[i]->atmoMat != NULL)
			children[i]->atmoMat->matShader->setUniformV3(uniformName, vec);
		if (children[i]->ringMat != NULL)
			children[i]->ringMat->matShader->setUniformV3(uniformName, vec);
	}
}

void PlanetaryBody::draw() {
	glEnable(GL_CULL_FACE);

	if (this->lineMat != NULL){
		drawLines();
	}
	if (this->bodyMat != NULL){
		bodyMat->draw();
	}
	if (this->ringMat != NULL){
		ringMat->draw();
	}
	if (this->atmoMat != NULL){
		glFrontFace(GL_CW);
		atmoMat->draw();
		glFrontFace(GL_CCW);
	}
	for (int i = 0; i < this->children.size(); ++i) {
		children[i]->draw();
	}

	glDisable(GL_CULL_FACE);
}

PlanetarySystem::PlanetarySystem() {
	texHandler->loadTexture("skybox");
	skybox = new Skybox();
	skybox->setScale(10000.0);
	skyboxMat = new Material("light", "skybox");
	skyboxMat->addObj(skybox);


	furnsh_c("de435.bsp");
	furnsh_c("pck00010.tpc");
	furnsh_c("de-403-masses.tpc");
	furnsh_c("geophysical.ker");
}

void PlanetarySystem::loadPlanets() {
	//Loads the sun
	PlanetaryBody* sun = new PlanetaryBody("sun", "10", "light", false, false, false, false);
	sun->systemIndex = nrPlanets;
	this->Planets.insert({ "sun", sun });
	++nrPlanets; printf("\n");
	
	//earth loader
	Earth* earth = new Earth("earth", "399", "planet", true, false, false, true);
	earth->systemIndex = nrPlanets;
	this->Planets.insert({"earth", earth});
	++nrPlanets; printf("\n");

	//moon loader
	Luna* luna = new Luna("moon", "301", "DS_1", true, false, false, false, this->Planets["earth"]);
	this->Planets["earth"]->children.push_back(luna);

	//mars loader
	PlanetaryBody* mars = new PlanetaryBody("mars", "4", "DS_1", true, false, false, false);
	mars->systemIndex = nrPlanets;
	this->Planets.insert({"mars", mars});
	++nrPlanets; printf("\n");

	//Saturn loader
	PlanetaryBody* saturn = new PlanetaryBody("saturn", "6", "DS_1", true, false, true, false);
	saturn->systemIndex = nrPlanets;
	this->Planets.insert({"saturn", saturn});
	++nrPlanets; printf("\n");

}

void PlanetarySystem::draw() {
	for (auto const& [key, planet]: Planets){
		planet->draw();
	}
	for(auto const& [name, craft] : this->spacecrafts){
		if(craft->shouldDraw)
			craft->draw();
	}

}

void PlanetarySystem::drawSkybox(){
	if (skyboxMat != NULL) {
		skyboxMat->draw(true);
	}
}

void PlanetarySystem::setUniform4dv(const char * uniformName, double * uniformPtr) {
	if (this->skyboxMat != NULL){
		this->skyboxMat->matShader->setUniform4dv(uniformName, uniformPtr);
	}

	for (auto const& [key, planet]: this->Planets){
		planet->setUniform4dv(uniformName, uniformPtr);
	}

	for(auto const& [name,craft] : this->spacecrafts){
		craft->setUniform4dv(uniformName, uniformPtr);
	}

}

void PlanetarySystem::setUniformV3(const char * uniformName, float * vec) {
	if (skyboxMat != NULL)
		skyboxMat->matShader->setUniformV3(uniformName, vec);
	for (auto const& [key, planet]: Planets) {
		planet->setUniformV3(uniformName, vec);
	}
	for(auto const& [name,craft] : this->spacecrafts){
		craft->setUniformV3(uniformName, vec);
	}
}

void PlanetarySystem::setUniformV3(const char * uniformName, double * vec) {
	if (skyboxMat != NULL)
		skyboxMat->matShader->setUniformV3(uniformName, vec);
	for (auto const& [key, planet]: Planets) {
		planet->setUniformV3(uniformName, vec);
	}
	for(auto const& [name,craft] : this->spacecrafts){
		craft->setUniformV3(uniformName, vec);
	}
}

void PlanetarySystem::setUniformV3(const char * uniformName, vec3f vec) {
	if (skyboxMat != NULL)
		skyboxMat->matShader->setUniformV3(uniformName, vec);
	for (auto const& [key, planet]: Planets) {
		planet->setUniformV3(uniformName, vec);
	}
	for(auto const& [name,craft] : this->spacecrafts){
		craft->setUniformV3(uniformName, vec);
	}
}

void PlanetarySystem::setUniformV3(const char * uniformName, vec3 vec) {
	if (skyboxMat != NULL)
		skyboxMat->matShader->setUniformV3(uniformName, vec);
	for (auto const& [key, planet]: Planets) {
		planet->setUniformV3(uniformName, vec);
	}
	for(auto const& [name,craft] : this->spacecrafts){
		craft->setUniformV3(uniformName, vec);
	}
}

double PlanetarySystem::getClosestPlanetDistance(vec3 position) {
	double dist = INFINITY;
	for (auto const& [key, planet]: Planets) {
		if (len(position - planet->position) < dist) {
			dist = len(position - planet->position);
		}
		for (int j = 0; j < planet->children.size(); ++j) {
			if (len(position - planet->children[j]->position) < dist) {
				dist = len(position - planet->children[j]->position);
			}
		}
	}
	return dist;
}

void PlanetarySystem::updatePlanetPositions(double julian)
{
	this->curTime = julian;
	for (auto const& [key, planet]: Planets) {
		planet->updateAttitude(julian);
		planet->interpToTime(julian);
		for (int j = 0; j < planet->children.size(); ++j) {
			planet->children[j]->updateAttitude(julian);
			planet->children[j]->interpToTime(julian);
		}
	}
	for ( auto const& [name, craft] : this->spacecrafts ){
		if(craft->getYeet() <= julian && julian < craft->getYote()){
			craft->shouldDraw = true;
			craft->interpToTime(julian);
		}
		else{
			craft->shouldDraw = false;
		}
	}
}

void PlanetarySystem::updateOrbitLines()
{
	for (auto const& [key, planet]: Planets) {
		planet->updateOrbitLines();
		for (int j = 0; j < planet->children.size(); ++j) {
			planet->children[j]->updateOrbitLines();
		}
	}
}

void PlanetarySystem::genOrbitLines(){
	for (auto const& [key, planet]: Planets) {
		if(planet->name!= "sun"){
			planet->genOrbitLines();
			planet->updateOrbitLines();
			planet->genAxisLines();
		}
		for (int j = 0; j < planet->children.size(); ++j) {
			planet->children[j]->genOrbitLines();
			planet->children[j]->updateOrbitLines();
			planet->children[j]->genAxisLines();
		}
	}
}

void PlanetarySystem::addSpacecraft(PlanetaryBody* lineParent, string name, vec3 startPos, vec3 startVel, double duration, string objName){
	Spacecraft* newCraft = new Spacecraft(name, this->curTime, startPos, startVel, objName);
	this->spacecrafts[name] = newCraft;
	newCraft->propagate(solarSystem->curTime, duration, 1e-5);
	
	(lineParent == NULL)? newCraft->genOrbitLines(): newCraft->genOrbitLines(lineParent);
	//std::thread propThread([](Spacecraft* cr){cr->propagate(solarSystem->curTime, 10.0, 1e-5);}, std::ref(newCraft));
}

// ########################################   EARTH  ###################################


vec3 Earth::calculateAbsoluteAccelerationFromPosition(double julian, vec3 position){
	double unitConv = 86400.0*86400.0;
	vec3 acceleration = vec3{ 0.0,0.0,0.0 };
	statevec pstate = this->getStateAtTime(julian);
	vec3 pos = pstate[0] - position;
	double r = len(pos);
	acceleration += this->mu*unitConv*pos / (r*r*r);

	// J2
	pos = -1.0*pos;
	mat3 att = (mat4reduce(this->getAttitude(julian)));
	pos = transpose(att) * pos;
	double J2 = 1.081874E-3;
	double Re2 = 6378.0*6378.0;
	r = len(pos);
	double r2 = r*r;
	double r5 = r2*r2*r;
	double ai = -unitConv*(3*J2*mu*Re2*pos[0] / (2 * r5))*(1 - 5*pos[1]*pos[1] / (r2));
	double aj = -unitConv*(3*J2*mu*Re2*pos[1] / (2 * r5))*(1 - 5*pos[1]*pos[1] / (r2));
	double ak = -unitConv*(3*J2*mu*Re2*pos[2] / (2 * r5))*(3 - 5*pos[1]*pos[1] / (r2));
	vec3 j2acc = vec3{ ai, aj, ak};
	j2acc = (att)*j2acc;
	
	acceleration += j2acc;

	return acceleration;
}

// ########################################   MOON  ###################################


vec3 Luna::calculateAbsoluteAccelerationFromPosition(double julian, vec3 position){
	double unitConv = 86400.0*86400.0;
	vec3 acceleration = vec3{ 0.0,0.0,0.0 };
	statevec pstate, mstate;
	pstate = this->parentBody->getStateAtTime(julian);
	mstate = this->getStateAtTime(julian);
	vec3 pos = pstate[0] + mstate[0] - position;
	double r = len(pos);
	acceleration += this->mu*unitConv*pos / (r*r*r);

	// J2
	pos = -1.0*pos;
	mat3 att = mat4reduce(this->getAttitude(julian));
	pos = transpose(att) * pos;
	double J2 = 0.2027E-3;
	double Re2 = 1737.4*1737.4;
	r = len(pos);
	double r2 = r*r;
	double r5 = r2*r2*r;
	double ai = -unitConv*(3*J2*mu*Re2*pos[0] / (2 * r5))*(1 - 5*pos[1]*pos[1] / (r2));
	double aj = -unitConv*(3*J2*mu*Re2*pos[1] / (2 * r5))*(1 - 5*pos[1]*pos[1] / (r2));
	double ak = -unitConv*(3*J2*mu*Re2*pos[2] / (2 * r5))*(3 - 5*pos[1]*pos[1] / (r2));
	vec3 j2acc = vec3{ ai, aj, ak};
	j2acc = (att)*j2acc;
	
	acceleration += j2acc;

	return acceleration;
}

statevec Luna::getStateAtTime(double julian){
	double time = (julian - 2451545.0)*86400;
	SpiceDouble cstate[6];
	double lt;
	spkezr_c(this->idCode.c_str(), time, "eclipj2000", "NONE", "399", cstate, &lt);
	statevec state = {vec3{cstate[0], cstate[1], cstate[2]}, vec3{cstate[3], cstate[4], cstate[5]}};
	
	return state;
}