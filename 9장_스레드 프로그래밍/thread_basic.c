//--------------------------------------------------------------
// 파일명 : thread_basic.c
// 동작   : 스레드 생성과 스레드 ID, PID 확인
//
// 컴파일 : gcc -o thread_basic thread_basic.c -pthread -D_REENTRANT
// 실행   : ./thread_basic
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

char who[10];

void *thrfunc(void *arg);   // 스레드 시작 함수

int main(int argc, char **argv)
{
    int status;
    pthread_t tid;
    pid_t pid;

    // 자식 프로세스 생성
    pid = fork();

    if (pid == 0)
    {
        sprintf(who, "child");
    }
    else
    {
        sprintf(who, "parent");
    }

    // 프로세스 ID와 초기 스레드 ID 확인
    printf("(%s's main) Process ID = %d\n", who, getpid());
    printf("(%s's main) Init thread ID = %lu\n", who, (unsigned long)pthread_self());

    // 스레드 생성
    status = pthread_create(&tid, NULL, thrfunc, NULL);

    if (status != 0)
    {
        printf("thread create error: %s\n", strerror(status));
        exit(0);
    }

    // 인자로 지정한 스레드 ID가 종료하기를 기다림
    pthread_join(tid, NULL);

    printf("\n(%s) [%lu] 스레드가 종료했습니다.\n", who, (unsigned long)tid);

    return 0;
}

void *thrfunc(void *arg)
{
    printf("(%s's thread routine) Process ID = %d\n", who, getpid());
    printf("(%s's thread routine) Thread ID = %lu\n", who, (unsigned long)pthread_self());

    return NULL;
}
