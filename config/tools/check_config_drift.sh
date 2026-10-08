#!/usr/bin/env bash
# =============================================================================
#  check_config_drift.sh — BSW configuration drift detection (CI gate)
# =============================================================================
#  Re-renders the configuration headers from config/bsw_config.json into a
#  temporary directory and compares them against the committed versions in
#  config/generated/.  A non-zero exit code means someone edited a generated
#  header (or committed a config change without regenerating).
#
#  Usage:
#      ./config/tools/check_config_drift.sh              # full drift check
#      ./config/tools/check_config_drift.sh -h           # help
#      EXTRA_ARGS are forwarded to generate_bsw_config.py, e.g.:
#      ./config/tools/check_config_drift.sh --no-rte     # headers only
#
#  Exit codes:
#      0  no drift (committed config/generated matches fresh generation)
#      1  drift detected (or committed outputs missing/out of date)
#      2  environment error (python/jinja2 missing, repo layout broken)
# =============================================================================

set -euo pipefail

# ── Locate the repository root (script lives in <repo>/config/tools) ────────
SCRIPT_SOURCE="${BASH_SOURCE[0]:-$0}"
SCRIPT_DIR="$(cd "$(dirname "${SCRIPT_SOURCE}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

GENERATOR="${REPO_ROOT}/config/tools/generate_bsw_config.py"
COMMITTED_DIR="${REPO_ROOT}/config/generated"

usage() {
    sed -n '2,22p' "${SCRIPT_SOURCE}" | sed 's/^#//; s/^ //'
}

# ── Help --------------------------------------------------------------------
for arg in "$@"; do
    case "${arg}" in
        -h|--help)
            usage
            exit 0
            ;;
    esac
done

# ── Preflight checks --------------------------------------------------------
if [[ ! -f "${GENERATOR}" ]]; then
    echo "[FAIL] Generator not found: ${GENERATOR}" >&2
    exit 2
fi
if [[ ! -d "${COMMITTED_DIR}" ]]; then
    echo "[FAIL] Committed output directory missing: ${COMMITTED_DIR}" >&2
    echo "       Run: python3 config/tools/generate_bsw_config.py && commit the results" >&2
    exit 1
fi

# Pick a python interpreter (PYTHON_BIN override supported for CI images)
PYTHON_BIN="${PYTHON_BIN:-}"
if [[ -z "${PYTHON_BIN}" ]]; then
    if command -v python3 >/dev/null 2>&1; then
        PYTHON_BIN="$(command -v python3)"
    elif command -v python >/dev/null 2>&1; then
        PYTHON_BIN="$(command -v python)"
    else
        echo "[FAIL] No python interpreter found (need python3 with jinja2)" >&2
        exit 2
    fi
fi

# ── Regenerate into a scratch directory -------------------------------------
TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/bsw_cfg_drift.XXXXXX")"
trap 'rm -rf "${TMP_DIR}"' EXIT

echo "[..] Regenerating configuration from config/bsw_config.json ..."
if ! "${PYTHON_BIN}" "${GENERATOR}" --output "${TMP_DIR}" "$@"; then
    echo "[FAIL] Configuration regeneration failed — see errors above" >&2
    exit 2
fi

# ── Compare against committed versions ---------------------------------------
echo "[..] Comparing generated output against ${COMMITTED_DIR} ..."
if diff_out="$(diff -r "${COMMITTED_DIR}" "${TMP_DIR}" 2>&1)"; then
    echo "[OK] No configuration drift: committed outputs match config/bsw_config.json"
    exit 0
fi

echo "${diff_out}"
echo "" >&2
echo "[FAIL] Configuration drift detected — generated headers differ from committed versions." >&2
echo "       Fix: run 'python3 config/tools/generate_bsw_config.py' and commit config/generated/" >&2
exit 1
