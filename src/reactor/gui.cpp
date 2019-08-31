
#pragma once

#include "rmath.h"
#include "gui.h"


void rDebug(rTransform& transform, string text, bool snap)
{
	ImGui::PushID(&transform);
	ImGui::DragFloat3("location", &transform.location.x, 0.01);
	ImGui::DragFloat3("rotation", &transform.rotation.x, 0.01);
	//transform.rotation.x -= fmod(transform.rotation.x ,(TAU / 8));// try snap later
	ImGui::DragFloat3("scale", &transform.scale.x, 0.01);
	ImGui::PopID();
}

void rDebug(mat4& mat, string name) {
	float scroll_speed = 0.01;
	float limit = 10.;
	ImGui::PushID(&mat);
	ImGui::DragFloat4("mat[0]", &mat.m[0], scroll_speed, -limit, limit);
	ImGui::DragFloat4("mat[1]", &mat.m[4], scroll_speed, -limit, limit);
	ImGui::DragFloat4("mat[2]", &mat.m[8], scroll_speed, -limit, limit);
	ImGui::DragFloat4("mat[3]", &mat.m[12], scroll_speed, -limit, limit);
	ImGui::PopID();
}


void rCameraTick(rOrbitCamera & camera)
{
	auto io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
	{
		if (io.MouseDown[0]) {
			let delta = io.MouseDelta;
			let speed = 0.3 deg;
			camera.yaw += delta.x * -speed;
			camera.pitch += delta.y * speed;
		}
		if (io.MouseWheel) {
			let delta = io.MouseWheel;
			let speed = 0.2;
			camera.distance *= exp(delta * speed);
		}
	}

}

