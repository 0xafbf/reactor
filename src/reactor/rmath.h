
#pragma once

#include <math.h>



let TAU = 3.141592 * 2;

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

	vector3 normalized() {
		let len = length();
		vector3 r = *this;
		r.x /= len;
		r.y /= len;
		r.z /= len;

		return r;
	}

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
		m[0] = m[5] = m[10] = m[15] = scale;
	}
	
	float& operator[](u32 idx)
	{
		return m[idx];
	}

	float& operator()(u32 idx, u32 jdx)
	{
		assert(idx >= 0);
		assert(jdx >= 0);
		assert(idx < 4);
		assert(jdx < 4);
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

	mat4 operator*(mat4& rhs)
	{
		mat4& lhs = *this;
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
		var r = mat4(1);
		r(3, 0) = loc.x;
		r(3, 1) = loc.y;
		r(3, 2) = loc.z;

		return r;
	}

	static mat4 orbit(float dist, float pitch, float yaw) {
		
		vec3 viewLocation;
		viewLocation.x = dist * sinf(yaw / 360.0 * TAU) * cosf(pitch / 360 * TAU);
		viewLocation.y = dist * cosf(yaw / 360.0 * TAU) * cosf(pitch / 360 * TAU);
		viewLocation.z = dist * sinf(pitch / 360.0 * TAU);

		// vectores de proyeccion
		var viewForward = vec3(-viewLocation.x, -viewLocation.y, -viewLocation.z);
		viewForward = viewForward.normalized();
		//viewUp = viewUp.normalized();
		var viewUp = vec3(0.0, 0.0, 1.0);
		var viewRight = vec3::cross(viewUp, viewForward).normalized();
		viewUp = vec3::cross(viewForward, viewRight);
		
		//view = mat4::location(viewLocation );
		//////
		var view = mat4(1);
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
	static mat4 cam_perspective(float fov, float aspect_ratio = 16/9, aspect_mode mode = ASPECT_VERTICAL,float near = 0.01, float far = 1000) {

		float aspect_x = tanf(fov);
		float aspect_y = aspect_x;
		switch (mode)
		{
		case ASPECT_HORIZONTAL:
			aspect_y /= aspect_ratio;
			break;
		case ASPECT_VERTICAL:
			aspect_x *= aspect_ratio;
			break;
		default:
			assert(false); // Unimplemented cam mode
		}
		
		var r = mat4(0);
		r(0, 0) = -far/ (near - far);
		r(0, 3) = 1.0;

		r(1, 1) = 1 / aspect_x;
		r(2, 2) = 1  /aspect_y;

		r(3, 0) = (near * far) / (near - far);
		
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