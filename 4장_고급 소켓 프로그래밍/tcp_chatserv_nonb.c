//-------------------------------------------------------------
// 파일명 : tcp_chatserv_nonb.c
// 기능   : 논블록 모드의 채팅 서버
// 컴파일 : gcc -o tcp_chatserv_nonb tcp_chatserv_nonb.c
// 사용법 : ./tcp_chatserv_nonb 4001
//-------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 511
#define MAX_SOCK 1024

char *EXIT_STRING = "exit";
char *START_STRING = "Connected to chat server\n";

int num_chat = 0;                  // 채팅 참가자 수
int clisock_list[MAX_SOCK];        // 채팅 참가자 소켓 번호 목록
int listen_sock;                   // listen 소켓

// 함수 선언
void addClient(int s, struct sockaddr_in *newcliaddr);
void removeClient(int i);
int set_nonblock(int sockfd);
int is_nonblock(int sockfd);
int tcp_listen(int host, int port, int backlog);
void errquit(char *mesg);

int main(int argc, char *argv[])
{
    char buf[MAXLINE + 1];

    int i, j;
    int nbyte;
    int count;
    int accp_sock;
    socklen_t clilen;

    struct sockaddr_in cliaddr;

    if (argc != 2) {
        printf("사용법 : %s port\n", argv[0]);
        exit(1);
    }

    listen_sock = tcp_listen(INADDR_ANY, atoi(argv[1]), 5);

    if (listen_sock == -1) {
        errquit("tcp_listen fail");
    }

    // listen 소켓을 논블로킹 모드로 설정
    if (set_nonblock(listen_sock) == -1) {
        errquit("set_nonblock fail");
    }

    for (count = 0; ; count++) {
        if (count == 100000) {
            putchar('.');
            fflush(stdout);
            count = 0;
        }

        //-----------------------------------------------------
        // 새로운 클라이언트 접속 처리
        //-----------------------------------------------------
        clilen = sizeof(cliaddr);

        accp_sock = accept(listen_sock,
                           (struct sockaddr *)&cliaddr,
                           &clilen);

        if (accp_sock == -1) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                errquit("accept fail");
            }
        } else {
            // 통신용 소켓도 논블로킹 모드로 설정
            if (is_nonblock(accp_sock) != 0) {
                if (set_nonblock(accp_sock) < 0) {
                    errquit("set_nonblock fail");
                }
            }

            addClient(accp_sock, &cliaddr);

            send(accp_sock, START_STRING, strlen(START_STRING), 0);

            printf("%d번째 사용자 추가.\n", num_chat);
        }

        //-----------------------------------------------------
        // 클라이언트가 보낸 메시지를 모든 클라이언트에게 방송
        //-----------------------------------------------------
        for (i = 0; i < num_chat; i++) {
            errno = 0;

            nbyte = recv(clisock_list[i], buf, MAXLINE, 0);

            // 정상적으로 연결 종료
            if (nbyte == 0) {
                removeClient(i);
                i--;
                continue;
            }

            // 아직 읽을 데이터가 없는 경우
            else if (nbyte == -1 &&
                    (errno == EWOULDBLOCK || errno == EAGAIN)) {
                continue;
            }

            // 그 외 recv 에러
            else if (nbyte == -1) {
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

        // CPU를 너무 많이 쓰지 않도록 약간 쉬기
        usleep(1000);
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
void removeClient(int i)
{
    close(clisock_list[i]);

    // 중간 사용자가 나가면 마지막 사용자를 그 자리로 이동
    if (i != num_chat - 1) {
        clisock_list[i] = clisock_list[num_chat - 1];
    }

    num_chat--;

    printf("채팅 참가자 1명 탈퇴. 현재 참가자 수 = %d\n", num_chat);
}

//-------------------------------------------------------------
// 소켓이 nonblock인지 확인
// return 0  : nonblock 상태
// return -1 : nonblock 아님
//-------------------------------------------------------------
int is_nonblock(int sockfd)
{
    int val;

    // 기존 플래그 값을 얻어온다
    val = fcntl(sockfd, F_GETFL, 0);

    if (val == -1) {
        return -1;
    }

    // 논블로킹 모드인지 확인
    if (val & O_NONBLOCK) {
        return 0;
    }

    return -1;
}

//-------------------------------------------------------------
// 소켓을 논블로킹 모드로 설정
//-------------------------------------------------------------
int set_nonblock(int sockfd)
{
    int val;

    // 기존 플래그 값을 얻어온다
    val = fcntl(sockfd, F_GETFL, 0);

    if (val == -1) {
        return -1;
    }

    if (fcntl(sockfd, F_SETFL, val | O_NONBLOCK) == -1) {
        return -1;
    }

    return 0;
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
