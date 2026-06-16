//--------------------------------------------------------------
// 파일명 : tcp_chatcli.c
// 기능   : 서버에 접속한 후 키보드 입력을 서버로 전달하고,
//          서버로부터 오는 메시지를 화면에 출력한다.
// 컴파일 : gcc -o chat_client tcp_chatcli.c
// 사용법 : ./chat_client 127.0.0.1 4001 name
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 1000
#define NAME_LEN 20

char *EXIT_STRING = "exit";

// 소켓 생성 및 서버 연결, 생성된 소켓 리턴
int tcp_connect(int af, char *servip, unsigned short port);

void errquit(char *mesg)
{
    perror(mesg);
    exit(1);
}

int main(int argc, char *argv[])
{
    char bufall[MAXLINE + NAME_LEN];   // 이름 + 메시지를 위한 버퍼
    char *bufmsg;                      // bufall에서 메시지 부분의 포인터

    int maxfdp1;       // 최대 소켓 디스크립터 + 1
    int s;             // 소켓
    int namelen;       // 이름의 길이

    fd_set read_fds;

    if (argc != 4) {
        printf("사용법 : %s server_ip port name\n", argv[0]);
        exit(1);
    }

    // bufall의 앞부분에 이름 저장
    snprintf(bufall, sizeof(bufall), "[%s] : ", argv[3]);

    namelen = strlen(bufall);

    // bufmsg는 실제 메시지가 들어갈 위치를 가리킴
    bufmsg = bufall + namelen;

    // 서버에 연결
    s = tcp_connect(AF_INET, argv[1], atoi(argv[2]));

    if (s == -1) {
        errquit("tcp_connect fail");
    }

    puts("서버에 접속되었습니다.");

    maxfdp1 = s + 1;

    while (1) {
        FD_ZERO(&read_fds);

        // 표준 입력 stdin 감시
        FD_SET(0, &read_fds);

        // 서버 소켓 감시
        FD_SET(s, &read_fds);

        if (select(maxfdp1, &read_fds, NULL, NULL, NULL) < 0) {
            errquit("select fail");
        }

        //------------------------------------------------------
        // 서버에서 메시지가 온 경우
        //------------------------------------------------------
        if (FD_ISSET(s, &read_fds)) {
            int nbyte;

            nbyte = recv(s, bufmsg, MAXLINE, 0);

            if (nbyte <= 0) {
                puts("서버와 연결이 종료되었습니다.");
                close(s);
                exit(0);
            }

            bufmsg[nbyte] = '\0';

            printf("%s", bufmsg);
        }

        //------------------------------------------------------
        // 키보드에서 입력이 들어온 경우
        //------------------------------------------------------
        if (FD_ISSET(0, &read_fds)) {
            if (fgets(bufmsg, MAXLINE, stdin) != NULL) {
                if (send(s, bufall, namelen + strlen(bufmsg), 0) < 0) {
                    puts("Error : write error on socket.");
                }

                // exit 입력 시 종료
                if (strstr(bufmsg, EXIT_STRING) != NULL) {
                    puts("Good bye.");
                    close(s);
                    exit(0);
                }
            }
        }
    }

    return 0;
}

//--------------------------------------------------------------
// 서버에 TCP 연결
//--------------------------------------------------------------
int tcp_connect(int af, char *servip, unsigned short port)
{
    struct sockaddr_in servaddr;
    int s;

    // 소켓 생성
    s = socket(af, SOCK_STREAM, 0);

    if (s < 0) {
        return -1;
    }

    // 채팅 서버의 소켓 주소 구조체 servaddr 초기화
    bzero((char *)&servaddr, sizeof(servaddr));

    servaddr.sin_family = af;
    servaddr.sin_port = htons(port);

    if (inet_pton(AF_INET, servip, &servaddr.sin_addr) <= 0) {
        close(s);
        return -1;
    }

    // 서버에 연결 요청
    if (connect(s, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        close(s);
        return -1;
    }

    return s;
}
