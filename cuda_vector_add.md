# CUDA Vector Addition

## Problem

We are given two arrays, or vectors:

```txt
A = [1.0, 2.0, 3.0, 4.0]
B = [5.0, 6.0, 7.0, 8.0]
```

We need to compute a third array `C`, where each element is the sum of the matching elements from `A` and `B`.

```c
C[i] = A[i] + B[i];
```

So:

```txt
C[0] = A[0] + B[0] = 1.0 + 5.0 = 6.0
C[1] = A[1] + B[1] = 2.0 + 6.0 = 8.0
C[2] = A[2] + B[2] = 3.0 + 7.0 = 10.0
C[3] = A[3] + B[3] = 4.0 + 8.0 = 12.0
```

Final result:

```txt
C = [6.0, 8.0, 10.0, 12.0]
```

The important part is that this computation should run on the GPU, not the CPU.

---

## CPU Version

A normal CPU solution would use a loop:

```c
for (int i = 0; i < N; i++) {
    C[i] = A[i] + B[i];
}
```

This works, but it processes the array sequentially.

That means one CPU thread walks through the array one element at a time.

---

## GPU Version

On the GPU, we want many threads running at the same time.

Conceptually:

```txt
Thread 0 computes C[0]
Thread 1 computes C[1]
Thread 2 computes C[2]
Thread 3 computes C[3]
...
```

Each thread handles one array index.

This is a good fit for the GPU because every addition is independent.

For example:

```txt
C[0] does not depend on C[1]
C[1] does not depend on C[2]
C[2] does not depend on C[3]
```

So for a large `N`, like `25,000,000`, the GPU can launch many threads and do a lot of the work in parallel.

---

## CUDA Code

```cuda
#include <cuda_runtime.h>

__global__ void vector_add(const float* A, const float* B, float* C, int N) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;

    if (index < N) {
        C[index] = A[index] + B[index];
    }
}

// A, B, and C are device pointers.
// That means they point to memory on the GPU.
extern "C" void solve(const float* A, const float* B, float* C, int N) {
    int threadsPerBlock = 256;
    int blocksPerGrid = (N + threadsPerBlock - 1) / threadsPerBlock;

    vector_add<<<blocksPerGrid, threadsPerBlock>>>(A, B, C, N);

    cudaDeviceSynchronize();
}
```

---

## Kernel Function

```cuda
__global__ void vector_add(const float* A, const float* B, float* C, int N)
```

The keyword `__global__` means this function is a CUDA kernel.

A CUDA kernel is a function that:

- Runs on the GPU
- Is launched from the CPU
- Runs with many GPU threads

Each thread runs the same kernel code, but each thread gets a different index.

---

## Thread Index Calculation

```cuda
int index = blockIdx.x * blockDim.x + threadIdx.x;
```

This line gives each GPU thread a unique global index.

The GPU organizes threads into blocks.

So:

```txt
blockIdx.x   = which block this thread is in
blockDim.x   = how many threads are in each block
threadIdx.x  = this thread's position inside its block
```

For example, if each block has `256` threads:

```txt
Block 0 has indexes 0 to 255
Block 1 has indexes 256 to 511
Block 2 has indexes 512 to 767
```

The formula:

```cuda
blockIdx.x * blockDim.x + threadIdx.x
```

converts a block-local thread number into a global array index.

---

## Bounds Check

```cuda
if (index < N) {
    C[index] = A[index] + B[index];
}
```

This check is necessary because the number of GPU threads may be slightly larger than `N`.

For example, if `N = 1000` and each block has `256` threads:

```txt
blocksPerGrid = 4
total threads = 4 * 256 = 1024
```

That means the GPU launches `1024` threads, but the array only has `1000` elements.

The extra threads should not access the array.

So we check:

```cuda
if (index < N)
```

before doing the addition.

---

## Choosing the Number of Blocks

```cuda
int threadsPerBlock = 256;
int blocksPerGrid = (N + threadsPerBlock - 1) / threadsPerBlock;
```

We use `256` threads per block.

The `blocksPerGrid` calculation rounds up so that we launch enough blocks to cover all `N` elements.

For example, if:

```txt
N = 1000
threadsPerBlock = 256
```

then:

```txt
blocksPerGrid = (1000 + 256 - 1) / 256
blocksPerGrid = 1255 / 256
blocksPerGrid = 4
```

So CUDA launches `4` blocks.

That gives:

```txt
4 * 256 = 1024 threads
```

which is enough to cover all `1000` elements.

---

## Launching the Kernel

```cuda
vector_add<<<blocksPerGrid, threadsPerBlock>>>(A, B, C, N);
```

The triple angle bracket syntax is CUDA's kernel launch syntax.

```cuda
<<<blocksPerGrid, threadsPerBlock>>>
```

means:

```txt
Launch blocksPerGrid blocks.
Each block has threadsPerBlock threads.
```

So if:

```txt
blocksPerGrid = 4
threadsPerBlock = 256
```

then CUDA launches:

```txt
4 blocks * 256 threads = 1024 total threads
```

---

## Synchronizing the GPU

```cuda
cudaDeviceSynchronize();
```

CUDA kernel launches are asynchronous.

That means the CPU can continue running before the GPU finishes.

Calling:

```cuda
cudaDeviceSynchronize();
```

forces the CPU to wait until the GPU kernel is done.

This is useful when we need to make sure the computation is complete before checking the result.

---

## Final Takeaway

Vector addition is a simple but important CUDA example.

The CPU version uses one loop to process elements one at a time.

The GPU version launches many threads, where each thread computes one element of the output array.

This works well because vector addition is massively parallel:

```txt
C[i] = A[i] + B[i]
```

Each element can be computed independently.
