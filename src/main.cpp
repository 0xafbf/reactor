
#include "reactor.h"

// this should be called something else
rGeometry rGeometryGrid(rEngine& engine, u32 x_steps, u32 y_steps, u32 z_steps) {
	CHECK(x_steps > 0);
	CHECK(y_steps > 0);
	CHECK(z_steps > 0);

	auto r = rGeometry(engine);
	let size = x_steps * y_steps * z_steps;
	let dx = 1.0f / float(x_steps-1);
	let dy = 1.0f / float(y_steps-1);
	let dz = 1.0f / float(z_steps-1);

	auto& vertices = r.vertices;
	vertices.resize(size);
	for (u32 idx = 0; idx < z_steps; ++idx) {
		for (u32 jdx = 0; jdx < y_steps; ++jdx) {
			for (u32 kdx = 0; kdx < y_steps; ++kdx) {
				let index = (idx * z_steps + jdx) *y_steps + kdx;
				vertices[index] = { vec3(kdx * dx, jdx * dy, idx * dz) };
			}
		}
	}
	auto& indices = r.indices;
	indices.resize(size); // naive indexing
	for (u32 idx = 0; idx < size; ++idx) {
		indices[idx] = idx;
	}

	rGeometryFillBuffers(r);

	return r;
}


rGeometry rGeometryGrid(rEngine& engine, u32 x_steps = 21, u32 y_steps=21, bool generate_inner_vertices = true) {
	CHECK(x_steps > 0);
	CHECK(y_steps > 0);

	auto r = rGeometry(engine);
	let dx = 1.0f / float(x_steps - 1);
	let dy = 1.0f / float(y_steps - 1);

	let x_2 = x_steps - 1;
	let y_2 = y_steps - 1;

	auto& vertices = r.vertices;
	auto& indices = r.indices;

	if (generate_inner_vertices)
	{
		let size = x_steps * y_steps;
		vertices.resize(size);
		u32 index = 0;
		for (u32 idx = 0; idx < y_steps; ++idx) {
			for (u32 jdx = 0; jdx < x_steps; ++jdx) {
				float u = float(jdx) * dx;
				float v = float(idx) * dy;
				vertices[index].location = vec3(u, v, 0);
				vertices[index].uv = vec2(u, v);
				index += 1;
			}
		}

		let indices_size = 2*( 2 * x_steps * y_steps - x_steps - y_steps);
		indices.resize(indices_size);
		index = 0;
		for (u32 idx = 0; idx < y_steps; ++idx) {
			for (u32 jdx = 0; jdx < x_steps; ++jdx) {
				if (jdx + 1 < x_steps) {
					indices[index++] = (idx * x_steps) + jdx;
					indices[index++] = (idx * x_steps) + jdx + 1;
				}
				if (idx + 1 < y_steps) {
					indices[index++] = (idx * x_steps) + jdx;
					indices[index++] = ((idx + 1) * x_steps) + jdx;
				}
			}
		}
	}
	else {
		let size = 2 * (x_steps + y_steps - 2);
		vertices.resize(size);
		u32 index = 0;
		for (u32 idx = 0; idx < y_steps; ++idx) {
			for (u32 jdx = 0; jdx < x_steps; ++jdx) {
				if (((idx% y_2)* (jdx% x_2)) == 0)
				{
					vertices[index].location = vec3(jdx * dx, idx * dy, 0);
					vertices[index].uv = vec2(jdx * dx, idx * dy);

					index += 1;

				}
			}
		}

		let indices_size = 2 * (x_steps + y_steps);
		indices.resize(indices_size); // naive indexing
		for (u32 idx = 0; idx < x_steps; ++idx) {
			// vertical lines
			auto first = idx;
			auto second = size - x_steps + idx;
			indices[2 * idx] = first;
			indices[2 * idx + 1] = second;
		}
		let offset = 2 * x_steps;
		{
			indices[offset] = 0;
			let iterations = 2 * (y_steps - 1);
			for (u32 idx = 0; idx < iterations; ++idx)
			{
				indices[offset + idx + 1] = idx + x_steps - 1;
			}
			indices[indices_size - 1] = size - 1;
		}
	}

	rGeometryFillBuffers(r);

	return r;
}

struct rGrid
{
	rGeometry geometry;
	rGraphicsPipeline pipeline;
	rState state;
	rBuffer transform_buffer;
	rGrid(rEngine &inEngine):geometry(inEngine) {};

	operator rState&()
	{
		return state;
	}

	operator rState*()
	{
		return &state;
	}
};

rGrid rGridNew(rEngine& engine, float size = 5.0, u32 subdivs = 21, bool with_inner_vertices = true)
{
	auto grid = rGrid(engine);

	// maybe we need a parameter to configure if the grid is made from limit points, or from every point in the grid

	grid.geometry = rGeometryGrid(engine, subdivs, subdivs, with_inner_vertices);
	grid.pipeline = rPipeline(engine, "shaders/sphere.slang");
	grid.pipeline.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	rPipelineUpdate(grid.pipeline);

	grid.state.pipeline = &grid.pipeline;
	grid.state.geometry = &grid.geometry;

	auto transform_mat = mat4(2. * size) * mat4::location(vec3(-size, -size, 0.));

	// we have a problem here, it is that this buffer gets destroyed when this scope ends
	// let's give it to the grid struct, I would like to know if there is a better way around this.
	// Ideally, this buffer should only live in gpu, and be destroyed from the cpu
	grid.transform_buffer = rBuffer(engine, &transform_mat, sizeof(mat4));
	//auto transform_buffer = rBuffer(engine, &transform_mat, sizeof(mat4));
	rBufferSync(grid.transform_buffer);

	grid.state.descriptor_sets = rDescriptorSets(engine.device, engine.descriptor_pool, grid.pipeline.shader.descriptor_set_layouts);
	
	rStateSetDescriptor(engine.device, grid.state, 1, 0, grid.transform_buffer);

	return grid;
}

void rDebug(rGrid& grid) {

}


int main()
{
	auto engine = rEngine("My Great App");
	auto window = rWindow(engine, "My Greatest Window", 1024, 1024);
	
	//auto pipeline = rPipeline(engine, "shaders/basic.slang");

	auto grid = rGridNew(engine, 4, 51);

	rScene scene;
	scene.primitives.push_back(grid);
	window.scene = &scene;
	
/*
	auto transform = rTransform();
	auto transform_mat = rTransformMatrix(transform);
	auto transform_buffer = rBuffer(engine, &transform_mat, sizeof(mat4));
	rStateSetDescriptor(engine.device, state.descriptor_sets[0], 0, transform_buffer);
	//rStateSetDescriptor(engine.device, grid.state.descriptor_sets[0], 0, grid.transform_buffer);
	*/
	
	//auto image = rImage(engine, "content/uv.png");
	//rStateSetDescriptor(engine.device, state, 2, image, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	//rStateSetDescriptor(engine.device, state, 3, image, VK_DESCRIPTOR_TYPE_SAMPLER);

	// per scene
	auto camera = rOrbitCamera();
	camera.distance = 3.0;
	auto projection = mat4(0.);
	auto projection_buffer = rBuffer(engine, &projection, sizeof(mat4));
	//rStateSetDescriptor(engine.device, state.descriptor_sets[0], 1, projection_buffer);
	rStateSetDescriptor(engine.device, grid.state, 2, 0, projection_buffer);

	auto transf2 = rTransform();

	auto z = vec3(1, 1, 1);
//	auto z_mem = rBufferCreate(engine, z);
	auto z_mem = rBuffer(engine, &z, sizeof(vec3));
	rStateSetDescriptor(engine.device, grid.state, 0, 0, z_mem);

	rState state;
	scene.primitives.push_back(&state);
	auto geom = rGeometry(engine, "content/proj.obj");
	state.geometry = &geom;
	
	state.pipeline = &grid.pipeline;
	state.descriptor_sets = rDescriptorSets(engine.device, engine.descriptor_pool, grid.pipeline.shader.descriptor_set_layouts);
	rStateSetDescriptor(engine.device, state, 0, 0, z_mem);
	auto transf = rTransform();
	auto transf_mat = rTransformMatrix(transf);
	auto transf_mem = rBuffer(engine, &transf_mat, sizeof(mat4));
	rStateSetDescriptor(engine.device, state, 1, 0, transf_mem);
	rStateSetDescriptor(engine.device, state, 2, 0, projection_buffer);


	
	while ( rEngineStartFrame(engine))
	{
		ImGui::Begin("Debug");
		ImGui::DragFloat3("z", &z.x, 0.02);
		rDebug(transf, "transform");
		transf_mat = rTransformMatrix(transf);
		rBufferSync(transf_mem);
		rBufferSync(z_mem);

		rDebug(grid.pipeline);
		ImGui::End();
//		rDebug(transform, "transform");
//		transform_mat = rTransformMatrix(transform) * rTransformMatrix(transf2);
//		rDebug(transform_mat, "matrix");
//		rBufferSync(transform_buffer);

		rCameraTick(camera);
		float aspect_ratio = float(window.width) / float(window.height);
		projection = rCameraProject(camera, aspect_ratio) * mat4::screen();
		rBufferSync(projection_buffer);

		rShaderShowLog();
		rEngineEndFrame(engine);
	}
	
	return 0;
}