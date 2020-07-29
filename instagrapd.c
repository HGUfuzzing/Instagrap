// Instagrap.c
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
#include <time.h>
#include "socket_file_io.h"

int main (int argc, char *argv[]) {
    FILE *fp = NULL;
    int serv_sock, clnt_sock;
    int serv_port = atoi(argv[2]);
    char userID[BUF_SIZE];
    char buf[BUF_SIZE];
    
    if (argc != 6) {
        printf("Usage: %s -p <port number> -w <IP>:<port> <testcase_path>");
    }

    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    serv_sock = socket (PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
        error_handling("socket() error!");

    memset (&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    serv_addr.sin_port = htons (serv_port);

    //완료
    if(bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        // error_handling("bind() error!");
        perror("[-]Error in bind");
    }
    if(listen (serv_sock, 100) == -1)
        error_handling("listen() error!");

    while(1) {
        printf("Connect new client\n");
        clnt_sock = -1;
        memset (&clnt_addr, 0, sizeof(clnt_addr));
        clnt_sock = accept (serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
        // <-- 여기 위치 쯤 클라이언트가 이미 전달된 유저인지 판별해야 한다.
        
        if(clnt_sock == -1){
            perror("accept() error!");
        }

        //Submitter의 정보 받기
        recv(clnt_sock, buf, 9, 0);
        strncpy(userID, buf, 9);
        printf("UserID: %s\n", userID);

        //FILE 받기
        recv_file(clnt_sock, "submitter_file.c");
        send(clnt_sock, buf, 1, 0);
        int times = 1;
        while(1){
            recv(clnt_sock, buf, 5, 0);
            printf("received ping %d\n", times);
            if(times++ >= 6) {
                printf("send 0x33 to client\n");
                buf[0] = 0x33; 
                send(clnt_sock, buf, 5, 0);
                break;
            }
            buf[0] = 0x00;
            send(clnt_sock, buf, 5, 0);
            printf("send ping %d\n", times - 1);
            sleep(1);
        }

        close(clnt_sock);
    }
    close(serv_sock);
}
