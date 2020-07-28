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

#define BUF_SIZE 1024 // 100?

void error_handling(char * message);
int send_file(int socket, const char * filename);
int recv_file(int socket, const char * filename);

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
    while(1){
        send(clnt_sock,buf,5,0);
        printf("send %d\n", times);
        recv(clnt_sock, buf, 5, 0);
        printf("recv %d\n", times++);
        if(buf[0] == 0x33) {
            printf("Success\n");
            break;
        }

        printf("Waiting for results..\n");
        sleep(1);
    }
    printf("exit..\n");
    return 0;
}

void error_handling(char * message){
    fputs(message, stderr);
    fputc('\n',stderr);
    exit(1);
}

int send_file(int socket, const char * filename) {
    /*
        send할 파일의 size를 먼저 보냄.
    */
    FILE * fp;
    int file_size = 0;
    int buffer_num = 0;
    int total_send_bytes = 0;
    int send_bytes;
    int total_buffer_num;
    
    char buf[BUF_SIZE];
    if((fp = fopen(filename, "rb")) == NULL) 
        error_handling("File not Exist");

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    total_buffer_num = (file_size - 1) / sizeof(buf) + 1;
    fseek(fp, 0, SEEK_SET);
    snprintf(buf, sizeof(buf), "%d", file_size);    

    send_bytes = send(socket, buf, sizeof(buf), 0);  //input file 사이즈 보내기

    buffer_num = 0;
    total_send_bytes = 0;

    while((send_bytes = fread(buf, sizeof(char), sizeof(buf), fp)) > 0) {
        send(socket, buf, send_bytes, 0);
        buffer_num++;
        total_send_bytes += send_bytes;
        printf("%s progress: %d/%dByte(s) [%d%%] (sending)\n", filename, total_send_bytes, file_size, buffer_num * 100 / total_buffer_num);
    }
    fclose(fp);
    return 0;
}


int recv_file(int socket, const char * filename) {
    /*
        receive할 파일의 size를 먼저 보냄.
    */
    FILE * fp;
    fp = fopen(filename, "wb");

    char buf[1024];
    int read_bytes;
    int file_size;
    int total_buf_num;
    int buf_num = 0;
    int total_read_bytes = 0;
    read_bytes = recv(socket, buf, sizeof(buf), 0);
    file_size = atoi(buf);
    total_buf_num = (file_size - 1) / sizeof(buf) + 1;

    while(buf_num != total_buf_num) {
        read_bytes = recv(socket, buf, sizeof(buf), 0);
        fwrite(buf, sizeof(char), read_bytes, fp);
        buf_num++;
        total_read_bytes += read_bytes;
        printf("%s progress: %d/%dByte(s) [%d%%] (receiving)\n", filename , total_read_bytes, file_size, buf_num * 100 / total_buf_num);
    }
    fclose(fp);
    return 0;
}