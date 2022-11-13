#define SEM_2 "/00671428_write-access"
#define SEM_3 "/00671428_free"
#define SEM_4 "/00671428_used"
#define SHM_NAME "/00671428_myshm"
#define MAX_BUFFER_LENGTH (96)

struct myshm
{
    int wr_pos;
    int rd_pos;
    char solutions[MAX_BUFFER_LENGTH][200];
    int terminate;
};