#pragma once
#include "Globals.h"
#include <array>
#include "Transforms.h"
#include "Mission.h"

#define STEP_POS  0x101
#define STEP_ATT  0x102
#define STEP_FULL 0x103

typedef std::function<vec6 (double, vec6)> v6Fun;

class PlanetaryBody;

struct state {
	double JDTDB;
	vec3 position;
	vec3 velocity;
};
struct spacecraftState;

struct stageEvent;

struct missionStep;

typedef std::function<spacecraftState(double, spacecraftState)> scStateFunc;

typedef std::function<spacecraftState(double, spacecraftState, int, bool&)> missionStateFunc;

spacecraftState operator*(const double &d,const spacecraftState &st);
spacecraftState operator+(const spacecraftState &s1, const spacecraftState &s2);
spacecraftState operator-(const spacecraftState &s1, const spacecraftState &s2);

spacecraftState interpScState(spacecraftState ps1, spacecraftState ps2, double julian);

void vec3_rkf45_state(vec6 initial, double initTime, double deltaTime, double tolerance, v6Fun fun,vector<state>* stateOutput);

void vec3_rkf45_sc(spacecraftState initialState, double deltaTime, double tolerance, scStateFunc fun,vector<spacecraftState>* stateOutput);

void vec3_rkf45_stg(spacecraftState initialState, stageEvent toStg, double tolerance, missionStateFunc fun,vector<spacecraftState>* stateOutput);


