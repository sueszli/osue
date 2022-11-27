/**
 * @file forkFFT.h
 * @author Valentin Hrusa 11808205
 * @brief includes structs and the PI-constant important for forkFFT.c
 * 
 * @date 2021-12-05
 * 
 */
#define PI 3.141592654f

/**
 * @brief head-node for the FloatingPointList
 * @details the input numbers are organised in a linked-list data-structure.
 *          This struct acts as the head of the structure.
 *          It also keeps the overall length saved.
 *
 */
struct FPListHead{
    size_t length;
    struct FPList *next;
} FPListHead;

/**
 * @brief list-node for the FloatingPointList
 * @details the input numbers are organised in a linked-list data-structure.
 *          This struct acts as an ordinary entry in said linked-list
 *          It holds the next element in the linked-list, the index of the current entry in the list
 *          and the representation of the complex number in string and float form. 
 * 
 */
struct FPList{
    float rfpn;
    float ifpn;
    char *str;
    int index;
    struct FPList *next;
} FPList;

/**
 * @brief struct for saving data of child-process
 * @details when a child-process is created this struct is created and fed the pid of the process.
 *          It also holds two file-descriptors to the read and write end of two different pipes which 
 *          are duplicated to hold stdin and stdout of the child-process
 * 
 */
struct Communicator{
    pid_t pid;
    int write;
    int read;
} Communicator;