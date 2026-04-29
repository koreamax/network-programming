#include <stdio.h>
#include <math.h>

typedef double real;

#define A 16807L
#define M 2147483647L

static long In[16] = {
    0L, 1973272912L, 747177549L, 20464843L,
    640830765L, 1098742207L, 78126602L, 84743774L,
    831312807L, 124667236L, 1172177002L, 1124933064L,
    1223960546L, 1878892440L, 1449793615L, 553303732L
};

static int strm = 1;

real ranf() {
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
    if (*q & 0x8000) k++;

    Lo -= M;
    Lo += k;
    if (Lo < 0) Lo += M;

    In[strm] = Lo;
    return (real)Lo * 4.656612875E-10;
}

real uniform(real a, real b) {
    return a + (b - a) * ranf();
}

int geometric(real p) {
    return 1 + (int)(log(1 - ranf()) / log(1 - p));
}

real exponential(real lambda) {
    return (-1) * (log(1 - ranf()) / lambda);
}