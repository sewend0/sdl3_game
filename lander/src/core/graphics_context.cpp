

#include <graphics_context.h>

auto Graphics_context::init(int width, int height, const std::string& title) -> utils::Result<> {

    CHECK_BOOL(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO), "Failed to init SDL");

    window = TRY(create_window(width, height, title));
    device = TRY(create_device());

    return {};
}

auto Graphics_context::quit() -> void {
    if (device && window)
        SDL_ReleaseWindowFromGPUDevice(device, window);
    if (device)
        SDL_DestroyGPUDevice(device);
    if (window)
        SDL_DestroyWindow(window);
}

auto Graphics_context::create_window(int width, int height, const std::string& title)
    -> utils::Result<SDL_Window*> {
    const float scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};

    return CHECK_PTR(
        SDL_CreateWindow(
            title.c_str(), static_cast<int>(width * scale), static_cast<int>(height * scale),
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
        ),
        "Failed to create window"
    );
}

auto Graphics_context::create_device() -> utils::Result<SDL_GPUDevice*> {
    SDL_GPUDevice* gpu_device{CHECK_PTR(
        SDL_CreateGPUDevice(SDL_ShaderCross_GetSPIRVShaderFormats(), true, nullptr),
        "Failed to create GPU device"
    )};

    CHECK_BOOL(SDL_ClaimWindowForGPUDevice(gpu_device, window), "Failed claiming window");

    // // Extra configuration of GPU
    // SDL_GPUPresentMode present_mode{SDL_GPU_PRESENTMODE_VSYNC};
    // if (SDL_WindowSupportsGPUPresentMode(gpu_device, window, SDL_GPU_PRESENTMODE_IMMEDIATE))
    //     present_mode = SDL_GPU_PRESENTMODE_IMMEDIATE;
    // else if (SDL_WindowSupportsGPUPresentMode(gpu_device, window, SDL_GPU_PRESENTMODE_MAILBOX))
    //     present_mode = SDL_GPU_PRESENTMODE_MAILBOX;
    //
    // SDL_SetGPUSwapchainParameters(
    //     gpu_device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, present_mode
    // );

    return gpu_device;
}
