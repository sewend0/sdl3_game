cbuffer uniforms : register(b0, space1)
{
    row_major float4x4 proj_view : packoffset(c0);
    row_major float4x4 model : packoffset(c4);
};

struct vs_input
{
    float2 position : TEXCOORD0;
    float4 color : TEXCOORD1;
    float2 tex_coord : TEXCOORD2;
};

struct vs_output
{
    float4 color : TEXCOORD0;
    float2 tex_coord : TEXCOORD1;
    float4 position : SV_Position;
};

vs_output main(vs_input input)
{
    vs_output output;
    output.color = input.color;
    output.tex_coord = input.tex_coord;
    output.position = mul(float4(input.position, 0.0F, 1.0F), mul(model, proj_view));
    return output;
}

// dxc.exe -spirv -T vs_6_0 -E main text.vert.hlsl -Fo text.vert.spv