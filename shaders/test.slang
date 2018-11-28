// shaders.slang

[shader("vertex")]
void vert(
    uint InIndex : INDEX,
    out float2 OutPosition : POSITION
    )
{
    OutPosition = float2(0.0, 0.0);
}

// Fragment Shader
[shader("fragment")]
void frag(
    in float2 Position : POSITION,
    out float4 OutColor : SV_Target0
    )
{
    OutColor = float4(1.0, 0.0, 0.0, 1.0);
}


// compile with:
// slangc.exe .\test.slang -profile ps_5_0 -entry fragmentMain -o test-frag.glsl