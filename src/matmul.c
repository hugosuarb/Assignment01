#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <sys/resource.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

double* allocate_matrix(int n) {
    double* mat = (double*)calloc((size_t)n * (size_t)n, sizeof(double));
    if (!mat) {
        exit(2);
    }
    return mat;
}

void generate_matrix(double* mat, int n, uint64_t seed) {
    uint64_t state = seed;
    size_t total = (size_t)n * (size_t)n;
    for (size_t idx = 0; idx < total; idx++) {
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        uint64_t bits = state >> 11;
        mat[idx] = (double)bits * (1.0 / 9007199254740992.0);
    }
}

void identity_matrix(double* mat, int n) {
    memset(mat, 0, (size_t)n * (size_t)n * sizeof(double));
    for (int i = 0; i < n; i++) {
        mat[(size_t)i * n + i] = 1.0;
    }
}

void clear_matrix(double* mat, int n) {
    memset(mat, 0, (size_t)n * (size_t)n * sizeof(double));
}

void multiply_kernel(const double* A, const double* B, double* C, int n) {
    for (int i = 0; i < n; i++) {
        size_t i_offset = (size_t)i * n;
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[i_offset + k] * B[(size_t)k * n + j];
            }
            C[i_offset + j] = sum;
        }
    }
}

double compute_checksum(const double* C, int n) {
    double sum = 0.0;
    size_t total = (size_t)n * (size_t)n;
    for (size_t idx = 0; idx < total; idx++) {
        sum += C[idx];
    }
    return sum;
}

double get_monotonic_seconds(void) {
#ifdef _WIN32
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
#endif
}

double get_peak_rss_kb(void) {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return (double)pmc.PeakWorkingSetSize / 1024.0;
    }
    return 0.0;
#else
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
#ifdef __APPLE__
    return (double)usage.ru_maxrss / 1024.0;
#else
    return (double)usage.ru_maxrss;
#endif
#endif
}

int main(int argc, char* argv[]) {
    if (argc < 6) {
        return 1;
    }
    const char* mode = argv[1];
    int n = atoi(argv[2]);
    int warmup = atoi(argv[3]);
    int reps = atoi(argv[4]);
    uint64_t seed = (uint64_t)strtoull(argv[5], NULL, 10);

    double* A = allocate_matrix(n);
    double* B = allocate_matrix(n);
    double* C = allocate_matrix(n);

    if (strcmp(mode, "identity") == 0) {
        generate_matrix(A, n, seed);
        identity_matrix(B, n);
        multiply_kernel(A, B, C, n);
        double max_diff = 0.0;
        size_t total = (size_t)n * (size_t)n;
        for (size_t idx = 0; idx < total; idx++) {
            double diff = fabs(C[idx] - A[idx]);
            if (diff > max_diff) {
                max_diff = diff;
            }
        }
        printf("IDENTITY,%d,%.17e,%.17e\n", n, max_diff, compute_checksum(C, n));
        free(A);
        free(B);
        free(C);
        return 0;
    }

    if (strcmp(mode, "zero") == 0) {
        generate_matrix(A, n, seed);
        clear_matrix(B, n);
        multiply_kernel(A, B, C, n);
        double max_diff = 0.0;
        size_t total = (size_t)n * (size_t)n;
        for (size_t idx = 0; idx < total; idx++) {
            double diff = fabs(C[idx]);
            if (diff > max_diff) {
                max_diff = diff;
            }
        }
        printf("ZERO,%d,%.17e,%.17e\n", n, max_diff, compute_checksum(C, n));
        free(A);
        free(B);
        free(C);
        return 0;
    }

    if (strcmp(mode, "dump") == 0) {
        generate_matrix(A, n, seed);
        generate_matrix(B, n, seed + 1);
        multiply_kernel(A, B, C, n);
        size_t total = (size_t)n * (size_t)n;
        for (size_t idx = 0; idx < total; idx++) {
            if (idx > 0) {
                printf(",");
            }
            printf("%.17e", C[idx]);
        }
        printf("\n");
        free(A);
        free(B);
        free(C);
        return 0;
    }

    generate_matrix(A, n, seed);
    generate_matrix(B, n, seed + 1);

    for (int w = 0; w < warmup; w++) {
        multiply_kernel(A, B, C, n);
    }

    for (int r = 1; r <= reps; r++) {
        clear_matrix(C, n);
        double t0 = get_monotonic_seconds();
        multiply_kernel(A, B, C, n);
        double t1 = get_monotonic_seconds();
        double elapsed_s = t1 - t0;
        double chk = compute_checksum(C, n);
        double mem_kb = get_peak_rss_kb();
        printf("C,%d,float64,%d,%.9f,%.2f,%.12e,1\n", n, r, elapsed_s, mem_kb, chk);
    }

    free(A);
    free(B);
    free(C);
    return 0;
}
