
cbuffer Uniforms
{
	float4x4 projection;
}


struct Vertex
{
	float3 location;
}

[shader("vertex")]
float4 vert(Vertex inVert) : SV_Position
{
	return mul(projection, float4(inVert.location, 1.0));
}

[shader("fragment")]
float4 frag(float4 inColor) : SV_Target
{
	return float4(1.0f);
}