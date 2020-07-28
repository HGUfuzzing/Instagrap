#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 1024

void error_handling(char* message);
int send_file(int socket, const char * filename);
int recv_file(int socket, char * filename);


int main(int argc, char * argv[]) {
    int clnt_sock;
    struct sockaddr_in serv_addr;
    char message[1024] = {0,};

    int status;
    char path[1000];

    if(argc != 4) {
        printf("Usage : %s <ip address> <port> <worker space>\n", argv[0]);
        exit(1);
    }

    //TCP�����������̰� ipv4 �������� ���� ������ ����
    clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(clnt_sock == -1) 
        error_handling("socket error");
    
    //���ڷ� ���� �����ּ� ������ ����
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;  //�����ּ�ü��� ipv4.
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); //�����ּ�(ip)����
    serv_addr.sin_port = htons(atoi(argv[2]));  //���� ��Ʈ��ȣ ����

    //Ŭ���̾�Ʈ ���Ϻκп� ������ ����
    if(connect(clnt_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect error");

    char buf[BUF_SIZE];

    snprintf(buf, sizeof(buf), "%s/new.c", argv[3]);

    //������ ���������� �Ǿ����� ������ �ޱ�
    recv_file(clnt_sock, buf);

    
    FILE * fp;

    //���� (.c)���� ������ �� �����ϱ�
    snprintf(buf, sizeof(buf), "gcc -w %s/new.c 2> /dev/stdout", argv[3]);
    
    if((fp = popen(buf, "r")) == NULL)
        error_handling("popen() error (1)");
    memset(buf, 0, sizeof(buf));
    fgets(buf, sizeof(buf), fp);
    
    send(clnt_sock, buf, 1, 0);    //compile error ���� �˷��ֱ�

    if(strlen(buf) != 0) {
        buf[0] = 0x0;
        error_handling("compile error");
    }
    pclose(fp);
    
    while(fgets(buf, sizeof(buf), fp))
        printf("%s", buf);    

    for(int i = 1; i <= 5; i++) {
        //input file �ް�
        snprintf(buf, sizeof(buf), "%s/%d.in", argv[3], i);
        recv_file(clnt_sock, buf);

        //��������
        snprintf(buf, sizeof(buf), "./a.out < %s/%d.in > %s/%d.out", argv[3], i, argv[3], i);
        if((fp = popen(buf, "r")) == NULL) 
            error_handling("popen() error (2)");

        //output file ����.
        snprintf(buf, sizeof(buf), "./%s/%d.out", argv[3], i);
        send_file(clnt_sock, buf);
        pclose(fp);
    }


    //���� �ݱ�
    close(clnt_sock);
    puts("FINISHED");
    return 0;
}



void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

int send_file(int socket, const char * filename) {
    /*
        send�� ������ size�� ���� ����.
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

    send_bytes = send(socket, buf, sizeof(buf), 0);  //input file ������ ������

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


int recv_file(int socket, char * filename) {
    /*
        receive�� ������ size�� ���� ����.
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
