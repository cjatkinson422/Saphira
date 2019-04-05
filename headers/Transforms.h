#pragma once

#include <array>
#include <iostream>
#include <math.h>
#include "cspice/SpiceUsr.h"

#define PI  3.14159265358979323846

using std::cout;
using std::endl;

typedef std::array<double, 4> vec4;
typedef std::array<vec4, 4> mat4;
typedef std::array<double, 3> vec3;
typedef std::array<vec3, 3> mat3;
typedef std::array<float, 3> vec3f;
typedef std::array<double, 6> vec6;
typedef std::array<double, 2> vec2;
typedef std::array<float, 2> vec2f;
typedef std::array<int, 2> ivec2;
typedef std::array<vec3, 2> statevec;

extern int intcnt;

//Declarations

double radians(double degrees);
double degrees(double radians);
mat4 Mat4Mult(mat4 &m1, const mat4 &m2);
void mat4print(mat4 mat);
void vec4print(vec4 vec);
vec4 Mat4Vec(mat4 m, vec4 v);
vec3 Mat3Vec(mat3 m, vec3 v);
vec4 Normalize(vec4 v);
double Dot(vec3 v1, vec3 v2);
vec3 Cross(vec3 v1, vec3 v2);

//end Declations

//need to check if args are being passed by copy or reference

// MATRIX OPERATOR OVERLOADS
inline mat4 operator*(mat4 m1, mat4 m2) {
	return Mat4Mult(m1, m2);
}
inline mat3 operator*(mat3 m, double d){
	return mat3{
		vec3{m[0][0]*d, m[0][1]*d, m[0][2]*d},
		vec3{m[1][0]*d, m[1][1]*d, m[1][2]*d},
		vec3{m[2][0]*d, m[2][1]*d, m[2][2]*d}
	};
}
inline mat3 operator+(const mat3 &m1, const mat3 &m2){
	return mat3{
		vec3{m1[0][0]+m2[1][0], m1[0][1]+m2[1][1], m1[0][2]+m2[1][2]},
		vec3{m1[1][0]+m2[2][0], m1[1][1]+m2[2][1], m1[1][2]+m2[2][2]},
		vec3{m1[2][0]+m2[3][0], m1[2][1]+m2[3][1], m1[2][2]+m2[3][2]}
	};
}
inline mat3 operator-(const mat3 &m1, const mat3 &m2){
	return mat3{
		vec3{m1[0][0]-m2[1][0], m1[0][1]-m2[1][1], m1[0][2]-m2[1][2]},
		vec3{m1[1][0]-m2[2][0], m1[1][1]-m2[2][1], m1[1][2]-m2[2][2]},
		vec3{m1[2][0]-m2[3][0], m1[2][1]-m2[3][1], m1[2][2]-m2[3][2]}
	};
}

inline vec4 operator*(mat4 m, vec4 v) {
	return Mat4Vec(m, v);
}
inline vec4 operator/(vec4 v, double d) {
	return {v[0] / d,v[1] / d,v[2] / d , v[3] / d};
}
inline vec4 operator*(vec4 v, double d) {
	return {v[0] * d,v[1] * d,v[2] * d , v[3] * d};
}

inline vec3 operator*(mat3 m, vec3 v){
	return Mat3Vec(m,v);
}

inline vec3 operator*(double d, vec3 v) {
	return {d*v[0], d*v[1], d*v[2]};
}
inline vec3 operator*(float d, vec3 v) { 
	return { d*v[0], d*v[1], d*v[2] };
}
inline vec3 operator/(vec3 v, float d) {
	return { v[0]/d,v[1]/d,v[2]/d };
}
inline vec3 operator/(vec3 v, double d) {
	return {v[0] / d,v[1] / d,v[2] / d };
}

inline vec3 operator+(vec3 v1, vec3 v2) {
	return { v1[0] + v2[0],v1[1] + v2[1],v1[2] + v2[2] };
}
inline vec4 operator+(const vec4 &v1, const vec4 &v2) {
	return { v1[0] + v2[0],v1[1] + v2[1],v1[2] + v2[2], v1[3] + v2[3]};
}
inline vec4 operator-(const vec4 &v1, const vec4 &v2) {
	return { v1[0] - v2[0],v1[1] - v2[1],v1[2] - v2[2], v1[3] - v2[3]};
}
inline vec3 operator-(vec3 v1, vec3 v2) {
	return { v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2] };
}
inline vec3 operator+=(vec3& v1, vec3 v2) {
	v1[0] += v2[0];
	v1[1] += v2[1];
	v1[2] += v2[2];
	return v1;
}
inline vec2 operator+(vec2 v1, vec2 v2){
	return vec2{v1[0]+v2[0], v1[1]+v2[1]};
}

inline std::ostream& operator<<(std::ostream& os, vec3 v) {
	os << v[0] << "," << v[1] << "," << v[2] ;
	return os;
}
inline std::ostream& operator<<(std::ostream& os, vec4 v) {
	os << v[0] << "," << v[1] << "," << v[2] << "," << v[3] ;
	return os;
}
inline std::ostream& operator<<(std::ostream& os, mat4 m) {
	os << m[0] << endl << m[1] << endl << m[2] << endl << m[3] ;
	return os;
}
inline std::ostream& operator<<(std::ostream& os, vec6 v) {
	os << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ", " << v[4] << ", " << v[5] << " ";
	return os;
}


//returns a new quaternion from the input angle and axis
inline vec4 quaternion(double angle, vec3 axis) {
	double coeff = sin(angle / 2.0f);
	return vec4{ cos(angle / 2.0f), coeff*axis[0], coeff*axis[1], coeff*axis[2] };
}

//Converts a quaternion to a roation matrix -- try optimizing with non-dynamic memory allocation
inline mat4 qtomat4(vec4 q) {
	//normalize the quaternion
	double n = sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
	for (short i = 0; i < 4; i++) {
		q[i] = q[i] / n;
	}

	//do some magic here to convert the normalized quaternion to a rotation matrix
	mat4 m1 = {
		vec4{ q[0], q[3],-q[2], q[1] },
		vec4{ -q[3], q[0], q[1], q[2] },
		vec4{ q[2],-q[1], q[0], q[3] },
		vec4{ -q[1],-q[2],-q[3], q[0] }
	};
	mat4 m2 = {
		vec4{ q[0], q[3],-q[2],-q[1] },
		vec4{ -q[3], q[0], q[1],-q[2] },
		vec4{ q[2],-q[1], q[0],-q[3] },
		vec4{ q[1], q[2], q[3], q[0] }
	};

	return Mat4Mult(m1, m2);
}

//Performs the hamilton of two vec4's
inline vec4 hamilton(vec4 a, vec4 b) {
	return vec4{
		a[0] * b[0] - a[1] * b[1] - a[2] * b[2] - a[3] * b[3],
		a[0] * b[1] + a[1] * b[0] + a[2] * b[3] - a[3] * b[2],
		a[0] * b[2] - a[1] * b[3] + a[2] * b[0] + a[3] * b[1],
		a[0] * b[3] + a[1] * b[2] - a[2] * b[1] + a[3] * b[0]
	};
}
// quaternion based rotation of vector in global space. angle in degrees
inline vec3 qrot(vec3 v, vec4 q) {
	vec3 u = { q[1],q[2],q[3] };
	vec3 ret = 2*Dot(u,v)*u + (q[0]*q[0] - Dot(u,u))*v + 2*q[0]*Cross(u,v);
	return ret;
}
// quaternion based rotation of vector in local space
inline vec3 qrotl(vec3 v, double angle, vec3 axis) {
	mat4 m = qtomat4(quaternion(angle, axis));
	vec4 r = m * vec4{ v[0], v[1], v[2], 1.0 };
	return { r[0],r[1],r[2] };
}

// Composes two quaternions
inline vec4 quaternionCompose(vec4 q2, vec4 q1){
	vec3 qv; double qs;
	double qs2 = q2[3];
	double qs1 = q1[3];
	vec3 qv2 = {q2[0],q2[1],q2[2]};
	vec3 qv1 = {q1[0],q1[1],q1[2]};
	qs = qs1*qs2 - Dot(qv2,qv1);
	qv = qs1*qv2+ qs2*qv1 - Cross(qv2, qv1);
	
	return vec4{qv[0],qv[1],qv[2],qs};
}

// Returns an identity mat4
inline mat4 eye4() {
	return mat4{
		vec4{ 1.0, 0.0, 0.0, 0.0 },
		vec4{ 0.0, 1.0, 0.0, 0.0 },
		vec4{ 0.0, 0.0, 1.0, 0.0 },
		vec4{ 0.0, 0.0, 0.0, 1.0 }
	};
}
inline mat4 zeros4() {
	return mat4{
		vec4{ 0.0, 0.0, 0.0, 0.0 },
		vec4{ 0.0, 0.0, 0.0, 0.0 },
		vec4{ 0.0, 0.0, 0.0, 0.0 },
		vec4{ 0.0, 0.0, 0.0, 0.0 }
	};
}
inline mat3 eye3(){
	return mat3{
		vec3{1.0,0.0,0.0},
		vec3{0.0,1.0,0.0},
		vec3{0.0,0.0,1.0}
	};
}
inline mat3 zeros3(){
	return mat3{
		vec3{0.0,0.0,0.0},
		vec3{0.0,0.0,0.0},
		vec3{0.0,0.0,0.0}
	};
}

//Multiplies two mat4's together 

inline mat4 Mat4Mult(mat4 &m1, const mat4 &m2) {
	mat4 retmat = zeros4();

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				//possibly optimize with a conditional for sparce matrices?
				retmat[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}
	return retmat;
}
//Multiplies a mat4 with a vector
inline vec4 Mat4Vec(mat4 m, vec4 v) {
	vec4 ret = { 0.0,0.0,0.0,0.0 };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			ret[i] += m[i][j] * v[j];
		}
	}
	return ret;
}

//Cross producs
inline vec3 Cross(vec3 v1, vec3 v2) {
	vec3 ret = { 0.0,0.0,0.0};
	ret[0] = v1[1] * v2[2] - v1[2] * v2[1];
	ret[1] = v1[2] * v2[0] - v1[0] * v2[2];
	ret[2] = v1[0] * v2[1] - v1[1] * v2[0];
	return ret;
}
inline double Dot(vec3 v1, vec3 v2) {
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

inline vec3 Normalize(vec3 v) {
	double w = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] /= w;
	v[1] /= w;
	v[2] /= w;
	return v;
}
inline vec4 Normalize(vec4 v) {
	double w = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
	v[0] /= w;
	v[1] /= w;
	v[2] /= w;
	v[3] /= w;
	return v;
}


inline double len(vec3 v) {
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}
inline double len(vec6 v) {
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3] + v[4] * v[4] + v[5] * v[5]);
}
// these 3 return the basic  transormation matrices
inline mat4 TransMat(vec3 dir) {
	mat4 trans = eye4();
	trans[0][3] = dir[0];
	trans[1][3] = dir[1];
	trans[2][3] = dir[2];
	return trans;
}
inline mat4 RotMat(double angle, vec3 axis) {
	return qtomat4(quaternion(angle, axis));
}
inline mat4 ScaleMat(vec3 scale) {
	mat4 ret = eye4();
	ret[0][0] = scale[0];
	ret[1][1] = scale[1];
	ret[2][2] = scale[2];
	return ret;
}
inline mat4 ProjectionMat(double fovy, double aspect, double nearClip, double farClip) {
	mat4 ret = zeros4();
	double tanHalf = tan(fovy / 2.0);

	ret[0][1] = 1.0 / (aspect*tanHalf); // n/t

	ret[1][2] = 1.0 / tanHalf; // n/r

	ret[2][0] = -(farClip + nearClip) / (farClip - nearClip);
	ret[2][3] = -(2 * farClip*nearClip) / (farClip - nearClip);

	ret[3][0] = -1.0;
	
	return ret;
}

inline mat4 OrthographicMat(double left, double right, double bottom, double top, double zNear, double zFar) {
	mat4 ret = eye4();
	ret[0][0] = 2.0 / (right - left);
	ret[1][1] = 2.0 / (top - bottom);
	ret[2][2] = -2.0 / (zFar - zNear);

	ret[0][3] = -(right + left) / (right - left);
	ret[1][3] = -(top + bottom) / (top - bottom);
	ret[2][3] = -(zFar + zNear) / (zFar - zNear);

	return ret;

}

inline mat3 mat4reduce(mat4 m){
	return mat3{
		vec3{m[0][0],m[0][1],m[0][2]},
		vec3{m[1][0],m[1][1],m[1][2]},
		vec3{m[2][0],m[2][1],m[2][2]}
	};
}

inline vec3 Mat3Vec(mat3 m, vec3 v) {
	vec3 ret = { 0.0,0.0,0.0 };
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			//possibly optimize with a conditional for sparce matrices?
			ret[i] += m[i][j] * v[j];
		}
	}
	return ret;
}

inline mat3 transpose(mat3 m){
	return mat3{
		vec3{m[0][0], m[1][0], m[2][0]},
		vec3{m[0][1], m[1][1], m[2][1]},
		vec3{m[0][2], m[1][2], m[2][2]}
	};
}
inline mat4 transpose(mat4 m){
	return mat4{
		vec4{m[0][0], m[1][0], m[2][0], m[3][0]},
		vec4{m[0][1], m[1][1], m[2][1], m[3][1]},
		vec4{m[0][2], m[1][2], m[2][2], m[3][2]},
		vec4{m[0][3], m[1][3], m[2][3], m[3][3]}
	};
}

inline double getAngularDist(vec3 v1, vec3 v2){
	return acos(Dot(Normalize(v1), Normalize(v2)));
}

//converts degrees to radians
inline double radians(double degrees) {
	return degrees * 3.14159265359f / 180.0f;
}
//converts radians to degrees
inline double degrees(double radians) {
	return radians * 180.0f / 3.14159265359f;
}

inline void mat4print(mat4 mat) {
	cout << mat[0][0] << " " << mat[0][1] << " " << mat[0][2] << " " << mat[0][3] << endl;
	cout << mat[1][0] << " " << mat[1][1] << " " << mat[1][2] << " " << mat[1][3] << endl;
	cout << mat[2][0] << " " << mat[2][1] << " " << mat[2][2] << " " << mat[2][3] << endl;
	cout << mat[3][0] << " " << mat[3][1] << " " << mat[3][2] << " " << mat[3][3] << endl;
}
inline void vec4print(vec4 vec) {
	cout << vec[0] << " " << vec[1] << " " << vec[2] << " " << vec[3] << endl;
}





// VEC6 stuff for ODE

inline vec6 operator*(double d, vec6 v) {
	return vec6{ d*v[0], d*v[1], d*v[2], d*v[3], d*v[4], d*v[5] };
}
inline vec6 operator/(double d, vec6 v) {
	return vec6{ v[0]/d, v[1]/d, v[2]/d, v[3]/d, v[4]/d, v[5]/d };
}
inline vec6 operator/(vec6 v, double f ) {
	return vec6{ v[0] / f, v[1] / f, v[2] / f, v[3] / f, v[4] / f, v[5] / f };
}
inline vec6 operator+(vec6 v1, vec6 v2) {
	return vec6{ v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2], v1[3] + v2[3], v1[4] + v2[4], v1[5] + v2[5] };
}
inline vec6 operator-(vec6 v1, vec6 v2) {
	return vec6{ v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2], v1[3] - v2[3], v1[4] - v2[4], v1[5] - v2[5] };
}

//TIME MANAGEMENT 
inline double getJulianFromUnixS(unsigned long unixS) {
	return (unixS / 86400.0) + 2440587.5;
}
inline double getJulianFromUnixMS(unsigned long long unixMS) {
	return (unixMS / 86400000.0) + 2440587.5;
}
inline double secondsToJulianOffset(double seconds) {
	return seconds / (24.0 * 60.0 * 60.0);
}
inline long int getUnixSFromJulian(double julian) {
	return floor((julian - 2440587.5)*86400.0);
}

//     ORIBITAL CALCULATION FUNCTIONS
inline vec6 rv2oe(vec3 r_ijk, vec3 v_ijk, double mu) {

	double r = len(r_ijk);
	double v = len(v_ijk);

	vec3 h = Cross(r_ijk, v_ijk);
	vec3 n = Normalize(Cross(vec3{ 0.0,0.0,1.0 }, h));
	vec3 ebar = Cross(v_ijk, h) / mu - Normalize(r_ijk);

	double a = -mu / (2.0*((v*v) / 2.0 - mu / r));
	double e = len(ebar);

	double omega = atan2(n[1], n[0]);
	double i = acos(h[2] / len(h));
	double w = acos(Dot(n, Normalize(ebar)));
	

	if (ebar[2] < 0)
		w = 2 * PI - w;

	double nu = acos(Dot(r_ijk, ebar) / (r*e));

	if (Dot(r_ijk, v_ijk) < 0)
		nu = 2*PI-nu;


	return vec6{ a,e,i,w,omega,nu };

}
// a / e / i / w / omega / nu
// semi-major axis / eccentricity / inclination / argument of periapse / longitude of ascending node / true anomaly
inline std::array<vec3,2> oe2rv(vec6 oe, double mu){
	double a = oe[0];
	double e = oe[1];
	double i = oe[2];
	double w = oe[3];
	double omega = oe[4];
	double nu = oe[5];

	double r = a * (1 - e * e) / (1 + e * cos(nu));
	vec3 r_pq = { r*cos(nu), r*sin(nu), 0 };

	double p = a * (1 - e * e);
	vec3 v_pq = {-sin(nu)*sqrt(mu / p), sqrt(mu / p)*(e + cos(nu)), 0 };

	vec3 r_ijk = qrotl(r_pq, -w, vec3{ 0.0,0.0,1.0 });
	r_ijk = qrotl(r_ijk,-i, vec3{ 1.0,0.0,0.0 });
	r_ijk = qrotl(r_ijk, -omega, vec3{ 0.0,0.0,1.0 });

	vec3 v_ijk = qrotl(v_pq, -w, vec3{ 0.0,0.0,1.0 });
	v_ijk = qrotl(v_ijk, -i, vec3{ 1.0,0.0,0.0 });
	v_ijk = qrotl(v_ijk, -omega, vec3{ 0.0,0.0,1.0 });

	std::array<vec3, 2> ret = {r_ijk, v_ijk};
	return ret;
}

inline double cubicInterpolation(double f0, double f1, double fp0, double fp1, double x) {
	double a = 2 * f0 + fp0 - 2 * f1 + fp1;
	double b = -3 * f0 -2 * fp0 + 3 * f1 - fp1;
	return a * x*x*x + b * x*x + fp0 * x + f0;
}

inline mat4 getFrameTransformSpice(double julian, std::string from, std::string to){
	SpiceDouble data[6][6];
	double time = (julian - 2451545.0)*86400;
	sxform_c(from.c_str(), to.c_str(), time, data);
	mat4 mat = eye4();
	mat[0] = {data[0][0],data[0][1],data[0][2],0.0};
	mat[1] = {data[1][0],data[1][1],data[1][2],0.0};
	mat[2] = {data[2][0],data[2][1],data[2][2],0.0};
	mat[3] = {0.0, 0.0, 0.0, 1.0};
	return mat;
}

inline std::string capitalize(std::string in){
	in[0] = toupper(in[0]);
	return in;
}
