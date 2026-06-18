#define COBJMACROS
#define CINTERFACE

extern "C" {
    #include <vkd3d_windows.h>
    #include <vkd3d_d3dcommon.h>
    #include <vkd3d_d3dcompiler.h>
}

#include "shader_compiler.h"
#undef ID3DBlob
#undef D3DCompile

extern "C" {

const void* LinuxC_GetBufferPointer(::ID3D10Blob* blob) {
    if (!blob) return nullptr;
    return blob->lpVtbl->GetBufferPointer(blob);
}

size_t LinuxC_GetBufferSize(::ID3D10Blob* blob) {
    if (!blob) return 0;
    return static_cast<size_t>(blob->lpVtbl->GetBufferSize(blob));
}

void LinuxC_ReleaseBlob(::ID3D10Blob* blob) {
    if (!blob) return;
    blob->lpVtbl->Release(blob);
}

}

LinuxD3DBlob_Wrapper::LinuxD3DBlob_Wrapper(void* nativeBlob) : m_nativeBlob(nativeBlob) {}

void* LinuxD3DBlob_Wrapper::GetBufferPointer() { 
    return const_cast<void*>(LinuxC_GetBufferPointer(reinterpret_cast<::ID3D10Blob*>(m_nativeBlob))); 
}

size_t LinuxD3DBlob_Wrapper::GetBufferSize() { 
    return LinuxC_GetBufferSize(reinterpret_cast<::ID3D10Blob*>(m_nativeBlob)); 
}

void LinuxD3DBlob_Wrapper::Release() { 
    LinuxC_ReleaseBlob(reinterpret_cast<::ID3D10Blob*>(m_nativeBlob)); 
    delete this; 
}

HRESULT D3DCompile_Bridge(
    const void *data, size_t data_size, const char *filename,
    const void *macros, void *include, const char *entrypoint,
    const char *profile, unsigned int flags, unsigned int effect_flags,
    LinuxD3DBlob_Wrapper **shader, LinuxD3DBlob_Wrapper **error_messages)
{
    ::ID3D10Blob* vkd3d_shader = nullptr;
    ::ID3D10Blob* vkd3d_error = nullptr;

    HRESULT hr = ::D3DCompile(
        data, 
        static_cast<SIZE_T>(data_size), 
        filename, 
        reinterpret_cast<const D3D_SHADER_MACRO*>(macros), 
        reinterpret_cast<::ID3DInclude*>(include), 
        entrypoint, 
        profile, 
        flags, 
        effect_flags,
        &vkd3d_shader, 
        &vkd3d_error
    );
    
    if (vkd3d_shader && shader) {
        *shader = new LinuxD3DBlob_Wrapper(vkd3d_shader);
    }
    if (vkd3d_error && error_messages) {
        *error_messages = new LinuxD3DBlob_Wrapper(vkd3d_error);
    }
    return hr;
}
