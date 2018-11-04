
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