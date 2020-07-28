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

#define BUFFUNIT 1024 // 100?

int main (int argc, char *argv[]) {

    int clnt_sock;
    int n=0, bytes;
    int serv_port;
    char user_info [BUFFUNIT];
    char * serv_ip = NULL;
    char sendbuffer[BUFFUNIT];

    struct sockaddr_in serv_addr;

    if(argc != 6){
        printf("%s -n <server ip>:<port> -u <userID> <filename>\n", argv[0]);
    }
    else if (!strcmp(argv[1],"-n")) {

    }
    strcpy(user_info, argv[4]);

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
    

    write (clnt_sock, user_info, sizeof(user_info)); /* Send user's ID */

    FILE * fp = fopen(argv[5], "rb"); /* Send user's file */
    if(fp == NULL)
        error_handling("fopen() error!");

    while( (bytes = fread(sendbuffer, 1, sizeof(sendbuffer), fp)) >0 ){
        printf("Send %d bytes\n", bytes);
        if( send(clnt_sock, sendbuffer, bytes, 0) == -1 )
            error_handling("send() error!");
    }


    //OR send request message / 2sec - 그 후 서버가 체크
    close(clnt_sock);

    //Connect2?????: requesting for results / 3sec
    printf("Another Connection request for results\n");
    clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
    while(1){
        if(connect(clnt_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
            printf("Connection failed.....................Wait for a time");
        else{
            printf("Connected! You'll get the result\n");
            /* *
             * Reques
             * 여기다가 Server로 부터 결과 받는
             *
             * */
        }
        sleep(3);
    }
    

    fclose(fp);

    return 0;
}

void error_handling(char * message){
    fputs(message, stderr);
    fputc('\n',stderr);
    exit(1);
}