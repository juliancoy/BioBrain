#!/bin/bash
# Fast development build - quicker compile, still runnable
set -e

JOBS=$(nproc)
export CCACHE_DIR="${HOME}/.ccache"
mkdir -p "$CCACHE_DIR"

echo "=== BioBrain Dev Build (fast iteration) ==="

# Configure with RelWithDebInfo (fast compile + debug symbols)
if [ ! -f build/CMakeCache.txt ]; then
    cmake -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_CUDA_COMPILER=/usr/bin/nvcc \
        -DCMAKE_CUDA_HOST_COMPILER=/usr/bin/g++-12 \
        -DCMAKE_C_COMPILER_LAUNCHER=ccache \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
        -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" \
        -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF
fi

# Build only changed files
echo "Building with $JOBS jobs..."
cmake --build build -j$JOBS

echo ""
echo "=== Done ==="
echo "Run: ./build/BioBrain"
