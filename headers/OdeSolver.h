#pragma once
#include "Globals.h"
#include <array>
#include "Transforms.h"

typedef std::function<vec6 (double, vec6)> v6Fun;


struct state {
	double JDTDB;
	vec3 position;
	vec3 velocity;
};
struct spacecraftState {
	double JDTDB;
	vec3 position;
	vec3 velocity;
	vec3 angularVel;
	double mass;
	vec4 attitude;
	mat3 inertialMat;
};

typedef std::function<spacecraftState(double, spacecraftState)> scStateFunc;

inline spacecraftState operator*(double d, spacecraftState st){
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
inline spacecraftState operator+(spacecraftState s1, spacecraftState s2){
	spacecraftState rt;
	rt.angularVel = s1.angularVel + s2.angularVel;
	rt.attitude = s1.attitude + s2.attitude;
	rt.inertialMat = s1.inertialMat + s2.inertialMat;
	rt.JDTDB = s1.JDTDB+s2.JDTDB;
	rt.mass = s1.mass + s2.mass;
	rt.position = s1.position+s2.position;
	rt.velocity = s1.velocity+s2.velocity;
	return rt;
}
inline spacecraftState operator-(spacecraftState s1, spacecraftState s2){
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
	double hmin = 1.0 / (24.0 * 60.0 * 60.0 ); // 1 second
	double hmax = 1.0 / (24.0 *12.0); // 15 minutes
	double h = 1.0 / (24.0*60.0);// (hmin + hmax) / 2.0;
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
	double outputTime = 1.0 / (24.0*60.0*4.0); //output a point every ~15 seconds
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
				y1.JDTDB = t;
				stateOutput->push_back(y1);
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
	y1.JDTDB = t;
	stateOutput->push_back(y1);
}

inline void vec3_dopri5(vec6 initial, double initTime, double deltaTime, double tolerance, v6Fun fun, vector<state>* stateOutput) {
	stateOutput->push_back(state{ initTime, vec3{ initial[0],initial[1],initial[2] }, vec3{ initial[3],initial[4],initial[5] } });
	double h = deltaTime / 1000.0;
	double hmin = 1.0 / (24.0 * 60.0*60.0); // 1 second
	double hmax = 1.0 / (24.0 * 4.0); // 15 minutes
	double t = initTime;
	double err, s;
	double totErr = 0;
	vec6 y = initial;
	vec6 y1;
	vec6 z1;
	vec6 k1, k2, k3, k4, k5, k6, k7;

	//define the RKF constants
	double h2 = (1.0 / 5.0), h3 = (3.0 / 10.0), h4 = (4.0 / 5.0), h5 = (8.0 / 9.0);
	double b1 = (1.0 / 5.0);
	double c1 = (3.0/40.0), c2 = (9.0 / 40.0);
	double d1 = (44.0 / 45.0), d2 = (-56.0 / 15.0), d3 = (32.0 / 9.0);
	double e1 = (19372.0 / 6561.0), e2 = (-25360.0 / 2187.0), e3 = (64448.0 / 6561.0), e4 = (-212.0 / 729.0);
	double f1 = (9017.0 / 3168.0), f2 = (-355.0 / 33.0), f3 = (46732.0 / 5247.0), f4 = (49.0 / 176.0),f5 = (-5103.0 / 18656.0);
	double g1 = (35.0 / 384.0), g3 = (500.0 / 1113.0), g4 = (125.0 / 192.0), g5 = (-2187.0 / 6784.0), g6 = (11.0 / 84.0);

	double p1 = (35.0 / 384.0), p3 = (500.0 / 1113.0), p4 = (125.0 / 192.0), p5 = (-2187.0 / 6784.0), p6 = (11.0 / 84.0);
	double q1 = (5179.0 / 57600.0), q3 = (7571.0 / 16695.0), q4 = (393.0 / 640.0), q5 = (-92097.0 / 339200.0), q6 = (187.0 / 2100.0), q7 = (1.0 / 40.0);


	double outputStep = 0.0;
	double outputTime = 1.0 / (24.0*60.0*4.0); //output a point every ~15 seconds
	bool contFlag = false;
	bool warnFlag = false;
	while (t + h < initTime + deltaTime) {
		k1 = fun(t, y);
		k2 = fun(t + h2 * h, y + h * (b1*k1));
		k3 = fun(t + h3 * h, y + h * (c1 * k1 + c2 * k2));
		k4 = fun(t + h4 * h, y + h * (d1 * k1 + d2 * k2 + d3 * k3));
		k5 = fun(t + h5 * h, y + h * (e1 * k1 + e2 * k2 + e3 * k3 + e4 * k4));
		k6 = fun(t +      h, y + h * (f1 * k1 + f2 * k2 + f3 * k3 + f4 * k4 + f5 * k5));
		k7 = fun(t +      h, y + h * (g1 * k1 +           g3 * k3 + g4 * k4 + g5 * k5 + g6 * k6));

		y1 = y + h * (p1*k1 + p3 * k3 + p4 * k4 + p5 * k5 + p6 * k6);
		z1 = y + h * (q1*k1 + q3 * k3 + q4 * k4 + q5 * k5 + q6 * k6 + q7 * k7);

		err = len(z1 - y1);

		s = sqrt(sqrt((tolerance*h) / (2 * err)));

		if (err > tolerance) {
			if (h == hmin) {
				contFlag = true;
				if (!warnFlag) {
					cout << endl;
					cout << "Warning: error exceeding limits. " << endl;
					warnFlag = true;
				}
			}
			else if (h*s < hmin) {
				h = hmin;

				continue;

			}
			else {
				h = h * s;
				continue;
			}
		}
		if (err <= tolerance || contFlag) {
			contFlag = false;
			t += h;
			y = y1;
			totErr += err;
			outputStep += h;
			if (outputStep>outputTime) {
				stateOutput->push_back(state{ t, vec3{ y1[0],y1[1],y1[2] }, vec3{ y1[3],y1[4],y1[5] } });
				outputStep = 0;
			}

			if (h*s > hmax) {
				h = hmax;
			}
			else {
				h = h * s;
			}
		}

	}
	h = (initTime + deltaTime) - t;

	k1 = fun(t, y);
	k2 = fun(t + h2 * h, y + h * (b1*k1));
	k3 = fun(t + h3 * h, y + h * (c1 * k1 + c2 * k2));
	k4 = fun(t + h4 * h, y + h * (d1 * k1 + d2 * k2 + d3 * k3));
	k5 = fun(t + h5 * h, y + h * (e1 * k1 + e2 * k2 + e3 * k3 + e4 * k4));
	k6 = fun(t + h, y + h * (f1 * k1 + f2 * k2 + f3 * k3 + f4 * k4 + f5 * k5));
	k7 = fun(t + h, y + h * (g1 * k1 + g3 * k3 + g4 * k4 + g5 * k5 + g6 * k6));

	y1 = y + h * (p1*k1 + p3 * k3 + p4 * k4 + p5 * k5 + p6 * k6);
	z1 = y + h * (q1*k1 + q3 * k3 + q4 * k4 + q5 * k5 + q6 * k6 + q7 * k7);

	err = len(z1 - y1);

	totErr += err;
	cout << "total error: " << totErr << " ";
	t = initTime + deltaTime;
	stateOutput->push_back(state{ t, vec3{ y1[0],y1[1],y1[2] }, vec3{ y1[3],y1[4],y1[5] } });
}
