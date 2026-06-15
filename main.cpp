#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <d3d9.h>
#include <cstdlib>

struct Vertex {
    float x, y, z;
    uint32_t color;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)

SDL_Window* window = nullptr;
IDirect3D9* d3d = nullptr;
IDirect3DDevice9* device = nullptr;
IDirect3DVertexBuffer9* vertexBuffer = nullptr;
int width = 800;
int height = 600;
bool isDeviceLost = false;

bool CreateTriangle() {
    Vertex vertices[] = {
        {  0.0f,  0.5f, 0.0f, 0xFFFF0000 },
        {  0.5f, -0.5f, 0.0f, 0xFF00FF00 },
        { -0.5f, -0.5f, 0.0f, 0xFF0000FF }
    };

    HRESULT hr = device->CreateVertexBuffer(sizeof(vertices), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_MANAGED, &vertexBuffer, nullptr);
    if (FAILED(hr)) {
        SDL_Log("Failed to create Vertex Buffer.");
        return false;
    }

    void* pVoid = nullptr;
    if (SUCCEEDED(vertexBuffer->Lock(0, 0, &pVoid, 0))) {
        SDL_memcpy(pVoid, vertices, sizeof(vertices));
        vertexBuffer->Unlock();
        return true;
    }
    return false;
}

void DestroyTriangle() {
    if (vertexBuffer) {
        vertexBuffer->Release();
        vertexBuffer = nullptr;
    }
}

void ResetD3D9Device() {
    SDL_GetWindowSizeInPixels(window, &width, &height);
    if (width <= 0 || height <= 0) return;

    DestroyTriangle();

    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferWidth = static_cast<UINT>(width);
    d3dpp.BackBufferHeight = static_cast<UINT>(height);
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    HRESULT hr = device->Reset(&d3dpp);
    if (hr == D3DERR_DEVICELOST) {
        isDeviceLost = true;
        return;
    }

    isDeviceLost = false;
    device->SetRenderState(D3DRS_LIGHTING, FALSE);

    D3DVIEWPORT9 vp = { 0, 0, static_cast<DWORD>(width), static_cast<DWORD>(height), 0.0f, 1.0f };
    device->SetViewport(&vp);

    CreateTriangle();
}

void RenderFrame() {
    if (isDeviceLost) {
        HRESULT hr = device->TestCooperativeLevel();
        if (hr == D3DERR_DEVICENOTRESET) {
            ResetD3D9Device();
        } else {
            return;
        }
    }

    device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(40, 40, 45), 1.0f, 0);

    if (SUCCEEDED(device->BeginScene())) {
        if (vertexBuffer) {
            device->SetFVF(D3DFVF_CUSTOMVERTEX);
            device->SetStreamSource(0, vertexBuffer, 0, sizeof(Vertex));
            device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
        }
        device->EndScene();
    }

    HRESULT hrPresent = device->Present(nullptr, nullptr, nullptr, nullptr);
    if (hrPresent == D3DERR_DEVICELOST) {
        isDeviceLost = true;
    }
}

bool SDLCALL WindowResizeWatcher(void* userdata, SDL_Event* event) {
    if (event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
        ResetD3D9Device();
        RenderFrame();
    }
    return true;
}

class ApplicationGuard {
public:
    ApplicationGuard() = default;

    ~ApplicationGuard() {
        SDL_RemoveEventWatch(WindowResizeWatcher, nullptr);
        DestroyTriangle();
        if (device) { device->Release(); device = nullptr; }
        if (d3d) { d3d->Release(); d3d = nullptr; }
        if (window) { SDL_DestroyWindow(window); window = nullptr; }
        SDL_Quit();
    }

    ApplicationGuard(const ApplicationGuard&) = delete;
    ApplicationGuard& operator=(const ApplicationGuard&) = delete;
} appGuard;

HWND GetNativeWindowHandle(SDL_Window* sdl_window) {
    if (!sdl_window) return nullptr;
#if defined(_WIN32)
    SDL_PropertiesID props = SDL_GetWindowProperties(sdl_window);
    return static_cast<HWND>(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
#else
    return sdl_window;
#endif
}

class DxvkWsiDriverGuard {
public:
    DxvkWsiDriverGuard() {
#if !defined(_WIN32)
        setenv("DXVK_WSI_DRIVER", "SDL3", 1);
#endif
    }
}_;

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("CRITICAL: SDL_Init(VIDEO) Failed: %s", SDL_GetError());
        return -1;
    }
    SDL_Log("STATUS: SDL_Init successful.");

    SDL_PropertiesID win_props = SDL_CreateProperties();
    SDL_SetStringProperty(win_props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "SDL3 d3d9/dxvk example");
    SDL_SetNumberProperty(win_props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
    SDL_SetNumberProperty(win_props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
    SDL_SetBooleanProperty(win_props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);

#if !defined(SDL_PLATFORM_WINDOWS)
    SDL_SetBooleanProperty(win_props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);
#endif

    window = SDL_CreateWindowWithProperties(win_props);
    SDL_DestroyProperties(win_props);
    
    if (!window) {
        SDL_Log("CRITICAL: SDL_CreateWindow Failed: %s", SDL_GetError());
        return -1;
    }
    SDL_Log("STATUS: Window created successfully.");

    SDL_GetWindowSizeInPixels(window, &width, &height);

    HWND nativeWindowHandle = GetNativeWindowHandle(window);

    if (!nativeWindowHandle) {
        SDL_Log("CRITICAL: Native Window Handle is NULL!");
        return -1;
    }
    SDL_Log("STATUS: Native window handle retrieved.");

    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferWidth = static_cast<UINT>(width);
    d3dpp.BackBufferHeight = static_cast<UINT>(height);
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    d3dpp.hDeviceWindow = nativeWindowHandle;

    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) {
        SDL_Log("CRITICAL: Direct3DCreate9 returned NULL! (d3d9.dll missing or broken in Wine)");
        return -1;
    }
    SDL_Log("STATUS: Direct3D9 Interface created.");

    HRESULT hr = d3d->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, nativeWindowHandle,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &device
        );
    if (FAILED(hr)) {
        SDL_Log("CRITICAL: CreateDevice Failed. HRESULT: 0x%X", (unsigned int)hr);
        return -1;
    }
    SDL_Log("STATUS: D3D9 Device created successfully.");

    device->SetRenderState(D3DRS_LIGHTING, FALSE);

    D3DVIEWPORT9 vp = { 0, 0, static_cast<DWORD>(width), static_cast<DWORD>(height), 0.0f, 1.0f };
    device->SetViewport(&vp);

    if (!CreateTriangle()) {
        SDL_Log("CRITICAL: CreateTriangle geometry initialization failed!");
        return -1;
    }
    SDL_Log("STATUS: Geometry buffer created. Entering main loop...");

    RenderFrame();

    SDL_AddEventWatch(WindowResizeWatcher, nullptr);

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                }
            }
        }
        RenderFrame();
    }

    SDL_Log("STATUS: Application closed cleanly.");
    return 0;
}
