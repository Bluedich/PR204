#include "common_impl.h"

int main(int argc, char **argv)
{
   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */
   int i;
   char **newargv = malloc((argc)*sizeof(char *));
   for(i=0;i<(argc);i++){
     newargv[i] = malloc(BUFFER_SIZE*sizeof(char));
   }
   for(i=0;i<argc-1;i++){
     strcpy(newargv[i],argv[i+1]);
   }
   newargv[i+1] = NULL;

   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */

   /* Envoi du nom de machine au lanceur */

   /* Envoi du pid au lanceur */

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */

   /* on execute la bonne commande */
   if(-1==execvp(newargv[0], newargv)) ERROR_EXIT("ERROR doing execv in dsmwrap");
   return 0;
}
