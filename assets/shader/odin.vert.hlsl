cbuffer uniform_buffer : register(b0, space1) {
    float4x4 mvp;
};

struct vs_output {
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

vs_output main(uint v_id : SV_VERTEXID) {

    float2 pos_in;
    switch (v_id) {
        case 0: pos_in = float2(-50.0F, -50.0F); break;
        case 1: pos_in = float2(50.0F, -50.0F); break;
        case 2: pos_in = float2(0.0F, 50.0F); break;
    }

    float4 pos_out = float4(pos_in, 0.0F, 1.0F);

    vs_output output;
    output.position = mul(mvp, pos_out);
    output.color = float4(1.0F, 0.0F, 0.0F, 1.0F);

    return output;
}

// dxc.exe -spirv -T vs_6_0 -E main odin.vert.hlsl -Fo odin.vert.spv