#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#define exit_on_error(s,m) if (s < 0) { perror(m); exit(1); }
#define PERMISSOES 0666 /* permissões para outros utilizadores */
#define MAXMSG 100 /* assumir que chega */
#define MAXFNAME 256 /* máximo tamanho do nome dos ficheiros */
#define MAXCONT 1024 /* máximo tamanho do conteúdo dos ficheiros */
#define MAXSHM 10 /* máximo tamanho de clientes na SHM */
#define MAXFREAD 20 /* máximo número de ficheiros lidos */

struct s_msg
{
        long para; /* 1º campo: long obrigatório = PID destinatário*/
        int pid; /* PID remetente */
	int pid_d; /* PID destinatário */
	char opcao[MAXMSG]; /* Opção de mensagem */
        char ficheiro[MAXFNAME]; /* Nome do ficheiro */
};

struct msg 
{
	long type;
        char nome[MAXFNAME];
        char conteudo[MAXCONT];
};

struct s_shm
{
	int tabela[MAXSHM];
};

struct s_shm shm = {0};
