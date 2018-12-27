
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
	//float4 color = float4(1.0);
	color *= 0.3;

	float2 tile_coordinates = in.uv;

	float4x4 distorted;
	for(int i = 0; i < 3; ++i){
		for(int j = 0; j < 3; ++j){
			float2 inner = 3. * sin(1. * in.uv * float2(i,j));
			float2 new_uv = in.uv + frac(  inner);
			int2 swapped = int2(new_uv * 3)%3;
			distorted[i][j] = abs(transform[swapped.x][swapped.y]);
			distorted[i][j] = lerp(distorted[i][j], 0, -1) + 0.2;
		}
	}
	float4x4 merge = mul(transform, distorted);
	color = mul(merge, color) ;
	color = (color * color );
	return color; 
}
