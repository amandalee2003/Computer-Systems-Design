#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    int valid;
    char data[256];
    int used;
} CacheEntry;

// Cache structure
typedef struct {
    int size;
    int count;
    int trueSize;
    int nextIndex;
    CacheEntry *entries;
} Cache;

void initCache(Cache *cache, int size) {
    cache->size = size;
    cache->count = 0;
    cache->trueSize = 0;
    cache->nextIndex = 0;
    cache->entries = (CacheEntry *) malloc(1000 * sizeof(CacheEntry));
    for (int i = 0; i < 1000; i++) {
        cache->entries[i].valid = 0;
        cache->entries[i].used = 0;
    }
}

bool isCacheFull(const Cache *cache) {
    return cache->trueSize == cache->size;
}

void evictClock(Cache *cache, char *item) {
    if (isCacheFull(cache) == true) {
        //int existingIndex = -1;
        for (int i = 0; i < cache->count; i++) {
            if (cache->entries[i].valid == 1) {
                //existingIndex = i;
                for (int j = i; j < cache->count; j++) {
                    //cache->entries[existingIndex].valid = 0;
                    if (cache->entries[j].used == 1) { //OUT OF THE FOR LOOP? LIKE IN LRU?
                        cache->entries[j].used = 0;
                        cache->entries[j].valid = 0;
                        strcpy(cache->entries[cache->count].data, cache->entries[j].data);
                        cache->entries[cache->count].valid = 1;
                        cache->entries[cache->count].used = 0;
                        cache->count++;
                    } else {
                        cache->entries[j].used = 0;
                        cache->entries[j].valid = 0;
                        strcpy(cache->entries[cache->count].data, item);
                        cache->entries[cache->count].valid = 1;
                        cache->entries[cache->count].used = 0;
                        cache->count++;
                        cache->trueSize++;
                        return;
                    }

                } //return;
                //break;
            }
        }
    } else {
        strcpy(cache->entries[cache->count].data, item);
        cache->entries[cache->count].valid = 1;
        cache->entries[cache->count].used = 0;
        cache->count++;
        cache->trueSize++;
        return;
    }
}
/*while (1) {
        if (cache->entries[cache->nextIndex].valid == 0
            || cache->entries[cache->nextIndex].used == 0) {
            cache->nextIndex = (cache->nextIndex + 1) % cache->size;
            return;
        } else {
            if (cache->entries[cache->nextIndex].used > 0) {
                cache->entries[cache->nextIndex].used += 1;
            } else {
                cache->entries[cache->nextIndex].used = 1;
            }
            cache->nextIndex = (cache->nextIndex + 1) % cache->size;
        }
    }*/

/*void FIFO(Cache *cache){
    /printf("IN FIFO!!!!!!!!!!!!!!!!!!\n");
    //int index = cache->count % cache->size;
    cache->count++;
    return;
}*/

void evictLRU(Cache *cache, char *item) {
    //printf("IN LRU!!!!!!!!!!!!!!!!!\n");
    // Check if the item is already in the cache
    if (isCacheFull(cache) == true) {
        int existingIndex = -1;
        for (int i = 0; i < cache->count; i++) {
            if (cache->entries[i].valid == 1) {
                existingIndex = i;
                cache->entries[existingIndex].valid = 0;
                break;
            }
        }
        //if (existingIndex != -1) {
        //for (int i = existingIndex; i < cache->count; i++) {///-1
        //    cache->entries[i] = cache->entries[];///
        //}
        strcpy(cache->entries[cache->count].data, item);
        cache->entries[cache->count].valid = 1;
        cache->count++;
        return;
        //} else {
        //for (int i = 0; i < cache->count; i++) {
        //cache->entries[i] = cache->entries[i];
        //}
        //strcpy(cache->entries[cache->count].data, item);
        //cache->entries[cache->count].valid = 1;
        //cache->count++;
        //return -1;
    } else {
        strcpy(cache->entries[cache->count].data, item);
        cache->entries[cache->count].valid = 1;
        cache->count++;
        cache->trueSize++;
        return;
    }
}

void accessCache(
    Cache *cache, char *item, char policy, int *compulsoryMisses, int *capacityMisses) {
    int existingIndex = -1;
    //fprintf(stderr, "%s: iterating thru cache\n", item);
    for (int i = 0; i < cache->count; i++) {
        //fprintf(stderr, "%s %d | ",cache->entries[i].data, cache->entries[i].valid);
        if (cache->entries[i].valid == 1 && strcmp(cache->entries[i].data, item) == 0) {
            existingIndex = i;
            //printf("%s %d | ",cache->entries[i].data, cache->entries[i].valid);
            break;
        }
    }
    //fprintf(stderr, "\n");

    if (existingIndex != -1) {
        //cache->entries[existingIndex].used = 1;
        //printf("HIT\n");
        if (policy == 'L') {
            cache->entries[existingIndex].valid = 0;
            strcpy(cache->entries[cache->count].data, item);
            cache->entries[cache->count].valid = 1;
            cache->count++;
        }
        if (policy == 'C') {
            //cache->entries[existingIndex].valid = 0;
            cache->entries[existingIndex].used = 1;
            //strcpy(cache->entries[cache->count].data, item);
            //cache->entries[cache->count].valid = 1;
            //cache->entries[cache->count].used = 1;
            //cache->count++;
        }

        printf("HIT\n");
        return;
    } else {
        printf("MISS\n");
        int missIndex = -1;
        for (int i = 0; i <= cache->count; i++) {
            if (strcmp(cache->entries[i].data, item) == 0) {
                missIndex = i;
                break;
            }
        }
        if (missIndex != -1) {
            (*capacityMisses)++;
        } else {
            (*compulsoryMisses)++;
        }
        if (policy == 'C') {
            evictClock(cache, item);
        } else if (policy == 'F') {
            evictLRU(cache, item);
        } else if (policy == 'L') {
            evictLRU(cache, item);
        } else {
            evictLRU(cache, item);
        }
    }
}

void freeCache(Cache *cache) {
    free(cache->entries);
}

int main(int argc, char *argv[]) {
    int cacheSize = 0;
    char *evictionPolicy = "-F"; // Default to FIFO

    if (argc == 4 && strcmp(argv[1], "-N") == 0) {
        cacheSize = atoi(argv[2]);
        evictionPolicy = argv[3];

        if (!(strcmp(evictionPolicy, "-F") == 0 || strcmp(evictionPolicy, "-L") == 0
                || strcmp(evictionPolicy, "-C") == 0)) {
            fprintf(stderr, "Invalid policy. Use -F (FIFO), -L (LRU), or -C (Clock).\n");
            return 1;
        }
    } else {
        fprintf(stderr, "Invalid arguments. Usage: %s [-N size] <policy>\n", argv[0]);
        return 1;
    }

    //fprintf(stderr, "CACHE SIZE = %d\n", cacheSize);

    if (strcmp(evictionPolicy, "-F") == 0) {
        evictionPolicy = "F";
    } else if (strcmp(evictionPolicy, "-C") == 0) {
        evictionPolicy = "C";
    } else if (strcmp(evictionPolicy, "-L") == 0) {
        evictionPolicy = "L";
    } else {
        evictionPolicy = "F";
    }

    Cache cache;
    initCache(&cache, cacheSize);

    char item[256];
    int compulsoryMisses = 0;
    int capacityMisses = 0;
    while (scanf("%s", item) != EOF) {
        //printf("%s\n", evictionPolicy);
        accessCache(&cache, item, *evictionPolicy, &compulsoryMisses, &capacityMisses);
    }
    //printf("%s",evictionPolicy);
    //printLRUQueue(&cache);
    printf("%d %d\n", compulsoryMisses, capacityMisses);
    freeCache(&cache);
    return 0;
}
