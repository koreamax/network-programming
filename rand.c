#include <stdio.h>
#include <math.h>

// 옛날에는 cpu마다 코드가 달랐기 때문에 이렇게 정의하고 시작
// 지금은 안 써도 무방
#define CPU 8086

// 예전에 수치 계산 / 시뮬레이션 코드에서 많이 쓰던 방식
// double을 float이나 다른 타입으로 쉽게 바꿔서 성능 평가하기 위해
typedef double real;

#define A 16807L // 16807L = (long)16807과 같은 의미
#define M 2147483647L

static long In[16] = {0L,
    1973272912L, 747177549L, 20464843L, 640830765L, 1098742207L,
    78126602L, 84743774L, 831312807L, 124667236L, 1172177002L,
    1124933064L, 1223960546L, 1878892440L, 1449793615L, 553303732L
};

static int strm = 1;

void error(int a, char* b)
{
    printf("%d -> %s\n", a, b);
}

real ranf()
{
    short *p, *q, k;
    long Hi, Lo;

    p = (short *)&In[strm];
    Hi = *(p + 1) * A;
    *(p + 1) = 0;
    Lo = In[strm] * A;

    p = (short *)&Lo;
    Hi += *(p + 1);

    q = (short *)&Hi;
    *(p + 1) = *q & 0x7FFF;

    k = (*(q + 1) << 1);
    if (*q & 0x8000) {
        k++;
    }

    Lo -= M;
    Lo += k;

    if (Lo < 0) {
        Lo += M;
    }

    In[strm] = Lo;

    return (real)Lo * 4.656612875E-10;
}

int stream(int n)
{
    if (n < 0 || n > 15) {
        error(0, "stream Argument Error");
    }
    if (n) {
        strm = n;
    }
    return strm;
}

long seed(long Ik, int n)
{
    if (n < 1 || n > 15) {
        error(0, "seed Argument Error");
    }
    if (Ik > 0L) {
        In[n] = Ik;
    }
    return In[n];
}

real uniform(real a, real b)
{
    if (a > b) {
        error(0, "uniform Argument Error: a > b");
    }

    return a + (b - a) * ranf();
}

void rsp(real a)
{
    if (a < 1.0/3.0) { // a의 타입이 double이라 소수점으로 표시
        printf("Win\n");
    }
    else if (a < 2.0/3.0) {
        printf("Draw\n");
    }
    else {
        printf("Lose\n");
    }
}

real random(int n, int k) {
    return n - 1 + (k - n + 1) * ranf();
}

int geometric(real p) {
    return 1 + (int)(log(1 - ranf()) / log(1 - p));
}

real exponential(real lambda) {
    return (-1) * (log(1 - ranf()) / lambda);
}

int main()
{
    /*
    // 난수 10번 불러오기
    for(int i = 0; i < 10; i++) {
        printf("%f\n", ranf());
    }
    */
    /*
    //가위, 바위, 보
    for(int i = 0; i < 10; i++) {
        rsp(ranf());
    }
    */
    /*
    // 균등분포
    for (int i = 0; i < 10; i++) {
        printf("%f ", uniform(0.9, 1.5));
    }
    */
    /*
    // 기하분포
    for (int i = 0; i < 10; i++) {
        printf("%d ", geometric(0.1));
    }
    */
    /*
    // 지수분포
    for (int i = 0; i < 10; i++) {
        printf("%f ", exponential(0.1));
    }
    */

    // 고객이 랜덤하게 도착하고 은행원이 한 명씩 처리하는 시스템

    int Te = 10000000; // 은행 전체 시간
    int t1, t2, time; // 다음 고객 도착 시간, 현재 서비스 종료 시간, 현재 시간
    int Ta, Ts;
    double Pa = 0.1, Ps = 0.1; // 도착 확률, 서비스 확률
    int n; // 은행에 있는 고객 수
    int cnt_arri=0, cnt_off=0; // 도착한 고객 수, 서비스 시작한 고객 수
    int res_time = 0; // 고객들이 거주하고 있는 시간
    double mean_time = 0; // mean number의 적분
    int next_time; // 다음 시간이 t1인지 t2인지 예측

    n = 0; t1 = 0; t2 = Te; time = 0;
    while (time < Te) { // 은행이 문 닫을 때까지 
        next_time = (t1 < t2) ? t1 : t2; // t1, t2 중 하나
        mean_time += (double)n * (next_time - time); // 적분
        if (t1 < t2) { // 서비스가 진행되고 있는 중에 고객이 도착
            time = t1; // 현재 시간은 고객이 도착한 시간
            n++; // 은행에 있는 고객 추가
            cnt_arri++; // 도착한 고객 추가
            // printf("at time %d, customer %d arrive\n", time, cnt_arri); 
            t1 = time + geometric(Pa); // 다음 고객 도착 시간 랜덤
            if (n == 1) { // 은행에 고객이 하나라면 바로 서비스 시작
                cnt_off++; // 서비스 고객 수 추가
                // printf("at time %d, customer %d begin\n", time, cnt_off);
                t2 = time + geometric(Ps); // 서비스 종료 시간 랜덤
                res_time += t2 - t1; // 고객 상주 시간 추가
            }
        }
        else { // 서비스가 끝났을 때
            time = t2; n--; // 현재 시간을 끝난 시간으로 맞추고 남아 있는 은행 고객 한 명 줄이기
            // printf("at time %d, customer %d end\n", time, cnt_off);
            if (n > 0) { // 끝나고도 고객이 남아있다면
                cnt_off++; // 서비스 시작
                // printf("at time %d, customer %d begin\n", time, cnt_off);
                t2 = time + geometric(Ps); // 끝나는 시간 랜덤
                res_time += t2 - time; // 고객 상주 시간 추가
            }
            else {
                t2 = Te; // 고객이 더 안 들어온다면 은행 종료
            }
        }
    }
    printf("throughput = %f\n", (double)cnt_arri / Te);
    printf("utilization = %f\n", (double)res_time / Te);
    printf("mean no. in system = %f\n", mean_time / Te);
    printf("mean residence time = %f", mean_time / cnt_arri);
}