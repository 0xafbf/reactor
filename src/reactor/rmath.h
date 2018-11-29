
#pragma once

#include <math.h>

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

	static mat4 location(vec3 loc)
	{
		var r = mat4(1);
		r(3, 0) = loc.x;
		r(3, 1) = loc.y;
		r(3, 2) = loc.z;

		return r;
	}
};




template <class T>
T min(T a, T b)
{
	if (a < b) return a;
	return b;
}
