//--------------------------------------------------------------
// 파일명  : udp_echocli.c
// 기능    : 에코 서비스를 요청하는 UDP(비연결형) 클라이언트
// 컴파일  : gcc -o udp_echocli udp_echocli.c
// 사용법  : ./udp_echocli 203.252.65.3 9999
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLINE 511

int main(int argc, char *argv[]) {
    struct sockaddr_in servaddr;
    int s, nbyte;
    socklen_t addrlen;
    char buf[MAXLINE + 1];

    if (argc != 3) {
        printf("usage: %s ip_address port_number\n", argv[0]);
        exit(1);
    }

    // 소켓 생성 (UDP)
    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket fail");
        exit(1);
    }

    // 서버 주소 설정
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        perror("inet_pton fail");
        exit(1);
    }

    printf("입력: ");
    if (fgets(buf, MAXLINE, stdin) == NULL) {
        printf("fgets 실패\n");
        exit(1);
    }

    addrlen = sizeof(servaddr);

    // 서버로 데이터 전송
    if (sendto(s, buf, strlen(buf), 0,
               (struct sockaddr *)&servaddr, addrlen) < 0) {
        perror("sendto fail");
        exit(1);
    }

    // 서버로부터 echo 데이터 수신
    if ((nbyte = recvfrom(s, buf, MAXLINE, 0,
                          (struct sockaddr *)&servaddr, &addrlen)) < 0) {
        perror("recvfrom fail");
        exit(1);
    }

    buf[nbyte] = '\0';
    printf("수신: %s\n", buf);

    close(s);
    return 0;
}