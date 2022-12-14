#ifndef _COMMON_H_
#define _COMMON_H_

#define SHM_NAME "/osue_shm"
#define SEM_NAME_SERVER "/osue_server"
#define SEM_NAME_CLIENT "/osue_client"
#define SEM_NAME_READY "/osue_ready"

#define PERMISSIONS 0666  // shm

// BLZ, KONTONR, AT, PRUEFZIFFERN + '\0'
#define LEN_IBAN (5 + 11 + 2 + 2 + 1)

typedef enum { WITHDRAW, DEPOSIT } bank_account_cmd_t;

typedef struct shm_struct {
  char iban[LEN_IBAN];
  bank_account_cmd_t cmd;
  int amount;  // amount to withdraw or deposit
} shm_data_t;

#endif