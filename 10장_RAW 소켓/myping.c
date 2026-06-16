//--------------------------------------------------------------
// 파일명 : myping.c
// 동작   : 간단한 ping 프로그램
//
// 컴파일 : gcc -o myping myping.c
// 실행   : sudo ./myping ip_addr
// 예시   : sudo ./myping 8.8.8.8
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>

#define BUFSIZE 4096

int seqnum = 0;                 // ping 메시지 일련번호
char recvbuf[BUFSIZE];          // 수신 버퍼
char sendbuf[BUFSIZE];          // 송신 버퍼
int rawsock;                    // Raw 소켓 번호
int notrecv = 0;                // ping 응답을 받지 못한 횟수

struct sockaddr_in sendaddr, recvaddr;

int send_ping(void);
int prn_rcvping(char *ipdata, int recvsize);
void prn_icmp(struct icmphdr *icmp, int icmpsize);
unsigned short in_cksum(unsigned short *addr, int len);

void errquit(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char **argv)
{
    int recvsize;
    socklen_t addrlen;
    fd_set readset;
    struct timeval tv;
    int ret;

    if (argc != 2)
    {
        printf("Usage : %s ip_address\n", argv[0]);
        exit(1);
    }

    addrlen = sizeof(struct sockaddr_in);

    memset(&recvaddr, 0, sizeof(recvaddr));
    memset(&sendaddr, 0, sizeof(sendaddr));

    sendaddr.sin_family = AF_INET;
    sendaddr.sin_port = htons(0);

    if (inet_pton(AF_INET, argv[1], &sendaddr.sin_addr.s_addr) <= 0)
    {
        printf("Invalid IP address: %s\n", argv[1]);
        exit(1);
    }

    // raw 소켓 생성
    rawsock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (rawsock < 0)
    {
        errquit("socket fail");
    }

    // 커널에 상대의 주소를 기억해둠
    if (connect(rawsock, (struct sockaddr *)&sendaddr, sizeof(sendaddr)) != 0)
    {
        errquit("connect fail");
    }

    while (1)
    {
        FD_ZERO(&readset);
        FD_SET(rawsock, &readset);

        tv.tv_sec = 1;      // 1초 타이머
        tv.tv_usec = 0;

        // ping request 전송
        send_ping();

        // 응답 대기
        ret = select(rawsock + 1, &readset, NULL, NULL, &tv);

        if (ret == 0)
        {
            // 타임아웃
            if (++notrecv == 3)
            {
                notrecv = 0;
                puts("Request Timeout ...");
            }

            sleep(1);
            continue;
        }
        else if (ret < 0)
        {
            errquit("select fail");
        }

        // select() 정상 리턴, ping 응답 읽기
        recvsize = recvfrom(
            rawsock,
            recvbuf,
            sizeof(recvbuf),
            0,
            (struct sockaddr *)&recvaddr,
            &addrlen
        );

        if (recvsize < 0)
        {
            errquit("recvfrom fail");
        }

        notrecv = 0;

        // 수신된 응답 처리
        prn_rcvping(recvbuf, recvsize);

        sleep(1);   // 1초 간격으로 ping 전송
    }

    close(rawsock);

    return 0;
}

// ICMP 헤더 출력
void prn_icmp(struct icmphdr *icmp, int icmpsize)
{
    printf("[icmp] ");
    printf("(id: %d ", ntohs(icmp->un.echo.id));
    printf("seq: %d ", ntohs(icmp->un.echo.sequence));
    printf("code: %d ", icmp->code);
    printf("type: %d)\n", icmp->type);
}

// 수신된 메시지 출력
int prn_rcvping(char *ipdata, int recvsize)
{
    int ip_headlen;
    int icmp_len;

    struct icmphdr *icmp;
    struct iphdr *ip;
    char buf[512];

    ip = (struct iphdr *)ipdata;

    ip_headlen = ip->ihl * 4;
    icmp_len = recvsize - ip_headlen;

    icmp = (struct icmphdr *)(ipdata + ip_headlen);

    if (icmp->type != ICMP_ECHOREPLY)
    {
        return -1;
    }

    if (ntohs(icmp->un.echo.id) != (getpid() & 0xffff))
    {
        return -1;
    }

    inet_ntop(AF_INET, (void *)&ip->saddr, buf, sizeof(buf));

    printf("%d bytes recv from (%s) ", icmp_len, buf);

    prn_icmp(icmp, icmp_len);

    return 0;
}

// ping request 보내기
int send_ping(void)
{
    struct icmphdr *icmp;
    int len;
    int sendsize;

    icmp = (struct icmphdr *)sendbuf;

    memset(icmp, 0, sizeof(struct icmphdr));

    icmp->code = 0;
    icmp->type = ICMP_ECHO;                         // ICMP ECHO = 8
    icmp->un.echo.sequence = htons(seqnum++);       // Ping 메시지 일련번호
    icmp->un.echo.id = htons(getpid() & 0xffff);    // pid를 ID로 설정

    icmp->checksum = 0;
    icmp->checksum = in_cksum((unsigned short *)icmp, sizeof(struct icmphdr));

    len = sizeof(struct icmphdr);   // ICMP 헤더 8byte

    sendsize = sendto(
        rawsock,
        sendbuf,
        len,
        0,
        (struct sockaddr *)&sendaddr,
        sizeof(sendaddr)
    );

    if (sendsize < 0)
    {
        errquit("sendto fail");
    }

    prn_icmp(icmp, sendsize);

    return sendsize;
}

// ICMP checksum 구하기
unsigned short in_cksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    answer = ~sum;

    return answer;
}
