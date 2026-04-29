#include <stdio.h>
#include "rand.c"

int main() {
    int Te = 200;
    int t1 = 0, t2 = Te, time = 0;
    int n = 0;
    int cnt_arri = 0, cnt_off = 0;
    int res_time = 0;
    double mean_time = 0;

    double Pa = 0.1, Ps = 0.1;

    while (time < Te) {
        int next_time = (t1 < t2) ? t1 : t2;
        mean_time += n * (next_time - time);

        if (t1 < t2) {
            time = t1;
            n++; cnt_arri++;
            t1 = time + geometric(Pa);

            if (n == 1) {
                cnt_off++;
                t2 = time + geometric(Ps);
                res_time += t2 - time;
            }
        } else {
            time = t2;
            n--;

            if (n > 0) {
                cnt_off++;
                t2 = time + geometric(Ps);
                res_time += t2 - time;
            } else {
                t2 = Te;
            }
        }
    }

    printf("throughput = %f\n", (double)cnt_arri / Te);
    printf("utilization = %f\n", (double)res_time / Te);
    printf("mean no. in system = %f\n", mean_time / Te);
    printf("mean residence time = %f\n", mean_time / cnt_arri);
}