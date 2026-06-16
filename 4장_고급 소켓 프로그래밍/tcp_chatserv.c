//-------------------------------------------------------------
// 파일명 : tcp_chatserv.c
// 기능   : 채팅 참가자 관리, 채팅 메시지 수신 및 방송
// 컴파일 : gcc -o chat_server tcp_chatserv.c
// 사용법 : ./chat_server 4001
//-------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 511
#define MAX_SOCK 1024

char *EXIT_STRING = "exit";                       // 클라이언트 종료 요청 문자열
char *START_STRING = "Connected to chat server\n"; // 클라이언트 환영 메시지

int maxfdp1;                    // 최대 소켓 번호 + 1
int num_chat = 0;               // 채팅 참가자 수
int clisock_list[MAX_SOCK];     // 채팅 참가자 소켓 목록
int listen_sock;                // 서버 listen 소켓

// 함수 선언
void addClient(int s, struct sockaddr_in *newcliaddr);
void removeClient(int idx);
int getmax(void);
int tcp_listen(int host, int port, int backlog);
void errquit(char *mesg);

int main(int argc, char *argv[])
{
    struct sockaddr_in cliaddr;
    char buf[MAXLINE + 1];

    int i, j;
    int nbyte;
    int accp_sock;
    socklen_t addrlen;

    fd_set read_fds;    // 읽기를 감지할 fd_set 구조체

    if (argc != 2) {
        printf("사용법 : %s port\n", argv[0]);
        exit(1);
    }

    // listen 소켓 생성
    listen_sock = tcp_listen(INADDR_ANY, atoi(argv[1]), 5);

    while (1) {
        FD_ZERO(&read_fds);

        // listen 소켓 등록
        FD_SET(listen_sock, &read_fds);

        // 현재 접속 중인 클라이언트 소켓 등록
        for (i = 0; i < num_chat; i++) {
            FD_SET(clisock_list[i], &read_fds);
        }

        // select의 첫 번째 인자는 최대 fd + 1
        maxfdp1 = getmax() + 1;

        puts("wait for client");

        if (select(maxfdp1, &read_fds, NULL, NULL, NULL) < 0) {
            errquit("select fail");
        }

        //-----------------------------------------------------
        // 새로운 클라이언트 접속 처리
        //-----------------------------------------------------
        if (FD_ISSET(listen_sock, &read_fds)) {
            addrlen = sizeof(cliaddr);

            accp_sock = accept(listen_sock,
                               (struct sockaddr *)&cliaddr,
                               &addrlen);

            if (accp_sock == -1) {
                errquit("accept fail");
            }

            addClient(accp_sock, &cliaddr);

            send(accp_sock, START_STRING, strlen(START_STRING), 0);

            printf("%d번째 사용자 추가.\n", num_chat);
        }

        //-----------------------------------------------------
        // 기존 클라이언트 메시지 처리
        //-----------------------------------------------------
        for (i = 0; i < num_chat; i++) {
            if (FD_ISSET(clisock_list[i], &read_fds)) {
                nbyte = recv(clisock_list[i], buf, MAXLINE, 0);

                // 클라이언트 연결 종료
                if (nbyte <= 0) {
                    removeClient(i);
                    i--;
                    continue;
                }

                buf[nbyte] = '\0';

                // 종료 문자열 처리
                if (strstr(buf, EXIT_STRING) != NULL) {
                    removeClient(i);
                    i--;
                    continue;
                }

                // 모든 채팅 참가자에게 메시지 방송
                for (j = 0; j < num_chat; j++) {
                    send(clisock_list[j], buf, nbyte, 0);
                }

                printf("%s", buf);
            }
        }
    }

    return 0;
}

//-------------------------------------------------------------
// 에러 처리 함수
//-------------------------------------------------------------
void errquit(char *mesg)
{
    perror(mesg);
    exit(1);
}

//-------------------------------------------------------------
// 새로운 채팅 참가자 처리
//-------------------------------------------------------------
void addClient(int s, struct sockaddr_in *newcliaddr)
{
    char buf[20];

    if (num_chat >= MAX_SOCK) {
        printf("too many clients\n");
        close(s);
        return;
    }

    inet_ntop(AF_INET, &newcliaddr->sin_addr, buf, sizeof(buf));
    printf("new client: %s\n", buf);

    // 채팅 클라이언트 목록에 추가
    clisock_list[num_chat] = s;
    num_chat++;
}

//-------------------------------------------------------------
// 채팅 탈퇴 처리
//-------------------------------------------------------------
void removeClient(int idx)
{
    close(clisock_list[idx]);

    // 중간 사용자가 나가면 마지막 사용자를 그 자리로 이동
    if (idx != num_chat - 1) {
        clisock_list[idx] = clisock_list[num_chat - 1];
    }

    num_chat--;

    printf("채팅 참가자 1명 탈퇴. 현재 참가자 수 = %d\n", num_chat);
}

//-------------------------------------------------------------
// 최대 소켓 번호 찾기
//-------------------------------------------------------------
int getmax(void)
{
    int max = listen_sock;
    int i;

    for (i = 0; i < num_chat; i++) {
        if (clisock_list[i] > max) {
            max = clisock_list[i];
        }
    }

    return max;
}

//-------------------------------------------------------------
// listen 소켓 생성 및 listen
//-------------------------------------------------------------
int tcp_listen(int host, int port, int backlog)
{
    int sd;
    int opt = 1;
    struct sockaddr_in servaddr;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        perror("socket fail");
        exit(1);
    }

    // 포트 재사용 옵션
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // servaddr 구조체 초기화
    bzero((char *)&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(host);
    servaddr.sin_port = htons(port);

    if (bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind fail");
        exit(1);
    }

    // 클라이언트 연결 요청 대기
    if (listen(sd, backlog) < 0) {
        perror("listen fail");
        exit(1);
    }

    return sd;
}
