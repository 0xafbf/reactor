// shaders.slang

float2 positions[3] =
{ 
    {0.0, -0.5},
    {0.5, 0.5},
    {-0.5, 0.5}
};

[shader("vertex")]
void vertexMain(
    uint InIndex : INDEX,
    out float2 OutPosition : POSITION
    )
{
    OutPosition = positions[InIndex];
}

// Fragment Shader
[shader("fragment")]
void fragmentMain(
    in float2 Position : POSITION,
    out float4 OutColor : SV_Target0
    )
{
    OutColor = float4(1.0, 0.0, 0.0, 1.0);
}


// compile with:
// .\bin\windows-x64\release\slangc.exe .\test.slang -profile ps_5_0 -entry fragmentMain -o test-frag.glsl