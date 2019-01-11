#include "dsm.h"
#include "common.h"

int main(int argc, char **argv)
{
   char *pointer;
   char *current;
   int value = 10;

   pointer = dsm_init(argc,argv);
   current = pointer;

   printf("[%i] Coucou, mon adresse de base est : %p\n", DSM_NODE_ID, pointer);

   if (DSM_NODE_ID == 0)
     {
       current += 2*PAGE_SIZE;
       current += 16;
       *current = 42;
       value = *((int *)current);
       printf("[%i] valeur de l'entier : %i\n", DSM_NODE_ID, value);
     }
   else if (DSM_NODE_ID == 1)
     {
       current += 2*PAGE_SIZE;
       current += 16;
       printf("[%i] valeur de l'entier : %i\n", DSM_NODE_ID, value);
       value = *((int *)current);
       printf("[%i] valeur de l'entier : %i\n", DSM_NODE_ID, value);
       fflush(stdout);
     }
   dsm_finalize();
}
