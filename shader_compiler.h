#pragma once
#include <stddef.h>

#ifdef __cplusplus

typedef int HRESULT;

extern "C" {
struct LinuxShaderBridgeResult {
    HRESULT hr;
    void* shaderBlob;
    void* errorBlob;
};

struct LinuxShaderBridgeResult Call_D3DCompile_LinuxC(
    const void *data, size_t data_size, const char *filename,
    const void *macros, void *include, const char *entrypoint,
    const char *profile, unsigned int flags, unsigned int effect_flags
);

const void* LinuxC_GetBufferPointer(void* blob);
size_t LinuxC_GetBufferSize(void* blob);
void LinuxC_ReleaseBlob(void* blob);
}

class LinuxD3DBlob_Wrapper {
private:
    void* m_nativeBlob;
public:
    LinuxD3DBlob_Wrapper(void* nativeBlob) : m_nativeBlob(nativeBlob) {}
    void* GetBufferPointer() { return (void*)LinuxC_GetBufferPointer(m_nativeBlob); }
    size_t GetBufferSize()   { return LinuxC_GetBufferSize(m_nativeBlob); }
    void Release()           { LinuxC_ReleaseBlob(m_nativeBlob); delete this; }
};

typedef LinuxD3DBlob_Wrapper ID3DBlob;

inline HRESULT D3DCompile(
    const void *data, size_t data_size, const char *filename,
    const void *macros, void *include, const char *entrypoint,
    const char *profile, unsigned int flags, unsigned int effect_flags,
    ID3DBlob **shader, ID3DBlob **error_messages)
{
    struct LinuxShaderBridgeResult res = Call_D3DCompile_LinuxC(
        data, data_size, filename, macros, include, entrypoint, profile, flags, effect_flags
    );
    
    if (res.shaderBlob && shader) {
        *shader = new LinuxD3DBlob_Wrapper(res.shaderBlob);
    }
    if (res.errorBlob && error_messages) {
        *error_messages = new LinuxD3DBlob_Wrapper(res.errorBlob);
    }
    return res.hr;
}

#else

#include <vkd3d_windows.h>
#include <vkd3d_d3dcommon.h>
#include <vkd3d_d3dcompiler.h>

struct LinuxShaderBridgeResult {
    HRESULT hr;
    void* shaderBlob;
    void* errorBlob;
};

struct LinuxShaderBridgeResult Call_D3DCompile_LinuxC(
    const void *data, size_t data_size, const char *filename,
    const D3D_SHADER_MACRO *macros, ID3DInclude *include, const char *entrypoint,
    const char *profile, UINT flags, UINT effect_flags
);

const void* LinuxC_GetBufferPointer(ID3DBlob* blob);
size_t LinuxC_GetBufferSize(ID3DBlob* blob);
void LinuxC_ReleaseBlob(ID3DBlob* blob);
#endif
