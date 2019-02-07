#pragma once

#include "log.h"


inline void rBreak() {
	__debugbreak();
}

#define CHECK(cond) {if(!(cond)){ERROR("Assertion Failed: %s %s:%d", #cond, __FILE__, __LINE__);rBreak();}}


// maybe move these to another place later
#define VK_CHECK(op) CHECK(op == VK_SUCCESS)
#define SPV_CHECK(op) CHECK(op == SPV_REFLECT_RESULT_SUCCESS)

#include <initializer_list> // not a fan, but it is a nice syntax

template <class T, size_t N>
bool rDebugCombo(const char* name, T* data, const char* (&fields)[N]) {

	int* ptr = (int*)data;
	return ImGui::Combo(name, ptr, fields, N);
}

template <class T>
bool rDebugCombo(const char* name, T* data, std::initializer_list<const char*> items) {

	int* ptr = (int*)data;
	return ImGui::Combo(name, ptr, items.begin(), items.size(), -1);
}
