#!/bin/bash
# Clean build artifacts

echo "Cleaning build directory..."
rm -rf build

echo "Clearing ccache..."
ccache -C 2>/dev/null || true

echo "Done. Run ./build.sh for fresh build."
