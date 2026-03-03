# Building for Multiple Architectures

## macOS Universal Binary

The project now supports building universal binaries that run natively on both Apple Silicon (M1/M2/M3) and Intel Macs.

### Quick Build

```bash
./build_cpp.sh
```

This creates `run.out` as a universal binary containing both arm64 and x86_64 architectures.

### Verify Build

```bash
./verify_binary.sh
```

### How It Works

The build script:
1. Compiles the source for arm64 (Apple Silicon)
2. Compiles the source for x86_64 (Intel)
3. Uses `lipo` to merge both binaries into a universal binary
4. Verifies the result

### Architecture-Specific Builds

If you need to build for a specific architecture only:

```bash
# Apple Silicon only
g++ src/*.cpp -std=c++17 -arch arm64 -o run_arm64.out

# Intel only
g++ src/*.cpp -std=c++17 -arch x86_64 -o run_x86_64.out
```

### Cross-Compilation

The universal binary approach means:
- On Apple Silicon Macs, the binary runs the arm64 code natively
- On Intel Macs, the binary runs the x86_64 code natively
- No performance penalty - each architecture runs its native code
- Single binary for distribution

### Testing on Different Architectures

To test the x86_64 version on Apple Silicon (via Rosetta 2):

```bash
arch -x86_64 ./run.out
```

To test the arm64 version (native):

```bash
arch -arm64 ./run.out
```

## Linux

For Linux systems, use the standard build:

```bash
g++ src/*.cpp -std=c++17 -o run.out
```

## Windows

Use the provided batch file or WSL:

```bash
build_cpp.bat
```
