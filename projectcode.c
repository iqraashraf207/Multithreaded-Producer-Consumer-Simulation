#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define EMPTY_SLOT (-1)

int BUFFER_SIZE;
int NUM_PRODUCERS;
int NUM_CONSUMERS;
int MAX_ITEMS;

int *sharedBuffer;
int nextIn = 0;
int nextOut = 0;

sem_t emptySem;
sem_t fullSem;
pthread_mutex_t bufMutex;

int *producedCount;
int *consumedCount;

static void printDivider(void)
{
    printf("--------------------------------------------------\n");
}

static void showBuffer(void)
{
    printf("  Buffer: ");
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        if (sharedBuffer[i] == EMPTY_SLOT)
            printf("[   ]");
        else
            printf("[%3d]", sharedBuffer[i]);
    }
    printf("  (in = %d, out = %d)\n", nextIn, nextOut);
}

static int insertItem(int item, int prodID)
{
    if (sem_wait(&emptySem) != 0)
    {
        perror("sem_wait emptySem failed!!");
        return -1;
    }

    if (pthread_mutex_lock(&bufMutex) != 0)
    {
        perror("mutex lock failed in insertItem!!");
        sem_post(&emptySem);
        return -1;
    }

    if (sharedBuffer[nextIn] != EMPTY_SLOT)
    {
        fprintf(stderr, "[ERROR] insertItem: slot %d should be empty but isnt!!\n", nextIn);
        pthread_mutex_unlock(&bufMutex);
        sem_post(&emptySem);
        return -1;
    }

    sharedBuffer[nextIn] = item;
    nextIn = (nextIn + 1) % BUFFER_SIZE;
    producedCount[prodID]++;

    printf("[PRODUCER %d] inserted  item: %3d\n", prodID, item);
    showBuffer();
    printDivider();

    if (pthread_mutex_unlock(&bufMutex) != 0)
    {
        perror("mutex unlock failed in insertItem!!");
        return -1;
    }

    if (sem_post(&fullSem) != 0)
    {
        perror("sem_post fullSem failed!!");
        return -1;
    }

    return 0;
}

static int removeItem(int *result, int consID)
{
    if (sem_wait(&fullSem) != 0)
    {
        perror("sem_wait fullSem failed!!");
        return -1;
    }

    if (pthread_mutex_lock(&bufMutex) != 0)
    {
        perror("mutex lock failed in removeItem!!");
        sem_post(&fullSem);
        return -1;
    }

    if (sharedBuffer[nextOut] == EMPTY_SLOT)
    {
        fprintf(stderr, "[ERROR] removeItem: slot %d should be full but isnt!!\n", nextOut);
        pthread_mutex_unlock(&bufMutex);
        sem_post(&fullSem);
        return -1;
    }

    *result = sharedBuffer[nextOut];
    sharedBuffer[nextOut] = EMPTY_SLOT;
    nextOut = (nextOut + 1) % BUFFER_SIZE;
    consumedCount[consID]++;

    printf("[CONSUMER %d] removed   item: %3d\n", consID, *result);
    showBuffer();
    printDivider();

    if (pthread_mutex_unlock(&bufMutex) != 0)
    {
        perror("mutex unlock failed in removeItem!!");
        return -1;
    }

    if (sem_post(&emptySem) != 0)
    {
        perror("sem_post emptySem failed!!");
        return -1;
    }

    return 0;
}

void *producerThread(void *arg)
{
    int tid = *(int *)arg;
    free(arg);

    unsigned int seed = (unsigned int)(time(NULL) ^ ((unsigned long)(tid + 1) * 11111UL));

    for (int i = 0; i < MAX_ITEMS; i++)
    {
        usleep((useconds_t)(rand_r(&seed) % 500000));

        int newItem = (int)(rand_r(&seed) % 100);

        printf("[PRODUCER %d] generating item: %3d  (%d of %d)\n", tid, newItem, i + 1, MAX_ITEMS);

        if (insertItem(newItem, tid) != 0)
        {
            fprintf(stderr, "[PRODUCER %d] insertItem failed, exiting thread!!\n", tid);
            pthread_exit(NULL);
        }
    }

    printf("[PRODUCER %d] done!! Produced %d items.\n", tid, producedCount[tid]);
    pthread_exit(NULL);
}

void *consumerThread(void *arg)
{
    int tid = *(int *)arg;
    free(arg);

    unsigned int seed = (unsigned int)(time(NULL) ^ ((unsigned long)(tid + 1) * 99999UL));

    for (int i = 0; i < MAX_ITEMS; i++)
    {
        usleep((useconds_t)(rand_r(&seed) % 500000));

        printf("[CONSUMER %d] trying to consume  (%d of %d)\n", tid, i + 1, MAX_ITEMS);

        int consumed = 0;
        if (removeItem(&consumed, tid) != 0)
        {
            fprintf(stderr, "[CONSUMER %d] removeItem failed, exiting thread!!\n", tid);
            pthread_exit(NULL);
        }
    }

    printf("[CONSUMER %d] done!! Consumed %d items.\n", tid, consumedCount[tid]);
    pthread_exit(NULL);
}

int main(void)
{
    printf("==================================================\n");
    printf("   Producer-Consumer Simulation - Setup\n");
    printf("==================================================\n");

    printf("  Enter buffer size: ");
    if (scanf("%d", &BUFFER_SIZE) != 1 || BUFFER_SIZE < 1)
    {
        fprintf(stderr, "Invalid buffer size!!\n");
        return EXIT_FAILURE;
    }

    printf("  Enter number of producers: ");
    if (scanf("%d", &NUM_PRODUCERS) != 1 || NUM_PRODUCERS < 1)
    {
        fprintf(stderr, "Invalid number of producers!!\n");
        return EXIT_FAILURE;
    }

    printf("  Enter number of consumers: ");
    if (scanf("%d", &NUM_CONSUMERS) != 1 || NUM_CONSUMERS < 1)
    {
        fprintf(stderr, "Invalid number of consumers!!\n");
        return EXIT_FAILURE;
    }

    printf("  Enter items per thread: ");
    if (scanf("%d", &MAX_ITEMS) != 1 || MAX_ITEMS < 1)
    {
        fprintf(stderr, "Invalid items per thread!!\n");
        return EXIT_FAILURE;
    }

    if (NUM_PRODUCERS != NUM_CONSUMERS)
    {
        printf("Mismatch in producers and consumers!!\n");
        return EXIT_FAILURE;
    }

    sharedBuffer = malloc(sizeof(int) * BUFFER_SIZE);
    producedCount = calloc(NUM_PRODUCERS, sizeof(int));
    consumedCount = calloc(NUM_CONSUMERS, sizeof(int));

    pthread_t *producers = malloc(sizeof(pthread_t) * NUM_PRODUCERS);
    pthread_t *consumers = malloc(sizeof(pthread_t) * NUM_CONSUMERS);

    pthread_mutex_init(&bufMutex, NULL);
    sem_init(&emptySem, 0, BUFFER_SIZE);
    sem_init(&fullSem, 0, 0);

    for (int i = 0; i < BUFFER_SIZE; i++)
        sharedBuffer[i] = EMPTY_SLOT;

    for (int i = 0; i < NUM_PRODUCERS; i++)
    {
        int *tid = malloc(sizeof(int));
        *tid = i;
        pthread_create(&producers[i], NULL, producerThread, tid);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++)
    {
        int *tid = malloc(sizeof(int));
        *tid = i;
        pthread_create(&consumers[i], NULL, consumerThread, tid);
    }

    for (int i = 0; i < NUM_PRODUCERS; i++)
        pthread_join(producers[i], NULL);

    for (int i = 0; i < NUM_CONSUMERS; i++)
        pthread_join(consumers[i], NULL);

    int totalProduced = 0;
    int totalConsumed = 0;

    for (int i = 0; i < NUM_PRODUCERS; i++)
        totalProduced += producedCount[i];

    for (int i = 0; i < NUM_CONSUMERS; i++)
        totalConsumed += consumedCount[i];

    printf("Total produced : %d\n", totalProduced);
    printf("Total consumed : %d\n", totalConsumed);

    pthread_mutex_destroy(&bufMutex);
    sem_destroy(&emptySem);
    sem_destroy(&fullSem);

    free(sharedBuffer);
    free(producedCount);
    free(consumedCount);
    free(producers);
    free(consumers);

    return 0;
}
