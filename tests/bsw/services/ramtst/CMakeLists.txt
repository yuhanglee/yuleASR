# RamTst unit tests — Phase 3 duplicate-module convergence.
# services/ramtst is now a tombstone shim; canonical implementation lives in
# src/bsw/mcal/ramtst/ (target: mcal_ramtst, v2.0.0 with 6 algorithms).
#
# The test_ramtst.c source was written against the old services/ramtst API
# (v1.0.0, 2 algorithms) which differs from the canonical mcal/ramtst
# (v2.0.0, 6 algorithms, error records, Lcfg).
# Dedicated mcal/ramtst tests exist at:
#   tests/bsw/mcal/ramtst/test_ramtst.c (substantiated MCAL test)
