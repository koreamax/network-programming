//--------------------------------------------------------------
// 파일명  : fork_test.c
// 기능    : fork() 시스템 콜 사용 예
// 컴파일  : gcc -o fork_test fork_test.c
// 사용법  : ./fork_test
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int global_var = 0;   // 전역 변수 선언

int main(void) {
    pid_t pid;
    int local_var = 0;   // 지역 변수 선언

    if ((pid = fork()) < 0) {
        printf("fork error\n");
        exit(1);
    }
    // 자식 프로세스
    else if (pid == 0) {
        global_var++;
        local_var++;

        printf("CHILD - my pid is %d and parent's pid is %d\n",
               getpid(), getppid());
    }
    // 부모 프로세스
    else {
        sleep(2);

        global_var += 5;
        local_var += 5;

        printf("PARENT - my pid is %d, child's pid is %d\n",
               getpid(), pid);
    }

    printf("\t global var : %d\n", global_var);
    printf("\t local var  : %d\n", local_var);

    return 0;
}