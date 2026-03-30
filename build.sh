#!/bin/bash
set -e

echo "=== BioBrain Fast Build ==="
echo ""

# Use all CPU cores
JOBS=$(nproc)

# Check for required tools
for tool in ninja ccache ld.lld; do
    if ! which $tool &>/dev/null; then
        echo "Warning: $tool not found. Install with: sudo apt install ninja-build ccache lld"
    fi
done

# Enable ccache
export CCACHE_DIR="${HOME}/.ccache"
mkdir -p "$CCACHE_DIR"

# Show ccache stats before
echo ""
echo "CCache stats (before):"
ccache -s 2>/dev/null | grep -E "cache hit|cache miss|files in cache" || true

# Clean previous build only if CMake cache is stale
if [ -f build/CMakeCache.txt ]; then
    # Check if compiler settings changed
    if ! grep -q "CMAKE_CUDA_HOST_COMPILER:FILEPATH=/usr/bin/g++-12" build/CMakeCache.txt 2>/dev/null; then
        echo "CMake cache stale, cleaning..."
        rm -rf build
    fi
fi

# Configure with fast settings
echo ""
echo "=== Configuring (Ninja + ccache + LLD) ==="
# Note: Use CMAKE_*_COMPILER_LAUNCHER for ccache (not CC/CXX env vars)
# to avoid conflicts with CMake's AUTOMOC
rm -rf build
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CUDA_COMPILER=/usr/bin/nvcc \
    -DCMAKE_CUDA_HOST_COMPILER=/usr/bin/g++-12 \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" \
    -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld"

# Build with Ninja
echo ""
echo "=== Building with $JOBS parallel jobs ==="
cmake --build build -j$JOBS

# Show ccache stats after
echo ""
echo "CCache stats (after):"
ccache -s 2>/dev/null | grep -E "cache hit|cache miss|files in cache" || true

echo ""
echo "=== Build complete ==="
echo "Run GUI:      ./build/BioBrain"
echo "Run headless: ./build/BioBrainHeadless"
echo ""
echo "Cache location: $CCACHE_DIR"
echo "Cache size: $(du -sh "$CCACHE_DIR" 2>/dev/null | cut -f1)"
