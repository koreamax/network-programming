//--------------------------------------------------------------
// 파일명  shmbusyaccess.c
// 동  작  공유메모리에서 동기화 문제를 보임
// 컴파일  gcc -o shmbusyaccess shmbusyaccess.c
// 실  행  ./shmbusyaccess shmkey
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>

void errquit(char *msg)
{
    perror(msg);
    exit(1);
}

// 자식 프로세스를 생성하여 busy() 함수를 호출하도록 함
void fork_and_run(void);

// 각 프로세스가 공유메모리에 경쟁적으로 접근하는 함수
void busy(void);

// 공유메모리에 접근하는 함수
void access_shm(int count);

char *shm_data;   // 공유메모리 포인터
int shmid;        // 공유메모리 ID

int main(int argc, char *argv[])
{
    key_t shmkey;   // 공유메모리 키

    if (argc < 2) {
        printf("Usage: %s shmkey\n", argv[0]);
        exit(1);
    }

    shmkey = atoi(argv[1]);

    // 공유메모리 생성
    shmid = shmget(shmkey, 128, IPC_CREAT | 0660);
    if (shmid < 0) {
        errquit("shmget fail");
    }

    // 공유메모리 연결
    shm_data = (char *)shmat(shmid, (void *)0, 0);
    if (shm_data == (char *)-1) {
        errquit("shmat fail");
    }

    // 자식 프로세스 2개 생성
    fork_and_run();
    fork_and_run();

    // 부모 프로세스도 busy() 수행
    busy();

    // 자식 프로세스 2개 종료 대기
    wait(NULL);
    wait(NULL);

    // 공유메모리 제거
    shmctl(shmid, IPC_RMID, 0);

    return 0;
}

// 자식 프로세스 생성 및 busy() 수행
void fork_and_run(void)
{
    pid_t pid = fork();

    if (pid < 0) {
        errquit("fork fail");
    } else if (pid == 0) {
        busy();
        exit(0);
    }

    return;
}

// 각 프로세스가 수행하는 함수
void busy(void)
{
    int i = 0;

    for (i = 0; i < 500000; i++) {
        access_shm(i);
    }

    // 공유메모리 분리
    shmdt(shm_data);
}

// 공유메모리에 접근하는 부분
void access_shm(int count)
{
    int i;
    pid_t pid;

    // 공유메모리에 자신의 PID 기록
    sprintf(shm_data, "%d", getpid());

    // 공유메모리 접근 시간에 포함되는 지연
    for (i = 0; i < 1000; i++)
        ;

    // 공유메모리에 기록된 PID 읽기
    pid = atoi(shm_data);

    // 공유메모리에 기록한 PID가 자신의 PID가 아니면 Error
    if (pid != getpid()) {
        printf("Error(count=%d) 다른 프로세스도 동시에 공유메모리 접근함\n", count);
    } else {
        // 정상이며 아무 출력도 하지 않음
    }

    return;
}
