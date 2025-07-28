Texture2D<float4> Texture : register(t0, space2);
SamplerState Sampler : register(s0, space2);

struct Input
{
    float2 TexCoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
};

float4 main(Input input) : SV_Target0
{
    return input.Color * Texture.Sample(Sampler, input.TexCoord);
}

// dxc.exe -spirv -T ps_6_0 -E main shaders/textured_quad_color.frag.hlsl -Fo shaders/textured_quad_color.frag.spv