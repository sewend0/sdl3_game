// Constant buffer, which maps to a GLSL uniform block.
// The GLSL binding "set = 3, binding = 0" translates to "register(b0, space3)".
cbuffer uniform_block : register(b0, space3)
{
    float time;
};

// Defines the input structure for the pixel shader.
// The GLSL "layout (location = 0) in" corresponds to an input semantic like COLOR0.
struct v_input
{
    float4 color : COLOR0;
};

// The main entry point for the pixel shader.
// The GLSL "layout (location = 0) out" corresponds to the SV_Target system-value semantic.
float4 main(v_input input) : SV_Target
{
    // The core logic is identical to GLSL, using 'f' suffixes for float literals.
    float pulse = sin(time * 2.0f) * 0.5f + 0.5f; // range [0,1]

    // Calculate the final RGB values.
    float3 final_rgb = input.color.rgb * (0.8f + pulse * 0.5f);

    // Return the final color, combining the new RGB with the original alpha.
    return float4(final_rgb, input.color.a);
}