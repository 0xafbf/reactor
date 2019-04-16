
#pragma once
#include <math.h>

#include "types.h"
#include "debug.h"

// TODO move this to another place, with rDebug functions too.
#include "gui.h"


// just placed here to bring these constants to where they can be looked for, and remove M_ prefix
// I am a big lover of tau, and this is my way of pushing it forward
// Other defines are

// tau is the circle constant, it is a proportion between a
// circle radius and its
#define TAU	6.283185307179586476925286766559005768394338798750211641
#define PI (TAU*0.5)

// e is the base of the natural logarithms, and growth constant used for
// most exponential stuff
#define E	2.718281828459045235360287471352662497757247093699959574

// phi is the golden ratio number, can be used for pattern generation
// and organic looking stuff
#define PHI	1.618033988749894848204586834365638117720309179805762862

#define turn * TAU
#define deg * TAU / 360.0

struct vector2
{
	float x;
	float y;
	vector2() {}
	vector2(float scale): x(scale), y(scale) {}
	vector2(float _x, float _y): x(_x), y(_y) {}
};
typedef vector2 vec2;

struct vector3
{
	float x;
	float y;
	float z;
	
	vector3() {};
	vector3(float in) : x(in), y(in), z(in) {};
	vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {};
	
	vector3 operator+(const vector3* rhs)
	{
		return vector3(this->x + rhs->x, this->y + rhs->y, this->z + rhs->z);
	};

	vector3 operator-()
	{
		return vector3(-this->x, -this->y, -this->z);
	}

	vector3 operator*(const float& rhs)
	{
		return vector3(this->x * rhs, this->y * rhs, this->z * rhs);
	}

	float length() {
		return sqrtf(x*x + y * y + z * z);
	}

	vector3 normalized();

	static vector3 cross(vector3 lhs, vector3 rhs)
	{
		vector3 r;
		r.x = lhs.y * rhs.z - lhs.z * rhs.y;
		r.y = lhs.z * rhs.x - lhs.x * rhs.z;
		r.z = lhs.x * rhs.y - lhs.y * rhs.x;
		return r;
	}
};


typedef vector3 vec3;


struct mat4
{
	float m[16];

	mat4() {}
	mat4(float scale) {
		let size = sizeof(float) * 16;
		memset(m, 0, size);
		m[0] = m[5] = m[10] = scale;
		m[15] = 1.;
	}
	
	float& operator[](u32 idx)
	{
		return m[idx];
	}

	float& operator()(u32 idx, u32 jdx)
	{
		CHECK(idx >= 0);
		CHECK(jdx >= 0);
		CHECK(idx < 4);
		CHECK(jdx < 4);
		return m[idx * 4 + jdx];
	}
	
	const float& operator()(u32 idx, u32 jdx) const
	{
		CHECK(idx >= 0);
		CHECK(jdx >= 0);
		CHECK(idx < 4);
		CHECK(jdx < 4);
		return m[idx * 4 + jdx];
	}
	
	mat4 operator+(mat4& rhs) {
		mat4 r;
		for (u32 idx = 0; idx < 4; ++idx) {
			for (u32 jdx = 0; jdx < 4; ++jdx) {
				r(idx, jdx) = (*this)(idx, jdx) + rhs(idx, jdx);
			}
		}
		return r;
	}

	mat4 operator*(const mat4& rhs) const
	{
		const mat4& lhs = *this;
		mat4 r;

		for (u32 idx = 0; idx < 4; ++idx)
		{
			for (u32 jdx = 0; jdx < 4; ++jdx)
			{
				r(idx, jdx) =
					lhs(idx, 0) * rhs(0, jdx) +
					lhs(idx, 1) * rhs(1, jdx) +
					lhs(idx, 2) * rhs(2, jdx) +
					lhs(idx, 3) * rhs(3, jdx) ;
			}
		}

		return r;
	}

	vec3 operator*(const vec3& rhs) const{
		auto r = vec3(0);
		let& m = *this;
		r.x = m(0, 0) * rhs.x + m(1, 0) * rhs.y + m(2, 0) * rhs.z;
		r.y = m(0, 1) * rhs.x + m(1, 1) * rhs.y + m(2, 1) * rhs.z;
		r.z = m(0, 2) * rhs.x + m(1, 2) * rhs.y + m(2, 2) * rhs.z;
		return r;
	}



	mat4 transposed()
	{
		mat4 r;
		for (u32 idx = 0; idx < 4; idx++) {
			for (u32 jdx = 0; jdx < 4; jdx++) {
				r(idx, jdx) = (*this)(jdx, idx);
			}
		}
		return r;
	}

	static mat4 location(vec3 loc)
	{
		auto r = mat4(1);
		r(3, 0) = loc.x;
		r(3, 1) = loc.y;
		r(3, 2) = loc.z;

		return r;
	}

	static mat4 orbit(float dist, float pitch, float yaw) {
		
		vec3 viewLocation;
		viewLocation.x = dist * sin(yaw) * cos(pitch);
		viewLocation.y = dist * cos(yaw) * cos(pitch);
		viewLocation.z = dist * sin(pitch);

		// projection vectors
		auto viewForward = vec3(-viewLocation.x, -viewLocation.y, -viewLocation.z).normalized();
		auto viewUp = vec3(0.0, 0.0, 1.0);
		auto viewRight = vec3::cross(viewUp, viewForward).normalized();
		viewUp = vec3::cross(viewForward, viewRight);
		
		auto view = mat4(1);
		view(0, 0) = viewForward.x;
		view(0, 1) = viewForward.y;
		view(0, 2) = viewForward.z;

		view(1, 0) = viewRight.x;
		view(1, 1) = viewRight.y;
		view(1, 2) = viewRight.z;

		view(2, 0) = viewUp.x;
		view(2, 1) = viewUp.y;
		view(2, 2) = viewUp.z;
		view = mat4::location(-viewLocation) *	view.transposed();

		return view;
	}

	enum aspect_mode {
		ASPECT_HORIZONTAL,
		ASPECT_VERTICAL ,
		ASPECT_MIN,
		ASPECT_MAX,
	};

	// I decided to call this cam_perspective, just because I want to do another perspective that is better suited for effects/data
	static mat4 cam_perspective(float fov, float aspect_ratio = 16 / 9, aspect_mode mode = ASPECT_VERTICAL, float near_plane = 0.01, float far_plane = 1000);
	
	

	static mat4 screen()
	{
		auto r = mat4(0.);
		r(0, 2) = 1.;
		r(1, 0) = 1.;
		r(2, 1) = -1.;
		r(3, 3) = 1.;
		return r;
	}
};


template <class T>
T min(T a, T b)
{
	if (a < b) return a;
	return b;
}

template <class T>
T lerp(T a, T b, float t)
{
	return (a * mat4(1 - t)) + b * mat4(t);
}


struct rOrbitCamera {
	float distance = 4.0;
	float yaw = 45 deg;
	float pitch = 30 deg;
	float fov = 30 deg;

	bool bDebug = false;
};

void rCameraTick(rOrbitCamera& camera);

mat4 rCameraProject(rOrbitCamera& camera, float aspect_ratio);
struct rTransform {
	vec3 location = vec3(0.);
	vec3 rotation = vec3(0.);// Euler angles XYZ
	vec3 scale = vec3(1.);
};

inline mat4 rRotateMatrix(vec3 axis, float angle)
{
	mat4 r(1.);
	r(0, 0) = cos(angle) + (axis.x * axis.x * (1. - cos(angle)));
	r(1, 1) = cos(angle) + (axis.y * axis.y * (1. - cos(angle)));
	r(2, 2) = cos(angle) + (axis.z * axis.z * (1. - cos(angle)));

	r(0, 1) = (axis.x * axis.y * (1. - cos(angle))) - (axis.z * sin(angle));
	r(1, 0) = (axis.y * axis.x * (1. - cos(angle))) + (axis.z * sin(angle));
	r(1, 2) = (axis.y * axis.z * (1. - cos(angle))) - (axis.x * sin(angle));
	r(2, 1) = (axis.z * axis.y * (1. - cos(angle))) + (axis.x * sin(angle));
	r(2, 0) = (axis.z * axis.x * (1. - cos(angle))) - (axis.y * sin(angle));
	r(0, 2) = (axis.x * axis.z * (1. - cos(angle))) + (axis.y * sin(angle));
	
	return r;
}

// it would be interesting if there was a language that allowed something like:
//   mat4 rRotateFwdMatrix(float angle) = #bake rRotateMatrix(vec3(1.,0.,0.));

inline mat4 rRotateMatrix(vec3 eulerAngles) {
	mat4 r(1.);
	r = r * rRotateMatrix(vec3(1., 0., 0.), eulerAngles.x);
	r = r * rRotateMatrix(vec3(0., 1., 0.), eulerAngles.y);
	r = r * rRotateMatrix(vec3(0., 0., 1.), eulerAngles.z);
	return r;
}
inline mat4 rScaleMatrix(vec3 scale) {
	mat4 r(1.);
	r(0, 0) = scale.x;
	r(1, 1) = scale.y;
	r(2, 2) = scale.z;
	return r;
}

inline mat4 rTransformMatrix(const rTransform& transform) {
	auto r = rScaleMatrix(transform.scale);
	r = r * rRotateMatrix(transform.rotation);
	r = r * mat4::location(transform.location);
	return r;
}

inline void rDebug(rTransform& transform, string text, bool snap = false)
{
	ImGui::PushID(&transform);
	ImGui::DragFloat3("location", &transform.location.x, 0.01);
	ImGui::DragFloat3("rotation", &transform.rotation.x, 0.01);
	//transform.rotation.x -= fmod(transform.rotation.x ,(TAU / 8));// try snap later
	ImGui::DragFloat3("scale", &transform.scale.x, 0.01);
	ImGui::PopID();
}

inline void rDebug(mat4& mat, string name) {
	float scroll_speed = 0.01;
	float limit = 10.;
	ImGui::PushID(&mat);
	ImGui::DragFloat4("mat[0]", &mat.m[0], scroll_speed, -limit, limit);
	ImGui::DragFloat4("mat[1]", &mat.m[4], scroll_speed, -limit, limit);
	ImGui::DragFloat4("mat[2]", &mat.m[8], scroll_speed, -limit, limit);
	ImGui::DragFloat4("mat[3]", &mat.m[12], scroll_speed, -limit, limit);
	ImGui::PopID();
}