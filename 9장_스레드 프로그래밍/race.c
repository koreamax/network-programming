//--------------------------------------------------------------
// 파일명 : race.c
// 기능   : 스레드 동기화 문제, 즉 경쟁 조건의 발생
//
// 컴파일 : gcc -o race race.c -pthread -D_REENTRANT
// 실행   : ./race
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_THR 2

void *thrfunc(void *arg);              // 스레드 시작 함수
void prn_data(unsigned long me);       // 스레드 ID 출력

unsigned long who_run = -1;            // prn_data() 수행 중인 스레드 ID
                                       // 초기값은 -1

int main(int argc, char **argv)
{
    pthread_t tid[MAX_THR];
    int i, status;

    for (i = 0; i < MAX_THR; i++)
    {
        if ((status = pthread_create(&tid[i], NULL, thrfunc, NULL)) != 0)
        {
            printf("thread create error: %s\n", strerror(status));
            exit(0);
        }
    }

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    return 0;
}

void *thrfunc(void *arg)
{
    while (1)
    {
        prn_data((unsigned long)pthread_self());
    }

    return NULL;
}

void prn_data(unsigned long me)
{
    who_run = me;

    if (who_run != (unsigned long)pthread_self())
    {
        printf("Error %lu 스레드 실행중 who_run = %lu\n", me, who_run);
    }

    who_run = -1;      // 초기값으로 환원
}
