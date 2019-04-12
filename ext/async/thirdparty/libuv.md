# Libuv Integration

Libuv `1.27.0` is bundled with the extension for static linking.

## Linux

Libuv is compiled during autoconf, the generated static lib is moved to `thirdparty/lib/libuv.a`.

## Windows

1. Ensure the Visual Studio compiler and Python 2 are installed on your system.
2. Download and unpack a libuv release from github.
3. Open the Developer Command Prompt for VS.
4. Change directory to the libuv directory.
5. Execute `vcbuild release static`, add eighter `x64` or `x86` to specify the target platform. Add `vs2017` if you are building with Visual Studio 2017.
6. Copy the generated lib file to `thirdparty/lib/libuv-{x86|x64}.lib`.
