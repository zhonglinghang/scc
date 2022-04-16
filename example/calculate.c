#include <stdio.h>

int minus(int a, int b) {
    int res;
    res = a - b;
    return res;
}

int add(int a, int b) {
    int res;
    res = a + b;
    return res;
}

int main() {
    printf("%d + %d = %d\n", 1, 2, add(1, 2));
    printf("%d - %d = %d\n", 1, 2, minus(1, 2));
    return 0;
}