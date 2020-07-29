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
    int n=0, bytes;
    int serv_port;
    char * serv_ip = NULL;
    char buf[BUF_SIZE];

    struct sockaddr_in serv_addr;

    if(argc != 6){
        printf("%s -n <server ip>:<port> -u <userID> <filename>\n", argv[0]);
    }
    else if (!strcmp(argv[1],"-n")) {

    }
    printf("size of stunum %d\n", strlen(argv[4]));
    serv_ip = strtok(argv[2],":");
    serv_port = atoi(strtok(NULL, ":"));
    printf("Server: %s\nPort: %d\n\n", serv_ip, serv_port);
    
    clnt_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(serv_ip);
    serv_addr.sin_port = htons(serv_port); 

    //Connect1: Sending User ID & a file 
    if( connect(clnt_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error!");
    else
        puts("Connected..............");
    

    send (clnt_sock, argv[4], 9, 0); /* Send user's ID */

    send_file(clnt_sock, argv[5]);
    sleep(1);
    printf("check\n");

    //OR send request message / 2sec - 그 후 서버가 체크
    buf[0] = 0xff;
    int times = 1;
    recv(clnt_sock, buf, 1, 0);
    while(1){
        send(clnt_sock,buf,5,0);
        printf("send ping %d\n", times);
        recv(clnt_sock, buf, 5, 0);
        printf("recv ping %d\n", times++);
        if(buf[0] == 0x33) {
            printf("Success\n");
            break;
        }
        sleep(1);
    }
    printf("exit..\n");
    return 0;
}