#!/bin/bash

# Build script for macOS supporting both Apple Silicon (arm64) and Intel (x86_64)
# Creates a universal binary that runs natively on both architectures

echo "Building CLI-Poker universal binary..."

# Detect current architecture
ARCH=$(uname -m)
echo "Current architecture: $ARCH"

# Create build directory if it doesn't exist
mkdir -p build

# Build for arm64 (Apple Silicon)
echo "Compiling for arm64 (Apple Silicon)..."
g++ src/*.cpp -std=c++17 -arch arm64 -o build/run_arm64.out

# Build for x86_64 (Intel)
echo "Compiling for x86_64 (Intel)..."
g++ src/*.cpp -std=c++17 -arch x86_64 -o build/run_x86_64.out

# Create universal binary using lipo
echo "Creating universal binary..."
lipo -create build/run_arm64.out build/run_x86_64.out -output run.out

# Verify the universal binary
echo ""
echo "Build complete! Binary info:"
lipo -info run.out
file run.out

# Clean up intermediate files
rm -rf build

echo ""
echo "Universal binary created: run.out"
echo "This binary will run natively on both Apple Silicon and Intel Macs."
