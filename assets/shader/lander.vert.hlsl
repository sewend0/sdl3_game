
cbuffer transform_buffer : register(b0) {
    float4x4 u_mvp;
};

// Hold a model-view-projection matrix sent from cpu
// This handles
//     - Model transform: position + rotation of lander
//     - View transform: typically identity for 2D
//     - Projection transform: convert from world to screen
//         - Usually orthographic in 2D

struct vs_input {
    float2 position : POSITION;
    float4 color : COLOR0;
};

struct vs_output {
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

vs_output main(vs_input input) {
    vs_output output;
    float4 pos = float4(input.position, 0.0f, 1.0f);
    output.position = mul(u_mvp, pos);
    output.color = input.color;
    return output;
};

// Take in 2D vertex position and color from vertex buffer
// Convert 2D position into 4D vector for matrix multiplication
//     - Set Z = 0, and W = 1, standard for 2D rendering in 3D pipelines
// Transform from model space to screen space
//     - Multiply by u_mvp matrix
// Pass transformed position and color out to fragment shader

// dxc.exe -spirv -T vs_6_0 -E main lander.vert.hlsl -Fo lander.vert.spv