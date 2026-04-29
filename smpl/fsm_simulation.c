#include <stdio.h>
#include "rand.c"

int main() {
    double sum = 0;
    int tries = 1000000;

    for (int i = 0; i < tries; i++) {
        int n = 1;
        int e = 2;

        while (e < 9) {
            switch (e) {
                case 1: e = 2; n++; break;
                case 2: e = (uniform(0,1) < 0.2) ? 3 : 5; n++; break;
                case 3: e = 4; n++; break;
                case 4: e = (uniform(0,1) < 0.25) ? 3 : 7; n++; break;
                case 5: e = (uniform(0,1) < 0.45) ? 6 : 7; n++; break;
                case 6: e = 7; n++; break;
                case 7: e = 8; n += 2; break;
                case 8: e = (uniform(0,1) < 0.3) ? 1 : 9; break;
            }
        }
        sum += n;
    }

    printf("%f\n", sum / tries);
}