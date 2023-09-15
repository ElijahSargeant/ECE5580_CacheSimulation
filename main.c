#include <stdio.h>

/******* CLI INPUTS *******
 * 
 * 
 * 
*/


int main (int argc, char *argv[])
{
    int counter;

    long fileSize;

    unsigned int address, configuredAccesses, i;
    char traceFileName[32], operation;
    FILE* fin;
    
    fin = fopen(traceFileName, "rb");
    
    if(fin == 0) { printf("Unable to open trace file\n"); exit(0); }


    /* configuredAccesses is the number of accesses to simulate */
    for(i=0; i<configuredAccesses; i++)
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