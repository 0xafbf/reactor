

#include "tiny_obj_loader.h"

#include "engine.h"
#include "buffer.h"
#include "rmath.h"

struct rGeometry {

	rEngine* engine;

	tinyobj::attrib_t attrib;
	array<tinyobj::shape_t> shapes;
	array<tinyobj::material_t> materials;

	struct vert_data {
		vec3 location;
	};

	array<u32> indices;
	array<vert_data> vertices;

	rBuffer indexBuffer;
	rBuffer vertexBuffer;
	rBuffer projectionBuffer;

	rGeometry() {
	}

	rGeometry(rEngine* inEngine, string source_path);
	
};

