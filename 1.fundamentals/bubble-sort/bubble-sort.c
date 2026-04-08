#include <stdio.h>
#include <stdint.h>

void bubble_sort(int arr[], int n) {
    for(int i = 0 ; i < n ; i++) {
        for(int j = 0 ; j < n - i - 1; j++) {
            if(arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j+ 1];
                arr[j + 1] = tmp;
            }
        }
        for(int i = 0 ; i < 5 ; i++) {
            printf("%d ", arr[i]);
        }
        printf("\n");
    }
}

int main() {
    int arr[5] = {5, 3, 1, 4, 2};
    bubble_sort(arr, 5);

    for(int i = 0 ; i < 5 ; i++) {
        printf("%d ", arr[i]);
    }
    return 0;
}
