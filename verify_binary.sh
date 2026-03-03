#!/bin/bash

# Script to verify the universal binary

echo "Verifying CLI-Poker binary..."
echo ""

if [ ! -f "run.out" ]; then
    echo "❌ run.out not found. Please build first using ./build_cpp.sh"
    exit 1
fi

echo "📦 Binary Information:"
file run.out
echo ""

echo "🏗️  Architectures:"
lipo -info run.out
echo ""

echo "✅ Current System:"
echo "   Architecture: $(uname -m)"
echo "   OS: $(uname -s)"
echo ""

# Check if both architectures are present
if lipo -info run.out | grep -q "arm64" && lipo -info run.out | grep -q "x86_64"; then
    echo "✅ Universal binary verified!"
    echo "   This binary will run natively on both Apple Silicon and Intel Macs."
else
    echo "⚠️  Warning: Binary may not be universal."
fi
