//--------------------------------------------------------------
// 파일명  : tcp_talkcli.c
// 기능    : 토크 서버와 1:1 통신을 하는 클라이언트 프로그램
// 컴파일  : gcc -o tcp_talkcli tcp_talkcli.c
// 사용법  : ./tcp_talkcli 127.0.0.1 3000
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLINE 511

char *EXIT_STRING = "exit";

int recv_and_print(int sd);
int input_and_send(int sd);

int main(int argc, char *argv[]) {
    pid_t pid;
    int s;
    struct sockaddr_in servaddr;

    if (argc != 3) {
        printf("사용법 : %s server_ip port\n", argv[0]);
        exit(1);
    }

    // 소켓 생성
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Client: Can't open stream socket.\n");
        exit(1);
    }

    // 서버 주소 설정
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        perror("inet_pton fail");
        exit(1);
    }

    // 서버에 연결 요청
    if (connect(s, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        printf("Client: can't connect to server.\n");
        exit(1);
    }

    pid = fork();

    if (pid > 0) {
        // 부모 프로세스: 키보드 입력 → 서버로 전송
        input_and_send(s);
    }
    else if (pid == 0) {
        // 자식 프로세스: 서버 메시지 수신 → 화면 출력
        recv_and_print(s);
    }
    else {
        perror("fork fail");
        exit(1);
    }

    close(s);
    return 0;
}

// 키보드 입력받고 상대에게 메시지 전달
int input_and_send(int sd) {
    char buf[MAXLINE + 1];
    int nbyte;

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        nbyte = strlen(buf);

        if (write(sd, buf, nbyte) < 0) {
            perror("write fail");
            close(sd);
            exit(1);
        }

        if (strstr(buf, EXIT_STRING) != NULL) {
            puts("Good bye.");
            close(sd);
            exit(0);
        }
    }

    return 0;
}

// 상대로부터 메시지 수신 후 화면 출력
int recv_and_print(int sd) {
    char buf[MAXLINE + 1];
    int nbyte;

    while (1) {
        if ((nbyte = read(sd, buf, MAXLINE)) < 0) {
            perror("read fail");
            close(sd);
            exit(1);
        }

        if (nbyte == 0) {
            puts("상대방이 연결을 종료했습니다.");
            close(sd);
            exit(0);
        }

        buf[nbyte] = '\0';

        if (strstr(buf, EXIT_STRING) != NULL) {
            puts("상대방이 종료했습니다.");
            break;
        }

        printf("%s", buf);
    }

    return 0;
}