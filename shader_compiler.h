#pragma once
#include <stddef.h>

class LinuxD3DBlob_Wrapper {
private:
    void* m_nativeBlob;
public:
    LinuxD3DBlob_Wrapper(void* nativeBlob);
    void* GetBufferPointer();
    size_t GetBufferSize();
    void Release();
};

HRESULT D3DCompile_Bridge(
    const void *data, size_t data_size, const char *filename,
    const void *macros, void *include, const char *entrypoint,
    const char *profile, unsigned int flags, unsigned int effect_flags,
    LinuxD3DBlob_Wrapper **shader, LinuxD3DBlob_Wrapper **error_messages
);

#define ID3DBlob LinuxD3DBlob_Wrapper
#define D3DCompile D3DCompile_Bridge