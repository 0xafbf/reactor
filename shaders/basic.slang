
cbuffer instance
{
	float4x4 transform;
}

cbuffer world
{
	float4x4 projection;
}


Texture2D <float4> MyTex;
SamplerState s;

struct Vertex
{
	float3 location;
	float2 uv;
}

struct FragInput
{
	float3 location;
	float4 coarseLocation:SV_Position;
	float2 uv;
}
[shader("vertex")]
FragInput vert(Vertex inVert) 
{
	FragInput r;
	r.location = inVert.location;
	float4 point = float4(inVert.location, 1.0);
	point = mul(transform, point);
	point = mul(projection, point);
	r.coarseLocation = point;
	r.uv = inVert.uv;
	return r;
}

[shader("fragment")]
float4 frag(FragInput in) : SV_Target
{
	float4 color = MyTex.Sample(s, in.uv);
	return color; 
}
