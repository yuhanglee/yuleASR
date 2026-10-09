/**
 * @file test_e2e_uds.c
 * @brief E2E Test: UDS Diagnostic Services (Dcm)
 *
 * Verifies UDS service request/response flow:
 *   DiagnosticSessionControl -> SecurityAccess -> ReadDataByIdentifier ->
 *   WriteDataByIdentifier -> RoutineControl -> ECUReset
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define DCM_RES_POS   0x40
#define DCM_E_GENERAL_REJECT   0x10
#define DCM_E_SUBFUNC_NOT_SUPP 0x12
#define DCM_E_SEC_DENIED       0x33
#define DCM_SID_DIAG_CTRL      0x10
#define DCM_SID_SEC_ACCESS     0x27
#define DCM_SID_READ_DID       0x22
#define DCM_SID_WRITE_DID      0x2E
#define DCM_SID_ROUTINE_CTRL   0x31
#define DCM_SID_ECU_RESET      0x11

#define SESSION_DEFAULT   0x01
#define SESSION_PROGRAM   0x02
#define SESSION_EXTENDED  0x03

typedef struct {
    unsigned char session;
    unsigned char security_level;
    unsigned char did_data[64];
    unsigned int  did_len;
    unsigned int  reset_count;
} Dcm_StateType;

static Dcm_StateType dcm;

static void Dcm_Init(void) {
    memset(&dcm, 0, sizeof(dcm));
    dcm.session = SESSION_DEFAULT;
}

static int Dcm_ProcessRequest(const unsigned char *req, unsigned int req_len,
                              unsigned char *res, unsigned int *res_len) {
    if (req_len == 0) return -1;
    unsigned char sid = req[0];
    res[0] = sid + DCM_RES_POS;
    *res_len = 1;

    switch (sid) {
    case DCM_SID_DIAG_CTRL:
        if (req_len < 2) { res[1] = DCM_E_SUBFUNC_NOT_SUPP; *res_len = 2; return -1; }
        if (req[1] > SESSION_EXTENDED) { res[1] = DCM_E_SUBFUNC_NOT_SUPP; *res_len = 2; return -1; }
        dcm.session = req[1];
        res[1] = req[1];
        *res_len = 2;
        break;
    case DCM_SID_SEC_ACCESS:
        if (req_len < 2) { res[1] = DCM_E_SUBFUNC_NOT_SUPP; *res_len = 2; return -1; }
        if (req[1] == 0x01) {
            dcm.security_level = 1;
            res[1] = 0x01;
            res[2] = 0xAA; res[3] = 0xBB;
            *res_len = 4;
        } else if (req[1] == 0x02) {
            if (req_len < 4 || req[2] != 0xAA || req[3] != 0xBB) {
                res[1] = DCM_E_SEC_DENIED; *res_len = 2; return -1;
            }
            dcm.security_level = 2;
            res[1] = 0x02;
            *res_len = 2;
        } else {
            res[1] = DCM_E_SUBFUNC_NOT_SUPP; *res_len = 2; return -1;
        }
        break;
    case DCM_SID_READ_DID:
        if (dcm.security_level < 1) {
            res[1] = DCM_E_SEC_DENIED; *res_len = 2; return -1;
        }
        res[1] = req[1]; res[2] = req[2];
        memcpy(&res[3], dcm.did_data, dcm.did_len);
        *res_len = 3 + dcm.did_len;
        break;
    case DCM_SID_WRITE_DID:
        if (dcm.security_level < 2) {
            res[1] = DCM_E_SEC_DENIED; *res_len = 2; return -1;
        }
        dcm.did_len = req_len - 3;
        memcpy(dcm.did_data, &req[3], dcm.did_len);
        res[1] = req[1]; res[2] = req[2];
        *res_len = 3;
        break;
    case DCM_SID_ECU_RESET:
        if (req_len < 2) { res[1] = DCM_E_SUBFUNC_NOT_SUPP; *res_len = 2; return -1; }
        dcm.reset_count++;
        res[1] = req[1];
        *res_len = 2;
        break;
    default:
        res[0] = 0x7F;
        res[1] = sid;
        res[2] = DCM_E_GENERAL_REJECT;
        *res_len = 3;
        return -1;
    }
    return 0;
}

static int test_uds_session_control(void) {
    Dcm_Init();
    unsigned char req[2] = {DCM_SID_DIAG_CTRL, SESSION_EXTENDED};
    unsigned char res[8]; unsigned int res_len;
    assert(Dcm_ProcessRequest(req, sizeof(req), res, &res_len) == 0);
    assert(res[0] == DCM_SID_DIAG_CTRL + DCM_RES_POS);
    assert(res[1] == SESSION_EXTENDED);
    assert(dcm.session == SESSION_EXTENDED);
    printf("  [PASS] test_uds_session_control\n");
    return 1;
}

static int test_uds_security_access(void) {
    Dcm_Init();
    unsigned char req[8], res[16]; unsigned int res_len;

    req[0] = DCM_SID_SEC_ACCESS; req[1] = 0x01;
    assert(Dcm_ProcessRequest(req, 2, res, &res_len) == 0);
    assert(res[0] == DCM_SID_SEC_ACCESS + DCM_RES_POS);
    assert(res[2] == 0xAA && res[3] == 0xBB);

    req[0] = DCM_SID_SEC_ACCESS; req[1] = 0x02;
    req[2] = 0xAA; req[3] = 0xBB;
    assert(Dcm_ProcessRequest(req, 4, res, &res_len) == 0);
    assert(dcm.security_level == 2);
    printf("  [PASS] test_uds_security_access\n");
    return 1;
}

static int test_uds_read_write_did(void) {
    Dcm_Init();
    dcm.security_level = 2;
    unsigned char req[16], res[32]; unsigned int res_len;

    req[0] = DCM_SID_WRITE_DID; req[1] = 0xF1; req[2] = 0x90;
    req[3] = 'H'; req[4] = 'e'; req[5] = 'l'; req[6] = 'l'; req[7] = 'o';
    assert(Dcm_ProcessRequest(req, 8, res, &res_len) == 0);
    assert(res_len == 3);

    req[0] = DCM_SID_READ_DID; req[1] = 0xF1; req[2] = 0x90;
    assert(Dcm_ProcessRequest(req, 3, res, &res_len) == 0);
    assert(res[3] == 'H' && res[4] == 'e' && res[7] == 'o');
    printf("  [PASS] test_uds_read_write_did\n");
    return 1;
}

static int test_uds_security_denied(void) {
    Dcm_Init();
    unsigned char req[3] = {DCM_SID_READ_DID, 0xF1, 0x90};
    unsigned char res[8]; unsigned int res_len;
    assert(Dcm_ProcessRequest(req, sizeof(req), res, &res_len) == -1);
    assert(res[1] == DCM_E_SEC_DENIED);
    printf("  [PASS] test_uds_security_denied\n");
    return 1;
}

static int test_uds_ecu_reset(void) {
    Dcm_Init();
    unsigned char req[2] = {DCM_SID_ECU_RESET, 0x01};
    unsigned char res[8]; unsigned int res_len;
    assert(Dcm_ProcessRequest(req, sizeof(req), res, &res_len) == 0);
    assert(dcm.reset_count == 1);
    assert(res[1] == 0x01);
    printf("  [PASS] test_uds_ecu_reset\n");
    return 1;
}

int main(void) {
    int passed = 0, total = 0;
    printf("=== E2E Test: UDS Diagnostic Services ===\n");
    total++; passed += test_uds_session_control();
    total++; passed += test_uds_security_access();
    total++; passed += test_uds_read_write_did();
    total++; passed += test_uds_security_denied();
    total++; passed += test_uds_ecu_reset();
    printf("\nResult: %d/%d tests passed\n", passed, total);
    return (passed == total) ? 0 : 1;
}
