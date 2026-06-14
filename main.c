#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <d3d9.h>

typedef struct {
    float x, y, z;
    uint32_t color;
} Vertex;

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)

SDL_Window* window = NULL;
IDirect3D9* d3d = NULL;
IDirect3DDevice9* device = NULL;
IDirect3DVertexBuffer9* vertexBuffer = NULL;
int width = 800;
int height = 600;

bool CreateTriangle() {
    Vertex vertices[] = {
        {  0.0f,  0.5f, 0.0f, 0xFFFF0000 },
        {  0.5f, -0.5f, 0.0f, 0xFF00FF00 },
        { -0.5f, -0.5f, 0.0f, 0xFF0000FF }
    };

    HRESULT hr = device->lpVtbl->CreateVertexBuffer(device, sizeof(vertices), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_MANAGED, &vertexBuffer, NULL);
    if (FAILED(hr)) {
        SDL_Log("Failed to create Vertex Buffer.");
        return false;
    }

    void* pVoid = NULL;
    vertexBuffer->lpVtbl->Lock(vertexBuffer, 0, 0, &pVoid, 0);
    SDL_memcpy(pVoid, vertices, sizeof(vertices));
    vertexBuffer->lpVtbl->Unlock(vertexBuffer);
    return true;
}

void DestroyTriangle() {
    if (vertexBuffer) {
        vertexBuffer->lpVtbl->Release(vertexBuffer);
        vertexBuffer = NULL;
    }
}

void ResetD3D9Device() {
    SDL_GetWindowSizeInPixels(window, &width, &height);
    if (width <= 0 || height <= 0) return;

    D3DPRESENT_PARAMETERS d3dpp = {
        .Windowed = TRUE,
        .SwapEffect = D3DSWAPEFFECT_DISCARD,
        .BackBufferWidth = width,
        .BackBufferHeight = height,
        .PresentationInterval = D3DPRESENT_INTERVAL_ONE
    };

    device->lpVtbl->Reset(device, &d3dpp);
    device->lpVtbl->SetRenderState(device, D3DRS_LIGHTING, FALSE);
    
    D3DVIEWPORT9 vp = { .X = 0, .Y = 0, .Width = (DWORD)width, .Height = (DWORD)height, .MinZ = 0.0f, .MaxZ = 1.0f };
    device->lpVtbl->SetViewport(device, &vp);
}

void RenderFrame() {
    device->lpVtbl->Clear(device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(40, 40, 45), 1.0f, 0);

    if (SUCCEEDED(device->lpVtbl->BeginScene(device))) {
        device->lpVtbl->SetFVF(device, D3DFVF_CUSTOMVERTEX);
        device->lpVtbl->SetStreamSource(device, 0, vertexBuffer, 0, sizeof(Vertex));
        device->lpVtbl->DrawPrimitive(device, D3DPT_TRIANGLELIST, 0, 1);
        device->lpVtbl->EndScene(device);
    }

    device->lpVtbl->Present(device, NULL, NULL, NULL, NULL);
}

bool SDLCALL WindowResizeWatcher(void* userdata, SDL_Event* event) {
    if (event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
        ResetD3D9Device();
        RenderFrame();
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow("SDL3 d3d9/dxvk example", width, height, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        goto cleanup;
    }

    SDL_GetWindowSizeInPixels(window, &width, &height);

    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    HWND nativeWindowHandle = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    if (!nativeWindowHandle) {
        SDL_Log("Failed to get native window handle.");
        goto cleanup;
    }

    D3DPRESENT_PARAMETERS d3dpp = {
        .Windowed = TRUE,
        .SwapEffect = D3DSWAPEFFECT_DISCARD,
        .BackBufferWidth = width,
        .BackBufferHeight = height,
        .PresentationInterval = D3DPRESENT_INTERVAL_ONE,
        .hDeviceWindow = nativeWindowHandle
    };

    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) {
        SDL_Log("Failed to create IDirect3D9 interface.");
        goto cleanup;
    }

    HRESULT hr = d3d->lpVtbl->CreateDevice(
        d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, nativeWindowHandle,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &device
    );
    if (FAILED(hr)) {
        SDL_Log("Failed to create D3D9 Device. HRESULT: 0x%X", (unsigned int)hr);
        goto cleanup;
    }

    device->lpVtbl->SetRenderState(device, D3DRS_LIGHTING, FALSE);
    
    D3DVIEWPORT9 vp = { .X = 0, .Y = 0, .Width = (DWORD)width, .Height = (DWORD)height, .MinZ = 0.0f, .MaxZ = 1.0f };
    device->lpVtbl->SetViewport(device, &vp);

    if (!CreateTriangle()) {
        goto cleanup;
    }
    
    RenderFrame();

    SDL_AddEventWatch(WindowResizeWatcher, NULL);

    SDL_Event event;
    while (SDL_WaitEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            goto cleanup;
        }
    }

cleanup:
    SDL_RemoveEventWatch(WindowResizeWatcher, NULL);
    DestroyTriangle();
    if (device) device->lpVtbl->Release(device);
    if (d3d) d3d->lpVtbl->Release(d3d);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
