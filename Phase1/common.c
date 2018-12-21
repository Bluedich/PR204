#include "common.h"

void test_conn_info(dsm_proc_conn_t * conn_infos, int num_procs){
  int i;
  dsm_proc_conn_t * conn_info;
  for(i=0;i<num_procs;i++){
    conn_info = conn_infos+i;
    printf("Process n°%d\n rank : %d\n num_porcs : %d\n pid : %d\n port : %d\n name_length : %d\n name : %s\n",i, conn_info->rank, conn_info->num_procs, conn_info->pid, conn_info->port, conn_info->name_length, conn_info->name);
  }
  fflush(stdout);
}

int do_socket(){
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd == -1)
    ERROR_EXIT("ERROR creating socket");
  int yes = 1;
  // set socket option, to prevent "already in use" issue when rebooting the server right on
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
      ERROR_EXIT("ERROR setting socket options");
  return fd;
}

void init_serv_addr(struct sockaddr_in * addr){
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = INADDR_ANY;
  addr->sin_port = 0;
}

int readline(int fd, char * buffer, int maxlen){
  assert(buffer);
  memset(buffer, 0, maxlen);
  int size_read = read(fd, buffer, maxlen);
  if(size_read==-1){
    ERROR_EXIT("ERROR reading line");
  }
  return size_read;
}

int writeline(int fd_rcv, char * buffer, int maxlen){
  assert(buffer);
  int i=0;
  int to_send=0;
  int sent=0;
  int res;
  to_send = maxlen;
   //try maximum of 1000 times
  do{
    res=write(fd_rcv, buffer+sent, to_send-sent);
    if (res==-1) ERROR_EXIT("ERROR writing line");
    sent+=res;
    i++;
  } while(to_send != sent && i<1000);
  return sent;
}

void do_bind(int sock, struct sockaddr_in * s_addr){
  assert(s_addr);
  if (bind(sock, (const struct sockaddr *) s_addr, sizeof(*s_addr)) == -1)
    ERROR_EXIT("ERROR binding");
}

int creer_socket(SOCK_TYPE type, int *port_num)
{
   int fd = 0;

   /* fonction de creation et d'attachement */
   /* d'une nouvelle socket */
   /* renvoie le numero de descripteur */
   /* et modifie le parametre port_num */

   //create the socket, check for validity!
   fd = do_socket();

   if(type == LISTEN){
     //init the serv_addr structure
     struct sockaddr_in addr;
     memset(&addr, 0, sizeof(addr));
     init_serv_addr(&addr);

     //perform the binding
     //we bind on the tcp port specified
     do_bind(fd, &addr);
     int addr_len = sizeof(addr);
     getsockname(fd, (struct sockaddr*) &addr, (socklen_t *) &addr_len);
     *port_num = (int) ntohs(addr.sin_port);
   }
   return fd;
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */
