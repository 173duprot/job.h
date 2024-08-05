JOB(3)                 Library Functions Manual                JOB(3)

NAME
    job.h - Job Queue and Thread Pool library

DESCRIPTION
    job.h provides a simple, high-performance
    Job Queue and Thread Pool framework in only
    123 lines of code.

    It's tiny, cache-friendly, static, opinionated
    design targets multi-thread high-performance,
    and secure applications.

    It allows the following:

    Job submission and execution,
    Thread pool initialization,
    Job queue management,
    Worker thread management,
    and Graceful thread pool shutdown.

SYNOPSIS
    #include "job.h"

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
        atomic_bool stop;
    } thread_pool_t;

    void queue_init(job_queue_t *q);
    int queue_push(job_queue_t *q, job_fn fn, void *data);
    job_t queue_pop(job_queue_t *q);
    void pool_init(thread_pool_t *p);
    int pool_submit(thread_pool_t *p, job_fn fn, void *data);
    void pool_shutdown(thread_pool_t *p);
    int queue_size(job_queue_t *q);

FUNCTIONS
    void queue_init(job_queue_t *q)
        Initializes the job queue. Should be called 
        before using the queue.

    int queue_push(job_queue_t *q, job_fn fn, void *data)
        Pushes a new job onto the queue. Returns 0 on 
        success or -1 on failure.

    job_t queue_pop(job_queue_t *q)
        Pops a job from the queue. Blocks if the queue is 
        empty until a job is available. Returns the job.

    void pool_init(thread_pool_t *p)
        Initializes the thread pool and starts the worker 
        threads. Should be called before submitting jobs.

    int pool_submit(thread_pool_t *p, job_fn fn, void *data)
        Submits a job to the thread pool. Returns 0 on 
        success or -1 on failure.

    void pool_shutdown(thread_pool_t *p)
        Gracefully shuts down the thread pool, ensuring all 
        jobs are completed before terminating the threads.

    int queue_size(job_queue_t *q)
        Returns the current number of jobs in the queue.

CONSTANTS
    MAX_THREADS
        Maximum number of threads in the pool (4).

    MAX_JOBS
        Maximum number of jobs in the queue (256).

    CACHE_LINE_SIZE
        Cache line size for alignment (64 bytes).

    PREFETCH_DISTANCE
        Distance for cache prefetching (3).

AUTHOR
    173duprot <https://github.com/173duprot>

COPYRIGHT
    This library is free software: you can redistribute
    it and/or modify it under the terms of the GNU
    General Public License as published by the Free
    Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This library is distributed in the hope that it
    will be useful, but WITHOUT ANY WARRANTY; without
    even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General
    Public License along with this library. If not,
    see <https://www.gnu.org/licenses/>.

JOB(3)                 Library Functions Manual                JOB(3)
