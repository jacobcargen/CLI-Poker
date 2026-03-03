Very basic networking in c++

This is a game supporting a few players and allows them to play poker through a terminal on LAN.

## Building

### macOS (Universal Binary)

The build script creates a universal binary that runs natively on both Apple Silicon (M1/M2/M3) and Intel Macs:

```bash
chmod +x build_cpp.sh
./build_cpp.sh
```

This will create `run.out` as a universal binary supporting both arm64 and x86_64 architectures.

### Linux

```bash
g++ src/*.cpp -std=c++17 -o run.out
```

## Running

```bash
./run.out
```

## Architecture Support

- **Apple Silicon (M1/M2/M3)**: Native arm64 support
- **Intel Macs**: Native x86_64 support
- **Universal Binary**: Single executable runs natively on both architectures

