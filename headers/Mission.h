#pragma once

#include "OdeSolver.h"
#include "Transforms.h"


class Stage;
class Mission;
class PlanetaryBody;
struct spacecraftState;

struct stageEvent{
    string type; // MET, SET, PERIAPSE, APOAPSE, ALTITUDE, OE_NU, Etc. 
    double value = 0.0;
    PlanetaryBody* parent = NULL;
};
struct spacecraftState {
	double JDTDB;
	vec3 position = {0.0,0.0,0.0};
	vec3 velocity = {0.0,0.0,0.0};
	vec3 angularVel = {0.0,0.0,0.0};
	double mass = 0.0;
	mat3 attitude = zeros3();
	mat3 inertialMat = zeros3();
    double thrust = 0.0;
};
struct stageFlags{
	bool staged;	
};
struct missionStep{
	stageFlags stgFlags;
	spacecraftState scState;
};
struct substage{
    string name;
    SceneObject* obj = NULL;
    Material* mat = NULL;
    SceneObject* thrustObj = NULL;
    Material* thrustMat = NULL;
    double dryMass = 0.0;
    double wetMass = 0.0;
    double specificImpulse = 250.0;
    double maxMdot = 0.0;
    double maxThrust = 0.0;
    double negz = 0.0;
    double posz = 0.0;
    mat3 inertialMat;
    stageEvent terminate;
};
struct maneuver {
    vec3 axis;
    double deltaV;
    double SET;
};

typedef std::function<void(Stage*, spacecraftState, double, vec3&, vec3&, bool&)> aocsFun;


class Mission{
private:
    
    

    double launchDate;

    string missionName;
    spacecraftState initialConditions;

    vector<aocsFun> Softwares;
    vector<vector<maneuver>> maneuvers;

public:
    vector<Stage*> stages;
    vector<substage> substages;

    Mission(string name, spacecraftState initState, vector<aocsFun> Softwares, vector<vector<maneuver>> maneuvers);

    //Adds a substage to the ordered substage vector.
    void addSubstage(string texMod, double dryMass, double wetMass, double negZ, double posZ,double ISP, double maxThrust, double maxMdot, mat3 inertialMat, stageEvent stgevnt);
    
    void propagate(double MET);

    void drawCraft();
    void drawLines();

    void setUniform4dv(const char* uniformName, double* uniformPtr);
    void setUniform4dvCraft(const char* uniformName, double* uniformPtr);
    void setUniformV3(const char * uniformName, float * vec);
	void setUniformV3(const char * uniformName, double * vec);
	void setUniformV3(const char * uniformName, vec3& vec);
	void setUniformV3(const char * uniformName, vec3f& vec);
};

class Stage{
private:
    // Data for drawing
    
    mat3 attitude;

    
    GLuint lineVAO, lineVBO;

    unsigned long int lineDataSize;

    //other data
    vector<spacecraftState> propagationData;

    double timeOfYeet = 0.0;
    double timeOfYote = 0.0;

    

    PlanetaryBody* pointTarget;
    std::function<void(Stage*, spacecraftState, double, vec3&, vec3&, bool&)> AOCSSoftware;
    

    spacecraftState interpEphIndex(double julian);

public: 
    vec3 velocity;
    vec3 position;
    vector<maneuver> maneuvers;
    double specificImpulse;
    double maxThrust, maxMdot;
    double wetMass= 0.0;
    double dryMass = 0.0;
    double fuelMass = 0.0;
    double fireDurationTemp = 0.0;
    mat3 inertialMat;

    PlanetaryBody* parentBody;
    bool firing = false;
    double curMass;
    double curThrust;
    double curOmega;
    vec3 curVel;

    Material* lineMat = NULL;
    bool shouldDraw = true;
    vector<substage> substages;
    stageEvent whenToStage;

    Stage(vector<substage> stgs,std::function<void(Stage*, spacecraftState, double, vec3&, vec3&, bool&)> Software, vector<maneuver> maneuvers);

    double getYeet();
    double getYote();
    double getWetMass(){return this->wetMass;}

    void interpToTime(double julian);
    void setPosition(vec3 pos);
    void setVelocity(vec3 vel);
    void setAttitude(mat3 att);

    spacecraftState getEndState();
    spacecraftState getStartState();

    void propagate(spacecraftState initstate);
    void genOrbitLines(PlanetaryBody* parent);
    spacecraftState dynamicsEquation(double time, spacecraftState cs, int stepFlag, bool& firing);
    void drawCraft();
    void drawLines();
    
};