struct vs_output {
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

float4 main(vs_output input) : SV_TARGET {
    return input.color;
}
// dxc.exe -spirv -T ps_6_0 -E main odin.frag.hlsl -Fo odin.frag.spv