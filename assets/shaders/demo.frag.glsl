#version 460

// vec4 fragment input at location 0 called v_color
// vec4 output at location 0 called frag_color

layout (location = 0) in vec4 v_color;
layout (location = 0) out vec4 frag_color;

// we set 'set' to 3, SDL has predefined sets for certain things,
// and fragment shaders are one of them, taking in uniforms at set 3
// for more on this: https://wiki.libsdl.org/SDL3/SDL_CreateGPUShader
// 'binding' works like a location, you can bind uniforms at different slots
// 'std140' is a standard that defines how memory is laid out
// SDL_gpu requires you use std140
layout (std140, set = 3, binding = 0) uniform uniform_block {
    float time;
};

void main() {
    // the frag_color ouput uses the v_color input
    // frag_color = v_color;
    float pulse = sin(time * 2.0) * 0.5 + 0.5; // range [0,1]
    frag_color = vec4(v_color.rgb * (0.8 + pulse * 0.5), v_color.a);
}

// glslc -fshader-stage=fragment shaders/fragment.glsl -o shaders/fragment.spv