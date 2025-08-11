
// testgputext_cpp.cpp
// Compile with a C++17 toolchain. Needs SDL, SDL_gpu, and SDL_ttf (with GPU text API).
//
// Notes:
//  - This is a 1:1 behavioral translation of the original testgputext.c example
//    but modernized with RAII and std::vector usage.
//  - You still need the generated shader header files used by the example (see includes below).
//  - Build with: g++ -std=c++17 testgputext_cpp.cpp -lSDL2 -lSDL2_gpu -lSDL2_ttf -o testgputext_cpp
//
// Author: translated for you
// Reference: original example file from SDL_ttf repository.

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

//
#include <glm/glm/ext/matrix_clip_space.hpp>
#include <glm/glm/ext/matrix_transform.hpp>
#include <glm/glm/glm.hpp>
#include <glm/glm/mat4x4.hpp>
#include <glm/glm/matrix.hpp>
#include <glm/glm/trigonometric.hpp>
#include <glm/glm/vec2.hpp>
//

// Include the same shader headers the example uses (these are generated binary blobs).
// Keep the same relative paths if you place the generated headers nearby.
// If you don't have these exact files, copy them from the SDL_ttf example shaders folder.
#include <format>

#include "testgputext/shaders/shader-sdf.frag.dxil.h"
#include "testgputext/shaders/shader-sdf.frag.msl.h"
#include "testgputext/shaders/shader-sdf.frag.spv.h"
#include "testgputext/shaders/shader.frag.dxil.h"
#include "testgputext/shaders/shader.frag.msl.h"
#include "testgputext/shaders/shader.frag.spv.h"
#include "testgputext/shaders/shader.vert.dxil.h"
#include "testgputext/shaders/shader.vert.msl.h"
#include "testgputext/shaders/shader.vert.spv.h"

// #define SDL_MATH_3D_IMPLEMENTATION
// #include "testgputext/SDL_math3d.h"

#define MAX_VERTEX_COUNT 4000
#define MAX_INDEX_COUNT 6000
#define SUPPORTED_SHADER_FORMATS \
    (SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL)

enum class ShaderKind { VertexShader, PixelShader, PixelShader_SDF };

using Vec2 = SDL_FPoint;
struct Vec3 {
    float x, y, z;
};

struct Vertex {
    Vec3 pos;
    SDL_FColor colour;
    Vec2 uv;
};

struct GeometryData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    GeometryData() {
        vertices.reserve(MAX_VERTEX_COUNT);
        indices.reserve(MAX_INDEX_COUNT);
    }
    void clear() {
        vertices.clear();
        indices.clear();
    }
    bool empty() const { return vertices.empty() && indices.empty(); }
};

class SdlError final : public std::runtime_error {
public:
    explicit SdlError(const std::string& msg) :
        std::runtime_error(std::format("{}: {}", msg, SDL_GetError())) {}

    SdlError() : std::runtime_error(SDL_GetError() ? SDL_GetError() : "Unknown SDL error") {}
};

inline void check_bool(bool ok) {
    if (!ok)
        throw SdlError();
}
template <typename T>
inline T* check_ptr(T* p) {
    if (!p)
        throw SdlError();
    return p;
}

// Context owns GPU resources and will clean them on destruction.
struct Context {
    SDL_GPUDevice* device = nullptr;
    SDL_Window* window = nullptr;

    SDL_GPUGraphicsPipeline* pipeline = nullptr;
    SDL_GPUBuffer* vertex_buffer = nullptr;
    SDL_GPUBuffer* index_buffer = nullptr;
    SDL_GPUTransferBuffer* transfer_buffer = nullptr;
    SDL_GPUSampler* sampler = nullptr;
    SDL_GPUCommandBuffer* cmd_buf = nullptr;

    void destroy() {
        // device-related releases require the device pointer.
        if (device) {
            if (transfer_buffer)
                SDL_ReleaseGPUTransferBuffer(device, transfer_buffer);
            if (sampler)
                SDL_ReleaseGPUSampler(device, sampler);
            if (vertex_buffer)
                SDL_ReleaseGPUBuffer(device, vertex_buffer);
            if (index_buffer)
                SDL_ReleaseGPUBuffer(device, index_buffer);
            if (pipeline)
                SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
            // Release window from device
            if (window)
                SDL_ReleaseWindowFromGPUDevice(device, window);
            SDL_DestroyGPUDevice(device);
            device = nullptr;
        }
        // Destroy the SDL_Window if it still exists
        if (window) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
    }

    // destructor releases resources in reverse order of creation where needed
    // ~Context() { destroy(); }
};

// load_shader uses the same selection logic as the C example (SPIR-V / DXIL / MSL)
SDL_GPUShader* load_shader(
    SDL_GPUDevice* device, ShaderKind shader, Uint32 sampler_count = 0,
    Uint32 uniform_buffer_count = 0, Uint32 storage_buffer_count = 0,
    Uint32 storage_texture_count = 0
) {
    SDL_GPUShaderCreateInfo createinfo;
    createinfo.num_samplers = sampler_count;
    createinfo.num_storage_buffers = storage_buffer_count;
    createinfo.num_storage_textures = storage_texture_count;
    createinfo.num_uniform_buffers = uniform_buffer_count;
    createinfo.props = 0;

    SDL_GPUShaderFormat format = SDL_GetGPUShaderFormats(device);

    if (format & SDL_GPU_SHADERFORMAT_DXIL) {
        createinfo.format = SDL_GPU_SHADERFORMAT_DXIL;
        switch (shader) {
            case ShaderKind::VertexShader:
                createinfo.code = shader_vert_dxil;
                createinfo.code_size = shader_vert_dxil_len;
                createinfo.entrypoint = "VSMain";
                break;
            case ShaderKind::PixelShader:
                createinfo.code = shader_frag_dxil;
                createinfo.code_size = shader_frag_dxil_len;
                createinfo.entrypoint = "PSMain";
                break;
            case ShaderKind::PixelShader_SDF:
                createinfo.code = shader_sdf_frag_dxil;
                createinfo.code_size = shader_sdf_frag_dxil_len;
                createinfo.entrypoint = "PSMain";
                break;
        }
    } else if (format & SDL_GPU_SHADERFORMAT_MSL) {
        createinfo.format = SDL_GPU_SHADERFORMAT_MSL;
        switch (shader) {
            case ShaderKind::VertexShader:
                createinfo.code = shader_vert_msl;
                createinfo.code_size = shader_vert_msl_len;
                createinfo.entrypoint = "main0";
                break;
            case ShaderKind::PixelShader:
                createinfo.code = shader_frag_msl;
                createinfo.code_size = shader_frag_msl_len;
                createinfo.entrypoint = "main0";
                break;
            case ShaderKind::PixelShader_SDF:
                createinfo.code = shader_sdf_frag_msl;
                createinfo.code_size = shader_sdf_frag_msl_len;
                createinfo.entrypoint = "main0";
                break;
        }
    } else {
        createinfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
        switch (shader) {
            case ShaderKind::VertexShader:
                createinfo.code = shader_vert_spv;
                createinfo.code_size = shader_vert_spv_len;
                createinfo.entrypoint = "main";
                break;
            case ShaderKind::PixelShader:
                createinfo.code = shader_frag_spv;
                createinfo.code_size = shader_frag_spv_len;
                createinfo.entrypoint = "main";
                break;
            case ShaderKind::PixelShader_SDF:
                createinfo.code = shader_sdf_frag_spv;
                createinfo.code_size = shader_sdf_frag_spv_len;
                createinfo.entrypoint = "main";
                break;
        }
    }

    createinfo.stage = (shader == ShaderKind::VertexShader) ? SDL_GPU_SHADERSTAGE_VERTEX
                                                            : SDL_GPU_SHADERSTAGE_FRAGMENT;

    return SDL_CreateGPUShader(device, &createinfo);
}

// Convert a single atlas draw sequence into our GeometryData (keeps indices relative to vertex
// base)
void append_sequence_to_geometry(
    GeometryData& g, const TTF_GPUAtlasDrawSequence* seq, const SDL_FColor& colour
) {
    if (!seq)
        return;
    const uint32_t base = static_cast<uint32_t>(g.vertices.size());
    for (int i = 0; i < seq->num_vertices; ++i) {
        const SDL_FPoint pos = seq->xy[i];
        Vertex v;
        v.pos = Vec3{pos.x, pos.y, 0.0f};
        v.colour = colour;
        v.uv = seq->uv[i];
        g.vertices.push_back(v);
    }
    for (int i = 0; i < seq->num_indices; ++i) {
        // original example used int for indices — convert to 32-bit uint and offset to base
        g.indices.push_back(static_cast<uint32_t>(seq->indices[i]) + base);
    }
}

void append_text(
    GeometryData& g, const TTF_GPUAtlasDrawSequence* sequence, const SDL_FColor& colour
) {
    for (const TTF_GPUAtlasDrawSequence* s = sequence; s; s = s->next) {
        append_sequence_to_geometry(g, s, colour);
    }
}

// Map the transfer buffer and place vertices first, indices second (keeps same layout as the
// example). This follows the example layout: [ Vertex area (MAX_VERTEX_COUNT) | Index area
// (MAX_INDEX_COUNT) ]
void set_geometry_data(Context& ctx, const GeometryData& g) {
    void* mapped = SDL_MapGPUTransferBuffer(ctx.device, ctx.transfer_buffer, false);
    check_ptr(mapped);

    // vertex area
    Vertex* vertArea = reinterpret_cast<Vertex*>(mapped);
    // index area sits after MAX_VERTEX_COUNT vertices
    char* basePtr = reinterpret_cast<char*>(mapped);
    void* indexArea = basePtr + (sizeof(Vertex) * MAX_VERTEX_COUNT);

    if (!g.vertices.empty()) {
        std::memcpy(vertArea, g.vertices.data(), g.vertices.size() * sizeof(Vertex));
    }
    if (!g.indices.empty()) {
        // original example used 'int' for indices; buffer is prepared for int-sized data.
        // We'll write 32-bit ints (uint32_t).
        std::memcpy(indexArea, g.indices.data(), g.indices.size() * sizeof(uint32_t));
    }
    SDL_UnmapGPUTransferBuffer(ctx.device, ctx.transfer_buffer);
}

void transfer_data(Context& ctx, const GeometryData& g) {
    SDL_GPUCopyPass* copy_pass = check_ptr(SDL_BeginGPUCopyPass(ctx.cmd_buf));

    // Upload vertices
    SDL_GPUTransferBufferLocation transferLocV{ctx.transfer_buffer, 0};
    SDL_GPUBufferRegion regionV{
        ctx.vertex_buffer, 0, static_cast<uint32_t>(sizeof(Vertex) * g.vertices.size())
    };
    SDL_UploadToGPUBuffer(copy_pass, &transferLocV, &regionV, false);

    // Upload indices (index area offset = sizeof(Vertex) * MAX_VERTEX_COUNT)
    SDL_GPUTransferBufferLocation transferLocI{
        ctx.transfer_buffer, static_cast<uint32_t>(sizeof(Vertex) * MAX_VERTEX_COUNT)
    };
    SDL_GPUBufferRegion regionI{
        ctx.index_buffer, 0, static_cast<uint32_t>(sizeof(uint32_t) * g.indices.size())
    };
    SDL_UploadToGPUBuffer(copy_pass, &transferLocI, &regionI, false);

    SDL_EndGPUCopyPass(copy_pass);
}

// Draw using the same render pipeline steps as the original file.
void draw(
    Context& ctx, const std::array<glm::mat4, 2>& matrices, TTF_GPUAtlasDrawSequence* draw_sequence
) {
    SDL_GPUTexture* swapchain_texture = nullptr;
    check_bool(SDL_WaitAndAcquireGPUSwapchainTexture(
        ctx.cmd_buf, ctx.window, &swapchain_texture, nullptr, nullptr
    ));
    if (!swapchain_texture)
        return;

    SDL_GPUColorTargetInfo color_target_info{};
    color_target_info.texture = swapchain_texture;
    color_target_info.clear_color = SDL_FColor{0.3f, 0.4f, 0.5f, 1.0f};
    color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
    color_target_info.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* render_pass =
        SDL_BeginGPURenderPass(ctx.cmd_buf, &color_target_info, 1, nullptr);
    if (!render_pass)
        return;

    SDL_BindGPUGraphicsPipeline(render_pass, ctx.pipeline);

    // Bind vertex buffer (slot 0)
    SDL_GPUBufferBinding vbind{ctx.vertex_buffer, 0};
    SDL_BindGPUVertexBuffers(render_pass, 0, &vbind, 1);

    // Bind index buffer
    SDL_GPUBufferBinding ibind{ctx.index_buffer, 0};
    SDL_BindGPUIndexBuffer(render_pass, &ibind, SDL_GPU_INDEXELEMENTSIZE_32BIT);

    // Push the matrices as vertex uniform slot 0
    SDL_PushGPUVertexUniformData(
        ctx.cmd_buf, 0, matrices.data(), sizeof(glm::mat4) * static_cast<int>(matrices.size())
    );

    // Draw sequences incrementally using offsets just like the original example.
    int index_offset = 0;
    int vertex_offset = 0;
    for (TTF_GPUAtlasDrawSequence* seq = draw_sequence; seq != nullptr; seq = seq->next) {
        SDL_GPUTextureSamplerBinding binding{seq->atlas_texture, ctx.sampler};
        SDL_BindGPUFragmentSamplers(render_pass, 0, &binding, 1);

        // Draw the chunk. Parameters: num_indices, num_instances, index_offset, vertex_offset,
        // first_instance
        SDL_DrawGPUIndexedPrimitives(
            render_pass, seq->num_indices, 1, index_offset, vertex_offset, 0
        );

        index_offset += seq->num_indices;
        vertex_offset += seq->num_vertices;
    }

    SDL_EndGPURenderPass(render_pass);
}

// -------- Main: mirrors the original example flow but modern C++ --------
int main_demo(int argc, char** argv) {
    try {
        // Argument parsing (same as original)
        const char* font_filename = "C:/Windows/Fonts/arial.ttf";
        bool use_SDF = false;
        // for (int i = 1; i < argc; ++i) {
        //     if (SDL_strcasecmp(argv[i], "--sdf") == 0) {
        //         use_SDF = true;
        //     } else if (argv[i][0] == '-') {
        //         break;
        //     } else {
        //         font_filename = argv[i];
        //         break;
        //     }
        // }
        // if (!font_filename) {
        //     SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Usage: testgputext [--sdf]
        //     FONT_FILENAME"); return 2;
        // }

        check_bool(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));

        Context ctx;

        // Create a window and a GPU device; claim the window for the device (exactly like the C
        // example)
        ctx.window = check_ptr(SDL_CreateWindow("GPU text test", 800, 600, 0));
        ctx.device = check_ptr(SDL_CreateGPUDevice(SUPPORTED_SHADER_FORMATS, true, nullptr));
        check_bool(SDL_ClaimWindowForGPUDevice(ctx.device, ctx.window));

        // Load shaders
        SDL_GPUShader* vertex_shader =
            check_ptr(load_shader(ctx.device, ShaderKind::VertexShader, 0, 1, 0, 0));
        SDL_GPUShader* fragment_shader = check_ptr(load_shader(
            ctx.device, use_SDF ? ShaderKind::PixelShader_SDF : ShaderKind::PixelShader, 1, 0, 0, 0
        ));

        // Pipeline create info (kept consistent with the original example)

        SDL_GPUColorTargetBlendState color_target_blend_state{
            .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
            .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            .color_blend_op = SDL_GPU_BLENDOP_ADD,
            .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
            .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_DST_ALPHA,
            .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
            .color_write_mask = 0xF,
            .enable_blend = true,
        };

        SDL_GPUColorTargetDescription color_target_description{
            .format = SDL_GetGPUSwapchainTextureFormat(ctx.device, ctx.window),
            .blend_state = color_target_blend_state,
        };

        SDL_GPUVertexBufferDescription vertex_buffer_description{
            .slot = 0,
            .pitch = sizeof(Vertex),
            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0,
        };

        SDL_GPUVertexAttribute a_position{
            .location = 0,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset = 0,
        };
        SDL_GPUVertexAttribute a_color{
            .location = 1,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
            .offset = sizeof(float) * 3,
        };
        SDL_GPUVertexAttribute a_uv{
            .location = 2,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset = sizeof(float) * 7,
        };

        std::array attributes{a_position, a_color, a_uv};

        SDL_GPUGraphicsPipelineTargetInfo target_info{
            .color_target_descriptions = &color_target_description,
            .num_color_targets = 1,
            .has_depth_stencil_target = false,
        };

        SDL_GPUVertexInputState vertex_input_state{
            .vertex_buffer_descriptions = &vertex_buffer_description,
            .num_vertex_buffers = 1,
            .vertex_attributes = attributes.data(),
            .num_vertex_attributes = 3,
        };

        SDL_GPUGraphicsPipelineCreateInfo pipeline_info{
            .vertex_shader = vertex_shader,
            .fragment_shader = fragment_shader,
            .vertex_input_state = vertex_input_state,
            .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
            .target_info = target_info,
        };

        ctx.pipeline = check_ptr(SDL_CreateGPUGraphicsPipeline(ctx.device, &pipeline_info));

        // Release shader objects (the pipeline holds references like the example)
        SDL_ReleaseGPUShader(ctx.device, vertex_shader);
        SDL_ReleaseGPUShader(ctx.device, fragment_shader);

        // Create buffers
        SDL_GPUBufferCreateInfo vbf_info{
            .usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = sizeof(Vertex) * MAX_VERTEX_COUNT
        };
        ctx.vertex_buffer = check_ptr(SDL_CreateGPUBuffer(ctx.device, &vbf_info));

        SDL_GPUBufferCreateInfo ibf_info{
            .usage = SDL_GPU_BUFFERUSAGE_INDEX, .size = sizeof(uint32_t) * MAX_INDEX_COUNT
        };
        ctx.index_buffer = check_ptr(SDL_CreateGPUBuffer(ctx.device, &ibf_info));

        SDL_GPUTransferBufferCreateInfo tbf_info{
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = (sizeof(Vertex) * MAX_VERTEX_COUNT) + (sizeof(uint32_t) * MAX_INDEX_COUNT)
        };
        ctx.transfer_buffer = check_ptr(SDL_CreateGPUTransferBuffer(ctx.device, &tbf_info));

        SDL_GPUSamplerCreateInfo sampler_info{
            .min_filter = SDL_GPU_FILTER_LINEAR,
            .mag_filter = SDL_GPU_FILTER_LINEAR,
            .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
            .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
        };
        ctx.sampler = check_ptr(SDL_CreateGPUSampler(ctx.device, &sampler_info));

        // Geometry container
        GeometryData geometry;

        // Font & TTF GPU text engine
        check_bool(TTF_Init());
        TTF_Font* font = check_ptr(TTF_OpenFont(font_filename, 50));
        TTF_SetFontSDF(font, use_SDF);
        TTF_SetFontWrapAlignment(font, TTF_HORIZONTAL_ALIGN_CENTER);

        TTF_TextEngine* engine = check_ptr(TTF_CreateGPUTextEngine(ctx.device));
        char str[] = " \nSDL is cool";    // same initial string as example
        TTF_Text* text = check_ptr(TTF_CreateText(engine, font, str, 0));

        // Projection + model matrices (two-matrix uniform like the example)
        std::array<glm::mat4, 2> matrices;
        matrices[0] = glm::perspective(SDL_PI_F / 2.0f, 800.0f / 600.0f, 0.1f, 100.0f);
        matrices[1] = glm::identity<glm::mat4>();

        float rot_angle = 0.0f;
        SDL_FColor color{1.0f, 1.0f, 0.0f, 1.0f};

        bool running = true;
        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_EVENT_KEY_UP:
                        if (event.key.key == SDLK_ESCAPE)
                            running = false;
                        break;
                    case SDL_EVENT_QUIT:
                        running = false;
                        break;
                }
            }

            // mutate first 5 characters randomly (same playful behavior as original)
            for (int i = 0; i < 5; ++i)
                str[i] = static_cast<char>(65 + SDL_rand(26));
            TTF_SetTextString(text, str, 0);

            int tw = 0, th = 0;
            check_bool(TTF_GetTextSize(text, &tw, &th));

            rot_angle = SDL_fmodf(rot_angle + 0.01f, 2 * SDL_PI_F);
            glm::mat4 model = glm::identity<glm::mat4>();
            // model =
            //     SDL_MatrixMultiply(model, SDL_MatrixTranslation((SDL_Vec3){0.0f, 0.0f, -80.0f}));
            // model = SDL_MatrixMultiply(model, SDL_MatrixScaling((SDL_Vec3){0.3f, 0.3f, 0.3f}));
            // model = SDL_MatrixMultiply(model, SDL_MatrixRotationY(rot_angle));
            // model = SDL_MatrixMultiply(
            //     model, SDL_MatrixTranslation((SDL_Vec3){-tw / 2.0f, th / 2.0f, 0.0f})
            // );
            model = glm::translate(model, {0.0F, 0.0F, -80.0F});
            model = glm::scale(model, {0.3F, 0.3F, 0.3F});
            model = glm::rotate(model, rot_angle, {0.0F, 1.0F, 0.0F});
            matrices[1] = model;

            // Build geometry from atlas draw data
            TTF_GPUAtlasDrawSequence* sequence = TTF_GetGPUTextDrawData(text);
            append_text(geometry, sequence, color);

            // Upload geometry
            ctx.cmd_buf = check_ptr(SDL_AcquireGPUCommandBuffer(ctx.device));
            set_geometry_data(ctx, geometry);
            transfer_data(ctx, geometry);

            // Draw & submit
            draw(ctx, matrices, sequence);
            SDL_SubmitGPUCommandBuffer(ctx.cmd_buf);

            // reset geometry counts (vectors -> clear)
            geometry.clear();
        }

        // Clean up TTF objects (they are C API objects)
        SDL_free(
            nullptr
        );    // noop but kept to show where original freed memory (vectors manage memory now)
        TTF_DestroyText(text);
        TTF_DestroyGPUTextEngine(engine);
        TTF_CloseFont(font);
        TTF_Quit();

        // ctx destructor will release GPU resources and destroy window
        ctx.destroy();
        SDL_Quit();
        return 0;
    } catch (const SdlError& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fatal SDL error: %s", e.what());
        SDL_Quit();
        return 1;
    } catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: %s", e.what());
        SDL_Quit();
        return 1;
    }
}
