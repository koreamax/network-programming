//--------------------------------------------------------------
// 파일명 : echoserv.c
// 기능   : xinetd에 등록되어 echo 요청 서비스를 수행
// 컴파일 : gcc -o myecho_serv echoserv.c
// 실행   : xinetd에 의해 클라이언트 요청이 들어오면 자동 실행
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int cnt = 0;
    char line[1024];

    // stdout을 line buffering으로 설정
    setvbuf(stdout, NULL, _IOLBF, 0);

    // xinetd가 클라이언트 소켓을 stdin/stdout에 연결해줌
    while (fgets(line, sizeof(line), stdin) != NULL) {
        printf("%3d: %s", ++cnt, line);
        fflush(stdout);
    }

    return 0;
}
