struct v_input
{
    float3 position : POSITION; // location = 0
    float4 color    : COLOR0;   // location = 1
};

struct v_output
{
    float4 position : SV_Position;
    float4 color    : COLOR0;
};

v_output main(v_input input)
{
    v_output output;
    output.position = float4(input.position, 1.0f);
    output.color = input.color;
    return output;
}
