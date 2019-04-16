// shaders.slang

cbuffer world {
float4x4 projection;
}

struct VertexInput {
float3 location;
float2 uv;
}

[shader("vertex")]
float4 vert(VertexInput vertex) : SV_Position {
	return mul(projection, float4(vertex.location, 1.));
}

struct FragmentInput {
float4 position : SV_POSITION;
}

// Fragment Shader
[shader("fragment")]
float4 frag(FragmentInput input) : SV_Target {
return input.position;
}
