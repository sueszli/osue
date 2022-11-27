/** 
 *@file cpair.c
 *@author Andreas Huber 11809629
 *@date 
 *
 *@brief 
 *
 *@details 
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>
#include <unistd.h>
#include <sys/wait.h>
#include "cpair.h"

float calcDistance(point a, point b){
    float dx = (a.xCord-b.xCord);
    float dy = (a.yCord-b.yCord);
    float result = sqrt(dx*dx+dy*dy);
    return result;
}

int main(int argc, char *argv[]){

    
    if(argc!=1){
        fprintf(stderr,"Usage: ./cpair (no parameters)");
        exit(EXIT_FAILURE);
    }
    
    const char *ProgramName = argv[0];

    //aloccate space for data
    int dataSize = 8;
    point* data = malloc(sizeof(point)*dataSize);
    if(!data){
        fprintf(stderr, "malloc data failed");
        exit(EXIT_FAILURE);
    }

    char tmp[64];
    int cnt = 0;
    char* end;

    //read line per line and save points into data
    while(true){

        fgets(tmp, 64, stdin);
        if(feof(stdin)){
            break;
        }

        point p;
        p.xCord = strtof(tmp, &end);
        p.yCord = strtof(end, NULL);
        data[cnt] = p;
        cnt++;
    
        //realloc if size of data is too small
        if(cnt==dataSize){
            dataSize*=2;
            data = realloc(data, sizeof(point)*dataSize);
            if(!data){
                fprintf(stderr, "realloc data failed");
                exit(EXIT_FAILURE);
            }
        }
    }
 
    // sort array ascending on x-values
    for(int i = 0; i < cnt; i++){
        for(int j = 0; j < cnt-i-1; j++){
            if(data[j].xCord>data[j+1].xCord){
                point temp = data[j+1];
                data[j+1]=data[j];
                data[j]=temp;
            }
        }
    }

    //create cpoints struct for cpair result
    cpoints* result = malloc(sizeof(cpoints));
    if(!result){
        free(data);
        fprintf(stderr, "malloc result failed");
        exit(EXIT_FAILURE);
    }

    //calculate mean
    float mean = 0;
    for(int i = 0; i < cnt; i++){
        mean+= data[i].xCord;
    }
    mean /=cnt;

    //return if input <= 1 point
    if(cnt <= 1){
        free(result);
        free(data);
        exit(EXIT_SUCCESS);
    }

    //if input = 2 points, return these
    if(cnt == 2){
        fprintf(stdout, "%f %f\n%f %f\n",
        data[0].xCord, data[0].yCord,
        data[1].xCord, data[1].yCord);
        fflush(stdout);
        free(data);
        free(result);
        exit(EXIT_SUCCESS);
    }

    //input > 2 points

    //open pipes
    int inPipeChild1[2];
    int outPipeChild1[2];
    int inPipeChild2[2];
    int outPipeChild2[2];

    pid_t c1pid, c2pid;

    if(pipe(inPipeChild1)==-1){
        fprintf(stderr,"pipe inPipeChild1 failed");
        free(data);
        free(result);
        exit(EXIT_FAILURE);
    }
    if(pipe(outPipeChild1)==-1){
        fprintf(stderr,"pipe outPipeChild1 failed");
        free(data);
        free(result);
        exit(EXIT_FAILURE);
    }
    if(pipe(inPipeChild2)==-1){
        fprintf(stderr,"pipe inPipeChild2 failed");
        free(data);
        free(result);
        exit(EXIT_FAILURE);
    }
    if(pipe(outPipeChild2)==-1){
        fprintf(stderr,"pipe outPipeChild2 failed");
        free(data);
        free(result);
        exit(EXIT_FAILURE);
    }

    //fork child 1
    c1pid = fork();
    if(c1pid == -1){
        fprintf(stderr, "fork child 1 failed");
        free(data);
        free(result);
        exit(EXIT_FAILURE);
    }

    //close unused pipeneds
    if(c1pid == 0){

        if((dup2(inPipeChild1[0], STDIN_FILENO))<0){ 
            fprintf(stderr, "dup2 inPipeChild1[0] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
        if((dup2(outPipeChild1[1], STDOUT_FILENO))<0){ 
            fprintf(stderr, "dup2 outPipeChild1[1] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
        if(close(inPipeChild2[0])!=0){
            fprintf(stderr, "close inPipeChild2[0] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
        if(close(inPipeChild2[1])!=0){
            fprintf(stderr, "close inPipeChild2[1] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
         if(close(outPipeChild2[0])!=0){
            fprintf(stderr, "close outPipeChild2[0] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
        if(close(outPipeChild2[1])!=0){
            fprintf(stderr, "close outPipeChild2[1] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
        if(close(inPipeChild1[1])!=0){
            fprintf(stderr, "close inPipeChild1[1] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
        if(close(outPipeChild1[0])!=0){
            fprintf(stderr, "close outPipeChild1[0] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
        if(close(inPipeChild1[0])!=0){
            fprintf(stderr, "close inPipeChild1[0] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
        if(close(outPipeChild1[1])!=0){
            fprintf(stderr, "close outPipeChild1[1] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }

        if(execlp(ProgramName, ProgramName, NULL)==-1){
            fprintf(stderr, "execlp child 1 failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
    }

    //fork child 2
    c2pid = fork();
    if(c2pid == -1){
        fprintf(stderr, "fork child 2 failed");
        free(data);
        free(result);
        exit(EXIT_FAILURE);
    }
    
    //close unused pipeneds
    if(c2pid == 0){
        if((dup2(inPipeChild2[0], STDIN_FILENO))<0){
            fprintf(stderr, "dup2 inPipeChild2[0] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
        if((dup2(outPipeChild2[1], STDOUT_FILENO))<0){
            fprintf(stderr, "dup2 outPipeChild2[1] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
        if(close(inPipeChild1[0])!=0){
            fprintf(stderr, "close inPipeChild1[0] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
        if(close(inPipeChild1[1])!=0){
            fprintf(stderr, "close inPipeChild1[1] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
         if(close(outPipeChild1[0])!=0){
            fprintf(stderr, "close outPipeChild1[0] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
        if(close(outPipeChild1[1])!=0){
            fprintf(stderr, "close outPipeChild1[1] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
        if(close(inPipeChild2[1])!=0){
            fprintf(stderr, "close inPipeChild2[1] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
        if(close(outPipeChild2[0])!=0){
            fprintf(stderr, "close outPipeChild2[0] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
        if(close(inPipeChild2[0])!=0){
            fprintf(stderr, "close inPipeChild2[0] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
        if(close(outPipeChild2[1])!=0){
            fprintf(stderr, "close outPipeChild2[1] failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }

        if(execlp(ProgramName, ProgramName, NULL)==-1){
            fprintf(stderr, "execlp child 2 failed");
            free(data);
            free(result);
            exit(EXIT_FAILURE);
        }
    }

   //close pipes in parent
   if( close(inPipeChild1[0])!=0){
        fprintf(stderr, "close inPipeChild1[0] in parent failed");
        free(data);
        free(result);
        exit(EXIT_FAILURE);
   }
   if( close(outPipeChild1[1])!=0){
        fprintf(stderr, "close outPipeChild1[1] in parent failed");
        free(data);
        free(result);
        exit(EXIT_FAILURE);
   }
   if( close(inPipeChild2[0])!=0){
        fprintf(stderr, "close inPipeChild2[0] in parent failed");
        free(data);
        free(result);
        exit(EXIT_FAILURE);
   }
   if( close(outPipeChild2[1])!=0){
        fprintf(stderr, "close outPipeChild2[1] in parent failed");
        free(data);
        free(result);
        exit(EXIT_FAILURE);
   }

    //calculate number of elements < mean
    int ltm = 0;
    for(int i = 0; i < cnt; i++){
       if(data[i].xCord<=mean){
           ltm++;
       }
    }

    //create 2 new datas to hand to childs
    int fhsize = ltm;
    int shsize = cnt-ltm;

    point* firstHalf = malloc(sizeof(point)*fhsize);
    if(!firstHalf){
        fprintf(stderr, "malloc firstHalf failed\n");
        free(result);
        free(data);
        exit(EXIT_FAILURE);
    }
    point* secondHalf = malloc(sizeof(point)*shsize);
    if(!secondHalf){
        fprintf(stderr, "malloc secondHalf failed\n");
        free(result);
        free(data);
        exit(EXIT_FAILURE);
    }
    memcpy(firstHalf, data, sizeof(point)*fhsize);
    memcpy(secondHalf, data+ltm, sizeof(point)*shsize);

    //write from parent to child1
    FILE *fdwchild1 = fdopen(inPipeChild1[1], "w");
    if(fdwchild1 == NULL){
        fprintf(stderr, "fdopen inPipeChild1[1] failed\n");
        free(result);
        free(data);
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < fhsize; i++){
        fprintf(fdwchild1, "%f %f\n",firstHalf[i].xCord, firstHalf[i].yCord);
        //fflush maby
    }
   
    //write from parent to child2
    FILE *fdwchild2 = fdopen(inPipeChild2[1], "w");
    if(fdwchild2 == NULL){
        fprintf(stderr, "fdopen inPipeChild2[1] failed\n");
        free(result);
        free(data);
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < shsize; i++){
        fprintf(fdwchild2, "%f %f\n",secondHalf[i].xCord, secondHalf[i].yCord);
       // fflush(fdchild1); //maby?
    }
  
    fclose(fdwchild1);
    fclose(fdwchild2);

    //wait for child 1 to finish
    int status;
    waitpid(c1pid, &status, 0);
    if(WEXITSTATUS(status)!=EXIT_SUCCESS){
        free(result);
        free(data);
        exit(EXIT_FAILURE);
    }

    //wait for child 2 to finish
    waitpid(c2pid, &status, 0);
    if(WEXITSTATUS(status)!=EXIT_SUCCESS){
        free(result);
        free(data);
        exit(EXIT_FAILURE);
    }

    //create cpoints for result of childs
    cpoints* cpoint1 = malloc(sizeof(cpoints));
    if(!cpoint1){
        fprintf(stderr, "malloc cpoint1 failed");
        free(result);
        free(data);
        exit(EXIT_FAILURE);
    }
    cpoint1->initialized=false;
    //TODO: init mit max int
    
    cpoints* cpoint2 = malloc(sizeof(cpoints));
    if(!cpoint2){
        fprintf(stderr, "malloc cpoint2 failed");
        free(result);
        free(data);
        exit(EXIT_FAILURE);
    }
    cpoint2->initialized=false;
    //TODO: init mit max int

    //read result from child 1
    FILE* fdrchild1 = fdopen(outPipeChild1[0], "r");
    if(fdrchild1 == NULL){
        fprintf(stderr, "fdopen outPipeChild1[0] failed\n");
        free(result);
        free(data);
        exit(EXIT_FAILURE);
    }

    char* c1line = NULL;
    size_t len1 = 0;
    ssize_t nread1;
    char* parseEnd1;

    if((nread1 = getline(&c1line, &len1, fdrchild1))!=-1){
        point p;
        p.xCord = strtof(c1line, &parseEnd1);
        p.yCord = strtof(parseEnd1, NULL);
        cpoint1->pointA = p;
    }

    if((nread1 = getline(&c1line, &len1, fdrchild1))!=-1){
        point p;
        p.xCord = strtof(c1line, &parseEnd1);
        p.yCord = strtof(parseEnd1, NULL);
        cpoint1->pointB = p;
        cpoint1->initialized = true;
    }

    free(c1line);
    fclose(fdrchild1);

    //read from child 2
    FILE* fdrchild2 = fdopen(outPipeChild2[0], "r");
    if(fdrchild2 == NULL){
        fprintf(stderr, "fdopen outPipeChild2[0] failed\n");
        free(result);
        free(data);
        exit(EXIT_FAILURE);
    } 

    char* c2line = NULL;
    size_t len2 = 0;
    ssize_t nread2;
    char* parseEnd2;

   if((nread2 = getline(&c2line, &len2, fdrchild2))!=-1){
        point p;
        p.xCord = strtof(c2line, &parseEnd2);
        p.yCord = strtof(parseEnd2, NULL);
        cpoint2->pointA = p;
    }

    if((nread2 = getline(&c2line, &len2, fdrchild2))!=-1){
        point p;
        p.xCord = strtof(c2line, &parseEnd2);
        p.yCord = strtof(parseEnd2, NULL);
        cpoint2->pointB = p;
        cpoint2->initialized = true;
    }
    free(c2line);
    fclose(fdrchild2);

    //set min, check if child-results have valid pairs
    float min = FLT_MAX;
    if(cpoint1->initialized && cpoint2->initialized){
       float dcp1 = calcDistance(cpoint1->pointA, cpoint1->pointB);
       float dcp2 = calcDistance(cpoint2->pointA, cpoint2->pointB);
       min =  MIN(dcp1, dcp2);
       if(min==dcp1){
           result->pointA = cpoint1->pointA;
           result->pointB = cpoint1->pointB;
           result->initialized = true;
       }
       if(min==dcp2){
           result->pointA = cpoint2->pointA;
           result->pointB = cpoint2->pointB;
           result->initialized = true;
       } 
    } 

    //set c1 as min if thats valid and c2 is not valid
    if(cpoint1->initialized && !cpoint2->initialized){
           min = calcDistance(cpoint1->pointA, cpoint1->pointB);
           result->pointA = cpoint1->pointA;
           result->pointB = cpoint1->pointB;
           result->initialized = true;
    }

    //set c2 as min if thats valid and c1 is not valid
    if(!cpoint1->initialized && cpoint2->initialized){
           min = calcDistance(cpoint2->pointA, cpoint2->pointB);
           result->pointA = cpoint2->pointA;
           result->pointB = cpoint2->pointB;
           result->initialized = true;
    }

    for(int i = 0; i < fhsize; i++){
        for(int j = 0; j < shsize; j++){
            float currDistance = calcDistance(firstHalf[i], secondHalf[j]);
            if(currDistance<min){
                min = currDistance;
                result->pointA = firstHalf[i];
                result->pointB = secondHalf[j];
                result->initialized = true;
            }            
        }
    }

    if(result->initialized){
        fprintf(stdout, "%f %f\n%f %f\n",
        result->pointA.xCord, 
        result->pointA.yCord, 
        result->pointB.xCord, 
        result->pointB.yCord);
    } else {
        fprintf(stdout, "%f %f\n%f %f\n",
        FLT_MAX, FLT_MAX, FLT_MIN, FLT_MIN);
    }
  
 free(cpoint1);
 free(cpoint2);
 free(firstHalf);   
 free(secondHalf);
 free(result);
 free(data);
}