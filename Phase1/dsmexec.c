#include "common_impl.h"
#include <unistd.h>

/* variables globales */

/* un tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t *proc_array = NULL;

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

void usage(void)
{
  fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
  fflush(stdout);
  exit(EXIT_FAILURE);
}

void sigchld_handler(int sig)
{
   /* on traite les fils qui se terminent */
   /* pour eviter les zombies */
}

int nb_machine_files(char machine_file[]){
  FILE * f = fopen(machine_file,"r");
  if(f==NULL) ERROR_EXIT("ERROR opening machine_file");
  int n=0; //we don't want to count eof as a line
  char buffer[BUFFER_SIZE];
  //Count number of lines
  while(fgets(buffer, BUFFER_SIZE, f))
    n++;
  fclose(f);
  return n;
}

int read_machine_file(int n, char machine_file[], char * machine_names[]){
  FILE * f = fopen(machine_file,"r");
  if(f==NULL) ERROR_EXIT("ERROR opening machine_file");
  char buffer[BUFFER_SIZE];
  int i;
  for(i=0;i<n;i++){
    memset(buffer, 0, BUFFER_SIZE);
    memset(machine_names[i], 0, BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, f);
    strcpy(machine_names[i], buffer);
    machine_names[i][strlen(machine_names[i])-1] = '\0';
  }
  fclose(f);
  return n;
}


int main(int argc, char *argv[])
{
  char buffer[BUFFER_SIZE];
  if (argc < 3){
    usage();
  }
  else {
    pid_t pid;
    int num_procs;
    int i;

    /* Mise en place d'un traitant pour recuperer les fils zombies*/
    /* XXX.sa_handler = sigchld_handler; */

    /* 1- on recupere le nombre de processus a lancer */
    num_procs = nb_machine_files(argv[1]);
    char **machine_names = malloc(num_procs*sizeof(char *));
    for(i=0;i<num_procs;i++){
      machine_names[i] = malloc(BUFFER_SIZE*sizeof(char));
    }
    /* 2- on recupere les noms des machines : le nom de */
    /* la machine est un des elements d'identification */
    num_procs = read_machine_file(num_procs, argv[1],machine_names);
    printf("Machine file read with %d machines : %s %s\n", num_procs, machine_names[0], machine_names[1]);

    /* creation de la socket d'ecoute */
    /* + ecoute effective */

    /* creation des fils */
    int pipe_fd_out[2];
    int pipe_fd_err[2];
    int *pipe_out = (int *) malloc(num_procs*sizeof(int));
    int *pipe_err = (int *) malloc(num_procs*sizeof(int));
    char **newargv = malloc((argc+1)*sizeof(char *));
    for(i=0;i<(argc+1);i++){
      newargv[i] = malloc(BUFFER_SIZE*sizeof(char));
    }
    int res;
    for(i = 0; i < num_procs ; i++) {
      /* creation du tube pour rediriger stdout */
      res = pipe(pipe_fd_out);
      if(res==-1) ERROR_EXIT("ERROR creating stdout pipe");

      /* creation du tube pour rediriger stderr */
      res = pipe(pipe_fd_err);
      if(res==-1) ERROR_EXIT("ERROR creating stderr pipe");

      //creation du fils
      pid = fork();
      if(pid == -1) ERROR_EXIT("ERROR forking");

      if (pid == 0) { /* fils */
        printf("fils %d in  the place \n",i);

        //fermeture des extremités en lecture des pipes du fils
        close(pipe_fd_out[0]);
        close(pipe_fd_err[0]);

        /* redirection stdout */
        res=dup2(pipe_fd_out[1],STDOUT_FILENO);
        if(res==-1) ERROR_EXIT("ERROR redirection stdout to pipe");
        /* redirection stderr */
        res=dup2(pipe_fd_err[1],STDERR_FILENO);
        if(res==-1) ERROR_EXIT("ERROR redirection stderr to pipe");

        /* Creation du tableau d'arguments pour le ssh */
        strcpy(newargv[0],"ssh");
        strcpy(newargv[1],machine_names[i]);
        for(i=2;i<argc;i++){
          strcpy(newargv[i],argv[i]);
        }
        newargv[i]=NULL;

        /* jump to new prog : */
        printf("newargv : %s %s %s %s %s\n", newargv[0], newargv[1], newargv[2], newargv[3], newargv[4]);
        sleep(5);
        printf("Bonjour les enfants\n");
        if(-1==execvp("ssh",newargv)) ERROR_EXIT("ERROR doing execv");


      } else  if(pid > 0) { /* pere */
        // fermeture des extremités en ecriture des pipes
        close(pipe_fd_out[1]);
        close(pipe_fd_err[1]);
        pipe_out[i] = pipe_fd_out[0];
        pipe_err[i] = pipe_fd_err[0];
        printf("Dady's pipe redirected\n");


        num_procs_creat++;
      }
    }

    for(i = 0; i < num_procs ; i++){

      /* on accepte les connexions des processus dsm */

      /*  On recupere le nom de la machine distante */
      /* 1- d'abord la taille de la chaine */
      /* 2- puis la chaine elle-meme */

      /* On recupere le pid du processus distant  */

      /* On recupere le numero de port de la socket */
      /* d'ecoute des processus distants */
    }

    /* envoi du nombre de processus aux processus dsm*/

    /* envoi des rangs aux processus dsm */

    /* envoi des infos de connexion aux processus */

    /* gestion des E/S : on recupere les caracteres */
    /* sur les tubes de redirection de stdout/stderr */
    /*je recupere les infos sur les tubes de redirection
    jusqu'à ce qu'ils soient inactifs (ie fermes par les
    processus dsm ecrivains de l'autre cote ...)*/
    struct pollfd * fds = malloc(2*num_procs*sizeof(struct pollfd));
    //init pollfd structs
    for(i=0;i<num_procs;i++){
      fds[i].fd =pipe_out[i];
      fds[i].events = POLLIN;
    }
    for(i=num_procs;i<2*num_procs;i++){
      fds[i].fd = pipe_err[i-num_procs];
      fds[i].events = POLLIN;
    }
    while(1){
      poll(fds, 2*num_procs, -1);
      for(i=0;i<num_procs;i++){
        if (fds[i].revents & POLLIN) {
          memset(buffer, 0, BUFFER_SIZE);
          read(pipe_out[i], buffer, BUFFER_SIZE);
          printf("stdout of process n°%d : %s", i, buffer);
        }
      }
      for(i=num_procs;i<2*num_procs;i++){
        if (fds[i].revents & POLLIN) {
          memset(buffer, 0, BUFFER_SIZE);
          read(pipe_err[i-num_procs], buffer, BUFFER_SIZE);
          printf("stderr off process n°%d : %s", i-num_procs, buffer);
        }
      }
    }

  /* on attend les processus fils */

  /* on ferme les descripteurs proprement */

  /* on ferme la socket d'ecoute */
  }
  exit(EXIT_SUCCESS);
}
