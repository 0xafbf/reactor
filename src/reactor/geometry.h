#pragma once

#include "tiny_obj_loader.h"

#include "engine.h"

#include "buffer.h"
#include "types.h"
#include "rmath.h"

struct vert_data {
	vec3 location;
	vec2 uv;
};

struct rGeometry {

	rEngine* engine;

	tinyobj::attrib_t attrib;
	array<tinyobj::shape_t> shapes;
	array<tinyobj::material_t> materials;

	array<u32> indices;
	array<vert_data> vertices;

	rBuffer indexBuffer;
	rBuffer vertexBuffer;

	rGeometry(rEngine& inEngine):engine(&inEngine) {
	}

	rGeometry(rEngine& inEngine, string source_path);
	
};

void rGeometryFillBuffers(rGeometry & geometry);
