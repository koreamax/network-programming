#include <stdio.h>
#include "rand.c"

void rsp(real a) {
    if (a < 1.0/3.0) printf("Win\n");
    else if (a < 2.0/3.0) printf("Draw\n");
    else printf("Lose\n");
}

int main() {
    for (int i = 0; i < 10; i++) {
        rsp(ranf());
    }
}