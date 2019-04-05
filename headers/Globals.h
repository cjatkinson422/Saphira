#pragma once

#define GUI_POS_UPPER_LEFT 0
#define GUI_POS_UPPER_MID 1
#define GUI_POS_UPPER_RIGHT 2
#define GUI_POS_MID_LEFT 3
#define GUI_POS_MID_MID 4
#define GUI_POS_MID_RIGHT 5
#define GUI_POS_BOTTOM_LEFT 6
#define GUI_POS_BOTTOM_MID 7
#define GUI_POS_BOTTOM_RIGHT 8

#define GUI_TEXT_ANCHOR_LEFT 0
#define GUI_TEXT_ANCHOR_MID 1
#define GUI_TEXT_ANCHOR_RIGHT 2

#include "TexHandler.h"
#include "Planet.h"

class PlanetarySystem;
class TexHandler;

extern PlanetarySystem* solarSystem;
extern TexHandler* texHandler;
extern double SOLAR_MU;

