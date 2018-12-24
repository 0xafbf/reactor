
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
	//return float4(in.uv.x, in.uv.y, 0., 1.);
	float4 color = MyTex.Sample(s, in.uv) ;
	color *= 0.3;

	float3 obj_zero_to_one = (in.location + 1.) * 0.5;
	float divisions = 4.;
	int3 co = int3(obj_zero_to_one * divisions);
	color.z = transform[co.x][co.y] ;
	return color; 
}
