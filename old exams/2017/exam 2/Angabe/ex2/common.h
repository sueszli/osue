#ifndef _COMMON_H_
#define _COMMON_H_

// Names of the shared resources
#define SHM_NAME        "/osue_shm"
#define SEM_NAME_SERVER "/osue_server"
#define SEM_NAME_CLIENT "/osue_client"
#define SEM_NAME_READY  "/osue_ready"

// Permissions for the shared resources
#define PERMISSIONS 0666

// BLZ, KONTONR, AT, PRUEFZIFFERN + '\0'
#define LEN_IBAN        (5 + 11 + 2 + 2 + 1)

typedef enum {
    WITHDRAW,
    DEPOSIT
} bank_account_cmd_t;

// Data structure for shared memory
typedef struct shm_struct {
    char iban[LEN_IBAN];
    bank_account_cmd_t cmd;
    int  amount; /**< Amount to withdraw or deposit. */
} shm_data_t;

#endif // _COMMON_H_
