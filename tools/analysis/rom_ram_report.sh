#!/usr/bin/env bash
# =============================================================================
# yuleASR ROM/RAM Usage Tracker
# =============================================================================
# Reports code/RO-data/RW-data/ZI-data sizes for built ELF binaries, and
# optionally parses .map files for per-module breakdown.
#
# Usage:
#   tools/analysis/rom_ram_report.sh [BUILD_DIR] [--map MAP_FILE] [--json]
#
# Defaults:
#   BUILD_DIR = build-native or build-release (first found)
#   --map     Parse a linker .map file for per-section breakdown
#   --json    Output machine-readable JSON instead of table
# =============================================================================
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$REPO_ROOT"

BUILD_DIR=""
MAP_FILE=""
JSON_OUT=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --map)  MAP_FILE="$2"; shift 2 ;;
        --json) JSON_OUT=1; shift ;;
        *)      BUILD_DIR="$1"; shift ;;
    esac
done

if [[ -z "$BUILD_DIR" ]]; then
    for d in build-native build-release build; do
        [[ -d "$d" ]] && BUILD_DIR="$d" && break
    done
fi

if [[ -z "$BUILD_DIR" ]]; then
    echo "ERROR: No build directory found. Run cmake --build first." >&2
    exit 1
fi

SIZE_TOOL="arm-none-eabi-size"
if ! command -v "$SIZE_TOOL" >/dev/null 2>&1; then
    SIZE_TOOL="size"
    if ! command -v "$SIZE_TOOL" >/dev/null 2>&1; then
        echo "ERROR: Neither arm-none-eabi-size nor size found" >&2
        exit 1
    fi
fi

ELFS=()
while IFS= read -r -d '' f; do
    ELFS+=("$f")
done < <(find "$BUILD_DIR" -maxdepth 4 \( -name '*.elf' -o -name '*.exe' \) -print0 2>/dev/null)

if [[ ${#ELFS[@]} -eq 0 ]]; then
    echo "No ELF binaries found in $BUILD_DIR"
    exit 0
fi

if [[ $JSON_OUT -eq 1 ]]; then
    echo "{"
    echo '  "build_dir": "'"$BUILD_DIR"'",'
    echo '  "binaries": ['
    first=1
    for elf in "${ELFS[@]}"; do
        [[ $first -eq 0 ]] && echo ","
        first=0
        read -r text data bss dec hex name <<< "$($SIZE_TOOL -A "$elf" 2>/dev/null | awk '
            /^\.text/   { t += $2 }
            /^\.data/   { d += $2 }
            /^\.bss/    { b += $2 }
            /^\.rodata/ { t += $2 }
            END { printf "%d %d %d %d %s %s\n", t, d, b, t+d+b, "0", FILENAME }
        ' 2>/dev/null || echo "0 0 0 0 0 $elf")"
        echo -n '    {"file":"'"$(basename "$elf")"'","text":'"$text"',"data":'"$data"',"bss":'"$bss"',"total":'"$((text+data+bss))"'}'
    done
    echo ""
    echo "  ]"
    echo "}"
else
    printf "%-40s %10s %10s %10s %10s\n" "Binary" "text" "data" "bss" "total"
    printf "%-40s %10s %10s %10s %10s\n" "------" "----" "----" "---" "-----"
    total_all=0
    for elf in "${ELFS[@]}"; do
        read -r text data bss dec <<< "$($SIZE_TOOL "$elf" 2>/dev/null | tail -1 | awk '{print $1,$2,$3,$4}')"
        text="${text:-0}"; data="${data:-0}"; bss="${bss:-0}"; dec="${dec:-0}"
        printf "%-40s %10s %10s %10s %10s\n" "$(basename "$elf")" "$text" "$data" "$bss" "$dec"
        total_all=$((total_all + dec))
    done
    printf "\n%-40s %10s %10s %10s %10s\n" "TOTAL" "" "" "" "$total_all"
fi

if [[ -n "$MAP_FILE" && -f "$MAP_FILE" ]]; then
    echo ""
    echo "=== Map file breakdown: $MAP_FILE ==="
    echo ""
    grep -E '^\.(text|data|bss|rodata)' "$MAP_FILE" 2>/dev/null | head -20 || \
        echo "(no standard sections found in map file)"
fi
