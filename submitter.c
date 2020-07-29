//submitter.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "socket_file_io.h"

int main (int argc, char *argv[]) {

    int clnt_sock;
    
    int serv_port;
    char * serv_ip = NULL;

    char buf[BUF_SIZE];

    struct sockaddr_in serv_addr;

    if(argc != 4){
        printf("%s <server ip>:<port> <userID> <filename>\n", argv[0]);
        exit(1);
    }
    
    if(strlen(argv[2]) != 8) 
        error_handling("userID must be 8 digits.");

    serv_ip = strtok(argv[1],":");
    serv_port = atoi(strtok(NULL, ":"));
    printf("Server: %s\nPort: %d\n\n", serv_ip, serv_port);
    
    clnt_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(serv_ip);
    serv_addr.sin_port = htons(serv_port); 

    
    if( connect(clnt_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error!");
    
    printf("Request Connection to server! (fd : %d)\n", clnt_sock);
    
    int result;
    fd_set fd_status;
    struct timeval timeout;

    

    //waiting
    while(1) {
        FD_ZERO(&fd_status);
        FD_SET(clnt_sock, &fd_status);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        result = select(clnt_sock + 1, &fd_status, 0, 0, &timeout);
        if(result == -1) {
            error_handling("unintended disconnect!");
        }
        else if(result == 0) {
            printf("wating for responds..\n");
        }
        else {
            //Sending User ID & a file
            recv(clnt_sock, buf, 1, 0);
            break;
        }
    }

    
    

    
    send(clnt_sock, argv[2], 9, 0); /* Send user's ID */
    //바로 send 두 번 되나??
    send_file(clnt_sock, argv[3]);

    buf[0] = 0x0;

    //compile error 여부 받기
    recv(clnt_sock, buf, sizeof(buf), 0);
    if(buf[0]) {
        printf("Compile error!\n");
        return 0;
    }

    for(int i = 1; i <= 5; i++) {
        recv(clnt_sock, buf, sizeof(buf), 0);
        printf("%s\n", buf);
    }

    return 0;
}