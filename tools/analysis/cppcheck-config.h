/*
 * cppcheck-config.h — AUTOSAR platform configuration for cppcheck MISRA addon
 *
 * This file provides default defines for AUTOSAR BSW platform constants
 * and configuration parameters that cppcheck cannot resolve from headers.
 * It enables the MISRA addon to perform complete analysis by eliminating
 * "Because of missing configuration" errors.
 */

#ifndef CPPCHECK_CONFIG_H
#define CPPCHECK_CONFIG_H

/*
 * ── Platform defines ──
 * Target: S32K312 (NXP S32K3xx family)
 */
#define S32K312                            1u
#define PLATFORM_S32K312                   1u
#define CPU_S32K312                        1u

/* ── AUTOSAR standard return types ── */
#define E_OK                               0u
#define E_NOT_OK                           1u
#define E_COM_OK                           0u
#define E_COM_BUSY                         1u
#define E_COM_LIMITED                      2u
#define E_COM_NOT_OK                       3u

/* ── Standard types (minimal subset for analysis) ── */
#define TRUE                               1u
#define FALSE                              0u
#define NULL_PTR                           ((void*)0)
#define NULL                               0

/* ── AUTOSAR standard constants ── */
#define STD_ON                             1u
#define STD_OFF                            0u
#define STD_HIGH                           1u
#define STD_LOW                            0u
#define STD_ACTIVE                         1u
#define STD_IDLE                           0x00u

/* ── Crypto config ── */
#define CRYPTO_NUM_KEYS                    16u
#define CRYPTO_NUM_DRIVER_OBJECTS          4u
#define CRYPTO_NUM_CHANNELS                8u

/* ── CSM (Crypto Service Manager) config ── */
#define CSM_MAX_KEY_LENGTH                 32u
#define CSM_MAX_HASH_LENGTH                64u
#define CSM_MAX_DATA_LENGTH                1024u
#define CSM_MAX_MAC_LENGTH                 32u
#define CSM_MAX_SIGNATURE_LENGTH           256u
#define CSM_MAX_KEY_ELEMENTS               32u
#define CSM_KEY_STATUS_VALID               1u
#define CSM_KEY_USAGE_ENCRYPT              (1u << 0)
#define CSM_KEY_USAGE_DECRYPT              (1u << 1)
#define CSM_KEY_USAGE_SIGN                 (1u << 2)
#define CSM_KEY_USAGE_VERIFY               (1u << 3)
#define CSM_KEY_USAGE_MAC_GENERATE         (1u << 4)
#define CSM_KEY_USAGE_MAC_VERIFY           (1u << 5)
#define CSM_KEY_USAGE_KEY_EXCHANGE         (1u << 6)
#define CSM_KEY_USAGE_DERIVE               (1u << 7)
#define CSM_OPERATION_MODE_START           0u
#define CSM_OPERATION_MODE_UPDATE           1u
#define CSM_OPERATION_MODE_FINISH           2u
#define CSM_SERVICE_KEY_GENERATE           0u
#define CSM_SERVICE_KEY_DERIVE             1u
#define CSM_SERVICE_KEY_EXCHANGE           2u

/* ── NvM (NVRAM Manager) config ── */
#define NVM_CFG_MAX_BLOCK_ID               64u
#define NVM_NUM_OF_NVRAM_BLOCKS            32u
#define NVM_SIZE_STANDARD_JOB_QUEUE        16u
#define NVM_SIZE_IMMEDIATE_JOB_QUEUE       8u
#define NVM_JOB_TYPE_READ                  0u
#define NVM_JOB_TYPE_WRITE                 1u
#define NVM_BLOCK_DATASET                  0u
#define NVM_BLOCK_REDUNDANT                1u
#define NVM_STATE_IDLE                     0u
#define NVM_STATE_BUSY                     1u
#define MEMIF_IDLE                         0u
#define MEMIF_BUSY                         1u
#define MEMIF_BUSY_INTERNAL                2u
#define MEMIF_JOB_OK                       0u

/* ── XCP config ── */
#define XCP_CTO_SIZE                       8u
#define XCP_DTO_SIZE                       8u
#define XCP_MAX_DAQ                        16u
#define XCP_MAX_ODT                        8u
#define XCP_MAX_STIM                       4u
#define XCP_MAX_SEGMENTS                   4u
#define XCP_MAX_KEY_SIZE                   16u
#define XCP_MAX_SEED_SIZE                  16u
#define XCP_MAX_DAQ_LISTS                  4u
#define XCP_MAX_EVENT_CHANNELS             8u
#define XCP_MAX_ODTS_PER_DAQ               4u
#define XCP_MAX_ODT_ENTRIES_PER_ODT        8u
#define XCP_SEED_LENGTH                    8u
#define XCP_SESSION_DAQ_RUNNING            1u
#define XCP_DAQ_MODE_DTO_CTR               0u
#define XCP_DAQ_MODE_PID_OFF               1u
#define XCP_DAQ_MODE_SELECTED              2u
#define XCP_DAQ_MODE_STIM                  3u
#define XCP_DAQ_MODE_TIMESTAMP             4u

/* ── DLT config ── */
#define DLT_MAX_CONTEXT_COUNT              32u
#define DLT_BUFFER_COUNT                   8u
#define DLT_ECU_ID_LENGTH                  20u

/* ── DoIP config ── */
#define DOIP_VIN_LENGTH                    17u
#define DOIP_EID_LENGTH                    6u
#define DOIP_GID_LENGTH                    6u

/* ── IpduM config ── */
#define IPDUM_MAX_TX_MUX_PDUS              16u
#define IPDUM_MAX_RX_MUX_PDUS              16u

/* ── J1939Tp config ── */
#define J1939TP_NUM_NSDUS                  4u

/* ── LinTp config ── */
#define LINTP_MAX_CHANNEL_COUNT            4u
#define LINTP_NSDU_COUNT                   4u

/* ── OS config ── */
#define OS_TASK_COUNT                      16u
#define OS_RESOURCE_COUNT                  8u

/* ── EcuM config ── */
#define ECUM_MAX_WAKEUP_SOURCES            8u
#define ECUM_SHUTDOWN_TARGET_SLEEP         0u
#define ECUM_STATE_POST_RUN                1u
#define ECUM_STATE_RUN                     2u
#define ECUM_WKSOURCE_NONE                 0u
#define ECUM_WKSTATUS_NONE                 0u
#define ECUM_WKSTATUS_PENDING              1u
#define ECUM_WKSTATUS_VALIDATED            2u
#define E_OK_OK                            0u

/* ── LOCKSTEP config ── */
#define LOCKSTEP_MODE_ENABLED              1u

/* ── S32K312 HSM (Hardware Security Module) ── */
#define S32K312_HSM_ECC_CURVE_SECP256R1    1u
#define S32K312_HSM_ECC_CURVE_SECP384R1    1u
#define S32K312_HSM_STATE_READY            1u
#define S32K312_HSM_STATE_ERROR            0u
#define S32K312_HSM_AES_KEY_SIZE_128       16u
#define S32K312_HSM_AES_KEY_SIZE_256       32u
#define S32K312_HSM_AES_MODE_ECB           0u
#define S32K312_HSM_AES_MODE_CBC           1u
#define S32K312_HSM_AES_MODE_GCM           2u
#define S32K312_HSM_AES_BLOCK_SIZE         16u
#define S32K312_HSM_AES_GCM_TAG_SIZE       16u
#define S32K312_HSM_SHA256_BLOCK_SIZE      64u
#define S32K312_HSM_SHA256_DIGEST_SIZE     32u
#define S32K312_HSM_MAX_KEY_SLOTS          16u
#define S32K312_HSM_STATUS_BUSY            0u
#define S32K312_HSM_CTRL_BUSY              0u
#define S32K312_HSM_CTRL_DONE              1u
#define S32K312_HSM_CTRL_ERROR             2u
#define S32K312_HSM_AES_CTRL_BUSY          S32K312_HSM_CTRL_BUSY
#define S32K312_HSM_AES_CTRL_DONE          S32K312_HSM_CTRL_DONE
#define S32K312_HSM_AES_CTRL_ERROR         S32K312_HSM_CTRL_ERROR
#define S32K312_HSM_ECC_CTRL_BUSY          S32K312_HSM_CTRL_BUSY
#define S32K312_HSM_ECC_CTRL_DONE          S32K312_HSM_CTRL_DONE
#define S32K312_HSM_ECC_CTRL_ERROR         S32K312_HSM_CTRL_ERROR
#define S32K312_HSM_SHA_CTRL_BUSY          S32K312_HSM_CTRL_BUSY
#define S32K312_HSM_SHA_CTRL_DONE          S32K312_HSM_CTRL_DONE
#define S32K312_HSM_SHA_CTRL_ERROR         S32K312_HSM_CTRL_ERROR

/* ── CSM platform config ── */
#define CDR_MAX_STRING_LENGTH              256u

/* ── CanNm (CAN Network Management) config ── */
#define CANNM_NUMBER_OF_CHANNELS             4u
#define CANNM_MAIN_FUNCTION_PERIOD_MS        10u
#define CANNM_USER_DATA_LENGTH               8u
#define CANNM_PDU_LENGTH                     8u
#define CANNM_DEV_ERROR_DETECT               STD_ON
#define CANNM_STATE_CHANGE_IND_ENABLED       STD_ON
#define CANNM_REMOTE_SLEEP_IND_CALLBACK      STD_ON
#define CANNM_REMOTE_SLEEP_IND_TIME          500u

/* CanNm SID (Service ID) constants */
#define CANNM_SID_PASSIVE_STARTUP            0x01u
#define CANNM_SID_NETWORK_REQUEST            0x02u
#define CANNM_SID_NETWORK_RELEASE            0x03u
#define CANNM_SID_SET_USER_DATA              0x04u
#define CANNM_SID_GET_USER_DATA              0x05u
#define CANNM_SID_GET_STATE                  0x0Bu
#define CANNM_SID_DISABLE_COMMUNICATION      0x0Cu
#define CANNM_SID_ENABLE_COMMUNICATION       0x0Du
#define CANNM_SID_SET_SLEEP_READY_BIT        0x12u

/* CanNm state enum values */
#define CANNM_STATE_BUS_SLEEP_MODE           0u
#define CANNM_STATE_PREPARE_BUS_SLEEP_MODE   1u
#define CANNM_STATE_READY_SLEEP_MODE         2u
#define CANNM_STATE_NORMAL_OPERATION_MODE    3u
#define CANNM_STATE_REPEAT_MESSAGE_MODE      4u

/* ── EthSM (Ethernet State Manager) config ── */
#define ETHSM_MAX_RETRIES                    3u
#define ETHSM_NETWORK_0                      0u
#define ETHSM_NETWORK_1                      1u
#define ETHSM_MAX_NETWORKS                   2u
#define ETHSM_TIMEOUT_WAIT_TRCVLINK          5000u
#define ETHSM_TIMEOUT_WAIT_ONLINE            10000u

/* ── ComM mode constants (for EthSM and other modules) ── */
#define COMM_NO_COMMUNICATION                0u
#define COMM_SILENT_COMMUNICATION            1u
#define COMM_FULL_COMMUNICATION              2u

/* ── TcpIp state constants ── */
#define TCPIP_STATE_OFFLINE                  0u
#define TCPIP_STATE_ONLINE                   1u
#define TCPIP_STATE_ONHOLD                   2u

/* ── AUTOSAR PDU types (for cppcheck type resolution) ── */
typedef unsigned short PduIdType;

/* ── SomeIpXf config ── */
#define SomeIpXf_ProtocolVersion           1u
#define SomeIpXf_InterfaceVersion          1u

#endif /* CPPCHECK_CONFIG_H */
