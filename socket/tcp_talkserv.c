//--------------------------------------------------------------
// 파일명  : tcp_talkserv.c
// 기능    : 토크 클라이언트와 1:1 통신을 한다
// 컴파일  : gcc -o tcp_talkserv tcp_talkserv.c
// 사용법  : ./tcp_talkserv 3000
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

char *EXIT_STRING = "exit";   // 종료 문자열

int recv_and_print(int sd);   // 상대로부터 메시지 수신 후 화면 출력
int input_and_send(int sd);   // 키보드 입력받고 상대에게 메시지 전달

int main(int argc, char *argv[]) {
    struct sockaddr_in cliaddr, servaddr;
    int listen_sock, accp_sock;
    socklen_t addrlen;
    pid_t pid;

    if (argc != 2) {
        printf("사용법: %s port\n", argv[0]);
        exit(1);
    }

    // 소켓 생성
    if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fail");
        exit(1);
    }

    // 서버 주소 구조체 초기화
    memset(&servaddr, 0, sizeof(servaddr));

    // 서버 주소 설정
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    // bind() 호출
    if (bind(listen_sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind fail");
        exit(1);
    }

    puts("서버가 클라이언트를 기다리고 있습니다.");

    // 연결 대기
    if (listen(listen_sock, 1) < 0) {
        perror("listen fail");
        exit(1);
    }

    // 클라이언트 연결 요청 수락
    addrlen = sizeof(cliaddr);
    if ((accp_sock = accept(listen_sock,
                            (struct sockaddr *)&cliaddr,
                            &addrlen)) < 0) {
        perror("accept fail");
        exit(1);
    }

    puts("클라이언트가 연결되었습니다.");

    pid = fork();

    if (pid > 0) {
        // 부모 프로세스: 키보드 입력 → 클라이언트로 전송
        input_and_send(accp_sock);
    }
    else if (pid == 0) {
        // 자식 프로세스: 클라이언트 메시지 수신 → 화면 출력
        recv_and_print(accp_sock);
    }
    else {
        perror("fork fail");
        exit(1);
    }

    close(listen_sock);
    close(accp_sock);

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

        // 종료 문자열 입력 처리
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

        // 종료 문자열 수신 시 종료
        if (strstr(buf, EXIT_STRING) != NULL) {
            puts("상대방이 종료했습니다.");
            break;
        }

        printf("%s", buf);
    }

    return 0;
}