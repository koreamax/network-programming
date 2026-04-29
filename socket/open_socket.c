//--------------------------------------------------------------
// 파일명  : open_socket.c
// 기능    : socket() 시스템 콜을 호출하고 생성된 소켓 번호 출력
// 컴파일  : cc -o open_socket open_socket.c
// 사용법  : ./open_socket
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    // 파일 및 소켓 디스크립터
    int fd1, fd2, sd1, sd2;

    // 파일 열기 (/etc/passwd)
    fd1 = open("/etc/passwd", O_RDONLY, 0);
    if (fd1 < 0) {
        perror("open error");
        exit(1);
    }
    printf("/etc/passwd's file descriptor: %d\n", fd1);

    // 스트림 소켓 생성 (TCP)
    sd1 = socket(PF_INET, SOCK_STREAM, 0);
    if (sd1 < 0) {
        perror("socket error");
        exit(1);
    }
    printf("stream socket descriptor: %d\n", sd1);

    // 데이터그램 소켓 생성 (UDP)
    sd2 = socket(PF_INET, SOCK_DGRAM, 0);
    if (sd2 < 0) {
        perror("socket error");
        exit(1);
    }
    printf("datagram socket descriptor: %d\n", sd2);

    // 또 다른 파일 열기 (/etc/hosts)
    fd2 = open("/etc/hosts", O_RDONLY, 0);
    if (fd2 < 0) {
        perror("open error");
        exit(1);
    }
    printf("/etc/hosts's file descriptor: %d\n", fd2);

    // 파일 및 소켓 닫기
    close(fd2);
    close(fd1);
    close(sd2);
    close(sd1);

    return 0;
}