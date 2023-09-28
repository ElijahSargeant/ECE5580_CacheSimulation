#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

/********************************* CLI INPUTS **********************************
 * 
 * benchmark:       file to be sim                    EX: ammp
 * accesses:        number of accesses to simulate:   EX: 10000000 (10 million accesses)
 * size1:           size of L1 cache in bytes:        EX: 16384 (16kB)
 * a1:              associativity                     EX: 2 (2-way)
 * b1:              line size in bytes                EX: 64 (64 Byte lines)
 * victim_size1:    size victim cache (0 is none)     EX: 256 (256 Bytes)
 * size2:           size of L2 cache in bytes         EX: 524288 (512kB)
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

#define ADDRESS_LEN 32


//counter for LRU 
ssize_t total_cache_accesses = 0;
//struct to keep track of total accesses and misses within the cache

// to differentiate the sort of cache that isnt associative
typedef enum 
{
    direct_mapped = 1,
    fully_associative,
    associative
}cache_type;

typedef struct
{
    ssize_t total_accesses;
    ssize_t hits;
    ssize_t total_misses;
    ssize_t cold_misses;
    ssize_t capacity_misses;
    ssize_t conflict_misses;
}cache_stats;

typedef struct
{
    //direct-mapped and fully associative will always have 0 tag
    unsigned tag;
    unsigned valid: 1;
    unsigned dirty: 1;
    ssize_t last_used_time;
}cache_line;

/*
 * structure for direct mapped and fully associative
*/

typedef struct cache_t cache_t;

struct cache_t
{
    cache_line* lines;
    cache_t* sets;
    unsigned n;
    cache_stats stats;
    cache_t* victim;
    cache_type type;
};

//struct to keep extracted address components
typedef struct
{
    unsigned index;
    unsigned tag;
    unsigned fa_tag;
    unsigned block_offset;
}address_info;

unsigned get_mask(unsigned);
unsigned get_address_len(unsigned );
cache_t* init_assoc_cache(unsigned,unsigned,unsigned);
cache_t* init_cache(unsigned,unsigned);
cache_t* init_fa_cache(unsigned);
void deinit_assoc_cache(cache_t*);
void deinit_cache(cache_t*);
int access_cache(cache_t*,address_info,char);

//produce number for masking to get certain bits
unsigned
get_mask(unsigned length)
{
    unsigned mask = pow(2,length) - 1;
    return mask;
}
//get address bit length
unsigned
get_address_len(unsigned address)
{
    unsigned bits=0;
    while(address)
    {
        address>>=1;
        ++bits;
    }
    return bits;
}



/*
 * initializing set-associative cache
 */
cache_t*
init_assoc_cache(unsigned n_lines, unsigned associativity,unsigned victim)
{
    cache_t* cache = realloc(NULL,sizeof(cache_t));
    unsigned n_sets = n_lines/associativity;
    cache->n = n_sets;
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
    cache->stats.hits = 0;
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
deinit_assoc_cache(cache_t* cache)
{
    for(int i = 0;i<cache->n;++i)
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
    cache->stats.hits = 0;

    if(victim)
        cache->victim = init_cache(victim/n_lines,0);
    else
        cache->victim = 0;
    cache->sets = NULL;
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
    printf("L%u Cache Stats:\t%x\t%x\t%x\t%x\t%x\n", cacheLevel, hits, victimHits, cold, capacity, conflict);
    printf("\n");
}

char* concat(const char *size1, const char *size2)
{
    char *result = malloc(strlen(size1) + strlen(size2) + 1);
    strcpy(result, size1);
    strcat(result, size2);
    return result;
}

int
access_cache(cache_t* cache,address_info af,char op)
{
    
    int hit = 0;
        switch(cache->type)
        {
            case(direct_mapped):
                switch(op)
                {
                    case('r'):
                            if((cache->lines[af.index].valid)&&(cache->lines[af.index].tag == af.tag))
                            {

                                ++cache->stats.hits;
                                cache->lines[af.index].last_used_time = total_cache_accesses;
                                return 1;
                            }
                            else
                            {
                                if(!cache->lines[af.index].valid)
                                    ++cache->stats.cold_misses;
                                if((cache->lines[af.index].valid)&&(cache->lines[af.index].tag != af.tag))
                                    ++cache->stats.cold_misses;
                                cache->lines[af.index].valid = 1;
                                cache->lines[af.index].tag = af.tag;
                                ++cache->stats.total_misses;
                                return 0;
                            }
                        break;
                    case('w'):
                        if((cache->lines[af.index].valid)&&(cache->lines[af.index].tag == af.tag))
                        {
                            cache->lines[af.index].dirty = 1;
                            ++cache->stats.hits;
                            return 1;
                        }

                        if(!cache->lines[af.index].valid)
                            ++cache->stats.cold_misses;
                        cache->lines[af.index].tag = af.tag;
                        cache->lines[af.index].dirty = 1;
                        cache->lines[af.index].valid = 1;
                        ++cache->stats.total_misses;
                        return 0;
                }
                break;
            case(associative):
                switch(op)
                {
                    case('r'):
                        for(int set = 0;set < cache->sets[af.index].n; ++set)
                        {
                            if((cache->sets[af.index].lines[set].valid)&&(cache->sets[af.index].lines[set].tag == af.tag))
                            {
                                // it hit
                                cache->sets[af.index].lines[set].dirty = 1;
                                cache->sets[af.index].lines[set].last_used_time = total_cache_accesses;
                                ++cache->stats.hits;
                                return 1;
                            }
                       }
                        
                        //calculate LRU
                        //
                        unsigned oldest_block = UINT_MAX;
                        unsigned oldest_line = 0;
                        for(int set = 0;set < cache->sets[af.index].n;++set)
                        {
                            if(cache->sets[af.index].lines[set].last_used_time < oldest_block)
                            {
                                oldest_block = cache->sets[af.index].lines[set].last_used_time;
                                oldest_line = set;
                            }
                        }
                        if(!cache->sets[af.index].lines[oldest_line].valid)
                            ++cache->stats.cold_misses;
                        cache->sets[af.index].lines[oldest_line].tag = af.tag;
                        cache->sets[af.index].lines[oldest_line].valid = 1;
                        cache->sets[af.index].lines[oldest_line].dirty = 1;
                        cache->sets[af.index].lines[oldest_line].last_used_time = total_cache_accesses;
                        ++cache->stats.total_misses;
                        return 0;
                    
                    case('w'):
                            for(int set = 0;set < cache->sets[af.index].n;++set)
                            {
                                if((cache->sets[af.index].lines[set].valid)&&(cache->sets[af.index].lines[set].tag == af.tag))
                                {
                                    cache->sets[af.index].lines[set].dirty = 1;
                                    ++cache->stats.hits;
                                    cache->sets[af.index].lines[set].last_used_time = total_cache_accesses;
                                    return 1;
                                }
                            }
                            if(!hit)
                            {
                                unsigned oldest_block = UINT_MAX;
                                unsigned oldest_line = 0;
                                for(int set = 0;set<cache->sets[af.index].n;++set)
                                {
                                    if(cache->sets[af.index].lines[set].last_used_time < oldest_block)
                                    {
                                        oldest_block = cache->sets[af.index].lines[set].last_used_time;
                                        oldest_line = set;
                                    }
                                }
                            if(!cache->sets[af.index].lines[oldest_line].valid)
                                ++cache->stats.cold_misses;
                            cache->sets[af.index].lines[oldest_line].tag = af.tag;
                            cache->sets[af.index].lines[oldest_line].valid = 1;
                            cache->sets[af.index].lines[oldest_line].dirty = 1;
                            cache->sets[af.index].lines[oldest_line].last_used_time = total_cache_accesses;
                            cache->stats.total_misses++;
                            return 0;
                            }
                        break;
                }
                break;
            case(fully_associative):
                switch(op)
                {
                    case('r'):
                        for(int line = 0;line < cache->n;++line)
                        {
                            if((cache->lines[line].valid)&&(cache->lines[line].tag = af.tag))
                            {
                                cache->lines[line].valid = 1;
                                cache->lines[line].last_used_time = total_cache_accesses;
                                ++cache->stats.hits;
                                hit = 1;
                                return 1;
                            }
                        }
                            unsigned oldest_block = UINT_MAX;
                            unsigned oldest_line = 0;
                            for(int line = 0;line < cache->n;++line)
                            {
                                if(cache->lines[line].last_used_time < oldest_block)
                                {
                                    oldest_block = cache->lines[line].last_used_time;
                                    oldest_line = line;
                                }
                            }
                            if(!cache->lines[oldest_line].valid)
                                ++cache->stats.cold_misses;
                            cache->lines[oldest_line].dirty = 1;
                            cache->lines[oldest_line].valid = 1;
                            cache->lines[oldest_line].last_used_time = total_cache_accesses;
                            ++cache->stats.total_misses;
                            return 0;
                    


                        break;
                    case('w'):
                        for(int line = 0;line < cache->n;++line)
                        {
                            if((cache->lines[line].valid)&&(cache->lines[line].tag = af.tag))
                            {
                                cache->lines[line].valid = 1;
                                cache->lines[line].last_used_time = total_cache_accesses;
                                ++cache->stats.hits;
                                hit = 1;
                                return 1;
                            }
                        }
                        if(!hit)
                        {
                                unsigned oldest_block = UINT_MAX;
                                unsigned oldest_line = 0;
                                for(int line = 0;line < cache->n;++line)
                                {
                                    if(cache->lines[line].last_used_time < oldest_block)
                                    {
                                        oldest_block = cache->lines[line].last_used_time;
                                        oldest_line = line;
                                    }
                                }
                                if(!cache->lines[oldest_line].valid)
                                    ++cache->stats.cold_misses;
                                cache->lines[oldest_line].dirty = 1;
                                cache->lines[oldest_line].valid = 1;
                                cache->lines[oldest_line].last_used_time = total_cache_accesses;
                                ++cache->stats.total_misses;

                        }


                        break;
                }
                break;
        }
    return hit;
}

int 
main (int argc, char *argv[])
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

    unsigned block_offset1 = log2(line_size1);
    unsigned block_offset2 = log2(line_size2);
    unsigned total_lines1 = size1/line_size1;
    unsigned total_lines2 = size2/line_size2;
    unsigned n_sets1 = total_lines1/assoc1;
    unsigned n_sets2 = total_lines2/assoc2;
    printf("%s\n",benchmark);
    //printf("block_offset1 = %i, block_offset2 = %i, total_lines1 = %i, total_lines2 = %i, n_sets1 = %i, \nn_sets2 = %i, tag1 = %i, tag2 = %i\n",
    //        block_offset1,block_offset2,total_lines1,total_lines2,n_sets1,n_sets2,tag1,tag2);

    if((size1 == 0) || (assoc1 == 0) || (line_size1 == 0) || (size2 == 0) || (assoc2 == 0) || (line_size2 == 0)) 
    {
        printf("Inavlid parameters, one or more inputs was an invalid string or 0!");
        exit(0);
    }

    // creating fully associative L1 and L2 caches with no victim cache.
    cache_t* fa_L1 = init_cache(total_lines1,0);
    cache_t* fa_L2 = init_cache(total_lines2,0);
    fa_L1->type = fully_associative;
    fa_L2->type = fully_associative;

    cache_t *L1, *L2;
    if(assoc1-1)
    {
        L1 = init_assoc_cache(size1/line_size1,assoc1,victim_size1);
        L1->type = associative;
    }
    else
        L1 = init_cache(size1/line_size1,victim_size1);
    if(assoc2-1)
    {
        L2 = init_assoc_cache(size2/line_size2,assoc2,victim_size2);
        L2->type = associative;
    }
    else
        L2 = init_cache(size2/line_size2,victim_size2);

    if((total_lines1 == assoc1)&&(!(assoc1 - 1)))
        L1->type = fully_associative;
    else if(!(assoc1-1))
        L1->type = direct_mapped;

    if((total_lines2 == assoc2)&&(!(assoc2 - 1)))
        L2->type = fully_associative;
    else if(!(assoc2-1))
        L2->type = direct_mapped;
    

    unsigned index_bits1=0,index_bits2 =0;
    unsigned tag_bits1 =0,tag_bits2 = 0;
    if((!(assoc1-1)))
    {
        if(L1->type==direct_mapped)
        {
            index_bits1 = log2(total_lines1);
            tag_bits1 = ADDRESS_LEN - block_offset1 - index_bits1;
        }else
            tag_bits1 = ADDRESS_LEN - block_offset1;
    }
    unsigned faL1_tag = ADDRESS_LEN - block_offset1;
    if((!(assoc2-1)))
    {
        if(L2->type==direct_mapped)
        {
            index_bits2 = log2(total_lines2);
            tag_bits2 = ADDRESS_LEN - block_offset2 - index_bits2;
        }else
            tag_bits2 = ADDRESS_LEN - block_offset2;
    }
    unsigned faL2_tag = ADDRESS_LEN - block_offset1;
    if((assoc1-1))
    {
        L1->n = n_sets1;
        index_bits1 = log2(L1->n);
        tag_bits1 = ADDRESS_LEN - block_offset1 - index_bits1;
    }
    
    if((assoc2-1))
    {
        L2->n = n_sets2;
        index_bits2 = log2(L2->n);
        tag_bits2 = ADDRESS_LEN - block_offset2 - index_bits2;
    }
    
    printf("index bits1 = %i\tindex bits2 = %i\n",index_bits1,index_bits2);
    

    

    FILE* fin;
    fin = fopen(benchmark, "rb");
    if(fin == 0) { printf("Unable to open trace file\n"); exit(0); }

    uint32_t address = 0;
                                
    char operation = 0;

    for(unsigned int i=0; i<accesses; i++)
    {
        address_info L1_info = {0,0,0,0};
        address_info L2_info = {0,0,0,0};
        address_info L1af_info = {0,0,0,0};
        address_info L2af_info = {0,0,0,0};
//        printf("L1 stats:\nindex length:%x\ntag length: %x\nblock offset: %x\nline size: %x\n",index_bits1,tag_bits1,block_offset1,line_size1);
 //       printf("L2 stats:\nindex length:%x\ntag length: %x\nblock offset: %x\nline size: %x\n",index_bits2,tag_bits2,block_offset2,line_size2);
        
        //address is 4 bytes (int)
        fread(&address, 1, 4, fin);
        
        //operation is 1 byte (char)
        fread(&operation, 1, 1, fin);
        /* 
        * the variable ‘address’ now contains the address of the data being
        * accessed, while the variable ‘operation’ contains the values ‘r’ for
        * a read or ‘w’ for a write. 
        */
        
        L1_info.block_offset = address & get_mask(block_offset1);
        L1_info.tag = (address & (get_mask(tag_bits1)<<(block_offset1+index_bits1)))>>(block_offset1+index_bits1);
        if(index_bits1)
            L1_info.index = (address & (get_mask(index_bits1)<<block_offset1))>>block_offset1;
        L1_info.fa_tag = faL1_tag;

        L2_info.block_offset = address & get_mask(block_offset2);
        L2_info.tag= (address & (get_mask(tag_bits2)<<(block_offset2+index_bits2)))>>(block_offset2+index_bits2);
        if(index_bits2)
            L2_info.index = (address & (get_mask(index_bits2)<<block_offset2))>>block_offset2;
        L2_info.fa_tag = faL2_tag;

        L1af_info.block_offset = L1_info.block_offset;       
        L1af_info.tag = faL1_tag;
        L1af_info.fa_tag = faL1_tag;
        L1af_info.index = 0;

        L2af_info.block_offset = L2_info.block_offset;       
        L2af_info.tag = faL2_tag;
        L2af_info.fa_tag = faL2_tag;
        L2af_info.index = 0;

       // printf("Address:%x\n L1: lines:%u tag: %x, block_offset:%x, index: %x\n L2:tag: %x,block offset %x, index %x\n\n",
       //        address,L1->n,L1_info.tag,L1_info.block_offset,L1_info.index,L2_info.tag,L2_info.block_offset,L2_info.index);
        switch(operation)
        {
            case('r'):
                //if returns successful read
                access_cache(fa_L1,L1af_info,operation);
                access_cache(fa_L2,L2af_info,operation);
                if(access_cache(L1,L1_info,operation))
                    break;
                else
                {
                    //if missed go through victim cache then L2
                    //access_cache(L1->victim,L1af_info,operation);
                    access_cache(L2,L2_info,operation);
                }
                break;
            case('w'):
                access_cache(fa_L1,L1af_info,operation);
                access_cache(fa_L2,L2af_info,operation);
                if(access_cache(L1,L1_info,operation))
                    break;
                else
                {
                    //if missed go through victim cache then L2
                    //access_cache(L1->victim,L1af_info,operation);
                    access_cache(L2,L2_info,operation);

                }

                break;
        }
        ++total_cache_accesses;
        // on read miss, access victim cache THEN L2
        // on write miss, access L2
        // check through L1 first 
        
        //now, check the address against index_bits1 = log2(ASSOC_cache(L1)->n_sets); the L1 address. If equal, hit. else, got to L1 victim and check

        //the largest address below the one you need that is multiple of 64 is the cache line
        //EX: 16384 cache size in bytes, line size of 64B, we have 256 lines. With a
        
        //printf("address: %u operation: %c\n", address, operation);
        //printf("i=%d\n",i);
        
    }
    fclose(fin);
    free(inter);
    free(benchmark);
    printf("total cache accesses:%zu\n",total_cache_accesses);
    printf("L1 hits: %zu\tmiss:%zu\tcold:%zu\nL2:hits: %zu\tmiss:%zu\tcold:%zu\n\n",L1->stats.hits,
            L1->stats.total_misses,L1->stats.cold_misses,L2->stats.hits,L2->stats.total_misses,L2->stats.cold_misses);
    deinit_cache(fa_L1);
    deinit_cache(fa_L2);
    if(L1->type == associative)
        deinit_assoc_cache(L1);
    else
        deinit_cache(L1);

    if(L2->type == associative)
        deinit_assoc_cache(L2);
    else
        deinit_cache(L2);

    return 0;
}
