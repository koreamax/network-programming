//--------------------------------------------------------------
// 파일명 : mth_echoserv.c
// 동작   : 멀티스레드 에코 서버
//
// 컴파일 : gcc -o mth_echoserv mth_echoserv.c -pthread
// 실행   : ./mth_echoserv msgkey port
// 예시   : ./mth_echoserv 1234 9000
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_BUFSZ 1024
#define RECV_THR_NUM 5

typedef struct _msg
{
    long msg_type;
    struct sockaddr_in addr;
    char msg_text[MAX_BUFSZ];
} msg_t;

struct sockaddr_in servaddr;
int sock;       // 서버 소켓
int msqid;      // 메시지큐 ID

void *echo_recv(void *arg);
void *echo_resp(void *arg);

void errquit(char *msg)
{
    perror(msg);
    exit(1);
}

void thr_errquit(char *msg, int errcode)
{
    printf("%s: %s\n", msg, strerror(errcode));
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    pthread_t tid[RECV_THR_NUM + 1];
    int port, status, i;
    socklen_t len = sizeof(struct sockaddr_in);
    key_t msqkey;

    if (argc != 3)
    {
        printf("Usage : %s msgkey port\n", argv[0]);
        exit(1);
    }

    msqkey = atoi(argv[1]);
    port = atoi(argv[2]);

    // 메시지큐 생성
    if ((msqid = msgget(msqkey, IPC_CREAT | 0660)) < 0)
    {
        errquit("msgget fail");
    }

    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        errquit("socket fail");
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&servaddr, len) < 0)
    {
        errquit("bind fail");
    }

    printf("UDP multi-thread echo server started. port: %d\n", port);

    // 에코 수신 스레드 5개 생성
    for (i = 0; i < RECV_THR_NUM; i++)
    {
        if ((status = pthread_create(&tid[i], NULL, echo_recv, NULL)) != 0)
        {
            thr_errquit("pthread_create", status);
        }
    }

    // 에코 응답 스레드 1개 생성
    if ((status = pthread_create(&tid[RECV_THR_NUM], NULL, echo_resp, NULL)) != 0)
    {
        thr_errquit("pthread_create", status);
    }

    for (i = 0; i < RECV_THR_NUM + 1; i++)
    {
        pthread_join(tid[i], NULL);
    }

    // 메시지큐 삭제
    msgctl(msqid, IPC_RMID, 0);

    close(sock);

    return 0;
}

// 에코 수신 스레드
void *echo_recv(void *arg)
{
    int nbytes;
    socklen_t len;
    msg_t pmsg;
    int size;

    size = sizeof(pmsg) - sizeof(long);

    while (1)
    {
        len = sizeof(struct sockaddr_in);

        // 메시지 타입은 반드시 양수여야 함
        pmsg.msg_type = 1;

        // 에코 메시지 수신
        nbytes = recvfrom(
            sock,
            pmsg.msg_text,
            MAX_BUFSZ - 1,
            0,
            (struct sockaddr *)&pmsg.addr,
            &len
        );

        if (nbytes < 0)
        {
            thr_errquit("recvfrom fail", errno);
        }

        pmsg.msg_text[nbytes] = '\0';

        printf("recv thread = %lu\n", (unsigned long)pthread_self());

        // 메시지큐로 전송
        if (msgsnd(msqid, &pmsg, size, 0) == -1)
        {
            thr_errquit("msgsnd fail", errno);
        }
    }

    return NULL;
}

// 에코 응답 스레드
void *echo_resp(void *arg)
{
    msg_t pmsg;
    int nbytes;
    socklen_t len;
    int size;

    size = sizeof(pmsg) - sizeof(long);

    while (1)
    {
        len = sizeof(struct sockaddr_in);

        // 메시지큐에서 읽음
        if (msgrcv(msqid, &pmsg, size, 0, 0) < 0)
        {
            perror("msgrcv fail");
            exit(0);
        }

        // 에코 응답
        nbytes = sendto(
            sock,
            pmsg.msg_text,
            strlen(pmsg.msg_text),
            0,
            (struct sockaddr *)&pmsg.addr,
            len
        );

        if (nbytes < 0)
        {
            thr_errquit("sendto fail", errno);
        }

        printf("response thread = %lu\n\n", (unsigned long)pthread_self());

        pmsg.msg_text[0] = '\0';
    }

    return NULL;
}
