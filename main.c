#include <stdio.h>


int main (int argc, char *argv[])
{
    int counter;

    unsigned int address, numEntries, i;
    char traceFileName[32], operation;
    FILE* fin;
    
    fin = fopen(traceFileName, "rb");
    
    if(fin == 0) { printf(“Unable to open trace file\n”); exit(0); }

    for(i=0; i<numEntries; i++)
    /* numEntries is the number of accesses to simulate */
    {
        fin.read((char*)&address, 4);
        fin.read((char*)&operation, 1);
        /* the variable ‘address’ now contains the address of the data being
        accessed, while the variable ‘operation’ contains the values ‘r’ for
        a read or ‘w’ for a write. */
    }
  
    return 0;
}