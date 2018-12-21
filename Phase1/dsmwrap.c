#include "common.h"

void get_addr_info(const char* addr, const char* port, struct addrinfo** res){
  assert(res);
  int status;
  struct addrinfo hints;

  memset(&hints,0,sizeof(hints));

  hints.ai_family=AF_INET;
  hints.ai_socktype=SOCK_STREAM;

  status = getaddrinfo(addr,port,&hints,res);
  if(status!=0){
    printf("getaddrinfo: returns %d aka %s\n", status, gai_strerror(status)); //fonction qui renvoie un message en rapport avec l'erreur détectée
    exit(1);
  }
}

void do_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  assert(addr);
  int res = connect(sockfd, addr, addrlen);
  if (res != 0) {
    ERROR_EXIT("ERROR connecting");
  }
  //printf("> Connected to host.\n");
}

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
   fflush(stdout);
   //connect to initialisation socket
   do_connect(sock_init, res->ai_addr, res->ai_addrlen);

   /* Envoi du nom de machine & longeur nom au lanceur */
   memset(buffer, 0, BUFFER_SIZE);
   gethostname(buffer, BUFFER_SIZE);
   sprintf(buffer2, "%d", (int) strlen(buffer));
   writeline(sock_init, buffer2, BUFFER_SIZE);
   writeline(sock_init, buffer, BUFFER_SIZE);

   /* Envoi du pid au lanceur */
   int pid = getpid();
   sprintf(buffer, "%d", pid);
   writeline(sock_init, buffer, BUFFER_SIZE);

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */
   printf("Port dans dsmwrap : %d", port_listen);
   sprintf(buffer, "%d", port_listen);
   writeline(sock_init, buffer, BUFFER_SIZE);

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
