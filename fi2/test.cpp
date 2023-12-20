#include <stdio.h>

#define VALUE 42

#define INDIRECT_REFERENCE(x) x

int main() {
    int value = INDIRECT_REFERENCE(VALUE);
    printf("The value is: %d\n", value);

    return 0;
}