/**
*@file intmul.c
*@author Klaus Trimbacher 11908086
*
*@brief Multiply two hexadecimical Numbers
*@details The Programm needs two Numbers in hexadecimical form and 
*
* 
*@date 11.12.2021
**/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <math.h>


/**
*Programm entry point/ main method
*@brief The programm starts here. The function takes all the arguments and put the Coordinates in a struct 
*and into an array. 
*@details It close all pipes which it opened.
**/
int main(int argc, char *argv[]){

	char *Pointstring = NULL;
	size_t len = 0;
	
	getline(&Pointstring, &len, stdin);
	
	char * A = malloc(strlen(Pointstring) + 1); 
	strcpy(A, Pointstring);
	
	getline(&Pointstring, &len, stdin);
	
	char * B = malloc(strlen(Pointstring) + 1); 
	strcpy(B, Pointstring);

	int n = strlen(A)-1;
	if(A[n-1] == 13){
		n = n-1;
	}
		
	if(n < 1){
		return EXIT_FAILURE;	
	}
	if(n == 1){
		fprintf(stdout,"%lx\n",strtol(A,NULL,16)*strtol(B,NULL,16));
		return EXIT_SUCCESS;
	}
	
	if(n % 2 != 0){
		return EXIT_FAILURE;	
	}
	
	char Al[n/2];
	for(int i = 0; i < n/2; i++){
		 Al[i] = A[n/2+i];
	}
	Al[n/2] = '\0';
	
	char Ah[n/2];
	for(int i = 0; i < n/2; i++){
		Ah[i] = A[i];
	}
	Ah[n/2] = '\0';
	
	char Bl[n/2];
	for(int i = 0; i < n/2; i++){
		 Bl[i] = B[n/2+i];
	}
	Bl[n/2] = '\0';
	
	char Bh[n/2];
	for(int i = 0; i < n/2; i++){
		Bh[i] = B[i];
	}
	Bh[n/2] = '\0';	
	
	//Pipes and Forks
	
	//Write -> C_STDIN
	int fd1i[2];
	//C_STOUDT -> P_STDIN
	int fd1o[2];
	//Write -> C_STDIN
	int fd2i[2];
	//C_STOUDT -> P_STDIN
	int fd2o[2];
	//Write -> C_STDIN
	int fd3i[2];
	//C_STOUDT -> P_STDIN
	int fd3o[2];
	//Write -> C_STDIN
	int fd4i[2];
	//C_STOUDT -> P_STDIN
	int fd4o[2];
	
	if(pipe(fd1i)){ fprintf(stderr,"[%s] fd1i Piping failed", argv[0]); return EXIT_FAILURE;}
	if(pipe(fd1o)){ fprintf(stderr,"[%s] fd1o Piping failed", argv[0]); return EXIT_FAILURE;}
	if(pipe(fd2i)){ fprintf(stderr,"[%s] fd2i Piping failed", argv[0]); return EXIT_FAILURE;}
	if(pipe(fd2o)){ fprintf(stderr,"[%s] fd2o Piping failed", argv[0]); return EXIT_FAILURE;}
	if(pipe(fd3i)){ fprintf(stderr,"[%s] fd3i Piping failed", argv[0]); return EXIT_FAILURE;}
	if(pipe(fd3o)){ fprintf(stderr,"[%s] fd3o Piping failed", argv[0]); return EXIT_FAILURE;}
	if(pipe(fd4i)){ fprintf(stderr,"[%s] fd4i Piping failed", argv[0]); return EXIT_FAILURE;}
	if(pipe(fd4o)){ fprintf(stderr,"[%s] fd4o Piping failed", argv[0]); return EXIT_FAILURE;}
	int status = EXIT_SUCCESS;
	int status2 = EXIT_SUCCESS;	
	int status3 = EXIT_SUCCESS;	
	int status4 = EXIT_SUCCESS;	
		
	pid_t pid = fork();
	if(pid == -1){
		fprintf(stderr,"[%s] Problems with the fork on first child", argv[0]);
		return EXIT_FAILURE;
	}
	
	if(pid == 0){
		close(fd1o[0]);
		close(fd1i[1]);
	
		dup2(fd1i[0],STDIN_FILENO);
		dup2(fd1o[1],STDOUT_FILENO);
		
		close(fd1o[1]);
		close(fd1i[0]);
		
		execlp("./intmul", "intmul",(char *)NULL);
	
		fprintf(stderr,"[%s] Cannot exec", argv[0]);
		exit(EXIT_FAILURE);
	}
	pid_t pid2 = fork();
		
	if(pid2 == 0){
		close(fd2i[1]);
		close(fd2o[0]);
		
		dup2(fd2i[0],STDIN_FILENO);
		dup2(fd2o[1],STDOUT_FILENO);

		close(fd2i[0]);
		close(fd2o[1]);
		
		execlp("./intmul", "intmul",(char *)NULL);
	
		fprintf(stderr,"[%s] Cannot exec", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	pid_t pid3 = fork();
	
	if(pid3 == 0){
		close(fd3i[1]);
		close(fd3o[0]);
		
		dup2(fd3i[0],STDIN_FILENO);
		dup2(fd3o[1],STDOUT_FILENO);

		close(fd3i[0]);
		close(fd3o[1]);
		
		execlp("./intmul", "intmul",(char *)NULL);
	
		fprintf(stderr,"[%s] Cannot exec", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	pid_t pid4 = fork();
	
	if(pid4 == 0){
		close(fd4i[1]);
		close(fd4o[0]);
		
		dup2(fd4i[0],STDIN_FILENO);
		dup2(fd4o[1],STDOUT_FILENO);

		close(fd4i[0]);
		close(fd4o[1]);
		
		execlp("./intmul", "intmul",(char *)NULL);
	
		fprintf(stderr,"[%s] Cannot exec", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	close(fd1i[0]);
	close(fd2i[0]);
	close(fd3i[0]);
	close(fd4i[0]);
	
	close(fd1o[1]);
	close(fd2o[1]);
	close(fd3o[1]);
	close(fd4o[1]);
	
	//Write to this shit
	dprintf(fd1i[1],"%s\n%s\n",Ah,Bh);
	dprintf(fd2i[1],"%s\n%s\n",Ah,Bl);
	dprintf(fd3i[1],"%s\n%s\n",Al,Bh);
	dprintf(fd4i[1],"%s\n%s\n",Al,Bl);
	
	close(fd1i[1]);
	close(fd2i[1]);
	close(fd3i[1]);
	close(fd4i[1]);
		
	
	clearerr(stdin);
	dup2(fd1o[0],STDIN_FILENO);
	close(fd1o[0]);
	
	//Wait for first children
	while(waitpid(pid, &status, 0) != pid){
		if(pid != -1) continue;
					
		fprintf(stderr, "[%s] First Child is not returning! \n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	getline(&Pointstring, &len, stdin);
	char * R1 = malloc(strlen(Pointstring) + 1); 
	strcpy(R1, Pointstring);
	
	clearerr(stdin);
	dup2(fd2o[0],STDIN_FILENO);
	close(fd2o[0]);
	
	//Wait for second children
	while(waitpid(pid2, &status2, 0) != pid2){
		if(pid2 != -1) continue;
		
		fprintf(stderr, "[%s] Second Child is not returning! \n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	getline(&Pointstring, &len, stdin);
	char * R2 = malloc(strlen(Pointstring) + 1); 
	strcpy(R2, Pointstring);
	
	clearerr(stdin);
	dup2(fd3o[0],STDIN_FILENO);
	close(fd3o[0]);
	
	//Wait for third children
	while(waitpid(pid3, &status3, 0) != pid3){
		if(pid2 != -1) continue;
		
		fprintf(stderr, "[%s] Third Child is not returning! \n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	getline(&Pointstring, &len, stdin);
	char * R3 = malloc(strlen(Pointstring) + 1); 
	strcpy(R3, Pointstring);
	
	clearerr(stdin);
	dup2(fd4o[0],STDIN_FILENO);
	close(fd4o[0]);
	
	//Wait for fourth children
	while(waitpid(pid4, &status4, 0) != pid4){
		if(pid2 != -1) continue;
		
		fprintf(stderr, "[%s] Fourth Child is not returning! \n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	getline(&Pointstring, &len, stdin);
	char * R4 = malloc(strlen(Pointstring) + 1); 
	strcpy(R4, Pointstring);

	if(WEXITSTATUS(status2) != EXIT_SUCCESS && WEXITSTATUS(status2) != EXIT_SUCCESS && WEXITSTATUS(status3) != EXIT_SUCCESS && WEXITSTATUS(status2) != EXIT_SUCCESS){
		fprintf(stderr, "[%s] One or more Child Process failed to work properly! \n", argv[0]);
		return EXIT_FAILURE;
	}
	
	int size= n+n-1;
	char result[size];
	int carry = 0;
	for(int i = 0; i<= size; i++){
		if(i< n/2){
			result[size-i] = R4[strlen(R4)-2-i];
			continue;
		}
		int sum = carry;
		if(i >= n/2){
			if(i <strlen(R4)){
				sum = sum + strtol((char[]){R4[strlen(R4)-2-i], 0}, NULL, 16);
			}
			if(i-n/2 <strlen(R2)){
				sum = sum + strtol((char[]){R2[strlen(R2)-2-i+n/2], 0},NULL,16);
			}
			if(i-n/2 <strlen(R3)){
				sum = sum + strtol((char[]){R3[strlen(R3)-2-i+n/2], 0},NULL,16);
			}
		}
		if(i >= n){
			if(i-n < strlen(R1)){
				sum = sum + strtol((char[]){R1[strlen(R1)-2-i+n], 0},NULL,16);
			}
		}
		carry = sum /16;
		char str[2];
		sprintf(str, "%x", sum%16);
		result[size-i] = str[0];
	}
	
	result[size+1] = '\0';
	fprintf(stdout,"%s\n",result);
	
	return EXIT_SUCCESS;
}


