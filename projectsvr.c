#include "chat1.h"

void sig_handler(int sig)
{                             
        printf("\n O servidor desligou-se!\n");
	system("ipcrm -Q 1"); 
	system("ipcrm -M 3");	
	exit(0);
	
}

int main(int argc, char *argv[])
{
	int chave, id, id_shm, r, i, j;
	bool existe;
	long type;

	struct s_msg msg;
	struct s_shm *shm;  
	
	/*Cria fila*/
	id=msgget(1, IPC_CREAT|IPC_EXCL|PERMISSOES);
	exit_on_error(id,"Erro ao tentar criar a fila");
	
	//id_shm=shmget(3, 0, 0); /*Obtem id*/
	id_shm = shmget(3, sizeof(struct s_shm), IPC_CREAT | PERMISSOES);
	shm = (struct s_shm *) shmat(id_shm,NULL,0);

	while (1) 
	{
		signal(SIGINT, sig_handler);

		/*Aguarda mensagens na fila */
		
		//char aux[200], aux1[200], aux2[20];
		int cliente,tamanho, j = 0;
		type=1; /* servidor = 1*/
		
		r=msgrcv(id, (struct msgbuf *) &msg, sizeof(msg)-sizeof(long),type,0);
		exit_on_error(r,"Erro na leitura de mensagem");
		
		//printf("aa");	
		existe = false;

		for(i = 0; i < MAXSHM; i++)
                                if(shm->tabela[i] == msg.pid)
                                {     
					existe = true;
					break;
		
				}
		
		if(!strcmp(msg.opcao,"ligar"))
		{
			if(!existe)
				for(i = 0; i < MAXSHM; i++)
                        		if(shm->tabela[i] == 0)
                                	{
           	                		shm->tabela[i] = msg.pid;
						printf("O cliente %d ligou-se!\n",msg.pid);
                                	    	break;
                                	}
			if(existe)
				printf("O cliente %d já se encontra ligado!\n",msg.pid);
		}
		
		if(!strcmp(msg.opcao,"desligar"))
		{
			if(existe)
				for(i = 0; i < MAXSHM; i++)
                        		if(shm->tabela[i] == msg.pid)
                                	{
                                		shm->tabela[i] = 0;
						printf("O cliente %d desconnectou-se!\n", msg.pid);
                                       	 	break;
                                	}
			if(!existe)
				printf("O cliente %d não se pode desconnectar pois não está ligado!\n",msg.pid);
		}
		
		/*
		for(i = 0; i < MAXSHM; i++)
			if(shm->tabela[i])
			{
				msg.para = shm->tabela[i];
				strcpy(msg.text, "\n O servidor desligou-se!\n");

				r = msgsnd(id,(struct msgbuf *) &msg, sizeof(msg)-sizeof(long), 0);
                                exit_on_error(r,"ERROUU");
			}
	
	*/
	}
}

