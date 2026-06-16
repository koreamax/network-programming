//--------------------------------------------------------------
// 파일명 : udpserv_echo.c
// 기능   : xinetd에 등록되어 UDP echo 요청을 처리함
// 컴파일 : gcc -o myecho_udpserv udpserv_echo.c
// 실행   : UDP 메시지가 들어오면 xinetd에 의해 자동 실행
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in peer;
    int rc;
    socklen_t len;
    int pidsz;
    char buf[120];

    len = sizeof(peer);

    // 응답 앞에 현재 프로세스 ID를 붙임
    pidsz = sprintf(buf, "%d: ", getpid());

    // xinetd가 넘겨준 UDP 소켓에서 데이터 수신
    rc = recvfrom(0, buf + pidsz, sizeof(buf) - pidsz - 1, 0,
                  (struct sockaddr *)&peer, &len);

    if (rc < 0) {
        exit(1);
    }

    buf[pidsz + rc] = '\0';

    // 받은 클라이언트 주소로 다시 전송
    sendto(1, buf, rc + pidsz, 0,
           (struct sockaddr *)&peer, len);

    exit(0);
}
