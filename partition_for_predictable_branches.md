# Stable Partition for Predictable Branches

## Problem

We are given an array of over `100,000` integers. We need to answer the same threshold query `100` times:

```c
if (arr[i] > 128) {
    total += arr[i];
}
```

The array is roughly split `50/50` around the value `128`, but the values are currently in random order.

This random ordering makes the branch difficult for the CPU to predict. As a result, we may get:

- High Branch MPKI
- More branch mispredictions
- Lower IPC

The goal is to rearrange the array so that the branch becomes more predictable, and eventually remove the branch from the hot loop completely.

---

## Initial Code

```c
#include <stdlib.h>

#define N 100000
#define REPEATS 100

long solve(int *arr, int n) {
    long total = 0;

    for (int r = 0; r < REPEATS; r++) {
        for (int i = 0; i < n; i++) {
            if (arr[i] > 128) {
                total += arr[i];
            }
        }
    }

    return total;
}
```

## Why This Performs Poorly

Since the array is randomly ordered and roughly half the values are greater than `128`, the branch condition is unpredictable.

The CPU sees something like:

```txt
true, false, true, false, false, true, true...
```

That pattern is difficult for the branch predictor to learn.

Because this branch runs inside the repeated loop, the cost of misprediction can become significant.

---

# Iteration 1: Partition Using a Temporary Array

```c
#include <stdlib.h>

#define N 100000
#define REPEATS 100

long solve(int *arr, int n) {
    long total = 0;

    int temp[n];
    int left = 0;
    int right = n - 1;

    for (int j = 0; j < n; j++) {
        if (arr[j] > 128) {
            temp[right] = arr[j];
            right--;
        } else {
            temp[left] = arr[j];
            left++;
        }
    }

    for (int i = 0; i < n; i++) {
        arr[i] = temp[i];
    }

    for (int r = 0; r < REPEATS; r++) {
        for (int i = 0; i < n; i++) {
            if (arr[i] > 128) {
                total += arr[i];
            }
        }
    }

    return total;
}
```

## Why This Improves Performance

This version partitions the array into two groups:

```txt
[values <= 128] [values > 128]
```

Now the repeated branch sees a much more predictable pattern:

```txt
false, false, false, false, true, true, true, true...
```

This is much easier for the CPU branch predictor to handle.

## Problem With Iteration 1

This version still has some drawbacks:

- It uses extra memory with `temp[n]`
- It copies the temporary array back into `arr`
- It still uses a branch during the partitioning step

```c
if (arr[j] > 128)
```

So the repeated query loop becomes more predictable, but the setup phase still adds extra work.

---

# Iteration 2: In-Place Partition Using Two Pointers

Instead of using a temporary array, we can partition the array in-place using two pointers.

```c
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

    for (int r = 0; r < REPEATS; r++) {
        for (int i = 0; i < n; i++) {
            if (arr[i] > 128) {
                total += arr[i];
            }
        }
    }

    return total;
}
```

## How the Two-Pointer Partition Works

We keep one pointer at the beginning of the array:

```c
int left = 0;
```

and one pointer at the end of the array:

```c
int right = n - 1;
```

The `left` pointer moves forward while values are already on the correct side:

```c
arr[left] <= 128
```

The `right` pointer moves backward while values are already on the correct side:

```c
arr[right] > 128
```

When both pointers stop, that means we found two misplaced values.

So we swap them.

After the loop finishes, the array is partitioned like this:

```txt
[values <= 128] [values > 128]
```

The array is not necessarily sorted. It is only grouped by the threshold condition.

---

# Iteration 3: Remove the Branch From the Hot Loop

Iteration 2 improves the data layout, but the repeated loop still checks:

```c
if (arr[i] > 128)
```

Since the array is now partitioned, we know that all values greater than `128` are on the right side.

So instead of scanning the entire array and checking every element, we can find the split point and only sum the values on the right side.

```c
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

    for (int r = 0; r < REPEATS; r++) {
        for (int i = left; i < n; i++) {
            total += arr[i];
        }
    }

    return total;
}
```

## Why This Is Better

After partitioning, the array looks like this:

```txt
[values <= 128] [values > 128]
```

The variable `left` becomes the first index where values greater than `128` begin.

So instead of doing this:

```c
for (int i = 0; i < n; i++) {
    if (arr[i] > 128) {
        total += arr[i];
    }
}
```

we do this:

```c
for (int i = left; i < n; i++) {
    total += arr[i];
}
```

Now the repeated loop has no threshold branch at all.

Originally, the branch ran:

```txt
N * REPEATS
```

times.

With:

```c
#define N 100000
#define REPEATS 100
```

that branch would run:

```txt
10,000,000 times
```

In Iteration 3, that branch is removed from the hot loop.

---

## Final Takeaway

The original version performs poorly because the branch condition is unpredictable.

Iteration 1 improves branch prediction by grouping the data, but it uses extra memory.

Iteration 2 partitions the array in-place using two pointers.

Iteration 3 improves the hot loop even more by removing the repeated `if` statement completely.

This optimization works well when:

- The same query is repeated many times
- The array is large
- The threshold does not change
- We do not care about preserving the original order of the array

---

## Important Note About Stability

Although the title says “Stable Partition,” the two-pointer version is technically **not stable**.

A stable partition keeps the original relative order of elements.

The two-pointer version swaps values from opposite sides of the array, so the original order may change.

For this problem, that does not matter because we only care whether each value is `<= 128` or `> 128`.
