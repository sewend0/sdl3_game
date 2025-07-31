struct vs_output {
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

float4 main(vs_output input) : SV_TARGET {
    return input.color;
}

// Take interpolated color from vertex shader
// Return it as final output color

// dxc.exe -spirv -T ps_6_0 -E main lander.frag.hlsl -Fo lander.frag.spv