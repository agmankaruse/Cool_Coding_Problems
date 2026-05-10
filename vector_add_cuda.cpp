#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>

__global__ void vector_add(const float* A, const float* B, float* C, int N) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    if (i < N) {
        C[i] = A[i] + B[i];
    }
}

// A, B, C are device pointers
extern "C" void solve(const float* A, const float* B, float* C, int N) {
    int threadsPerBlock = 256;
    int blocksPerGrid = (N + threadsPerBlock - 1) / threadsPerBlock;

    vector_add<<<blocksPerGrid, threadsPerBlock>>>(A, B, C, N);
    cudaDeviceSynchronize();
}

int main() {
    int N = 4;

    // CPU arrays
    float h_A[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    float h_B[4] = {5.0f, 6.0f, 7.0f, 8.0f};
    float h_C[4];

    // GPU pointers
    float* d_A;
    float* d_B;
    float* d_C;

    size_t size = N * sizeof(float);

    // Allocate memory on GPU
    cudaMalloc((void**)&d_A, size);
    cudaMalloc((void**)&d_B, size);
    cudaMalloc((void**)&d_C, size);

    // Copy input arrays from CPU to GPU
    cudaMemcpy(d_A, h_A, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B, size, cudaMemcpyHostToDevice);

    // Run GPU vector addition
    solve(d_A, d_B, d_C, N);

    // Copy result from GPU back to CPU
    cudaMemcpy(h_C, d_C, size, cudaMemcpyDeviceToHost);

    // Print result
    printf("C = [");
    for (int i = 0; i < N; i++) {
        printf("%.1f", h_C[i]);

        if (i < N - 1) {
            printf(", ");
        }
    }
    printf("]\n");

    // Free GPU memory
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

    return 0;
}
