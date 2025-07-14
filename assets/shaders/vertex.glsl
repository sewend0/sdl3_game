// indicates OpenGL 4.6 version
// doesn't matter, we will be converting to SPIRV
#version 460

// set variables to certain locations
// vec3 (3 floats), attribute (input), called a_position
// mapped to first vertex attribute at location 0
// attribute a_color that is a vec4 at location 1
// the shader will take inputs (in) from our vertex buffer
// then map these to the correct variables
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;

// a 'varying' (output) called v_color that is a vec4 at location 0
// this will be passed to our fragment shader later
layout (location = 0) out vec4 v_color;

// the shader simply sets the vertex position (using gl_position)
// to the position passed in by the vertex buffer
// since it is a vec4 we add 1.0f as the last value (w)
// (it has to do with cameras an perspective)
// we set the output v_color to the input a_color
// this just passes the variable from vertex buffer to the fragment shader
void main() {
    gl_Position = vec4(a_position, 1.0f);
    v_color = a_color;
}

// to actually compile this .glsl shader to the .spv format,
// you need the Vulkan SDK, and if installed correctly you will
// have access to the command 'glslc' that can compile the shader
// 'glslc -fshader-stage=vertex shaders/vertex.glsl -o shaders/vertex.spv'
// running that command will take in the .glsl shader and output in .spv
// it also sets the stage to be a vertex shader