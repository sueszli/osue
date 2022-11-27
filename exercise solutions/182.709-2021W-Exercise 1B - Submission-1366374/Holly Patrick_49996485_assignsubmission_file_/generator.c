#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h> //PROT_READ etc.
#include <regex.h>        

#define SEM_1 "/access" //For all generators
#define SEM_2 "/read"  //For the supervisor
#define SEM_3 "/inaccess" 
#define SEM_4 "/loggedIn"

#define SHM_NAME "/myshm"

struct myshm{
    unsigned int exit;
    unsigned int read_pos;
    unsigned int write_pos;
    char data[50][64];
    unsigned int deleted[50];
};

struct nodes {
   int data;
   struct nodes *next;
};

struct nodes* nodesHead = NULL;
int nodesSize=0;

struct edges {
    int from;
    int to;
    struct edges * next;
};

struct edges* edgesHead = NULL;
char *myprog;

//insert link at the first location
void insertFirstNode(int data) {
    nodesSize+=1;
   //create a link
   struct nodes *link = (struct nodes*) malloc(sizeof(struct nodes));
	
   link->data = data;
	
   //point it to old first node
   link->next = nodesHead;
	
   //point first to new first node
   nodesHead = link;
}


void printNodesList(void) {
   struct nodes *ptr = nodesHead;
   //start from the beginning
   while(ptr != NULL) {
        printf("\n");
        printf("%d",ptr->data);
        ptr = ptr->next;
   }
}

/**
    frees the list after it's use for cleanup.
**/
void freeNodesList(struct nodes* head){
    struct nodes* tmp;
   while (head != NULL)
    {
       tmp = head;
       head = head->next;
       free(tmp);
    }
}
/**
    frees the list after it's use for cleanup.
**/
void freeEdgesList(struct edges* head) {
    struct edges* tmp;
   while (head != NULL)
    {
       tmp = head;
       head = head->next;
       free(tmp);
    }
}

int nodesLength(void) {
   return nodesSize;
}

void printEdgesList(void) {
    struct edges *ptr = edgesHead;
    while(ptr != NULL) {
        printf("\n");
        printf("%d-%d",ptr->from,ptr->to);
        ptr = ptr->next;
   }
   printf("\n");
}

/**
    Inserts a new first entry of a linked list of type edge
**/
void insertFirstEdge(int from, int to){
    struct edges *link = (struct edges*) malloc(sizeof(struct edges));
    link->from=from;
    link->to=to;
    link->next = edgesHead;
    edgesHead=link;
}

/**
    When the Node number isn't already saved it gets saved.
**/
void addToNodesIfNew(int node){
    
    struct nodes *ptr = nodesHead;
    while(ptr != NULL) {
        if (node==ptr->data) {
           return;
        }
        ptr = ptr->next;
    }
    
    insertFirstNode(node);
}

struct nodes* getByNumber(int number){
    struct nodes *ptr = nodesHead;
    while(number!=0){
        ptr = ptr->next;
        number=number-1;
    }
    return ptr;
}

void shuffleNodes(int n[]){
    int new[nodesSize];
    for (int i=0;i<nodesSize;i++){
        int size=nodesSize-i;
        int random= rand()%(size);
        new[i]=n[random];
        for ( int ii=random; ii<nodesSize;ii++){
            n[ii]=n[ii+1];
        };
    }
    memcpy(n,new,sizeof(new));

}

int getpos(int n[],int tofind){
    for (int i=0;i<nodesSize;i++){
        if(n[i]==tofind){
            return i;
        }
    }
    fprintf(stderr," [%s] ERROR: Couldn't find the number %i \n",myprog,tofind);
    return EXIT_FAILURE;
}





int main(int argc, char *argv[]){
    myprog=argv[0];    
    time_t t;
    srand((unsigned) time(&t));

    regex_t regex;
    

    if(argc==1){
        fprintf(stderr," [%s] ERROR: No Edge given \n",myprog);
        return EXIT_FAILURE;
    }
    int value;

    value=regcomp(&regex,"^[0-9]+-[0-9]+$",REG_EXTENDED);

    if(value!=0){
        fprintf(stderr," [%s] ERROR: %s \n",myprog,strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    for(; optind < argc; optind++){ 
            char * value = argv[optind];
            int result = regexec(&regex,value , 0, NULL, 0);


            if (result!=0) {
                fprintf(stderr," [%s] ERROR: Node pattern wrong: %s\n",myprog,value);
                return EXIT_FAILURE;
            }

            char* from=strtok(argv[optind],"-");
            char* to = strtok(NULL,"-");
            addToNodesIfNew(strtol(from,NULL,10));
            addToNodesIfNew(strtol(to,NULL,10));
            insertFirstEdge(strtol(from,NULL,10),strtol(to,NULL,10));
        }
        
        int nodeslist[nodesSize];
        struct nodes *pt = nodesHead;
        int counter=0;
        while(pt != NULL) { //writes into an array
            nodeslist[counter]=pt->data;
            counter+=1;
            pt = pt->next;
        }
        

        int shmfd = shm_open(SHM_NAME,O_RDWR,0600);
        if(shmfd == -1){
            fprintf(stderr," [%s] ERROR: %s \n",myprog,strerror(errno));
            exit(EXIT_FAILURE);
        };

        struct myshm *myshm;
        myshm=mmap(NULL,sizeof(*myshm),PROT_READ | PROT_WRITE,MAP_SHARED,shmfd,0);

        if(myshm == MAP_FAILED){
            fprintf(stderr," [%s] ERROR: %s \n",myprog,strerror(errno));
            exit(EXIT_FAILURE);
        };

        /**
            Opens Semaphores
        **/
        sem_t *sem_access = sem_open(SEM_1, O_EXCL);
        sem_t *sem_read = sem_open(SEM_2,O_EXCL);
        sem_t *sem_inAccess = sem_open(SEM_3, O_EXCL);
        sem_t *sem_loggedIn = sem_open(SEM_4,O_EXCL);

        if (sem_access == SEM_FAILED){
            fprintf(stderr," [%s] ERROR: %s \n",myprog,strerror(errno));
            exit(EXIT_FAILURE);
        };

        if (sem_read == SEM_FAILED){
            fprintf(stderr," [%s] ERROR: %s \n",myprog,strerror(errno));
            exit(EXIT_FAILURE);
        };

        if(sem_inAccess == SEM_FAILED){
            fprintf(stderr," [%s] ERROR: %s \n",myprog,strerror(errno));
            exit(EXIT_FAILURE);
        };
        if (sem_loggedIn == SEM_FAILED){
        fprintf(stderr," [%s] ERROR: %s \n",myprog,strerror(errno));
        exit(EXIT_FAILURE);
        };
        sem_post(sem_loggedIn);

        //Until the supervisor closes
        while(myshm->exit!=1){
        //Nodes get randomly shuffled
        shuffleNodes(nodeslist); 
    
        
        char usedEdges[64]="";
        
        //a solution is generated 
        int numberOfRemovedEdges=0;
        struct edges *ptr = edgesHead;
        while(ptr != NULL) {
            if(getpos(nodeslist,ptr->to)<getpos(nodeslist,ptr->from)){
                char buf[64];
                sprintf(buf," %i-%i",ptr->from,ptr->to);
                strcat(usedEdges,buf);
                numberOfRemovedEdges+=1;
            }
            ptr=ptr->next;
            
        }
            

            if(numberOfRemovedEdges<=8){ //Solutions with too many edges get discarded
                sem_wait(sem_access);
                sem_wait(sem_inAccess);
                int number = myshm->write_pos; 
                myshm->write_pos=(myshm->write_pos+1)%50;
                sem_post(sem_inAccess);

                strcpy(myshm->data[number],usedEdges);
                myshm->deleted[number]=numberOfRemovedEdges;
            
                sem_post(sem_read);

            }


    }

    sem_wait(sem_loggedIn);
    
    freeNodesList(nodesHead);
    freeEdgesList(edgesHead);

    sem_close(sem_access);
    sem_close(sem_inAccess);
    sem_close(sem_read);
    sem_unlink(SEM_1);
    sem_unlink(SEM_2);
    sem_unlink(SEM_3);

    sem_close(sem_loggedIn);
    sem_unlink(SEM_4);

    return EXIT_SUCCESS;

}





