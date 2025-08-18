Texture2D<float4> tex : register(t0, space2);
SamplerState samp : register(s0, space2);

struct ps_input {
    float4 color : TEXCOORD0;
    float2 tex_coord : TEXCOORD1;
};

struct ps_output {
    float4 color : SV_Target;
};

ps_output main(ps_input input) {
    ps_output output;
    output.color = input.color * tex.Sample(samp, input.tex_coord);
    return output;
}

// dxc.exe -spirv -T ps_6_0 -E main text.frag.hlsl -Fo text.frag.spv