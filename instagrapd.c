#include <stdio.h>
#include <stdlib.h> //for atoi()
#include <string.h> // for memset() 등
#include <unistd.h> //sockadd_in, read, write 등
#include <arpa/inet.h> //htonol, htons, INDDR_ANY, sockaddr_in 등
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
    
    //TCP연결지향형이고 ipv4 도메인을 위한 소켓을 생성
    sock_to_worker = socket(PF_INET, SOCK_STREAM, 0);
    if(sock_to_worker == -1) 
        error_handling("socket error");
    
    //인자로 받은 서버주소 정보를 저장
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;  //서버주소체계는 ipv4.
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); //서버주소(ip)저장
    serv_addr.sin_port = htons(atoi(argv[2]));  //서버 포트번호 저장

    //클라이언트 소켓부분에 서버를 연결
    if(connect(sock_to_worker, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect error");


    FILE * fp;

    //.c 파일 전송
    send_file(sock_to_worker, argv[3]);

    //컴파일 에러 여부 체크
    recv(sock_to_worker, buf, 1, 0);
    if(buf[0]) {
        error_handling("compile error");
    }
    else 
        printf("compile success!\n");
    //testcase 하나 send, 하나 recv         (5개로 고정)
    for(int i = 1; i <= 5; i++) {
        snprintf(buf, sizeof(buf), "%s/%d.in", argv[4], i);
        
        send_file(sock_to_worker
, buf);

        //i.out 결과 받고
        snprintf(buf, sizeof(buf), "%s/%d.recv.out", argv[4], i);
        recv_file(sock_to_worker
, buf);

        //정답 맞는지 diff 후, 이 TC에 대한 결과 출력
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

    //생성된 파일 삭제 (*.recv.out)
    snprintf(buf, sizeof(buf), "rm -rf %s/*.recv.out", argv[4]);
    fp = popen(buf, "r");
    pclose(fp);

    //소켓들 닫기
    close(sock_to_worker);
    puts("FINISHED");
    return 0;
}


