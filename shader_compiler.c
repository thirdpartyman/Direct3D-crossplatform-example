#define COBJMACROS
#include "shader_compiler.h"

struct LinuxShaderBridgeResult Call_D3DCompile_LinuxC(
    const void *data, size_t data_size, const char *filename,
    const D3D_SHADER_MACRO *macros, ID3DInclude *include, const char *entrypoint,
    const char *profile, UINT flags, UINT effect_flags)
{
    struct LinuxShaderBridgeResult result = { S_OK, NULL, NULL };
    
    ID3DBlob* vkd3d_shader = NULL;
    ID3DBlob* vkd3d_error = NULL;

    result.hr = D3DCompile(
        data, (SIZE_T)data_size, filename, 
        macros, include, entrypoint, profile, flags, effect_flags, 
        &vkd3d_shader, 
        &vkd3d_error
    );
    
    result.shaderBlob = (void*)vkd3d_shader;
    result.errorBlob = (void*)vkd3d_error;
    return result;
}

const void* LinuxC_GetBufferPointer(ID3DBlob* blob) {
    if (!blob) return NULL;
    return blob->lpVtbl->GetBufferPointer(blob);
}

size_t LinuxC_GetBufferSize(ID3DBlob* blob) {
    if (!blob) return 0;
    return (size_t)blob->lpVtbl->GetBufferSize(blob);
}

void LinuxC_ReleaseBlob(ID3DBlob* blob) {
    if (!blob) return;
    blob->lpVtbl->Release(blob);
}
