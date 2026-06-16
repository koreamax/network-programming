//-----------------------------------
// 파일명  qsnd_pid.c
// 동  작  지정한 프로세스에게 메시지 전송
// 컴파일  gcc -o qsnd_pid qsnd_pid.c
// 실  행  ./qsnd_pid msgkey
//-----------------------------------

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSZ 512

// 메시지 내용 구조체 선언
typedef struct msg {
    long msg_type;
    char msg_text[BUFSZ];
} msg_t;

int main(int argc, char **argv)
{
    pid_t pid;
    int len, qid;
    ssize_t nbytes;

    // 전송할 메시지
    msg_t pmsg;

    // 메시지큐 key
    key_t key;

    if (argc != 2) {
        printf("Usage: %s msgkey\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    key = atoi(argv[1]);

    // 메시지큐 생성
    if ((qid = msgget(key, IPC_CREAT | 0600)) < 0) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    if ((pid = fork()) < 0) {
        printf("fork fail\n");
        exit(EXIT_FAILURE);
    }

    // 부모 프로세스
    if (pid > 0) {
        msg_t rmsg;

        printf("\n%d 프로세스 메시지큐 읽기 대기중...\n", getpid());

        nbytes = msgrcv(qid, &rmsg, BUFSZ, getpid(), 0);

        if (nbytes < 0) {
            perror("msgrcv fail");
            exit(EXIT_FAILURE);
        }

        printf("recv %ld bytes\n", nbytes);
        printf("type = %ld\n", rmsg.msg_type);
        printf("수신 프로세스 PID = %d\n", getpid());
        printf("value = %s\n", rmsg.msg_text);

        // 메시지큐 삭제
        msgctl(qid, IPC_RMID, 0);

        exit(EXIT_SUCCESS);
    }

    // 자식 프로세스
    puts("Enter message to post:");

    if (fgets(pmsg.msg_text, BUFSZ, stdin) == NULL) {
        puts("no message to post");
        exit(EXIT_FAILURE);
    }

    // 부모 프로세스의 PID가 메시지 타입
    pmsg.msg_type = getppid();

    len = strlen(pmsg.msg_text);

    // 메시지 전송
    if (msgsnd(qid, &pmsg, len, 0) < 0) {
        perror("msgsnd fail");
        exit(EXIT_FAILURE);
    }

    puts("message posted");

    return 0;
}
