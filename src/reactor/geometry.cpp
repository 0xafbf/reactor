
#include "geometry.h"

rGeometry::rGeometry(rEngine* inEngine, string source_path):engine(inEngine) {
	string warn;
	string error;

	let ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, source_path.c_str());

	let& mesh = shapes[0].mesh; // @hardcoded
	let vertCount = mesh.indices.size();

	indices.resize(vertCount);
	vertices.resize(vertCount);

	for (u32 idx = 0; idx < vertCount; ++idx)
	{
		indices[idx] = idx;
		let vertIdx = mesh.indices[idx].vertex_index;
		vertices[idx].location = {
			attrib.vertices[vertIdx * 3],
			attrib.vertices[vertIdx * 3 + 1],
			attrib.vertices[vertIdx * 3 + 2],
		};
	}

	let indexBufferSize = vertCount * sizeof(u32);
	indexBuffer = rBuffer(engine, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
	rBufferSetMemory(&indexBuffer, indexBufferSize, indices.data());

	let vertexBufferSize = vertCount * sizeof(vert_data);
	vertexBuffer = rBuffer(engine, vertexBufferSize,  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
	rBufferSetMemory(&vertexBuffer, vertexBufferSize, vertices.data());
	
	projectionBuffer = rBuffer(engine, sizeof(mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
	auto projection = mat4(1.0);
	rBufferSetMemory(&projectionBuffer, sizeof(mat4), &projection);

}



