
#pragma once

#define IM_ASSERT CHECK
#define STBTT_assert CHECK

#include "imgui.h"


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
