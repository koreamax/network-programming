//------------------------------------------------------------
// 파일명 : udpserv_pipeecho.c
// 동  작 : 파이프를 통해 에코 메시지를 전달하는 UDP 에코 서버
// 컴파일 : gcc -o udpserv_pipeecho udpserv_pipeecho.c
// 실행   : ./udpserv_pipeecho port
//------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#define MAX_BUFSZ 512

// 파이프에 쓰는 데이터 구조
typedef struct mesg {
    struct sockaddr_in addr;     // 클라이언트 주소
    char data[MAX_BUFSZ];        // 에코할 데이터
    int nbytes;                  // 받은 데이터 크기
} mesg_t;

void child_start(int sock, int pipefd[]);
void parent_start(int sock, int pipefd[]);
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
    int pipefd[2];
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

    // 파이프 생성
    if (pipe(pipefd) == -1) {
        errquit("pipe failed");
    }

    pid = fork();

    if (pid < 0) {
        errquit("fork failed");
    } else if (pid > 0) {
        parent_start(sock, pipefd);
    } else {
        child_start(sock, pipefd);
    }

    close(sock);
    return 0;
}

// 자식 프로세스
void child_start(int sock, int pipefd[])
{
    mesg_t pmsg;
    int nbytes;
    socklen_t len;

    // 자식은 파이프에서 읽기만 하므로 쓰기 파이프 닫음
    close(pipefd[1]);

    while (1) {
        // 파이프로부터 읽기 대기
        nbytes = read(pipefd[0], (char *)&pmsg, sizeof(mesg_t));
        if (nbytes < 0) {
            errquit("read failed");
        }

        printf("Child: read from pipe\n");

        len = sizeof(pmsg.addr);

        // 파이프로부터 읽은 데이터를 클라이언트에게 에코
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
}

// 부모 프로세스
void parent_start(int sock, int pipefd[])
{
    mesg_t pmsg;
    int nbytes;
    socklen_t len;

    // 부모는 파이프에 쓰기만 하므로 읽기 파이프 닫음
    close(pipefd[0]);

    printf("My echo server wait...\n");

    while (1) {
        len = sizeof(pmsg.addr);
        memset(&pmsg, 0, sizeof(pmsg));

        // 소켓으로부터 읽기
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

        // 소켓으로부터 읽은 데이터를 파이프에 쓰기
        if (write(pipefd[1], (char *)&pmsg, sizeof(mesg_t)) < 0) {
            perror("write failed");
        }

        printf("Parent: write to pipe\n");
    }
}
