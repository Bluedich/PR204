#include "common.h"

int main(int argc, char **argv)
{
   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */

   fflush(stdout);
   int i;
   char buffer[BUFFER_SIZE];
   char buffer2[BUFFER_SIZE];

   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */
   int port;
   int port_listen;
   int sock_init = creer_socket(CONNECT, &port);

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */
   int sock_listen = creer_socket(LISTEN, &port_listen);

   int newargc = argc-1;
   char **newargv = malloc((newargc)*sizeof(char *));
   for(i=0;i<(newargc);i++){
     newargv[i] = malloc(BUFFER_SIZE*sizeof(char));
   }
   for(i=0;i<newargc-2;i++){
     strcpy(newargv[i],argv[i+3]);
   }
   sprintf(newargv[newargc-2],"%d",sock_init);
   sprintf(newargv[newargc-1],"%d",sock_listen);
   newargv[newargc] = NULL;

   struct addrinfo* res;

   //get address info
   get_addr_info(argv[1], argv[2], &res);
   //connect to initialisation socket
   do_connect(sock_init, res->ai_addr, res->ai_addrlen);

   dsm_proc_conn_t * conn_info = malloc(sizeof(dsm_proc_conn_t));
   memset(conn_info, 0, sizeof(dsm_proc_conn_t));
   /* Envoi du nom de machine & longeur nom au lanceur */
   memset(buffer, 0, BUFFER_SIZE);
   gethostname(buffer, BUFFER_SIZE);
   strcpy(conn_info->name, buffer);
   conn_info->name_length = (int) strlen(buffer);
   conn_info->pid = getpid();
   conn_info->port = port_listen;

   int size_written = do_write(sock_init, conn_info, sizeof(dsm_proc_conn_t));
   printf("Size written : %d", size_written);

  //  for (i=0;i<argc;i++){
  //    printf("argument n %d : %s\n",i,argv[i]);
  //  }
  //  for (i=0;i<newargc+1;i++){
  //    printf("argument n %d : %s\n",i,newargv[i]);
  //  }
  //  fflush(stdout);

   if(-1==execvp(newargv[0], newargv)) ERROR_EXIT("ERROR doing execv in dsmwrap");
   return 0;
}
