#!/usr/bin/env bash
# Builds and runs the DSP test suite inside the distrobox.
#   scripts/test.sh          Release build, full suite + CPU benchmark -> tests/report.md
#   scripts/test.sh --asan   Debug + AddressSanitizer + UBSan (no benchmark) -> tests/report-asan.md
set -euo pipefail
cd "$(dirname "$0")/.."
DIR="$PWD"
if [ "${1:-}" = "--asan" ]; then
  # LSan: suppress known one-time allocations inside system libraries (fontconfig/X11/freetype caches)
  distrobox enter juce-build -- bash -lc "cd '$DIR' && cmake -B build-asan -G Ninja -DCMAKE_BUILD_TYPE=Debug -DDUO_SANITIZE=ON -DDUO_BUILD_SNAPSHOT=OFF >/dev/null \
    && cmake --build build-asan --target DuoDspTests DuoPluginTests -j${JOBS:-6} \
    && export ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 LSAN_OPTIONS=suppressions='$DIR'/tests/lsan.supp UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
    && ./build-asan/DuoDspTests --no-bench --report tests/report-asan.md \
    && ./build-asan/DuoPluginTests_artefacts/Debug/DuoPluginTests"
  exit $?
fi
distrobox enter juce-build -- bash -lc "cd '$DIR' && { [ -f build/CMakeCache.txt ] || cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release; } && cmake --build build --target DuoDspTests DuoPluginTests -j${JOBS:-6} && ctest --test-dir build --output-on-failure"
