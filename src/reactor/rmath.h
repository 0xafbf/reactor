
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
};


typedef vector3 vec3;

struct mat4
{
	float m[16];

	mat4() {}
	mat4(float scale = 0) {
		let size = sizeof(float) * 16;
		memset(m, 0, size);
		m[0] = m[5] = m[10] = m[15] = scale;
	}
	
	float& operator[](u32 idx)
	{
		return m[idx];
	}
};


template <class T>
T min(T a, T b)
{
	if (a < b) return a;
	return b;
}
