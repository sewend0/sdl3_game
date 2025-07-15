#version 460

// vec4 fragment input at location 0 called v_color
// vec4 output at location 0 called frag_color

layout (location = 0) in vec4 v_color;
layout (location = 0) out vec4 frag_color;

void main() {
    // the frag_color ouput uses the v_color input
    frag_color = v_color;
}

// glslc -fshader-stage=fragment shaders/fragment.glsl -o shaders/fragment.spv