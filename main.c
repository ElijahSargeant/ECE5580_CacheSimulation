#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/********************************* CLI INPUTS **********************************
 * 
 * benchmark: file to be sim                  EX: ammp
 * accesses:  number of accesses to simulate: EX: 10000000 (10 million accesses)
 * s1:        size of L1 cache in bytes:      EX: 16384 (16kB)
 * a1:        associativity                   EX: 2 (2-way)
 * b1:        line size in bytes              EX: 64 (64 Byte lines)
 * v1:        size victim cache (0 is none)   EX: 256 (256 Bytes)
 * s2:        size of L2 cache in bytes       EX: 524288 (512kB)
 * a2:        associativity                   EX: 8 (8-way)
 * b2:        line size in bytes              EX: 64 (64 Byte lines)
 * v2:        size victim cache (0 is none)   EX: 1024 (1kB)
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


int main (int argc, char *argv[])
{
    if(argc < 10) {printf("Invalid arguments!"); exit(0);}

    char* benchmark;
    unsigned int accesses, s1, a1, b1, v1, s2, a2, b2, v2;

    benchmark = argv[1];
    accesses = atoi(argv[2]);

    s1 = atoi(argv[3]);
    a1 = atoi(argv[4]);
    b1 = atoi(argv[5]);
    v1 = atoi(argv[6]);

    s2 = atoi(argv[7]);
    a2 = atoi(argv[8]);
    b2 = atoi(argv[9]);
    v2 = atoi(argv[10]);

    if((s1 == 0) || (a1 == 0) || (b1 == 0) || (v1 == 0) || (s2 == 0) || (a2 == 0) || (b2 == 0) || (v2 == 0)) 
    {
        printf("Inavlid parameters, one or more inputs was an invalid string or 0!");
        exit(0);
    }

    FILE* fin;
    fin = fopen(benchmark, "rb");
    if(fin == 0) { printf("Unable to open trace file\n"); exit(0); }

    unsigned int address;
    char operation;

    int hitsL1, victimHitsL1, coldL1, capacityL1, conflictL1;
    int hitsL2, victimHitsL2, coldL2, capacityL2, conflictL2;


    for(int i=0; i<accesses; i++)
    {
        //address is 4 bytes (int)
        fread(&address, 1, 4, fin);
        
        //operation is 1 byte (char)
        fread(&operation, 1, 1, fin);
        /* the variable ‘address’ now contains the address of the data being
        accessed, while the variable ‘operation’ contains the values ‘r’ for
        a read or ‘w’ for a write. */



    }
  
    return 0;
}


void printResults(int cacheLevel, int hits, int victimHits, int cold, int capacity, int conflict) 
{
    printf("L%u Cache Stats:\t%u\t%u\t%u\t%u\t%u\n", cacheLevel, hits, victimHits, cold, capacity, conflict);
    printf("\n");
}