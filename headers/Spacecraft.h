#pragma once
#include <iostream>
#include <functional>
#include "OdeSolver.h"
#include "Transforms.h"
#include "Planet.h"


#if defined _WIN32 || defined _WIN64
#define fscanf fscanf_s
#endif

class PlanetaryBody;
struct spacecraftState;
struct state;
typedef std::function<spacecraftState(double, spacecraftState)> scStateFunc;

class Spacecraft {
private:
	double mass = 1000.0;
	double ISP = 250.0;
	
	double timeOfYeet;
	double timeOfYote;
	vec3 position;
	vec3 velocity;
	vec3 angularVelocity;
	mat3 attitude;

	spacecraftState dynamicsEquation(double time, spacecraftState cs);
	vec3 motionEqnBarycentric(double time, vec3 position);

public:

	string name;
	vector<spacecraftState> propagationData;
	bool shouldDraw = false;

	SceneObject* bodyMesh = NULL;
	Material* bodyMat = NULL;
	Material* lineMat = NULL;

	scStateFunc solver;
	//std::function<vec6(double, vec6)> solver;
	PlanetaryBody* parentBody = NULL;
	PlanetaryBody* coordCenterBody = NULL;
	vector<PlanetaryBody*> affectingBodies;

	Spacecraft(string name, double startTime, vec3 startPos, vec3 startVel, string craftFile);
	void addPos(vec3 pos);
	void addVel(vec3 vel);
	void setPosition(vec3 pos);
	void setVelocity(vec3 vel);
	void setAttitude(mat3 q);
	
	double getMass();
	double getYeet();
	double getYote();
	
	// Calls the ODE solver and saves the returned ephemeris data.
	void propagate(double initTime, double deltaTime, double tolerance);
	// Loads saved craft ephemeris data instead of propagating it
	void loadEpm(string filename);

	unsigned int lineVAO, lineVBO, lineDataSize;
	//Generates lines of the spacecraft movement in inertial frame
	void genOrbitLines();
	//Generates the lines of SV movement with respect to a non-rotating body frame
	void genOrbitLines(PlanetaryBody* parent);
	void updateOrbitLines(PlanetaryBody* parent);

	void interpToTime(double julian);

	spacecraftState interpEphIndex(double julian);


	void draw();

	void setUniform4dv(const char * uniformName, double * uniformPtr);
	void setUniformV3(const char * uniformName, float * vec);
	void setUniformV3(const char * uniformName, double * vec);
	void setUniformV3(const char * uniformName, vec3 vec);
	void setUniformV3(const char * uniformName, vec3f vec);

};
