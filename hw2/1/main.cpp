#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>

#include "shell.h"
using namespace std;

int port = 7000;
int init_socket();
int char_to_int(char *);
int main(int argc, char *argv[]){
    if(argc == 2){
        port = char_to_int(argv[1]);
        cout << port << endl;
        cout.flush();
    }
    int sockfd;
    int new_sockfd;
    socklen_t client_len;
    int child_pid;
    struct sockaddr_in client_add;
    sockfd = init_socket();
    
    client_len = sizeof(client_add);
    while(1){

    new_sockfd = accept(sockfd, (struct sockaddr*) &client_add, &client_len);
        pid_t pid = fork();
        if(pid == 0){
            dup2(new_sockfd, 0);
            dup2(new_sockfd, 1);
            dup2(new_sockfd, 2);
            shell();
            close(new_sockfd);
            exit(0);
        }else{
            waitpid(pid, NULL, 0);
            close(new_sockfd);
        } 
    }    
    
}

int char_to_int(char *input){
    int re = 0;
    for(int i = 0; i < strlen(input); i++){
        re = re * 10 + input[i] - '0';
    }
    return re;
}
int init_socket(){
    int fd = 0;
    struct sockaddr_in server_add;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_add, sizeof(server_add));

    server_add.sin_family = AF_INET;
    server_add.sin_addr.s_addr = INADDR_ANY;
    server_add.sin_port = htons(port);//port 7000

    int optval = 1;    
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1){
        cout << "setsockopt err" << endl;
        return 0;
    }

    if(bind(fd, (struct sockaddr*)&server_add, sizeof(server_add))){
        cout << "bind err" << endl;
        return 0;
    }

    listen(fd, 1);

    return fd;
}
