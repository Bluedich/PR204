#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>

/* autres includes (eventuellement) */

#define ERROR_EXIT(str) {perror(str);exit(EXIT_FAILURE);}
#define BUFFER_SIZE 1024

typedef enum {LISTEN, CONNECT} SOCK_TYPE;

/* definition du type des infos */
/* de connexion des processus dsm */
struct dsm_proc_conn  {
   int pid;
   int port;
   int name_length;
   char name[BUFFER_SIZE];
};

int readline(int fd, char * buffer, int maxlen);

int do_read(int fd, void * buffer, int to_read);

int writeline(int fd_rcv, char * buffer, int maxlen);

int do_write(int fd_rcv, void * buffer, int to_send);

typedef struct dsm_proc_conn dsm_proc_conn_t;

void test_conn_info(dsm_proc_conn_t * conn_infos, int num_procs);

void get_addr_info(const char* addr, const char* port, struct addrinfo** res);

void do_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int do_accept(int sock, struct sockaddr * c_addr, socklen_t * c_addrlen);

/* definition du type des infos */
/* d'identification des processus dsm */
struct dsm_proc {
  pid_t pid;
  dsm_proc_conn_t connect_info;
};
typedef struct dsm_proc dsm_proc_t;

int creer_socket(SOCK_TYPE type, int *port_num);
