//--------------------------------------------------------------
// 파일명 : myecho_daemon.c
// 기능   : TCP echo server를 daemon으로 실행
// 컴파일 : gcc -o myecho_daemon myecho_daemon.c
// 실행   : ./myecho_daemon 6000
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 511
#define MAXFD   64

int tcp_listen(int host, int port, int backlog);
void daemon_init(void);
void write_log(const char *msg);

int main(int argc, char *argv[])
{
    struct sockaddr_in cliaddr;
    int listen_sock, accp_sock;
    socklen_t addrlen;
    int nbyte;
    char buf[MAXLINE + 1];

    if (argc != 2) {
        fprintf(stderr, "사용법: %s port\n", argv[0]);
        exit(1);
    }

    // 데몬 프로세스로 전환
    daemon_init();

    write_log("daemon started");

    listen_sock = tcp_listen(INADDR_ANY, atoi(argv[1]), 5);

    while (1) {
        addrlen = sizeof(cliaddr);

        accp_sock = accept(listen_sock, (struct sockaddr *)&cliaddr, &addrlen);
        if (accp_sock < 0) {
            write_log("accept fail");
            continue;
        }

        while ((nbyte = read(accp_sock, buf, MAXLINE)) > 0) {
            write(accp_sock, buf, nbyte);
        }

        close(accp_sock);
    }

    close(listen_sock);
    return 0;
}

//--------------------------------------------------------------
// 데몬 초기화 함수
//--------------------------------------------------------------
void daemon_init(void)
{
    pid_t pid;
    int i;
    struct sigaction sact;

    // 첫 번째 fork
    pid = fork();

    if (pid < 0) {
        exit(1);
    }

    if (pid != 0) {
        exit(0);   // 부모 종료
    }

    // 세션 리더가 됨
    setsid();

    // SIGHUP 무시
    sact.sa_handler = SIG_IGN;
    sigemptyset(&sact.sa_mask);
    sact.sa_flags = 0;
    sigaction(SIGHUP, &sact, NULL);

    // 두 번째 fork
    pid = fork();

    if (pid < 0) {
        exit(1);
    }

    if (pid != 0) {
        exit(0);   // 부모 종료
    }

    // 루트 디렉토리로 이동
    chdir("/");

    // 파일 생성 권한 제한 해제
    umask(0);

    // 열려 있을 수 있는 파일 디스크립터 닫기
    for (i = 0; i < MAXFD; i++) {
        close(i);
    }

    // 표준 입출력은 /dev/null로 연결
    open("/dev/null", O_RDONLY);  // stdin
    open("/dev/null", O_WRONLY);  // stdout
    open("/dev/null", O_WRONLY);  // stderr
}

//--------------------------------------------------------------
// listen 소켓 생성 함수
//--------------------------------------------------------------
int tcp_listen(int host, int port, int backlog)
{
    int sd;
    int opt = 1;
    struct sockaddr_in servaddr;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        write_log("socket fail");
        exit(1);
    }

    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(host);
    servaddr.sin_port = htons(port);

    if (bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        write_log("bind fail");
        exit(1);
    }

    if (listen(sd, backlog) < 0) {
        write_log("listen fail");
        exit(1);
    }

    write_log("listen success");

    return sd;
}

//--------------------------------------------------------------
// 로그 기록 함수
//--------------------------------------------------------------
void write_log(const char *msg)
{
    FILE *fp;

    fp = fopen("/tmp/myecho_daemon.log", "a");
    if (fp == NULL) {
        return;
    }

    fprintf(fp, "%s\n", msg);
    fclose(fp);
}
