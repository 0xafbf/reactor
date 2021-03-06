
#include "geometry.h"

rGeometry::rGeometry(rEngine& inEngine, string source_path):engine(&inEngine) {
	string warn;
	string error;

	let ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, source_path.c_str());

	let& mesh = shapes[0].mesh; // @hardcoded
	let vertCount = mesh.indices.size();

	indices.resize(vertCount);
	vertices.resize(vertCount);

	auto obj_matrix = mat4(0);
	obj_matrix(0, 0) = 1.;
	obj_matrix(1, 2) = 1.;
	obj_matrix(2, 1) = 1.;


	for (u32 idx = 0; idx < vertCount; ++idx)
	{
		indices[idx] = idx;
		let vertIdx = mesh.indices[idx].vertex_index;
		let uvIdx = mesh.indices[idx].texcoord_index;

		let loc_init = vec3(
			attrib.vertices[vertIdx * 3],
			attrib.vertices[vertIdx * 3 + 1],
			attrib.vertices[vertIdx * 3 + 2]);
		
		vertices[idx].location = obj_matrix * loc_init;
		// TODO: check if the model has uvs, if not, do something
		
		if (uvIdx != -1) {
			vertices[idx].uv = {
				attrib.texcoords[uvIdx * 2],
				attrib.texcoords[uvIdx * 2 + 1],
			};
		}
	}
	rGeometryFillBuffers(*this);
	
}

void rGeometryFillBuffers(rGeometry& geometry) {

	let indexBufferSize = geometry.indices.size() * sizeof(u32);
	geometry.indexBuffer = rBuffer(*geometry.engine, geometry.indices.data(), indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	rBufferSync(geometry.indexBuffer);

	let vertexBufferSize = geometry.vertices.size() * sizeof(vert_data);
	geometry.vertexBuffer = rBuffer(*geometry.engine, geometry.vertices.data(), vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	rBufferSync(geometry.vertexBuffer);
}



