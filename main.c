#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

/********************************* CLI INPUTS **********************************
 * 
 * benchmark:       file to be sim                    EX: ammp
 * accesses:        number of accesses to simulate:   EX: 10000000 (10 million accesses)
 * size1:          size of L1 cache in bytes:        EX: 16384 (16kB)
 * a1:              associativity                     EX: 2 (2-way)
 * b1:              line size in bytes                EX: 64 (64 Byte lines)
 * victim_size1:    size victim cache (0 is none)     EX: 256 (256 Bytes)
 * size2:          size of L2 cache in bytes         EX: 524288 (512kB)
 * a2:              associativity                     EX: 8 (8-way)
 * b2:              line size in bytes                EX: 64 (64 Byte lines)
 * victim_size2:    size victim cache (0 is none)     EX: 1024 (1kB)
 * 
 * *****************************************************************************
*/

/********************************* CLI OUTPUTS *********************************
 * 
 * L1 hits
 * L1 victim hits
 * L1 cold misses
 * L1 capacity misses
 * L1 conflict misses
 * 
 * L2 hits
 * L2 victim hits
 * L2 cold misses
 * L2 capacity misses
 * L2 conflict misses
 * 
 * *****************************************************************************
*/

//struct to keep track of total accesses and misses within the cache

typedef struct _cache_t cache_t;
typedef struct
{
    ssize_t total_accesses;

    ssize_t total_misses;
    ssize_t cold_misses;
    ssize_t capacity_misses;
    ssize_t conflict_misses;
}cache_stats;

typedef struct
{
    unsigned int tag;
    unsigned char valid;
    unsigned char dirty;
    unsigned int last_used_time;
}cache_line;

/*
 * structure for direct mapped and fully associative
*/
typedef struct _cache_t
{
    cache_line* lines;
    unsigned int n;
    cache_stats stats;
    cache_t* victim;
}cache_t;

/*
 * for set associative cache
*/
typedef struct
{
    cache_t* sets;
    unsigned int n_sets;
    cache_stats stats;
    cache_t* victim;
}assoc_cache;

assoc_cache* init_assoc_cache(unsigned,unsigned,unsigned);
cache_t* init_cache(unsigned,unsigned);
cache_t* init_fa_cache(unsigned);
void deinit_assoc_cache(assoc_cache*);
void deinit_cache(cache_t*);



/*
 * initializing set-associative cache
 */
assoc_cache*
init_assoc_cache(unsigned n_lines, unsigned associativity,unsigned victim)
{
    assoc_cache* cache = realloc(NULL,sizeof(assoc_cache));
    unsigned n_sets = n_lines/associativity;
    cache->n_sets = n_sets;
    cache->sets = malloc(n_sets*sizeof(cache_t));
    //initialize each set of cache
    for(int i = 0; i < n_sets;++i)
    {
        cache->sets[i].n = associativity;
        cache->sets[i].lines = malloc(associativity*sizeof(cache_line));
        // initialize each line of set
        for(int j = 0;j < associativity; ++j)
        {
            cache->sets[i].lines[j].tag = 0;
            cache->sets[i].lines[j].dirty = 0;
            cache->sets[i].lines[j].valid = 0;
            cache->sets[i].lines[j].last_used_time = 0;
        }
    }
    cache->stats.total_accesses = 0;
    cache->stats.conflict_misses = 0;
    cache->stats.cold_misses = 0;
    cache->stats.total_misses = 0;
    cache->stats.capacity_misses = 0;
    if(victim)
        cache->victim = init_cache(victim/n_lines,0);
    else
        cache->victim = NULL;
    return cache;
}

/*
 * freeing memory associative cache
 */
void
deinit_assoc_cache(assoc_cache* cache)
{
    for(int i = 0;i<cache->n_sets;++i)
        free(cache->sets[i].lines);
    free(cache->sets);
    if(cache->victim)
        deinit_cache(cache->victim);
    free(cache);
}


/*
 * initializing both-direct mapped and fully-associative
 */
cache_t*
init_cache(unsigned n_lines,unsigned victim)
{
    cache_t* cache = realloc(NULL,sizeof(cache_t));
    cache->n = n_lines;
    cache->lines = realloc(NULL,n_lines * sizeof(cache_line));
    for(int i = 0;i<n_lines;++i)
    {
        cache->lines[i].tag = 0;
        cache->lines[i].dirty = 0;
        cache->lines[i].valid = 0;
        cache->lines[i].last_used_time = 0;
    }
    cache->stats.total_accesses = 0;
    cache->stats.conflict_misses = 0;
    cache->stats.cold_misses = 0;
    cache->stats.total_misses = 0;
    cache->stats.capacity_misses = 0;

    if(victim)
        cache->victim = init_cache(victim/n_lines,0);
    else
        cache->victim = 0;
    return cache;
}

void
deinit_cache(cache_t* cache)
{
    free(cache->lines);
    if(cache->victim)
        deinit_cache(cache->victim);
    free(cache);
}

void printResults(int cacheLevel, int hits, int victimHits, int cold, int capacity, int conflict) 
{
    printf("L%u Cache Stats:\t%u\t%u\t%u\t%u\t%u\n", cacheLevel, hits, victimHits, cold, capacity, conflict);
    printf("\n");
}

char* concat(const char *size1, const char *size2)
{
    char *result = malloc(strlen(size1) + strlen(size2) + 1);
    strcpy(result, size1);
    strcat(result, size2);
    return result;
}


int main (int argc, char *argv[])
{
    if(argc < 10) 
    {
        printf("Invalid arguments!"); 
        exit(0);
    }

    char* benchmark;
    unsigned int accesses, size1, assoc1, line_size1,
                 victim_size1, size2, assoc2, line_size2, victim_size2 = 0;

    //assigning cli args
    char* inter = concat("CacheonlyTraces/Traces/", argv[1]);
    benchmark = concat(inter, ".trace");
    accesses = atoi(argv[2]);
    
    size1 = atoi(argv[3]);
    assoc1 = atoi(argv[4]);
    line_size1 = atoi(argv[5]);
    victim_size1 = atoi(argv[6]);

    size2 = atoi(argv[7]);
    assoc2 = atoi(argv[8]);
    line_size2 = atoi(argv[9]);
    victim_size2 = atoi(argv[10]);

    printf("%s",benchmark);

    if((size1 == 0) || (assoc1 == 0) || (line_size1 == 0) || (size2 == 0) || (assoc2 == 0) || (line_size2 == 0)) 
    {
        printf("Inavlid parameters, one or more inputs was an invalid string or 0!");
        exit(0);
    }

    // creating fully associative L1 and L2 caches with no victim cache.
    cache_t* fa_L1 = init_cache(size1/line_size1,0);
    cache_t* fa_L2 = init_cache(size2/line_size2,0);

    void *L1, *L2;
    bool is_assoc1, is_assoc2 = false;
    if(assoc1)
    {
        L1 = init_assoc_cache(size1/line_size1,assoc1,victim_size1);
        is_assoc1 = true;
    }
    else
        L1 = init_cache(size1/line_size1,victim_size1);

    if(assoc2)
    {
        L2 = init_assoc_cache(size2/line_size2,assoc2,victim_size2);
        is_assoc2 = true;
    }
    else
        L2 = init_cache(size2/line_size2,victim_size2);




    

    FILE* fin;
    fin = fopen(benchmark, "rb");
    if(fin == 0) { printf("Unable to open trace file\n"); exit(0); }

    unsigned int address;
    char operation;


    for(unsigned int i=0; i<accesses; i++)
    {
        //address is 4 bytes (int)
        fread(&address, 1, 4, fin);
        
        //operation is 1 byte (char)
        fread(&operation, 1, 1, fin);
        /* the variable ‘address’ now contains the address of the data being
        accessed, while the variable ‘operation’ contains the values ‘r’ for
        a read or ‘w’ for a write. */

        //now, check the address against the L1 address. If equal, hit. else, got to L1 victim and check

        //the largest address below the one you need that is multiple of 64 is the cache line
        //EX: 16384 cache size in bytes, line size of 64B, we have 256 lines. With a
        
        //printf("address: %x operation: %c\n", address, operation);
        //printf("i=%d\n",i);

    }
    fclose(fin);

    deinit_cache(fa_L1);
    deinit_cache(fa_L2);
    if(is_assoc1)
        deinit_assoc_cache(L1);
    else
        deinit_cache(L1);

    if(is_assoc2)
        deinit_assoc_cache(L2);
    else
        deinit_cache(L2);

    return 0;
}
