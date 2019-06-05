#include "OdeSolver.h"

inline spacecraftState operator*(const double &d,const spacecraftState &st){
	spacecraftState rt;
	rt.angularVel = d*st.angularVel;
	rt.attitude = st.attitude*d;
	rt.inertialMat = st.inertialMat * d;
	rt.JDTDB = st.JDTDB*d;
	rt.mass = st.mass*d;
	rt.position = d*st.position;
	rt.velocity = d*st.velocity;
	return rt;
}
inline spacecraftState operator+(const spacecraftState &s1, const spacecraftState &s2){
	spacecraftState rt;
	rt.angularVel = s1.angularVel + s2.angularVel;
	rt.attitude = s1.attitude + s2.attitude;
	rt.inertialMat = s1.inertialMat + s2.inertialMat;
	rt.JDTDB = s1.JDTDB + s2.JDTDB;
	rt.mass = s1.mass + s2.mass;
	rt.position = s1.position+s2.position;
	rt.velocity = s1.velocity+s2.velocity;
	return rt;
}
inline spacecraftState operator-(const spacecraftState &s1, const spacecraftState &s2){
	spacecraftState rt;
	rt.angularVel = s1.angularVel - s2.angularVel;
	rt.attitude = s1.attitude - s2.attitude;
	rt.inertialMat = s1.inertialMat - s2.inertialMat;
	rt.JDTDB = s1.JDTDB - s2.JDTDB;
	rt.mass = s1.mass - s2.mass;
	rt.position = s1.position - s2.position;
	rt.velocity = s1.velocity - s2.velocity;
	return rt;
}

void vec3_rkf45_stg(spacecraftState initialState, stageEvent toStg, double tolerance, missionStateFunc fun,vector<spacecraftState>* stateOutput) {

	double attStepSize = 1.0 / (24.0 * 60.0 * 60.0);
	double posStepSize = 1.0 / (24.0 * 60.0);// attStepSize = posStepSize;
	double tPos = initialState.JDTDB;
	double tAtt = initialState.JDTDB;
	double lastPosStep = initialState.JDTDB;
	double err,s;
	double totErr = 0;
	spacecraftState p1, p2, att1, att2;
	spacecraftState z1;
	spacecraftState k1, k2, k3, k4, k5, k6;

	//define the RKF constants
	double h2 = 0.25, h3 = (3.0 / 8.0), h4 = (12.0/13.0), h6 = 0.5;
	double a2 = 0.25, a3 = (3.0 / 32.0), a4 = (1932.0 / 2197.0), a5 = (439.0 / 216.0), a6 = (-8.0 / 27.0);
	double b3 = (9.0 / 32.0), b4 = (-7200.0 / 2197.0), b5 = (-8.0), b6 = (2.0);
	double c4 = (7296.0 / 2197.0), c5 = (3680.0 / 513.0), c6 = (-3544.0 / 2565.0);
	double d5 = (-845.0 / 4104.0), d6 = (1859.0 / 4104.0), e6=(-11.0/40.0);

	double f1 = (16.0 / 135.0), f3 = (6656.0 / 12825.0), f4 = (28561.0 / 56430.0), f5 = (-9.0 / 50.0), f6 = (2.0 / 55.0);
	double g1 = (25.0 / 216.0), g3 = (1408.0 / 2565.0), g4 = (2197.0 / 4104.0), g5 = (-1.0 / 5.0);
	
	int steps = 0;

	//adds the initial position (p1) to the stateOutput
	p1 = initialState;
	stateOutput->push_back(p1);

	bool staged = false;

	while (!staged) {

		//Start by propagating a position step
		bool firing=false;
		//Do a position step using the p1 variable 
		k1 = fun(tPos,          p1, STEP_POS, firing);
		k2 = fun(tPos + h2 * posStepSize, p1 + posStepSize*(a2 * k1), STEP_POS, firing);
		k3 = fun(tPos + h3 * posStepSize, p1 + posStepSize*(a3 * k1 + b3 * k2), STEP_POS, firing);
		k4 = fun(tPos + h4 * posStepSize, p1 + posStepSize*(a4 * k1 + b4 * k2 + c4 * k3), STEP_POS, firing);
		k5 = fun(tPos + posStepSize,      p1 + posStepSize*(a5 * k1 + b5 * k2 + c5 * k3 + d5 * k4), STEP_POS, firing);
		k6 = fun(tPos + h6 * posStepSize, p1 + posStepSize*(a6 * k1 + b6 * k2 + c6 * k3 + d6 * k4 + e6 * k5), STEP_POS, firing);


		// dont really care about the firing flag during the position step since attitude will catch it.
		firing = false;
		//output of position step is now stored in p2, note this is only valid for position, not attitude
		p2 = p1 + posStepSize*(g1*k1 + g3 * k3 + g4 * k4 + g5 * k5);//z1 = y + posStepSize*(f1*k1 + f3 * k3 + f4 * k4 + f5 * k5 + f6 * k6);
		tPos += posStepSize;
		p2.JDTDB = tPos; // JDTDB is the time (Julian date)
		
		att1 = p1;
		int STEP_TYPE = STEP_ATT;
		//now need to send in the position data while doing each attitude step, these steps should be the output data
		//stop the loop when the attitude time reaches the position time
		while(tAtt < tPos || firing){
			//p1 contains attitude data, however p2 only contains accurate position and velocity data.

			
			

			bool firing1 = false, firing2 = false, firing3 = false, firing4 = false, firing5 = false, firing6 = false;
			// integrate the attitude step using the att1 variable from the previous attitude step (or the p1 if first step)
			k1 = fun(tAtt,          att1, STEP_TYPE, firing1);
			k2 = fun(tAtt + h2 * attStepSize, att1 + attStepSize*(a2 * k1), STEP_TYPE, firing2);
			k3 = fun(tAtt + h3 * attStepSize, att1 + attStepSize*(a3 * k1 + b3 * k2), STEP_TYPE, firing3);
			k4 = fun(tAtt + h4 * attStepSize, att1 + attStepSize*(a4 * k1 + b4 * k2 + c4 * k3), STEP_TYPE, firing4);
			k5 = fun(tAtt + attStepSize,      att1 + attStepSize*(a5 * k1 + b5 * k2 + c5 * k3 + d5 * k4), STEP_TYPE, firing5);
			k6 = fun(tAtt + h6 * attStepSize, att1 + attStepSize*(a6 * k1 + b6 * k2 + c6 * k3 + d6 * k4 + e6 * k5), STEP_TYPE, firing6);

			// if any of the steps here fired, and if the stepping type is still atitude (IE, first time the loop has 'discovered' its supposed to fire)
			if((firing1 || firing2 || firing3 || firing4 || firing5 || firing6) && STEP_TYPE==STEP_ATT){
				firing = true;
				STEP_TYPE = STEP_FULL;
				//stepSize = fullStepSize;
				continue;
			}// if any of the steps fired, and if it was determined to be firing during the last loop (basically just continue to fire)
			else if((firing1 || firing2 || firing3 || firing4 || firing5 || firing6) && STEP_TYPE==STEP_FULL){
				att2 = att1 + attStepSize*(g1*k1 + g3 * k3 + g4 * k4 + g5 * k5);//fullStepSize
				att2.JDTDB = tAtt + attStepSize;//fullStepSize
				att2.thrust = (k1.thrust + k2.thrust + k3.thrust + k4.thrust + k5.thrust + k6.thrust)/6.0; // not weighted properly for accurate thrust data during transition
				tAtt += attStepSize;//fullStepSize
				att1 = att2;

				// add completed state to output vector
				stateOutput->push_back(att2);
			} // This will trigger if the previous loop was firing, but the current is not firing. Need to update p1 and p2
			else if(firing == true){
				STEP_TYPE = STEP_ATT;
				firing = false;
				//stepsize = attstepsize;

				att2 = att1 + attStepSize*(g1*k1 + g3 * k3 + g4 * k4 + g5 * k5);//fullStepSize
				att2.JDTDB = tAtt + attStepSize;//fullStepSize

				// add completed state to output vector
				stateOutput->push_back(att2);
				tAtt = att2.JDTDB;//fullStepSize
				att1 = att2;

				// p1 and p2 data for interpolation is now not accurate (post firing), so need to recalculate
				// so reset p1 and p2 to the latest step, and reset the times to the current tAtt time
				p1 = att1;
				p2 = att1;
				tPos = tAtt;
				break;
			}
			else{
				att2 = att1 + attStepSize*(g1*k1 + g3 * k3 + g4 * k4 + g5 * k5);
				att2.JDTDB = tAtt + attStepSize;
				tAtt += attStepSize;
				//the fraction from 0 to 1 of the attitude step compared to the position p1 and p2 time diiference
				double timeFraction = (tAtt- p1.JDTDB)/(p2.JDTDB-p1.JDTDB);
				// update the state using the interpolated position data
				att2.position = positionInterpolation(p1.position, p2.position, p1.velocity, p2.velocity, p1.JDTDB, p2.JDTDB, tAtt);
				att2.velocity = p1.velocity + timeFraction*(p2.velocity-p1.velocity);
				
				att1 = att2;

				//add completed state to the output vector
				stateOutput->push_back(att2);
			}

			if(toStg.type == "SET"){
				if(tAtt-initialState.JDTDB > toStg.value){
					staged = true;
					tPos = tAtt;
				}
			}
			
		}
		tPos = tAtt;
		p1 = att2;
		
		//stateOutput->push_back(y);			
		
	}
}

spacecraftState interpScState(spacecraftState ps1, spacecraftState ps2, double julian){
    spacecraftState returnState;
    returnState.JDTDB = julian;
    double timeFrac = (julian - ps1.JDTDB) / (ps2.JDTDB - ps1.JDTDB);
    double slopeFac = abs(ps2.JDTDB - ps1.JDTDB);

	returnState.attitude = ((ps2.attitude - ps1.attitude)*timeFrac + ps1.attitude);

    returnState.velocity = timeFrac * (ps2.velocity - ps1.velocity) + ps1.velocity;
    returnState.position = positionInterpolation(ps1.position, ps2.position, ps1.velocity, ps2.velocity, ps1.JDTDB, ps2.JDTDB, julian);
    returnState.JDTDB = julian;
    returnState.angularVel = timeFrac * (ps2.angularVel - ps1.angularVel) + ps1.angularVel;
    returnState.mass = timeFrac * (ps2.mass - ps1.mass) + ps1.mass;
    returnState.inertialMat = (ps2.inertialMat - ps1.inertialMat)*timeFrac + ps1.inertialMat;
	returnState.thrust = (ps2.thrust - ps1.thrust)*timeFrac + ps1.thrust;

    return returnState;
}

inline void vec3_rkf45_state(vec6 initial, double initTime, double deltaTime, double tolerance, v6Fun fun,vector<state>* stateOutput) {
	stateOutput->push_back(state{ initTime, vec3{ initial[0],initial[1],initial[2] }, vec3{ initial[3],initial[4],initial[5] } });
	double hmin = 1.0 / (24.0 * 60.0 * 60.0 ); // 1 second
	double hmax = 1.0 / (24.0 *12.0); // 15 minutes
	double h = 1.0 / (24.0*60.0);// (hmin + hmax) / 2.0;
	double t = initTime;
	double err,s;
	double totErr = 0;
	vec6 y = initial;
	vec6 y1;
	vec6 z1;
	vec6 k1, k2, k3, k4, k5, k6;

	//define the RKF constants
	double h2 = 0.25, h3 = (3.0 / 8.0), h4 = (12.0/13.0), h6 = 0.5;
	double a2 = 0.25, a3 = (3.0 / 32.0), a4 = (1932.0 / 2197.0), a5 = (439.0 / 216.0), a6 = (-8.0 / 27.0);
	double b3 = (9.0 / 32.0), b4 = (-7200.0 / 2197.0), b5 = (-8.0), b6 = (2.0);
	double c4 = (7296.0 / 2197.0), c5 = (3680.0 / 513.0), c6 = (-3544.0 / 2565.0);
	double d5 = (-845.0 / 4104.0), d6 = (1859.0 / 4104.0), e6=(-11.0/40.0);

	double f1 = (16.0 / 135.0), f3 = (6656.0 / 12825.0), f4 = (28561.0 / 56430.0), f5 = (-9.0 / 50.0), f6 = (2.0 / 55.0);
	double g1 = (25.0 / 216.0), g3 = (1408.0 / 2565.0), g4 = (2197.0 / 4104.0), g5 = (-1.0 / 5.0);
	
	double outputStep = 0.0;
	double outputTime = 1.0 / (24.0*60.0*4.0); //output a point every ~15 seconds
	bool contFlag = false;
	bool warnFlag = false;
	int steps = 0;
	while (t < initTime + deltaTime) {
		k1 = fun(t,          y);
		k2 = fun(t + h2 * h, y + h*(a2 * k1));
		k3 = fun(t + h3 * h, y + h*(a3 * k1 + b3 * k2));
		k4 = fun(t + h4 * h, y + h*(a4 * k1 + b4 * k2 + c4 * k3));
		k5 = fun(t + h,      y + h*(a5 * k1 + b5 * k2 + c5 * k3 + d5 * k4));
		k6 = fun(t + h6 * h, y + h*(a6 * k1 + b6 * k2 + c6 * k3 + d6 * k4 + e6 * k5));

		y1 = y + h*(g1*k1 + g3 * k3 + g4 * k4 + g5 * k5);
		z1 = y + h*(f1*k1 + f3 * k3 + f4 * k4 + f5 * k5 + f6 * k6);
		
		err = len(z1 - y1);

		s = sqrt(sqrt((tolerance*h)/(2*err)));

		if (false){//(err > tolerance) {
			if (h == hmin) {
				contFlag = true;
				if (!warnFlag) {
					cout << endl;
					cout << "Warning: error exceeding limits. " << endl;
					warnFlag = true;
				}
			}
			else if (h*s < hmin) {
				//h = hmin;
				
				//continue;

			}
			else {
				//h = h * s;
				//continue;
			}
		}
		if (true){//(err<=tolerance || contFlag) {
			if (err > tolerance) {
				if (!warnFlag) {
					printf("\nWarning: error exceeding limits. \n");
					warnFlag = true;
				}
			}
			contFlag = false;
			t += h;
			steps += 1;
			y = y1;
			totErr += err;
			outputStep += h;
			if (outputStep>outputTime) {
				stateOutput->push_back(state{ t, vec3{ y1[0],y1[1],y1[2] }, vec3{ y1[3],y1[4],y1[5] } });
				outputStep = 0;
			}
			
			if (h*s > hmax) {
				//h = hmax;
			}
			else {
				//h = h * s;
			}
		}
		
	}
	printf("Total ODE error: %lg\n", totErr);
	stateOutput->push_back(state{ t, vec3{ y1[0],y1[1],y1[2] }, vec3{ y1[3],y1[4],y1[5] } });
}

inline void vec3_rkf45_sc(spacecraftState initialState, double deltaTime, double tolerance, scStateFunc fun,vector<spacecraftState>* stateOutput) {
	stateOutput->push_back(initialState);
	//double hmin = 1.0 / (24.0 * 60.0 * 60.0 ); // 1 second
	//double hmax = 1.0 / (24.0 *12.0); // 15 minutes
	double h = 1.0 / (24.0 * 60.0 * 60.0);// (hmin + hmax) / 2.0;
	double t = initialState.JDTDB;
	double err,s;
	double totErr = 0;
	spacecraftState y = initialState;
	spacecraftState y1;
	spacecraftState z1;
	spacecraftState k1, k2, k3, k4, k5, k6;

	//define the RKF constants
	double h2 = 0.25, h3 = (3.0 / 8.0), h4 = (12.0/13.0), h6 = 0.5;
	double a2 = 0.25, a3 = (3.0 / 32.0), a4 = (1932.0 / 2197.0), a5 = (439.0 / 216.0), a6 = (-8.0 / 27.0);
	double b3 = (9.0 / 32.0), b4 = (-7200.0 / 2197.0), b5 = (-8.0), b6 = (2.0);
	double c4 = (7296.0 / 2197.0), c5 = (3680.0 / 513.0), c6 = (-3544.0 / 2565.0);
	double d5 = (-845.0 / 4104.0), d6 = (1859.0 / 4104.0), e6=(-11.0/40.0);

	double f1 = (16.0 / 135.0), f3 = (6656.0 / 12825.0), f4 = (28561.0 / 56430.0), f5 = (-9.0 / 50.0), f6 = (2.0 / 55.0);
	double g1 = (25.0 / 216.0), g3 = (1408.0 / 2565.0), g4 = (2197.0 / 4104.0), g5 = (-1.0 / 5.0);
	
	double outputStep = 0.0;
	double outputTime = h;//1.0 / (24.0*60.0*4.0); //output a point every ~15 seconds
	bool contFlag = false;
	bool warnFlag = false;
	int steps = 0;
	while (t < initialState.JDTDB + deltaTime) {
		k1 = fun(t,          y);
		k2 = fun(t + h2 * h, y + h*(a2 * k1));
		k3 = fun(t + h3 * h, y + h*(a3 * k1 + b3 * k2));
		k4 = fun(t + h4 * h, y + h*(a4 * k1 + b4 * k2 + c4 * k3));
		k5 = fun(t + h,      y + h*(a5 * k1 + b5 * k2 + c5 * k3 + d5 * k4));
		k6 = fun(t + h6 * h, y + h*(a6 * k1 + b6 * k2 + c6 * k3 + d6 * k4 + e6 * k5));

		y1 = y + h*(g1*k1 + g3 * k3 + g4 * k4 + g5 * k5);
		z1 = y + h*(f1*k1 + f3 * k3 + f4 * k4 + f5 * k5 + f6 * k6);
		
		err = len(z1.position - y1.position);

		s = sqrt(sqrt((tolerance*h)/(2*err)));
		/*
		if (false){//(err > tolerance) {
			if (h == hmin) {
				contFlag = true;
				if (!warnFlag) {
					cout << endl;
					cout << "Warning: error exceeding limits. " << endl;
					warnFlag = true;
				}
			}
			else if (h*s < hmin) {
				//h = hmin;
				
				//continue;

			}
			else {
				//h = h * s;
				//continue;
			}
		}
		*/
		if (true){//(err<=tolerance || contFlag) {
			if (err > tolerance) {
				if (!warnFlag) {
					printf("\nWarning: error exceeding limits. \n");
					warnFlag = true;
				}
			}
			contFlag = false;
			t += h;
			steps += 1;
			y = y1;
			totErr += err;
			outputStep += h;
			if (outputStep>outputTime) {
				y.JDTDB = t;
				stateOutput->push_back(y);
				outputStep = 0;
			}
		}
		
	}
	printf("Total ODE error: %lg\n", totErr);
	y1.JDTDB = t;
	stateOutput->push_back(y1);
}

