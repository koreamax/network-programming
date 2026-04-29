#include <stdio.h>
#include "rand.c"

int main() {
    for (int i = 0; i < 10; i++) {
        printf("%f\n", exponential(0.1));
    }
}