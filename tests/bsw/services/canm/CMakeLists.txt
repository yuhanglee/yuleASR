# CanNm unit tests — Phase 3 duplicate-module convergence.
# services/canm is now a tombstone shim; canonical implementation lives in
# src/bsw/ecual/canNm/ (target: ecual_canNm).
#
# The test_canm.c source was written against the old services/canm API
# (CANNM_E_INVALID_POINTER, silent double-init) which differs from the
# canonical ecual/canNm contract (CANNM_E_PARAM_POINTER, CANNM_E_ALREADY_INITIALIZED).
# Dedicated ecual/canNm tests exist at:
#   tests/bsw/ecual/canNm/test_cannm.c
#   tests/unit/autosar/ecual/test_canNm.c
