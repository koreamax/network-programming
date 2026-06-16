//--------------------------------------------------------------
// 파일명 : cond.c
// 동작   : 조건변수를 통해서 두 스레드 간에 통신하는 예
//
// 컴파일 : gcc -o cond cond.c -pthread -D_REENTRANT
// 실행   : ./cond 5
//          ./cond 1
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

// 뮤텍스, 조건변수 및 공유데이터
typedef struct complex
{
    pthread_mutex_t mutex;   // 뮤텍스
    pthread_cond_t cond;     // 조건변수
    int value;               // 공유데이터
} thread_control_t;

// 뮤텍스, 조건변수, 공유데이터의 초기화
thread_control_t data = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_COND_INITIALIZER,
    0
};

// 자식 스레드가 처음 잠자는 시간
int sleep_time;

// 에러 출력 및 스레드 종료
void thr_errquit(char *msg, int errcode)
{
    printf("%s : %s\n", msg, strerror(errcode));
    pthread_exit(NULL);
}

// 스레드 시작 함수
void *wait_thread(void *arg);

int main(int argc, char **argv)
{
    int status;
    pthread_t wait_thr;
    struct timespec timeout;

    if (argc != 2)
    {
        printf("사용법 : ./cond 5  (자식 스레드가 sleep할 시간)\n");
        exit(0);
    }

    sleep_time = atoi(argv[1]);

    if ((status = pthread_create(&wait_thr, NULL, wait_thread, NULL)) != 0)
    {
        printf("pthread_create fail : %s\n", strerror(status));
        exit(1);
    }

    timeout.tv_sec = time(NULL) + 3;   // 현재 시간 + 3초
    timeout.tv_nsec = 0;

    // 뮤텍스 잠금
    if ((status = pthread_mutex_lock(&data.mutex)) != 0)
    {
        printf("pthread_mutex_lock fail : %s\n", strerror(status));
        exit(1);
    }

    if (data.value == 0)
    {
        status = pthread_cond_timedwait(&data.cond, &data.mutex, &timeout);

        if (status == ETIMEDOUT)
        {
            printf("Condition wait time out\n");
        }
        else if (status != 0)
        {
            printf("pthread_cond_timedwait fail : %s\n", strerror(status));
        }
        else
        {
            printf("Wait on Condition .....\n");
        }
    }

    // data.value 값이 자식 스레드에서 1로 바뀜
    if (data.value == 1)
    {
        printf("Condition was signaled.\n");
    }

    if ((status = pthread_mutex_unlock(&data.mutex)) != 0)
    {
        printf("pthread_mutex_unlock fail : %s\n", strerror(status));
        exit(0);
    }

    // 자식 스레드가 종료하기를 기다림
    if ((status = pthread_join(wait_thr, NULL)) != 0)
    {
        printf("pthread_join fail : %s\n", strerror(status));
        exit(0);
    }

    return 0;
}

// 스레드 시작 함수
void *wait_thread(void *arg)
{
    int status;

    sleep(sleep_time);

    if ((status = pthread_mutex_lock(&data.mutex)) != 0)
    {
        thr_errquit("pthread_mutex_lock failure", status);
    }

    data.value = 1;

    if ((status = pthread_cond_signal(&data.cond)) != 0)
    {
        thr_errquit("pthread_cond_signal failure", status);
    }

    if ((status = pthread_mutex_unlock(&data.mutex)) != 0)
    {
        thr_errquit("pthread_mutex_unlock failure", status);
    }

    return NULL;
}
