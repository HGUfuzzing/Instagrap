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

#define BUFFUNIT 1024

int main (int argc, char *argv[]) {
    FILE *fp = NULL;
    int i, bytes, tot_bytes, str_len;
    int serv_sock, clnt_sock;
    // int worker_port = argv[4]; - ip:port 분리해야함
    int serv_port = atoi(argv[2]);
    char userID[BUFFUNIT];
    char recvbuff[BUFFUNIT];

    if (argc != 6) {
        printf("Usage: %s -p <port number> -w <IP>:<port> <testcase_path>");
    }

    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;

    serv_sock = socket (PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
        error_handling("socket() error!");

    memset (&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    serv_addr.sin_port = htons (serv_port);

    //완료
    if(bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error!");
    if(listen (serv_sock, 100) == -1)
        error_handling("listen() error!");

    for(i=0; i<100; i++) {
        clnt_sock = -1;
        memset (&clnt_addr, 0, sizeof(clnt_addr));
        clnt_sock = accept (serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
        // <-- 여기 위치 쯤 클라이언트가 이미 전달된 유저인지 판별해야 한다.
        //checkuser();
        printf("Clients' socket: %d\n", clnt_sock);

        if(clnt_sock == -1){
            perror("accept() error!");
            // error_handling("accept() error!"); 
            //--> 수정해야함..? 연결중 다른 유저가 request 시에 처리법
        }
        else{
            //Submitter의 정보 받기
            read(clnt_sock, recvbuff, sizeof(recvbuff));
            strncpy(userID, recvbuff, BUFFUNIT);
            printf("UserID: %s\n", userID);

            //FILE 받기
            tot_bytes = 0;
            fp = fopen("submitter_file.c","wb");
            if(fp == NULL)
                error_handling("fopen() error! You should check your file name");
            
            while((str_len = read(clnt_sock, recvbuff, BUFFUNIT)) != 0){
                printf("recbuff: \n\n%s\n", recvbuff);
                fprintf(fp, recvbuff);
            }
            fclose(fp);
        }
        close(clnt_sock);
    }
    close(serv_sock);
}

void error_handling(char * message){
    fputs(message, stderr);
    fputc('\n',stderr);
    exit(1);
}