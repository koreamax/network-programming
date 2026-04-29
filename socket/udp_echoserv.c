//--------------------------------------------------------------
// 파일명  : udp_echoserv.c
// 기능    : 에코 서비스를 수행하는 UDP 서버
// 컴파일  : gcc -o udp_echoserv udp_echoserv.c
// 사용법  : ./udp_echoserv 9999
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXLINE 511

int main(int argc, char *argv[]) {
    struct sockaddr_in servaddr, cliaddr;
    int s, nbyte;
    socklen_t addrlen;
    char buf[MAXLINE + 1];

    if (argc != 2) {
        printf("usage: %s port\n", argv[0]);
        exit(1);
    }

    // 소켓 생성
    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket fail");
        exit(1);
    }

    // 주소 구조체 초기화
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // 서버 주소 설정
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    // bind() 호출
    if (bind(s, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind fail");
        exit(1);
    }

    // iterative echo 서비스 수행
    while (1) {
        puts("Server waiting request.");

        addrlen = sizeof(cliaddr);

        nbyte = recvfrom(s, buf, MAXLINE, 0,
                         (struct sockaddr *)&cliaddr, &addrlen);

        if (nbyte < 0) {
            perror("recvfrom fail");
            exit(1);
        }

        buf[nbyte] = '\0';
        printf("%d byte recv: %s\n", nbyte, buf);

        if (sendto(s, buf, nbyte, 0,
                   (struct sockaddr *)&cliaddr, addrlen) < 0) {
            perror("sendto fail");
            exit(1);
        }

        puts("sendto complete");
    }

    close(s);
    return 0;
}