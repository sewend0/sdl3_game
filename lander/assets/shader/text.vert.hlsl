cbuffer uniform_buffer : register(b0, space1) {
    float4x4 mvp;
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
    float4 pos = float4(input.position, 0.0F, 1.0F);

    vs_output output;
    output.color = input.color;
    output.tex_coord = input.tex_coord;
    output.position = mul(mvp, pos);

    return output;
}

// dxc.exe -spirv -T vs_6_0 -E main text.vert.hlsl -Fo text.vert.spv