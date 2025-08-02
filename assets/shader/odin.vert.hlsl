cbuffer uniform_buffer : register(b0, space1) {
    float4x4 mvp;
};

struct vs_output {
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

vs_output main(uint v_id : SV_VERTEXID) {

    float2 pos_a;
    switch (v_id) {
        case 0: pos_a = float2(-0.5f, -0.5f); break;
        case 1: pos_a = float2(0.5f, -0.5f); break;
        case 2: pos_a = float2(0.0f, 0.5f); break;
    }

    float4 pos_b = float4(pos_a, 0.0f, 1.0f);

    vs_output output;
    output.position = mul(mvp, pos_b);
    output.color = float4(1.0f, 0.0f, 0.0f, 1.0f);

    return output;
}

// dxc.exe -spirv -T vs_6_0 -E main odin.vert.hlsl -Fo odin.vert.spv