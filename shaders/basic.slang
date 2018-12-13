
cbuffer Uniforms
{
	float4x4 projection;
	Texture2D <float4> MyTex;
	SamplerState s;
}


struct Vertex
{
	float3 location;
	float2 uv;
}

struct FragInput
{
	float4 coarseLocation:SV_Position;
	float2 uv;
}

//sampler MySamp;

[shader("vertex")]
FragInput vert(Vertex inVert) 
{
	FragInput r;
	r.coarseLocation = mul(projection, float4(inVert.location, 1.0));
	r.uv = inVert.uv;
	return r;
}

[shader("fragment")]
float4 frag(FragInput in) : SV_Target
{
	//return float4(in.uv.x, in.uv.y, 0., 1.);
	return  MyTex.Sample(s, in.uv);
}
