#include <stdio.h>
#include "inttypes.h"

int main() {

    uint8_t test = 255;
    test = test << 1;
    uint32_t temp = (uint32_t) test;
    printf("%d\n", temp);
    return 0;
}
