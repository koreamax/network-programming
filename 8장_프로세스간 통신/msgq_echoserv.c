//-------------------------------------
// 파일명  msgq_echoserv.c
// 동  작  에코 메시지를 수신하여 메시지큐에 넣어줌
// 컴파일  gcc -o msgq_echoserv msgq_echoserv.c
// 실  행  ./msgq_echoserv msgq_key port
//-------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_BUFSZ 512

// res_send 실행 파일 이름
// res_send가 현재 디렉터리에 있으면 "./res_send"로 해야 안전함
#define RES_SEND_PROC "./res_send"

typedef struct msg {
    long msg_type;
    struct sockaddr_in addr;
    char msg_text[MAX_BUFSZ];
} msg_t;

void fork_and_exec(char *key, char *port);

void errquit(char *mesg)
{
    perror(mesg);
    exit(1);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in servaddr;
    msg_t pmsg;
    key_t key;

    int msqid;
    int size;
    int nbytes;
    int sock;
    int port;

    socklen_t len;

    if (argc != 3) {
        printf("Usage: %s msgq_key port\n", argv[0]);
        exit(1);
    }

    key = atoi(argv[1]);
    port = atoi(argv[2]);

    // 메시지큐 생성
    msqid = msgget(key, IPC_CREAT | 0600);
    if (msqid == -1) {
        errquit("msgget fail");
    }

    // UDP 소켓 생성
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        errquit("socket fail");
    }

    // 서버 주소 설정
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // 소켓 바인딩
    if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        errquit("bind fail");
    }

    // 응답 전송 프로세스 생성
    fork_and_exec(argv[1], argv[2]);
    fork_and_exec(argv[1], argv[2]);
    fork_and_exec(argv[1], argv[2]);

    pmsg.msg_type = 1;

    // msgsnd()의 size는 long msg_type을 제외한 크기
    size = sizeof(msg_t) - sizeof(long);

    puts("Server starting ----");

    while (1) {
        len = sizeof(pmsg.addr);

        // UDP 클라이언트로부터 메시지 수신
        nbytes = recvfrom(
            sock,
            pmsg.msg_text,
            MAX_BUFSZ - 1,
            0,
            (struct sockaddr *)&pmsg.addr,
            &len
        );

        if (nbytes < 0) {
            perror("recvfrom fail");
            continue;
        }

        pmsg.msg_text[nbytes] = '\0';

        // 메시지큐에 쓰기
        if (msgsnd(msqid, &pmsg, size, 0) == -1) {
            errquit("msgsnd fail");
        }
    }

    return 0;
}

void fork_and_exec(char *key, char *port)
{
    pid_t pid = fork();

    if (pid < 0) {
        errquit("fork fail");
    } else if (pid > 0) {
        return;
    }

    // 자식 프로세스는 res_send 실행
    execlp(RES_SEND_PROC, RES_SEND_PROC, key, port, NULL);

    perror("execlp fail");
    exit(1);
}
