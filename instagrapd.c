#include <stdio.h>
#include <stdlib.h> //for atoi()
#include <string.h> // for memset() ��
#include <unistd.h> //sockadd_in, read, write ��
#include <arpa/inet.h> //htonol, htons, INDDR_ANY, sockaddr_in ��
#include <sys/socket.h>
#include "socket_file_io.h"

int main(int argc, char * argv[]) {
    
    int sock_to_worker;
    struct sockaddr_in serv_addr;
    char buf[BUF_SIZE];

    if(argc != 5) {
        printf("Usage : %s <ip address> <port> <filename> <testcase directory>\n", argv[0]);
        exit(1);
    }
    
    //TCP�����������̰� ipv4 �������� ���� ������ ����
    sock_to_worker = socket(PF_INET, SOCK_STREAM, 0);
    if(sock_to_worker == -1) 
        error_handling("socket error");
    
    //���ڷ� ���� �����ּ� ������ ����
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;  //�����ּ�ü��� ipv4.
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); //�����ּ�(ip)����
    serv_addr.sin_port = htons(atoi(argv[2]));  //���� ��Ʈ��ȣ ����

    //Ŭ���̾�Ʈ ���Ϻκп� ������ ����
    if(connect(sock_to_worker, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect error");


    FILE * fp;

    //.c ���� ����
    send_file(sock_to_worker, argv[3]);

    //������ ���� ���� üũ
    recv(sock_to_worker, buf, 1, 0);
    if(buf[0]) {
        error_handling("compile error");
    }
    else 
        printf("compile success!\n");
    //testcase �ϳ� send, �ϳ� recv         (5���� ����)
    for(int i = 1; i <= 5; i++) {
        snprintf(buf, sizeof(buf), "%s/%d.in", argv[4], i);
        
        send_file(sock_to_worker
, buf);

        //i.out ��� �ް�
        snprintf(buf, sizeof(buf), "%s/%d.recv.out", argv[4], i);
        recv_file(sock_to_worker
, buf);

        //���� �´��� diff ��, �� TC�� ���� ��� ���
        snprintf(buf, sizeof(buf), "diff %s/%d.recv.out %s/%d.out", argv[4], i, argv[4], i);
        if((fp = popen(buf, "r")) == NULL)
            error_handling("popen() error");
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf), fp);
        if(!strlen(buf)) {
            printf("@@@ TC[%d] CORRECT!\n", i);
        }
        else {
            printf("@@@ TC[%d] WRONG.\n", i);
        }
        pclose(fp);
    }

    //������ ���� ���� (*.recv.out)
    snprintf(buf, sizeof(buf), "rm -rf %s/*.recv.out", argv[4]);
    fp = popen(buf, "r");
    pclose(fp);

    //���ϵ� �ݱ�
    close(sock_to_worker);
    puts("FINISHED");
    return 0;
}


