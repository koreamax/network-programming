//--------------------------------------------------------------
// 파일명  : tcp_echocli.c
// 기능    : 에코 서비스를 요청하는 TCP(연결형) 클라이언트
// 컴파일  : gcc -o tcp_echocli tcp_echocli.c
// 사용법  : ./tcp_echocli 203.252.65.3
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAXLINE 127

int main(int argc, char *argv[]) {
    struct sockaddr_in servaddr;
    int s, nbyte;
    char buf[MAXLINE + 1];

    if (argc != 2) {
        printf("usage: %s ip_address\n", argv[0]);
        exit(1);
    }

    // 소켓 생성
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fail");
        exit(1);
    }

    // 서버 주소 구조체 설정
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(7);  // echo 서비스 포트

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        perror("inet_pton fail");
        exit(1);
    }

    // 연결 요청
    if (connect(s, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect fail");
        exit(1);
    }

    printf("입력: ");
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        exit(0);
    }

    nbyte = strlen(buf);

    // 서버로 데이터 전송
    if (write(s, buf, nbyte) < 0) {
        perror("write error");
        exit(1);
    }

    printf("수신: ");
    // 서버로부터 echo 데이터 수신
    if ((nbyte = read(s, buf, MAXLINE)) < 0) {
        perror("read fail");
        exit(1);
    }

    buf[nbyte] = '\0';
    printf("%s", buf);

    close(s);
    return 0;
}