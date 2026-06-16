//--------------------------------------------------------------
// 파일명 : chg_rcvbuf.c
// 동작   : 소켓 옵션을 사용하여 수신버퍼의 크기 변경
// 컴파일 : gcc -o chg_rcvbuf chg_rcvbuf.c
// 실행   : ./chg_rcvbuf
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(void)
{
    int s;
    int val;
    socklen_t len;

    // TCP 소켓 생성
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fail");
        exit(1);
    }

    // 현재 수신 버퍼 크기 확인
    len = sizeof(val);

    if (getsockopt(s, SOL_SOCKET, SO_RCVBUF, &val, &len) < 0) {
        perror("getsockopt fail");
        close(s);
        exit(1);
    }

    printf("디폴트 수신버퍼 크기 : %d\n", val);

    // 수신 버퍼 크기를 1024로 변경
    val = 1024;

    if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val)) < 0) {
        perror("setsockopt fail");
        close(s);
        exit(1);
    }

    // 변경된 수신 버퍼 크기 확인
    len = sizeof(val);

    if (getsockopt(s, SOL_SOCKET, SO_RCVBUF, &val, &len) < 0) {
        perror("getsockopt fail");
        close(s);
        exit(1);
    }

    printf("1024로 변경한 수신버퍼 크기 : %d\n", val);

    close(s);

    return 0;
}
