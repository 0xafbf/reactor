
/// TODO: check how cbuffer works, and how it compares with ParameterBlock

//cbuffer instance {
//  float4x4 transform;
//}

// it seems slang creates a descriptorset 0 with all the cbuffer
// and the textures that are out of parameterblocks
// ideally, this should be handled well, as it is something that
// starters will inevitably try out
//cbuffer instance1 {

//}
// Texture2D <float4> MyTex;
// SamplerState s;
struct PerInstance {
	float4x4 transform;
	Texture2D <float4> MyTex;
	SamplerState s;
};

ParameterBlock<PerInstance> instance;

struct PerView {
	float4x4 projection;
}
ParameterBlock<PerView> world;

cbuffer test{
float3 z2;
}

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
	point = mul(instance.transform, point);

//	point.z += sin(point.x * z);

	float3 original_point = point.xyz;
	float4 new_point = 0;

	
	float z = z2.x;
	float m = z2.y;
	float s = z2.z;

	float theta = atan2(original_point.y, original_point.x);
	float dist = sqrt(original_point.x*original_point.x+original_point.y*original_point.y) * m;
	float h = original_point.z;

	new_point.x	= (z+h*s) * sin(dist) * cos(theta);
	new_point.y	= (z+h*s) * sin(dist) * sin(theta);
	new_point.z	= (z+h*s) * cos(dist);
	new_point.w = 1;
	point = new_point;
	point = mul(world.projection, point);
	r.coarseLocation = point;
	r.uv = inVert.uv;
	return r;
}

[shader("fragment")]
float4 frag(FragInput in, float4 pos:SV_Position) : SV_Target
{
	//float4 color = float4(in.uv.xy, 0., 1.);//MyTex.Sample(s, in.uv);
	//color.z = sin(color.x * 20) * cos(color.y * 20) + z;
	//color = pos/500;
	float4 color = float4(1.0);//float4(in.uv * 2, 0, 0);
	return color; 
}
