#include "rmath.h"

// I decided to call this cam_perspective, just because I want to do another perspective that is better suited for effects/data

mat4 mat4::cam_perspective(float fov, float aspect_ratio, aspect_mode mode, float near_plane, float far_plane) {

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
	r(0, 0) = -far_plane / (near_plane - far_plane);
	r(0, 3) = 1.0;

	r(1, 1) = 1 / aspect_x;
	r(2, 2) = 1 / aspect_y;

	r(3, 0) = (near_plane * far_plane) / (near_plane - far_plane);

	return r;
}

vector3 vector3::normalized() {
	let len = length();
	vector3 r = *this;
	r.x /= len;
	r.y /= len;
	r.z /= len;

	return r;
}

void rCameraTick(rOrbitCamera & camera)
{
	var io = ImGui::GetIO();
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
			let speed = 0.1;
			camera.distance *= exp(delta * speed);
		}
	}

}

mat4 rCameraProject(rOrbitCamera & camera, float aspect_ratio)
{
	let view = mat4::orbit(camera.distance, camera.pitch, camera.yaw);
	let projection = mat4::cam_perspective(camera.fov, aspect_ratio);

	return view * projection;
}
