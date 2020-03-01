// CalculatingPI.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <time.h> 

static long num_steps = 1000000000;
double step, pi;

#define NUM_THREADS 8

double arraySum[NUM_THREADS];

void serial_code()
{
    int i;
    double x, sum = 0.0;

    step = 1.0 / (double)num_steps;

    clock_t t;
    t = clock();

    for (i = 0; i < num_steps; i++) {
        x = (i + 0.5) * step;
        sum = sum + 4.0 / (1.0 + x * x);
    }

    t = clock() - t;
    double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds 

    printf("Computation took %f seconds to execute \n", time_taken);

    pi = step * sum;
    printf("Pi = %.20f\n", pi);
}

DWORD WINAPI ComputationThread(LPVOID lpParam)
{
    int c = *(int*)lpParam;
    int i;
    double x, localSum = 0.0;
    arraySum[c] = 0;
    for (i = 0; i < num_steps; i += NUM_THREADS) {
        x = ((double)i + (double)c + 0.5) * step;
        localSum = localSum + 4.0 / (1.0 + x * x);
    }
    arraySum[c] = localSum;
    return 0;
}

void parallel_code()
{
    step = 1.0 / (double)num_steps;

    HANDLE handles[NUM_THREADS];
    int indices[NUM_THREADS];

    clock_t t;
    t = clock();

    for (int i = 0; i < NUM_THREADS; i++)
    {
        indices[i] = i;
        handles[i] = CreateThread(0, 0, ComputationThread, &indices[i], 0, 0);
    }

    WaitForMultipleObjects(NUM_THREADS, handles, TRUE, INFINITE);

    t = clock() - t;
    double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds 

    printf("Computation took %f seconds to execute \n", time_taken);

    double sum = 0.0;
    for (int i = 0; i < NUM_THREADS; i++)
        sum += arraySum[i];

    pi = step * sum;
    printf("Pi = %.20f\n", pi);
}

void main()
{
    parallel_code();
}