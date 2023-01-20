```c
//server socket

int beispiel1(...){

    int sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

    if(sockfd <0){
        //error
    }

    if(bind(sockfd, ai->ai_addr, ai->ai_addrlen) <0){
        //erorr
    }

    if(listen(sockfd, 1)<0){
        //error
    }

    return sockfd;
}

void beispiel2(...){

    socklen_t addr_len = sizeof(struct sockaddr_in);

    while(true){
        int connfd = accept(sockfd, (struct sockaddr ) &ai, (socklen_t) &addr_len);

        FILE conn = fdopen(connfd, "r+");

        char buf[MAX_LEN];

        fgets(buf, MAX_LEN, conn);

        FILEfile = returnstuffFunction(buf);

        char sendbuf[MAX_LEN];

        while(fgets(sendbuf, MAX_LEN, file) != NULL)){
            fputs(sendbuf, MAX_LEN, connfd);
        }

        fflush(connfd);
        close(connfd);
        fclose(conn);
        fclose(file);
    }
}


FILE* beispiel3(...){

    int pipe_array[2];

    if(pipe(pipe_array[2]) ==-1){
        //error
    }

    switch(fork()){
        case -1:
            //error
        case 0:
            //child process
            close(pipe_array[0]);
            close(pipe_array[1]);
            dup2(pipe_array[0], STDIN_FILENO);
            dup2(pipe_array[1], STDOUT_FILENO);
            execlp(program, program, NULL);
        default:
            close(pipe_array[0]);
            close(pipe_array[1]);
            FILE *child_r = fdopen(pipe_array[0], "r");
    }

    return(child_r);
}
```