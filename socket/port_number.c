//--------------------------------------------------------------
// 파일명  : port_number.c
// 기능    : 시스템이 자동으로 배정한 포트번호를 출력
// 컴파일  : gcc -o port_number port_number.c
// 사용법  : ./port_number
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MSG "Test Message"

int main(void) {
    int sd1, sd2;
    int addrlen;
    struct sockaddr_in servaddr, cliaddr;
    unsigned short port1, port2;

    // 서버 주소 설정 (localhost + echo port)
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
    servaddr.sin_port = htons(7); // echo 포트

    // 소켓 생성
    sd1 = socket(PF_INET, SOCK_STREAM, 0); // TCP
    sd2 = socket(PF_INET, SOCK_DGRAM, 0);  // UDP

    if (sd1 < 0 || sd2 < 0) {
        perror("socket fail");
        exit(1);
    }

    // TCP 연결 (자동 포트 할당 발생)
    if (connect(sd1, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect fail");
        exit(1);
    }

    // TCP 소켓의 로컬 포트 얻기
    addrlen = sizeof(cliaddr);
    getsockname(sd1, (struct sockaddr *)&cliaddr, (socklen_t *)&addrlen);
    port1 = ntohs(cliaddr.sin_port);

    // UDP send (자동 포트 할당 발생)
    sendto(sd2, MSG, strlen(MSG), 0,
           (struct sockaddr *)&servaddr, sizeof(servaddr));

    // UDP 소켓의 로컬 포트 얻기
    addrlen = sizeof(cliaddr);
    getsockname(sd2, (struct sockaddr *)&cliaddr, (socklen_t *)&addrlen);
    port2 = ntohs(cliaddr.sin_port);

    printf("스트림 소켓 포트번호 = %d\n", port1);
    printf("데이터그램 소켓 포트번호 = %d\n", port2);

    close(sd1);
    close(sd2);

    return 0;
}