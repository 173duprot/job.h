#ifndef JOB_H
#define JOB_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_THREADS 4
#define MAX_JOBS 256
#define CACHE_LINE_SIZE 64
#define PREFETCH_DISTANCE 3

typedef void (*job_fn)(void*);

typedef struct {
    job_fn fn;
    void *data;
} job_t;

typedef struct {
    alignas(CACHE_LINE_SIZE) job_t jobs[MAX_JOBS];
    alignas(CACHE_LINE_SIZE) int front;
    alignas(CACHE_LINE_SIZE) int rear;
    alignas(CACHE_LINE_SIZE) int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} job_queue_t;

typedef struct {
    pthread_t threads[MAX_THREADS];
    job_queue_t queue;
} thread_pool_t;

#ifndef prefetch
static inline void prefetch(const void* ptr) {
    __builtin_prefetch(ptr, 0, PREFETCH_DISTANCE);
}
#endif

static inline void queue_init(job_queue_t *q) {
    q->front = 0;
    q->rear = 0;
    q->count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

static inline int queue_push(job_queue_t *q, job_fn fn, void *data) {
    pthread_mutex_lock(&q->mutex);
    if (q->count < MAX_JOBS) {
        prefetch(&q->jobs[q->rear]);
        q->jobs[q->rear].fn = fn;
        q->jobs[q->rear].data = data;
        q->rear = (q->rear + 1) % MAX_JOBS;
        q->count++;
        pthread_cond_signal(&q->cond);
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }
    pthread_mutex_unlock(&q->mutex);
    return -1;
}

static inline job_t queue_pop(job_queue_t *q) {
    job_t job = {NULL, NULL};
    pthread_mutex_lock(&q->mutex);
    while (q->count == 0) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }
    prefetch(&q->jobs[q->front]);
    job = q->jobs[q->front];
    q->front = (q->front + 1) % MAX_JOBS;
    q->count--;
    pthread_mutex_unlock(&q->mutex);
    return job;
}

static void *worker_fn(void *arg) {
    job_queue_t *q = (job_queue_t *)arg;
    while (1) {
        job_t job = queue_pop(q);
        if (job.fn != NULL) {
            job.fn(job.data);
        }
    }
    return NULL;
}

static inline void pool_init(thread_pool_t *p) {
    queue_init(&p->queue);
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_create(&p->threads[i], NULL, worker_fn, &p->queue);
    }
}

static inline int pool_submit(thread_pool_t *p, job_fn fn, void *data) {
    return queue_push(&p->queue, fn, data);
}

#endif // JOB_H
