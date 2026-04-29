//--------------------------------------------------------------
// 파일명  : mydaytime.c
// 기능    : daytime 서비스를 요청하는 TCP(연결형) 클라이언트
// 컴파일  : gcc -o mydaytime mydaytime.c
// 사용법  : ./mydaytime 203.252.65.3
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 127

int main(int argc, char *argv[]) {
    int s, nbyte;
    struct sockaddr_in servaddr;
    char buf[MAXLINE + 1];

    if (argc != 2) {
        printf("Usage: %s ip_address\n", argv[0]);
        exit(1);
    }

    // 소켓 생성 (TCP)
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fail");
        exit(1);
    }

    // servaddr 초기화
    memset(&servaddr, 0, sizeof(servaddr));

    // 서버 주소 설정
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(13);  // daytime 서비스 포트 (13)

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        perror("inet_pton fail");
        exit(1);
    }

    // 서버 연결 요청
    if (connect(s, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect fail");
        exit(1);
    }

    // 서버로부터 데이터 수신
    if ((nbyte = read(s, buf, MAXLINE)) < 0) {
        perror("read fail");
        exit(1);
    }

    buf[nbyte] = '\0';
    printf("%s", buf);

    close(s);
    return 0;
}