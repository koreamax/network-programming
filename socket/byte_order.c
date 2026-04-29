//--------------------------------------------------------------
// 파일명  : byte_order.c
// 기능    : 호스트 바이트 순서 테스트 프로그램
// 컴파일  : cc -o byte_order byte_order.c
// 사용법  : ./byte_order
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(void) {
    struct servent *servent;

    // "echo" 서비스의 UDP 포트 정보 가져오기
    servent = getservbyname("echo", "udp");

    if (servent == NULL) {
        printf("서비스 정보를 얻을 수 없음.\n");
        exit(1);
    }

    // 네트워크 바이트 순서 그대로 출력
    printf("UDP echo 포트번호 (네트워크 순서): %d\n", servent->s_port);

    // 호스트 바이트 순서로 변환 후 출력
    printf("UDP echo 포트번호 (호스트 순서): %d\n", ntohs(servent->s_port));

    return 0;
}