#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint8_t u8;

#define NUM_ACCESSES 100000
#define L2_CACHE_SIZE 0x100000 // 1024KB => 1MB
#define CACHE_LINE_SIZE 64     // 64B


struct timespec start_time, end_time;


void read_cache_misses_sequential(volatile u8 *memory, u32 size)
{
    int i;

    	for (i = 0; i < size; i += CACHE_LINE_SIZE)
    	{
    		u8 value = memory[i];
    	    	(void)value;
    	}
}


void read_cache_misses_random(volatile u8 *memory, u32 size)
{
    int i;

    // Random read
    for (i = 0; i < size; i+= CACHE_LINE_SIZE)
    {
        int index = rand() % size;
        u8 value = memory[index];
        (void)value;
    }
}


void write_cache_misses_sequential(volatile u8 *memory, u32 size)
{
    int i;

    for (i = 0; i < size; i += CACHE_LINE_SIZE)
    {
        memory[i] = 0;
    }
}


void write_cache_misses_random(volatile u8 *memory, u32 size)
{
    int i;

    // Random access pattern
    for (i = 0; i < size; i += CACHE_LINE_SIZE)
    {
        int rand_index = rand() % size;
        memory[rand_index] = 0;
    }
}


void* thread_function() {
 
    volatile u8 *memory;
    u32 size = L2_CACHE_SIZE;
    int m_size = L2_CACHE_SIZE * 2;
    double elapsed_time;


    // Allocate a memory block that is larger than the L2 cache size
    memory = malloc(m_size);

    // Measure cache misses for sequential read
    clock_gettime(CLOCK_MONOTONIC, &start_time); // start clock
    for (int i = 0; i < NUM_ACCESSES; i++) {
    	read_cache_misses_sequential(memory, size);
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time); // end clock
    printf("\nSequential Read\n\nMalloc size: %d\nNum Access: %d\nElapsed time: %.6f seconds\n\n", m_size, NUM_ACCESSES, (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9);

    // Free the memory block
    free((void *)memory);


    // Allocate a memory block that is larger than the L2 cache size
    memory = malloc(m_size);

    // Measure cache misses for random read
    clock_gettime(CLOCK_MONOTONIC, &start_time); // start clock
    for (int i = 0; i < NUM_ACCESSES; i++) {
    	read_cache_misses_random(memory, size);
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time); // end clock
    printf("\nRandom Read\n\nMalloc size: %d\nNum Access: %d\nElapsed time: %.6f seconds\n\n", m_size, NUM_ACCESSES, (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9);

    // Free the memory block
    free((void *)memory);


    // Allocate a memory block that is larger than the L2 cache size
    memory = malloc(m_size);

    // Measure cache misses for sequential write
    clock_gettime(CLOCK_MONOTONIC, &start_time); // start clock
    for (int i = 0; i < NUM_ACCESSES; i++) {
    	write_cache_misses_sequential(memory, size);
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time); // end clock
    printf("\nSequential Write\n\nMalloc size: %d\nNum Access: %d\nElapsed time: %.6f seconds\n\n", m_size, NUM_ACCESSES, (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9);

    // Free the memory block
    free((void *)memory);


    
    // Allocate a memory block that is larger than the L2 cache size
    memory = malloc(m_size);

    // Measure cache misses for random write
    clock_gettime(CLOCK_MONOTONIC, &start_time); // start clock
    for (int i = 0; i < NUM_ACCESSES; i++) {
    	write_cache_misses_random(memory, size);
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time); // end clock
    printf("\nRandom Write\n\nMalloc size: %d\nNum Access: %d\nElapsed time: %.6f seconds\n\n", m_size, NUM_ACCESSES, (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9);

    // Free the memory block
    free((void *)memory);
}


int main()
{
    // process priority value
    int niceValue = -20;

    // set process priority
    // nice(niceValue);
    
    // Initialize random seed
    srand(1234);

    // Cortex-A72 Dual Core => 2 thread
    pthread_t thread1, thread2;  // thread identifiers
    
    // Create thread 1
    pthread_create(&thread1, NULL, thread_function, NULL);

    // Create thread 2
    pthread_create(&thread2, NULL, thread_function, NULL);

    // Wait for thread 1 to finish
    pthread_join(thread1, NULL);

    // Wait for thread 2 to finish
    pthread_join(thread2, NULL);
	
    return 0;
}
