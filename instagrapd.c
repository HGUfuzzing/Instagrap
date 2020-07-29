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

#define BUF_SIZE 1024

void error_handling(char * message);
int send_file(int socket, const char * filename);
int recv_file(int socket, const char * filename);

int main (int argc, char *argv[]) {
    FILE *fp = NULL;
    int i, bytes, tot_bytes, str_len;
    int serv_sock, clnt_sock;
    // int worker_port = argv[4]; - ip:port 분리해야함
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
        //checkuser();
        printf("Clients' socket: %d\n", clnt_sock);

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
            printf("Waiting?\n");
            recv(clnt_sock, buf, 5, 0);
            if(times++ >= 6) {
                printf("send 0x33 to client\n");
                buf[0] = 0x33; 
                send(clnt_sock, buf, 5, 0);
                break;
            }
            buf[0] = 0x00;
            send(clnt_sock, buf, 5, 0);
            sleep(1);
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