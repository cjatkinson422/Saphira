#pragma once
#include <map>
#include <thread>
#include "cspice/SpiceUsr.h"
#include "Transforms.h"
#include "SceneObject.h"
#include "Material.h"
#include "Spacecraft.h"

# define PI  3.14159265358979323846

class Spacecraft;

struct bodyState {
	double JDTDB;
	vec3 position;
	vec3 velocity;
	mat4 attitude;
};

class PlanetaryBody;

class PlanetarySystem {
public:
	double curTime;
	int nrPlanets = 0;
	Skybox* skybox = NULL;
	Material* skyboxMat = NULL;
	std::map<string,PlanetaryBody*> Planets;

	PlanetarySystem();

	std::map<string, Spacecraft*> spacecrafts;

	void loadPlanets();

	void draw();
	void drawSkybox();

	void setUniform4dv(const char* uniformName, double* uniformPtr);
	void setUniformV3(const char* uniformName, float* vec);
	void setUniformV3(const char* uniformName, double* vec);
	void setUniformV3(const char* uniformName, vec3f vec);
	void setUniformV3(const char* uniformName, vec3 vec);

	double getClosestPlanetDistance(vec3 position);

	void updatePlanetPositions(double julian);

	void updateOrbitLines();
	void genOrbitLines();

	void addSpacecraft(PlanetaryBody* lineParent, string name, vec3 startPos, vec3 startVel, double duration, string objName);
};

class PlanetaryBody{
protected:
	string idCode;

	void loadEphemeris(string body);
	void setPosition(vec3 pos);
	void setVelocity(vec3 vel);


	mat4 attitude = eye4();
	
	unsigned int lineVAO, lineVBO, lineDataSize, axisVAO, axisVBO;
public:
	string name;
	double mu;
	PlanetaryBody * parentBody=NULL;

	SceneObject* bodyMesh=NULL;
	SceneObject* atmoMesh=NULL;
	SceneObject* ringMesh=NULL;
	Material* bodyMat = NULL;
	Material* atmoMat = NULL;
	Material* ringMat = NULL;
	Material* lineMat = NULL;

	vec3 velocity;
	vec3 position;

	std::vector<bodyState> EphemerisData;
	std::map<string, double> properties;

	PlanetaryBody(string name, string idCode, string shaderType, bool specular, bool normal, bool rings, bool atmosphere, PlanetaryBody* parentBody = NULL);

	void genOrbitLines();
	void genAxisLines();
	void updateOrbitLines();
	void drawLines();
	void draw();

	vector<PlanetaryBody*> children;

	vec3 getPosition(bool relative = true);
	vec3 getVelocity(bool relative = true);
	mat4 getAttitude();
	mat4 getAttitude(double julian);
	virtual void updateAttitude(double julian);

	virtual vec3 calculateAbsoluteAccelerationFromPosition(double julian, vec3 position);

	virtual void interpToTime(double julian);
	virtual statevec getStateAtTime(double julian);

	int systemIndex = (int)NULL;

	void setUniform4dv(const char* uniformName, double* uniformPtr);
	void setUniformV3(const char* uniformName, float* vec);
	void setUniformV3(const char* uniformName, double* vec);
	void setUniformV3(const char* uniformName, vec3f vec);
	void setUniformV3(const char* uniformName, vec3 vec);

};

class Sun : public PlanetaryBody{

};

class Earth : public PlanetaryBody{
public:
	Earth(string name, string idCode, string shaderType, bool specular, bool normal, bool rings, bool atmosphere, PlanetaryBody* parentBody = NULL) : PlanetaryBody(name, idCode, shaderType, specular, normal, rings, atmosphere, parentBody){
	}
private:


	vec3 calculateAbsoluteAccelerationFromPosition(double julian, vec3 position);
};

class Luna : public PlanetaryBody{
public:
	Luna(string name, string idCode, string shaderType, bool specular, bool normal, bool rings, bool atmosphere, PlanetaryBody* earthPtr) : PlanetaryBody(name, idCode, shaderType, specular, normal, rings, atmosphere, earthPtr){
	}
	statevec getStateAtTime(double julian);
private:

	vec3 calculateAbsoluteAccelerationFromPosition(double julian, vec3 position);
};

