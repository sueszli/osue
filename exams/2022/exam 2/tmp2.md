```c
static int setup_socket(const char* port) {
    struct addrinfo hints;
    struct addrinfo *result;
    int sfd, s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    s = getaddrinfo(NULL, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo\n");
        exit(EXIT_FAILURE);
    }
    
    if ((sfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol))== -1){
        freeaddrinfo(result);
        fprintf(stderr, "socket\n");
        exit(EXIT_FAILURE);
    }

    if (bind(sfd, result->ai_addr, result->ai_addrlen) == -1){
        freeaddrinfo(result);
        fprintf(stderr, "bind\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);

    if (listen(sfd, 1) == -1){
        freeaddrinfo(result);
        fprintf(stderr, "listen\n");
        exit(EXIT_FAILURE);
    }
    
    return sfd;
    
}
```