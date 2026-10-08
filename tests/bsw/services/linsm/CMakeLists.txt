# LinSM unit tests — Phase 3 duplicate-module convergence.
# services/linsm is now a tombstone shim; canonical implementation lives in
# src/bsw/ecual/linSM/ (target: ecual_linSM).
#
# The test_linsm.c source was written against the old services/linsm API
# which uses non-standard types (Static, LinSM_ScheduleStatusType) not
# present in the canonical ecual/linSM.
# Dedicated linSM tests exist at:
#   tests/unit/autosar/services/test_linsm.c
#   tests/unit/linsm/test_linsm.c
