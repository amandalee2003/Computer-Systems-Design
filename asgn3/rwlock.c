//rwlock.c

#include <pthread.h>
#include <stdlib.h>

typedef enum { READERS, WRITERS, N_WAY } PRIORITY;

typedef struct rwlock {
    int read_count;
    int write_count;
    int n;
    pthread_cond_t reader;
    pthread_cond_t writer;
    pthread_mutex_t lock;
    PRIORITY priority;
} rwlock_t;

rwlock_t *rwlock_new(PRIORITY p, int n) {
    rwlock_t *rw = (rwlock_t *) malloc(sizeof(rwlock_t));
    if (rw == NULL) {
        return NULL;
    }
    if (rw != NULL) {
        rw->n = n;
        rw->priority = p;
        rw->read_count = 0;
        rw->write_count = 0;
        pthread_cond_init(&rw->reader, NULL);
        pthread_cond_init(&rw->writer, NULL);
        pthread_mutex_init(&rw->lock, NULL);
    }
    return rw;
}

void rwlock_delete(rwlock_t **l) {
    if (l != NULL && *l != NULL) {
        rwlock_t *rw = *l;
        pthread_cond_destroy(&rw->reader);
        pthread_cond_destroy(&rw->writer);
        pthread_mutex_destroy(&rw->lock);
        free(rw);
        *l = NULL;
    }
}

void reader_lock(rwlock_t *rw) {
    if (!rw) {
        return;
    }
    if (rw) {
        pthread_mutex_lock(&rw->lock);
        while ((rw->priority == WRITERS && rw->read_count > 0) || rw->write_count > 0) {
            pthread_cond_wait(&rw->reader, &rw->lock);
        }
        pthread_mutex_unlock(&rw->lock);
        rw->read_count = rw->read_count + 1;
    }
}

void reader_unlock(rwlock_t *rw) {
    if (!rw) {
        return;
    }
    if (rw) {
        rw->read_count = rw->read_count - 1;
        pthread_mutex_lock(&rw->lock);
        if (rw->read_count == 0 && rw->write_count > 0) {
            pthread_cond_signal(&rw->writer);
        }
        pthread_mutex_unlock(&rw->lock);
    }
}

void writer_lock(rwlock_t *rw) {
    if (!rw) {
        return;
    }
    if (rw) {
        pthread_mutex_lock(&rw->lock);
        rw->write_count = rw->write_count + 1;
        while (rw->read_count > 0 || (rw->priority == READERS && rw->write_count > 1)
               || (rw->priority == N_WAY && rw->write_count > 0 && rw->read_count < rw->n)) {
            pthread_cond_wait(&rw->writer, &rw->lock);
        }
        pthread_mutex_unlock(&rw->lock);
    }
}

void writer_unlock(rwlock_t *rw) {
    if (!rw) {
        return;
    }
    if (rw) {
        pthread_mutex_lock(&rw->lock);
        rw->write_count = rw->write_count - 1;
        if (rw->priority == N_WAY) {
            if (rw->write_count == 0) {
                if (rw->read_count > 0) {
                    pthread_cond_broadcast(&rw->reader);
                }
            }
        } else {
            pthread_cond_signal(&rw->writer);
        }
        pthread_mutex_unlock(&rw->lock);
    }
}
