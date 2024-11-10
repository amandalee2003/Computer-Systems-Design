//queue.c

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "queue.h"

//typedef struct queue queue_t;

typedef struct queue {
    int size;
    int counter;
    int start;
    int end;
    void **buf;
    pthread_cond_t full;
    pthread_cond_t empty;
    pthread_mutex_t block;
} queue;

queue_t *queue_new(int size) {
    queue_t *q = malloc(sizeof(queue)); //(queue *q) calloc(1, sizeof(queue_t));
    if (q == NULL) {
        free(q);
        q = NULL;
    }
    q->buf = malloc(size * sizeof(void *)); //calloc(1, size * sizeof(void *));
    if (q->buf == NULL) {
        free(q->buf);
    }
    q->size = size;
    pthread_cond_init(&q->full, NULL);
    pthread_cond_init(&q->empty, NULL);
    pthread_mutex_init(&q->block, NULL);
    q->counter = 0;
    q->start = 0;
    q->end = 0;
    return q;
}

void queue_delete(queue_t **q) {
    if (q != NULL && *q != NULL) {
        pthread_cond_destroy(&(*q)->full);
        pthread_cond_destroy(&(*q)->empty);
        pthread_mutex_destroy(&(*q)->block);
        free((*q)->buf);
        free(*q);
        *q = NULL;
    }
}

bool queue_push(queue_t *q, void *elem) {
    if (!q) {
        return false;
    }
    if (q) {
        pthread_mutex_lock(&q->block);
        while (q->counter == q->size) {
            pthread_cond_wait(&q->full, &q->block);
        }
        q->buf[q->end] = elem;
        q->end = (q->end + 1) % q->size;
        q->counter = q->counter + 1;
        pthread_cond_signal(&q->empty);
        pthread_mutex_unlock(&q->block);
        return true;
    }
    return false;
}

bool queue_pop(queue_t *q, void **elem) {
    if (!q) {
        return false;
    }
    if (q) {
        pthread_mutex_lock(&q->block);
        while (q->counter == 0) {
            pthread_cond_wait(&q->empty, &q->block);
        }
        *elem = q->buf[q->start];
        q->start = (q->start + 1) % q->size;
        q->counter = q->counter - 1;
        pthread_cond_signal(&q->full);
        pthread_mutex_unlock(&q->block);
        return true;
    }
    return false;
}
