Cross-platform D3D and SDL3 CMake template for Windows and Linux (via DXVK Native) with native HLSL shader compilation support via vkd3d.

Project demonstrates how to compile and run a unified Direct3D codebase natively on Windows and Linux using CMake.

## Features

* **SDL3 and D3D/DXVK Integration**: Direct SDL3 integration with Direct3D API on Windows and Linux (using DXVK Native).
* **HLSL Shader Support**: Runtime HLSL compilation from source text on Windows and Linux, using vkd3d under Linux.
* **vkd3d C++ Bridge**: Fixed runtime crashes caused by C++ interface incompatibility when calling D3DCompile from vkd3d on Linux. Implemented as lightweight wrapper to keep Direct3D code identical across platforms.

## Repository Tags

The repository includes specific tags to preserve both architectural solutions:
* `c-bridge`: The original pure C implementation using a flat `lpVtbl` mapping.
* `cpp-bridge`: The final, monolithic C++ refactored implementation.

## Requirements

### Windows (MSVC)
* Windows SDK (DirectX SDK headers)

## Linux Packages

### Arch Linux
```bash
sudo pacman -S sdl3 dxvk-d3d9 vkd3d
```

### Ubuntu / Debian
```bash
sudo apt install libsdl3-dev dxvk-d3d9-dev libvkd3d-utils-dev
```

## Windows Packages (MSYS2 UCRT64)
```bash
pacman -S mingw-w64-ucrt-x86_64-SDL3
```

## Cross-Compilation Dependencies (Arch Linux)
For cross-compilation from Linux to Windows/Wine, you need the MinGW toolchain and the Windows build of SDL3 (available via AUR):
```bash
sudo pacman -S mingw-w64-gcc
yay -S mingw-w64-sdl3
```

## Building

### Native Compilation (Windows / Linux)
```bash
cmake -B build
cmake --build build
```

### Cross-Compilation for Windows (Linux to Wine)
To cross-compile a Windows executable from Linux using MinGW and run it via Wine:
```bash
cmake -B build_wine -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw64.cmake
cmake --build build_wine
wine build_wine/sdl3_d3d9_dxvk_example.exe
```
