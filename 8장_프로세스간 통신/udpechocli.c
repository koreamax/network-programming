//--------------------------------------------------------------
// 파일명 : udpechocli.c
// 동  작 : UDP 에코 클라이언트
// 컴파일 : gcc -o udpechocli udpechocli.c
// 실행   : ./udpechocli ip port
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_BUFSZ 512

int main(int argc, char *argv[])
{
    struct sockaddr_in peer;
    int sock;
    int nbytes;
    char buf[MAX_BUFSZ];

    if (argc != 3) {
        printf("Usage: %s ip port\n", argv[0]);
        exit(1);
    }

    // UDP 소켓 생성
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket failed");
        exit(1);
    }

    memset(&peer, 0, sizeof(peer));

    peer.sin_family = AF_INET;
    peer.sin_port = htons(atoi(argv[2]));
    peer.sin_addr.s_addr = inet_addr(argv[1]);

    // 키보드 입력을 받고 서버로 보내는 부분
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        nbytes = sendto(
            sock,
            buf,
            strlen(buf),
            0,
            (struct sockaddr *)&peer,
            sizeof(peer)
        );

        if (nbytes < 0) {
            perror("sendto failed");
            close(sock);
            exit(1);
        }

        nbytes = recvfrom(
            sock,
            buf,
            sizeof(buf) - 1,
            0,
            NULL,
            NULL
        );

        if (nbytes < 0) {
            perror("recvfrom failed");
            close(sock);
            exit(1);
        }

        buf[nbytes] = '\0';
        fputs(buf, stdout);
    }

    close(sock);
    exit(0);
}
