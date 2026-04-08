#include <stdio.h>
#include <stdint.h>
int main() {

    uint8_t i = 256;
    printf("%d\n\n", i);
    int giang_truong = 0;
    scanf("%d", &giang_truong);
    printf("%d\n", sizeof(int));
    printf("Hello, World! %d\n", giang_truong);

    uint8_t x = 200;

if (x > -1) {
    printf("x lớn hơn -1\n");
} else {
    printf("x không lớn hơn -1\n");
}

int a= 5;
printf("%d %d\n", a++, ++a);
    return 0;
}
