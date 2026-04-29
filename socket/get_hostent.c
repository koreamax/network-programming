//--------------------------------------------------------------
// 파일명  : get_hostent.c
// 기능    : 도메인 이름을 IP 주소로 변환
// 컴파일  : gcc -o get_hostent get_hostent.c
// 사용법  : ./get_hostent www.kangwon.ac.kr
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    struct hostent *hp;
    struct in_addr in;
    int i;
    char buf[20];

    if (argc < 2) {
        printf("Usage : %s hostname\n", argv[0]);
        exit(1);
    }

    // 도메인 이름 → IP 정보 얻기
    hp = gethostbyname(argv[1]);
    if (hp == NULL) {
        printf("gethostbyname fail\n");
        exit(1);
    }

    printf("호스트 이름 : %s\n", hp->h_name);
    printf("호스트 주소타입 번호 : %d\n", hp->h_addrtype);
    printf("호스트 주소의 길이 : %d 바이트\n", hp->h_length);

    // IP 주소 출력 (여러 개 있을 수 있음)
    for (i = 0; hp->h_addr_list[i] != NULL; i++) {
        memcpy(&in.s_addr, hp->h_addr_list[i], sizeof(in.s_addr));
        inet_ntop(AF_INET, &in, buf, sizeof(buf));
        printf("IP 주소 (%d번째) : %s\n", i + 1, buf);
    }

    // alias 출력
    for (i = 0; hp->h_aliases[i] != NULL; i++) {
        printf("호스트 별명 (%d번째) : %s\n", i + 1, hp->h_aliases[i]);
    }

    puts("");

    return 0;
}