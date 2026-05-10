#include <stdio.h>
#include <stdlib.h>

#define REPEATS 100
#define THRESHOLD 128

long solve(int *arr, int n) {
    long total = 0;

    int left = 0;
    int right = n - 1;

    while (left < right) {
        while (left < right && arr[left] <= THRESHOLD) {
            left++;
        }

        while (left < right && arr[right] > THRESHOLD) {
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

    while (left < n && arr[left] <= THRESHOLD) {
        left++;
    }

    for (int r = 0; r < REPEATS; r++) {
        for (int i = left; i < n; i++) {
            total += arr[i];
        }
    }

    return total;
}

long expected_result(int *arr, int n) {
    long total = 0;

    for (int r = 0; r < REPEATS; r++) {
        for (int i = 0; i < n; i++) {
            if (arr[i] > THRESHOLD) {
                total += arr[i];
            }
        }
    }

    return total;
}

void copy_array(int *src, int *dest, int n) {
    for (int i = 0; i < n; i++) {
        dest[i] = src[i];
    }
}

void print_array(int *arr, int n) {
    printf("[ ");

    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }

    printf("]\n");
}

void run_test(int *arr, int n, char *test_name) {
    int *copy = malloc(n * sizeof(int));

    if (copy == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    copy_array(arr, copy, n);

    long expected = expected_result(copy, n);
    long actual = solve(arr, n);

    printf("\n%s\n", test_name);
    printf("Expected: %ld\n", expected);
    printf("Actual:   %ld\n", actual);

    if (expected == actual) {
        printf("Result: PASS\n");
    } else {
        printf("Result: FAIL\n");
    }

    printf("Array after partition: ");
    print_array(arr, n);

    free(copy);
}

int main(void) {
    int test1[] = {10, 20, 160, 128, 150, 140, 130};
    int n1 = sizeof(test1) / sizeof(test1[0]);

    int test2[] = {1, 2, 3, 4, 5};
    int n2 = sizeof(test2) / sizeof(test2[0]);

    int test3[] = {200, 300, 400, 500};
    int n3 = sizeof(test3) / sizeof(test3[0]);

    int test4[] = {128, 129, 127, 130, 128, 131};
    int n4 = sizeof(test4) / sizeof(test4[0]);

    int test5[] = {150, 10, 200, 20, 250, 30, 300, 40};
    int n5 = sizeof(test5) / sizeof(test5[0]);

    run_test(test1, n1, "Test 1: Mixed values");
    run_test(test2, n2, "Test 2: All values <= 128");
    run_test(test3, n3, "Test 3: All values > 128");
    run_test(test4, n4, "Test 4: Values around threshold");
    run_test(test5, n5, "Test 5: Alternating low and high values");

    return 0;
}
