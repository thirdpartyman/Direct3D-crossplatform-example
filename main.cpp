#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <d3d9.h>
#if defined(SDL_PLATFORM_WINDOWS)
#include <d3dcompiler.h>
#else
#include "shader_compiler.h"
#endif


struct Vertex {
    float x, y, z;
    uint32_t color;
};

const char* vertexShaderCode = 
    "struct VS_INPUT { float3 pos : POSITION; float4 col : COLOR0; };\n"
    "struct VS_OUTPUT { float4 pos : POSITION; float4 col : COLOR0; };\n"
    "VS_OUTPUT main(VS_INPUT input) {\n"
    "    VS_OUTPUT output;\n"
    "    output.pos = float4(input.pos, 1.0);\n"
    "    output.col = input.col;\n"
    "    return output;\n"
    "}\n";

const char* pixelShaderCode = 
    "float4 main(float4 col : COLOR0) : COLOR0 {\n"
    "    return col;\n"
    "}\n";

SDL_Window* window = nullptr;
IDirect3D9* d3d = nullptr;
IDirect3DDevice9* device = nullptr;
IDirect3DVertexBuffer9* vertexBuffer = nullptr;
IDirect3DVertexShader9* vertexShader = nullptr;
IDirect3DPixelShader9* pixelShader = nullptr;
IDirect3DVertexDeclaration9* vertexDecl = nullptr;
int width = 800;
int height = 600;
bool isDeviceLost = false;

void CleanShaders() {
    if (vertexShader) { vertexShader->Release(); vertexShader = nullptr; }
    if (pixelShader) { pixelShader->Release(); pixelShader = nullptr; }
    if (vertexDecl) { vertexDecl->Release(); vertexDecl = nullptr; }
}

bool InitShaders() {
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT hr = D3DCompile(vertexShaderCode, SDL_strlen(vertexShaderCode), nullptr, nullptr, nullptr, "main", "vs_3_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) { SDL_Log("VS Compile Error: %s", (char*)errorBlob->GetBufferPointer()); errorBlob->Release(); }
        return false;
    }
    device->CreateVertexShader((DWORD*)vsBlob->GetBufferPointer(), &vertexShader);
    vsBlob->Release();

    hr = D3DCompile(pixelShaderCode, SDL_strlen(pixelShaderCode), nullptr, nullptr, nullptr, "main", "ps_3_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) { SDL_Log("PS Compile Error: %s", (char*)errorBlob->GetBufferPointer()); errorBlob->Release(); }
        return false;
    }
    device->CreatePixelShader((DWORD*)psBlob->GetBufferPointer(), &pixelShader);
    psBlob->Release();

    D3DVERTEXELEMENT9 declElements[] = {
        { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
        D3DDECL_END()
    };
    return SUCCEEDED(device->CreateVertexDeclaration(declElements, &vertexDecl));
}

bool CreateTriangle() {
    Vertex vertices[] = {
        {  0.0f,  0.5f, 0.0f, 0xFFFF0000 },
        {  0.5f, -0.5f, 0.0f, 0xFF00FF00 },
        { -0.5f, -0.5f, 0.0f, 0xFF0000FF }
    };

    HRESULT hr = device->CreateVertexBuffer(sizeof(vertices), 0, 0, D3DPOOL_MANAGED, &vertexBuffer, nullptr);
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
        if (vertexBuffer && vertexShader && pixelShader && vertexDecl) {
            device->SetVertexDeclaration(vertexDecl);
            device->SetVertexShader(vertexShader);
            device->SetPixelShader(pixelShader);

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
        CleanShaders();
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

#if !defined(SDL_PLATFORM_WINDOWS)
#include <stdlib.h>
class DxvkWsiDriverGuard {
public:
    DxvkWsiDriverGuard() {
        setenv("DXVK_WSI_DRIVER", "SDL3", 1);
    }
} dxvkGuard;
#endif

bool IsRunningOnDXVK() {
#if defined(SDL_PLATFORM_WINDOWS)
    HMODULE hVulkan = GetModuleHandleA("vulkan-1.dll");
    return (hVulkan != nullptr);
#endif
    return true;
}

int main(int argc, char* argv[]) {
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
        SDL_Log("CRITICAL: Direct3DCreate9 returned NULL!");
        return -1;
    }
    SDL_Log("STATUS: Direct3D9 Interface created.");
    
    if (IsRunningOnDXVK()) {
        SDL_Log("STATUS: Runtime backend detected as DXVK (Vulkan).");
    } else {
        SDL_Log("STATUS: Runtime backend detected as Native Microsoft D3D9.");
    }    

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

    if (!InitShaders() || !CreateTriangle()) {
        SDL_Log("CRITICAL: Geometry or Shader initialization failed!");
        return -1;
    }
    SDL_Log("STATUS: Geometry buffer and shaders created. Entering main loop...");

    SDL_AddEventWatch(WindowResizeWatcher, nullptr);

    SDL_Event event;
    while (SDL_WaitEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT || 
           (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
            break;
        }
        RenderFrame();
    }

    SDL_Log("STATUS: Application closed cleanly.");
    return 0;
}
