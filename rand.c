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
    for (int i = 0; i < 10; i++) {
        printf("%f ", uniform(0.9, 1.5));
    }
    */
    /*
    for (int i = 0; i < 10; i++) {
        printf("%d ", geometric(0.1));
    }
    */
    for (int i = 0; i < 10; i++) {
        printf("%f ", exponential(0.1));
    }
} 
/* 
전부 다 한 줄 짜리로 짜기 
random(n, k){복권}, geometric(double p){기하분포}, exponential(lambda){지수분포}
-> 이렇게 짜기
pdf, pmf, PDF의 관계를 생각하면 한 줄로 짤 수 있음 
unifrom = 1 / b - a
*/