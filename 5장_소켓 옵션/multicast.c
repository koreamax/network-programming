//--------------------------------------------------------------
// 파일명 : multicast.c
// 기능   : 멀티캐스트를 이용한 채팅 프로그램
// 컴파일 : gcc -o multicast multicast.c
// 사용법 : ./multicast 239.0.3.3 3000 MyName
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 1023

int main(int argc, char *argv[])
{
    int send_s, recv_s;          // 송신용, 수신용 소켓
    int n;
    socklen_t len;
    pid_t pid;
    int yes = 1;

    struct sockaddr_in mcast_group;   // 멀티캐스트 그룹 주소
    struct sockaddr_in local_addr;    // 바인드용 주소
    struct ip_mreq mreq;              // 멀티캐스트 가입 구조체

    char name[64];

    if (argc != 4) {
        printf("사용법 : %s multicast_address port MyName\n", argv[0]);
        exit(1);
    }

    snprintf(name, sizeof(name), "[%s]", argv[3]);

    //----------------------------------------------------------
    // 멀티캐스트 그룹 주소 설정
    //----------------------------------------------------------
    memset(&mcast_group, 0, sizeof(mcast_group));
    mcast_group.sin_family = AF_INET;
    mcast_group.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &mcast_group.sin_addr) <= 0) {
        perror("inet_pton fail");
        exit(1);
    }

    //----------------------------------------------------------
    // 멀티캐스트 수신용 소켓 생성
    //----------------------------------------------------------
    recv_s = socket(AF_INET, SOCK_DGRAM, 0);
    if (recv_s < 0) {
        perror("receive socket fail");
        exit(1);
    }

    //----------------------------------------------------------
    // 같은 포트를 여러 프로세스가 사용할 수 있도록 설정
    //----------------------------------------------------------
    if (setsockopt(recv_s, SOL_SOCKET, SO_REUSEADDR,
                   &yes, sizeof(yes)) < 0) {
        perror("SO_REUSEADDR fail");
        exit(1);
    }

#ifdef SO_REUSEPORT
    setsockopt(recv_s, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));
#endif

    //----------------------------------------------------------
    // 수신 소켓 바인드
    // 멀티캐스트 수신은 보통 INADDR_ANY에 바인드한다
    //----------------------------------------------------------
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(atoi(argv[2]));

    if (bind(recv_s, (struct sockaddr *)&local_addr,
             sizeof(local_addr)) < 0) {
        perror("bind receive socket fail");
        exit(1);
    }

    //----------------------------------------------------------
    // 멀티캐스트 그룹 가입
    //----------------------------------------------------------
    mreq.imr_multiaddr = mcast_group.sin_addr;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(recv_s, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                   &mreq, sizeof(mreq)) < 0) {
        perror("IP_ADD_MEMBERSHIP fail");
        exit(1);
    }

    //----------------------------------------------------------
    // 멀티캐스트 메시지 송신용 소켓 생성
    //----------------------------------------------------------
    send_s = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_s < 0) {
        perror("send socket fail");
        exit(1);
    }

    //----------------------------------------------------------
    // fork()
    // child  : 메시지 수신 담당
    // parent : 키보드 입력 후 메시지 송신 담당
    //----------------------------------------------------------
    pid = fork();

    if (pid < 0) {
        perror("fork fail");
        exit(1);
    }

    // child process : 채팅 메시지 수신 담당
    if (pid == 0) {
        struct sockaddr_in from;
        char message[MAXLINE + 1];

        while (1) {
            len = sizeof(from);

            n = recvfrom(recv_s, message, MAXLINE, 0,
                         (struct sockaddr *)&from, &len);

            if (n < 0) {
                perror("recvfrom fail");
                exit(1);
            }

            message[n] = '\0';

            printf("\nReceived Message : %s", message);
            printf("Send Message : ");
            fflush(stdout);
        }
    }

    // parent process : 키보드 입력 및 메시지 송신 담당
    else {
        char message[MAXLINE + 1];
        char line[MAXLINE + 1];

        printf("Send Message : ");
        fflush(stdout);

        while (fgets(message, MAXLINE, stdin) != NULL) {
            snprintf(line, sizeof(line), "%s %s", name, message);

            len = strlen(line);

            if (sendto(send_s, line, len, 0,
                       (struct sockaddr *)&mcast_group,
                       sizeof(mcast_group)) < 0) {
                perror("sendto fail");
            }

            printf("Send Message : ");
            fflush(stdout);
        }
    }

    close(send_s);
    close(recv_s);

    return 0;
}
