#include <stdlib.h>

#define N 100000
#define REPEATS 100

long solve(int *arr, int n) {
    long total = 0;

    int left = 0;
    int right = n - 1;

    while (left < right) {
        while (left < right && arr[left] <= 128) {
            left++;
        }

        while (left < right && arr[right] > 128) {
            right--;
        }

        if (left < right) {
            int temp = arr[left];
            arr[left] = arr[right];
            arr[right] = temp;

            left++;
            right--;
        }
    }

    while (left < n && arr[left] <= 128) {
        left++;
    }

    for (int i = left; i < n; i++) {
        total += arr[i];
    }

    total = REPEATS * total;

    return total;
}
