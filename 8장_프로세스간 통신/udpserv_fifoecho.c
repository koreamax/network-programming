//------------------------------------------------------------
// 파일명 : udpserv_fifoecho.c
// 동  작 : FIFO를 이용하여 에코 메시지를 전달하는 UDP 에코 서버
// 컴파일 : gcc -o udpserv_fifoecho udpserv_fifoecho.c
// 실행   : ./udpserv_fifoecho port
//------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_BUFSZ 512
#define FIFONAME "myfifo"

// FIFO에 쓰는 데이터 구조
typedef struct mesg {
    struct sockaddr_in addr;     // 클라이언트 주소
    char data[MAX_BUFSZ];        // 읽은 데이터
    int nbytes;                  // 읽은 데이터 크기
} mesg_t;

void child_start(int sock);
void parent_start(int sock);
void errquit(char *mesg);

void errquit(char *mesg)
{
    perror(mesg);
    exit(1);
}

int main(int argc, char **argv)
{
    struct sockaddr_in servaddr;
    pid_t pid;
    int sock;
    int port;
    socklen_t len;

    if (argc != 2) {
        printf("\nUsage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        errquit("socket failed");
    }

    len = sizeof(servaddr);
    memset(&servaddr, 0, len);

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&servaddr, len) < 0) {
        errquit("bind failed");
    }

    // FIFO 생성
    if (mkfifo(FIFONAME, 0660) == -1 && errno != EEXIST) {
        errquit("mkfifo failed");
    }

    pid = fork();

    if (pid < 0) {
        errquit("fork failed");
    } else if (pid > 0) {
        parent_start(sock);
    } else {
        child_start(sock);
    }

    close(sock);
    return 0;
}

// 자식 프로세스
void child_start(int sock)
{
    mesg_t pmsg;
    int nbytes;
    int fiford;          // 읽기 모드의 FIFO
    socklen_t len;

    // 읽기 모드로 FIFO open
    fiford = open(FIFONAME, O_RDONLY);
    if (fiford == -1) {
        errquit("fifo open failed");
    }

    while (1) {
        // FIFO로부터 읽기 대기
        nbytes = read(fiford, (char *)&pmsg, sizeof(mesg_t));
        if (nbytes < 0) {
            errquit("read failed");
        }

        printf("Child: read from fifo\n");

        len = sizeof(pmsg.addr);

        // FIFO로부터 읽은 데이터를 클라이언트로 전송
        nbytes = sendto(
            sock,
            pmsg.data,
            pmsg.nbytes,
            0,
            (struct sockaddr *)&pmsg.addr,
            len
        );

        if (nbytes < 0) {
            errquit("sendto failed");
        }

        printf("Child: %d bytes echo response\n", nbytes);
        printf("--------------------------------\n");
    }

    close(fiford);
}

// 부모 프로세스
void parent_start(int sock)
{
    mesg_t pmsg;
    int nbytes;
    int fifowd;          // 쓰기 모드의 FIFO
    socklen_t len;

    // 쓰기 모드로 FIFO open
    fifowd = open(FIFONAME, O_WRONLY);
    if (fifowd == -1) {
        errquit("fifo open failed");
    }

    printf("My echo server wait...\n");

    while (1) {
        len = sizeof(pmsg.addr);
        memset(&pmsg, 0, sizeof(pmsg));

        // 소켓으로부터 읽기 대기
        nbytes = recvfrom(
            sock,
            pmsg.data,
            MAX_BUFSZ - 1,
            0,
            (struct sockaddr *)&pmsg.addr,
            &len
        );

        if (nbytes < 0) {
            errquit("recvfrom failed");
        }

        pmsg.data[nbytes] = '\0';
        pmsg.nbytes = nbytes;

        printf("Parent: %d bytes recv from socket\n", nbytes);

        // 소켓으로부터 읽은 데이터를 FIFO에 쓰기
        if (write(fifowd, (char *)&pmsg, sizeof(mesg_t)) < 0) {
            perror("write failed");
        }

        printf("Parent: write to fifo\n");
    }

    close(fifowd);
}
