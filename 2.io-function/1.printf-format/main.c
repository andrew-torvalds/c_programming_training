#include <stdio.h>
#include <stdint.h>

int main(void) {
    int x = -1;
    printf("%d\n", x);  // số nguyên có dấu
    printf("%u\n", x);  // số nguyên không dấu

    uint8_t reg = 0xAB;
    printf("%x\n",   reg);   // ab       ← thường dùng khi debug register
    printf("%02X\n", reg);   // AB       ← dùng khi cần in đúng 2 ký tự, chữ hoa
    printf("%08X\n", reg);   // 000000AB ← dùng khi in địa chỉ bộ nhớ

    printf("%.2f\n", 3.14159);  // 3.14
    printf("%.4f\n", 3.14159);  // 3.1416
    printf("%8.2f\n", 3.14159); // "    3.14" — rộng 8, 2 chữ số thập phân

    float fl = 0.1f;
    printf("%.20f\n", fl);  // in 20 chữ số thập phân

    float a = 0.1f + 0.2f;
    float b = 0.3f;
    if (a == b) printf("bang nhau\n");
    else        printf("khac nhau\n");
    return 0;
}