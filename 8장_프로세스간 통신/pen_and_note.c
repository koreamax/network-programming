//--------------------------------------------------------------
// 파일명  pen_and_note.c
// 동  작  공유데이터의 경쟁적 접근을 세마포어로 제어
// 컴파일  gcc -o pen_and_note pen_and_note.c
// 실  행  ./pen_and_note
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

void errquit(char *mesg)
{
    perror(mesg);
    exit(1);
}

#define PEN  0
#define NOTE 1

// 세마포어 조작 구조체
struct sembuf increase[] = {
    {PEN,  +1, SEM_UNDO},
    {NOTE, +1, SEM_UNDO}
};

struct sembuf decrease[] = {
    {PEN,  -1, SEM_UNDO},
    {NOTE, -1, SEM_UNDO}
};

// 세마포어 초기값: 연필 1자루, 노트 2권
unsigned short seminitval[] = {1, 2};

// semctl()에서 사용하는 union
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
    struct seminfo *__buf;
};

int semid;   // 세마포어 ID

void do_work(void);

#define Semop(val) \
    if (semop val == -1) errquit("semop")

int main(int argc, char *argv[])
{
    pid_t parent_pid;

    // 세마포어 2개 생성
    // PEN 세마포어, NOTE 세마포어
    semid = semget(0x1234, 2, IPC_CREAT | 0600);
    if (semid == -1) {
        errquit("semget");
    }

    // 세마포어 값 초기화
    union semun semarg;
    semarg.array = seminitval;

    if (semctl(semid, 0, SETALL, semarg) == -1) {
        errquit("semctl");
    }

    // 표준 출력 non-buffering
    setvbuf(stdout, NULL, _IONBF, 0);

    parent_pid = getpid();

    // 총 4개의 프로세스를 만듦
    fork();
    fork();

    do_work();

    // 원래 부모 프로세스만 자식들을 기다리고 세마포어 삭제
    if (getpid() == parent_pid) {
        while (wait(NULL) > 0)
            ;

        semctl(semid, 0, IPC_RMID, 0);  // 세마포어 삭제
    }

    return 0;
}

void do_work(void)
{
    int count = 0;

    while (count < 3) {
        // 연필 획득
        Semop((semid, &decrease[PEN], 1));
        printf("[pid:%5d] 연필을 들고\n", getpid());

        // 노트 획득
        Semop((semid, &decrease[NOTE], 1));
        printf("\t[pid:%5d] 노트를 들고\n", getpid());

        // 공부
        printf("\t[pid:%5d] 공부를 함\n", getpid());

        // 연필 반납
        Semop((semid, &increase[PEN], 1));
        printf("\t[pid:%5d] 연필을 내려놓음\n", getpid());

        // 노트 반납
        Semop((semid, &increase[NOTE], 1));
        printf("\t[pid:%5d] 노트를 내려놓음\n", getpid());

        sleep(1);
        count++;
    }
}
