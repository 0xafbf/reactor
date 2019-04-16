
cbuffer instance
{
	float4x4 transform;
}

cbuffer world
{
	float4x4 projection;
}

struct Vertex {
float3 location;
float2 uv;
}

struct FragInput
{
	float3 location;
	float2 uv;
}

struct VertexOutput {
	FragInput frag_input;
	float4 location : SV_Position;
	float size : SV_PointSize;
}

[shader("vertex")]
VertexOutput vert(Vertex in_vert)
{
	VertexOutput r;
	float4 point = float4(in_vert.location, 1.0);
	size = 5.;
	float size = 20.;
	r.size = 0.05*(2. + sin(size * in_vert.uv.x) + cos(size * in_vert.uv.y));
	point.z = r.size;
	point = mul(transform, point);
		
	r.frag_input.location = point.xyz;
	r.frag_input.uv = in_vert.uv;

	point = mul(projection, point);

	r.location = point;
	r.location.z = r.size;

	return r;
}

[shader("fragment")]
float4 frag(FragInput in) : SV_Target
{
	float3 location = in.location;
	return float4(location, 1.0);
}
