#include <stdio.h>
#include <stdlib.h> //for atoi()
#include <string.h> // for memset() ��
#include <unistd.h> //sockadd_in, read, write ��
#include <arpa/inet.h> //htonol, htons, INDDR_ANY, sockaddr_in ��
#include <sys/socket.h>
#define BUF_SIZE 1024

void error_handling(char * message);
int send_file(int socket, const char * filename);
int recv_file(int socket, char * filename);

int main(int argc, char * argv[]) {
    int serv_sock;
    int clnt_sock;

    //sockaddr_in�� ���� �ּ��� Ʋ�� �������ִ� ����ü�� AF_INET�� ��� ���
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;   //accept�Լ����� ����.
    socklen_t clnt_addr_size;

    if(argc != 4) {
        printf("Usage : %s <port> <filename> <testcase directory>\n", argv[0]);
        exit(1);
    }

    
    //TCPd�����������̰� ipv4 �������� ���� ������ ����
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
        error_handling("socket error");
    
    //�ּҸ� �ʱ�ȭ�� �� IP�ּҿ� ��Ʈ ����
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;                   //Ÿ��: ipv4
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);    //ip�ּ�
    serv_addr.sin_port=htons(atoi(argv[1]));         //port

    //���ϰ� ���� �ּҸ� ���ε�
    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
        error_handling("bind error");

    //���� ��⿭ 50�� ����
    if(listen(serv_sock, 50)==-1)
        error_handling("listen error");
    printf("listening..\n");    

    //Ŭ���̾�Ʈ�κ��� ��û�� ���� ���� ����
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);   //���ŷ!  
    if(clnt_sock==-1)
        error_handling("accept error");
    printf("Connection Request from Client [IP:%s, Port:%d] has been Accepted\n",
            inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
    
    
    char buf[BUF_SIZE];

    //.c ���� ����
    send_file(clnt_sock, argv[2]);

    //������ ���� ���� üũ
    recv(clnt_sock, buf, 1, 0);
    if(buf[0]) {
        error_handling("compile error");
    }
    else 
        printf("compile success!\n");
    //testcase �ϳ� send, �ϳ� recv         (5���� ����)
    for(int i = 1; i <= 5; i++) {
        FILE * fp;

        snprintf(buf, sizeof(buf), "%s/%d.in", argv[3], i);
        
        send_file(clnt_sock, buf);

        //i.out ��� �ް�
        snprintf(buf, sizeof(buf), "%s/%d.recv.out", argv[3], i);
        recv_file(clnt_sock, buf);

        //���� �´��� diff ��, �� TC�� ���� ��� ���
        snprintf(buf, sizeof(buf), "diff %s/%d.recv.out %s/%d.out", argv[3], i, argv[3], i);
        if((fp = popen(buf, "r")) == NULL)
            error_handling("popen() error");
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf), fp);
        if(!strlen(buf)) {
            printf("@@@ TC[%d] CORRECT!\n", i);
        }
        else {
            printf("@@@ TC[%d] WRONG. %s\n", i, buf);
        }
        pclose(fp);
    }

    //���ϵ� �ݱ�
    close(clnt_sock);
    close(serv_sock);
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
