//--------------------------------------------------------------
// 파일명  res_send.c
// 동  작  메시지큐에서 에코 메시지를 읽어서 응답해줌
// 컴파일  gcc -o res_send res_send.c
// 실  행  msgq_echoserv에서 execlp()로 실행됨
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_BUFSZ 512

typedef struct msg {
    long msg_type;
    struct sockaddr_in addr;
    char msg_text[MAX_BUFSZ];
} msg_t;

// 메시지큐에서 읽어서 응답하기
int qread_and_echoreply(int msqid, int sock);

void errquit(char *mesg)
{
    perror(mesg);
    exit(1);
}

int main(int argc, char **argv)
{
    key_t key;
    int sock;
    int msqid;

    if (argc != 3) {
        printf("Usage: %s msgq_key port\n", argv[0]);
        exit(1);
    }

    key = atoi(argv[1]);

    // 메시지큐 ID 얻기
    if ((msqid = msgget(key, 0)) == -1) {
        errquit("rep msgget fail");
    }

    // UDP 소켓 생성
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        errquit("socket fail");
    }

    // 메시지큐에서 읽어서 reply
    qread_and_echoreply(msqid, sock);

    close(sock);

    return 0;
}

// 메시지큐에서 읽어서 응답하기
int qread_and_echoreply(int msqid, int sock)
{
    int size;
    socklen_t len;
    msg_t pmsg;

    size = sizeof(msg_t) - sizeof(long);

    while (1) {
        // 메시지큐에서 아무 메시지나 하나 읽음
        if (msgrcv(msqid, (void *)&pmsg, size, 0, 0) < 0) {
            errquit("msgrcv fail");
        }

        printf("reply process PID = %d\n", getpid());
        printf("message = %s\n", pmsg.msg_text);

        len = sizeof(pmsg.addr);

        // 클라이언트에게 다시 에코 응답
        if (sendto(sock,
                   pmsg.msg_text,
                   strlen(pmsg.msg_text),
                   0,
                   (struct sockaddr *)&pmsg.addr,
                   len) < 0) {
            errquit("serv2 sendto fail");
        }

        pmsg.msg_text[0] = '\0';
    }

    return 0;
}
