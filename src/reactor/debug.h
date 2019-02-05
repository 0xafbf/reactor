#pragma once

#include "log.h"


inline void rBreak() {
	__debugbreak();
}

#define CHECK(cond) {if(!(cond)){ERROR("Assertion Failed: %s %s:%d", #cond, __FILE__, __LINE__);rBreak();}}


// maybe move these to another place later
#define VK_CHECK(op) CHECK(op == VK_SUCCESS)
#define SPV_CHECK(op) CHECK(op == SPV_REFLECT_RESULT_SUCCESS)