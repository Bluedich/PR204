#include "dsm.h"
#include "common.h"


int DSM_NODE_NUM; /* nombre de processus dsm */
int DSM_NODE_ID;  /* rang (= numero) du processus */

/* indique l'adresse de debut de la page de numero numpage */
static char *num2address( int numpage )
{
   char *pointer = (char *)(BASE_ADDR+(numpage*(PAGE_SIZE)));
   if( pointer >= (char *)TOP_ADDR ){
      fprintf(stderr,"[%i] Invalid address !\n", DSM_NODE_ID);
      return NULL;
   }
   else return pointer;
}

/* fonctions pouvant etre utiles */
static void dsm_change_info( int numpage, dsm_page_state_t state, dsm_page_owner_t owner)
{
   if ((numpage >= 0) && (numpage < PAGE_NUMBER)) {
	if (state != NO_CHANGE )
	table_page[numpage].status = state;
      if (owner >= 0 )
	table_page[numpage].owner = owner;
      return;
   }
   else {
	fprintf(stderr,"[%i] Invalid page number !\n", DSM_NODE_ID);
      return;
   }
}

int address2num( void * addr){

  return (( (long int)(addr-BASE_ADDR))/(PAGE_SIZE));
}

static dsm_page_owner_t get_owner( int numpage)
{
   return table_page[numpage].owner;
}

static dsm_page_state_t get_status( int numpage)
{
   return table_page[numpage].status;
}

/* Allocation d'une nouvelle page */
static void dsm_alloc_page( int numpage )
{
   char *page_addr = num2address( numpage );
   mmap(page_addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   return ;
}

/* Changement de la protection d'une page */
static void dsm_protect_page( int numpage , int prot)
{
   char *page_addr = num2address( numpage );
   mprotect(page_addr, PAGE_SIZE, prot);
   return;
}

static void dsm_free_page( int numpage )
{
   char *page_addr = num2address( numpage );
   munmap(page_addr, PAGE_SIZE);
   return;
}

static void *dsm_comm_daemon( void *arg)
{
  fflush(stdout);
  int retval;
  char buffer[BUFFER_SIZE];
  struct pollfd * fds = malloc((DSM_NODE_NUM-1)*sizeof(struct pollfd));
  int i;
  for(i=0;i<DSM_NODE_ID;i++){
    fflush(stdout);
    fds[i].fd = sock_tab[i];
    fds[i].events = POLLIN;
  }
  for(i=DSM_NODE_ID+1;i<DSM_NODE_NUM;i++){
    fflush(stdout);
    fds[i-1].fd = sock_tab[i];
    fds[i-1].events = POLLIN;
  }
  fflush(stdout);
  while(1){
    printf("[%i] (dsm_comm_daemon) Waiting for incoming reqs \n", DSM_NODE_ID);
    fflush(stdout);
    retval = poll(fds, DSM_NODE_NUM-1, -1);
    if(retval == -1) ERROR_EXIT("ERROR polling");
    for(i=0;i<DSM_NODE_NUM-1;i++){
      if(fds[i].revents&POLLIN){
        memset(buffer, 0, BUFFER_SIZE);
        int size_read = readline(fds[i].fd, buffer, BUFFER_SIZE);
        if(size_read==0) ERROR_EXIT("ERROR reading : remote socket was closed");
        printf("[%i] Daemon à reçu : %s\n", DSM_NODE_ID, buffer);
        fflush(stdout);
      }
    }
  }
  return;
}

static int dsm_send(int dest,void *buf,size_t size)
{
   /* a completer */
}

static int dsm_recv(int from,void *buf,size_t size)
{
   /* a completer */
}

static void dsm_handler( int page_num, void * addr)
{
  char buffer[BUFFER_SIZE];
   printf("[%i] FAULTY  ACCESS !!! \n",DSM_NODE_ID);
   memset(buffer, 0, BUFFER_SIZE);
   sprintf(buffer, "[%i] a reçu SIGSEGV à l'adresse : %p",DSM_NODE_ID, addr);
   fflush(stdout);
   writeline(sock_tab[get_owner(page_num)], buffer, BUFFER_SIZE);
   while(1){}
}

/* traitant de signal adequat */
static void segv_handler(int sig, siginfo_t *info, void *context)
{
   /* A completer */
   /* adresse qui a provoque une erreur */
   void  *addr = info->si_addr;
  //  printf("addresse qui a provoque l'erreur %i",info->si_addr);

  /* Si ceci ne fonctionne pas, utiliser a la place :*/
  /*
   #ifdef __x86_64__
   void *addr = (void *)(context->uc_mcontext.gregs[REG_CR2]);
   #elif __i386__
   void *addr = (void *)(context->uc_mcontext.cr2);
   #else
   void  addr = info->si_addr;
   #endif
   */
   /*
   pour plus tard (question ++):
   dsm_access_t access  = (((ucontext_t *)context)->uc_mcontext.gregs[REG_ERR] & 2) ? WRITE_ACCESS : READ_ACCESS;
  */


   /* adresse de la page dont fait partie l'adresse qui a provoque la faute */
   void  *page_addr  = (void *)(((unsigned long) addr) & ~(PAGE_SIZE-1));

   if ((addr >= (void *)BASE_ADDR) && (addr < (void *)TOP_ADDR))
     {
	dsm_handler(address2num(page_addr), addr);
     }
   else
     {
	/* SIGSEGV normal : ne rien faire*/
     }
}

/* Seules ces deux dernieres fonctions sont visibles et utilisables */
/* dans les programmes utilisateurs de la DSM                       */
char *dsm_init(int argc, char **argv)
{
   struct sigaction act;
   int index;
   int sock_init = atoi(argv[argc-2]);
   int sock_listen = atoi(argv[argc-1]);
   char buffer[BUFFER_SIZE];

   /* reception du nombre de processus dsm envoye */
   /* par le lanceur de programmes (DSM_NODE_NUM)*/
   readline(sock_init, buffer, BUFFER_SIZE);
   DSM_NODE_NUM = atoi(buffer);

   /* reception de mon numero de processus dsm envoye */
   /* par le lanceur de programmes (DSM_NODE_ID)*/
   readline(sock_init, buffer, BUFFER_SIZE);
   DSM_NODE_ID = atoi(buffer);

   /* reception des informations de connexion des autres */
   /* processus envoyees par le lanceur : */
   /* nom de machine, numero de port, etc. */
   dsm_proc_conn_t * conn_infos = malloc(sizeof(dsm_proc_conn_t)*DSM_NODE_NUM);
   int size_read = do_read(sock_init, conn_infos, DSM_NODE_NUM*sizeof(dsm_proc_conn_t));
  //  printf("Size read : %d\n", size_read);
   //printf("DSM_NODE_ID : %d\n", DSM_NODE_ID);
   //test_conn_info(conn_infos, DSM_NODE_NUM);

   struct addrinfo* res_tab[DSM_NODE_NUM];
   sock_tab = malloc(sizeof(int)*DSM_NODE_NUM);

   /* initialisation des connexions */
   /* avec les autres processus : connect/accept */
   int i;
   int port;

   for (i=0;i<DSM_NODE_ID;i++){
     sock_tab[i] = creer_socket(CONNECT, &port);
     sprintf(buffer,"%d",conn_infos[i].port);
     get_addr_info(conn_infos[i].name, buffer, &res_tab[i]);
     do_connect(sock_tab[i],res_tab[i]->ai_addr, res_tab[i]->ai_addrlen);
     //printf("We (%d) connected to DSM_NODE %d\n", DSM_NODE_ID, i);
     fflush(stdout);
   }
   sock_tab[DSM_NODE_ID] = -1; // Pour indiquer que c'est nous
   listen(sock_listen, -1);
   for(i=DSM_NODE_ID+1;i<DSM_NODE_NUM;i++){
     sock_tab[i] = do_accept(sock_listen, NULL, 0);
     //printf("DSM_NODE %d connected to us (%d)\n", i, DSM_NODE_ID);
     fflush(stdout);
   }
   //printf("DSM_NODE %d connected to everybody\n", DSM_NODE_ID);
   fflush(stdout);

   /* Allocation des pages en tourniquet */
   for(index = 0; index < PAGE_NUMBER; index ++){
     if ((index % DSM_NODE_NUM) == DSM_NODE_ID)
       dsm_alloc_page(index);
     dsm_change_info( index, WRITE, index % DSM_NODE_NUM);
   }

   /* mise en place du traitant de SIGSEGV */
   act.sa_flags = SA_SIGINFO;
   act.sa_sigaction = segv_handler;
   sigaction(SIGSEGV, &act, NULL);

   /* creation du thread de communication */
   /* ce thread va attendre et traiter les requetes */
   /* des autres processus */
   dsm_daemon_arg_t daemon_arg;
   pthread_create(&comm_daemon, NULL, dsm_comm_daemon, (void *)(&daemon_arg));

   /* Adresse de début de la zone de mémoire partagée */
   return ((char *)BASE_ADDR);
}

void dsm_finalize( void )
{
   /* fermer proprement les connexions avec les autres processus */

   /* terminer correctement le thread de communication */
   /* pour le moment, on peut faire : */
   pthread_cancel(comm_daemon);

  return;
}
