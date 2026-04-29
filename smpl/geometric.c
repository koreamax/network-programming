#include <stdio.h>
#include "rand.c"

int main() {
    for (int i = 0; i < 10; i++) {
        printf("%d\n", geometric(0.1));
    }
}