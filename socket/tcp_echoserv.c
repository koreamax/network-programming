//--------------------------------------------------------------
// 파일명  : tcp_echoserv.c
// 기능    : 에코 서비스를 수행하는 TCP 서버
// 컴파일  : gcc -o tcp_echoserv tcp_echoserv.c
// 사용법  : ./tcp_echoserv 2049
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAXLINE 127

int main(int argc, char *argv[]) {
    struct sockaddr_in servaddr, cliaddr;
    int listen_sock, accp_sock;
    socklen_t addrlen;
    int nbyte;
    char buf[MAXLINE + 1];

    if (argc != 2) {
        printf("usage : %s port\n", argv[0]);
        exit(1);
    }

    // 소켓 생성
    if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fail");
        exit(1);
    }

    // 서버 주소 구조체 초기화
    memset(&servaddr, 0, sizeof(servaddr));

    // 서버 주소 설정
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    // bind() 호출
    if (bind(listen_sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind fail");
        exit(1);
    }

    // 소켓을 수동 대기 모드로 설정
    if (listen(listen_sock, 5) < 0) {
        perror("listen fail");
        exit(1);
    }

    // iterative echo 서비스 수행
    while (1) {
        puts("서버가 연결 요청을 기다림...");

        addrlen = sizeof(cliaddr);

        // 연결 요청 대기
        accp_sock = accept(listen_sock, (struct sockaddr *)&cliaddr, &addrlen);
        if (accp_sock < 0) {
            perror("accept fail");
            continue;
        }

        puts("클라이언트가 연결됨...");

        nbyte = read(accp_sock, buf, MAXLINE);
        if (nbyte < 0) {
            perror("read fail");
            close(accp_sock);
            continue;
        }

        // 받은 데이터 그대로 다시 전송
        write(accp_sock, buf, nbyte);

        close(accp_sock);
    }

    close(listen_sock);
    return 0;
}