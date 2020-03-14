#include "chat1.h"

int mygetline(char line[], int max)
{
        int nch = 0, c;
        max = max - 1; /* espaço for '\0' */

        while((c = getchar()) != EOF)
        {
                if(c == '\n')break;
                        if(nch < max)
                        {
                                line[nch] = c;
                                nch = nch + 1;
                        }
        }

        if(c == EOF && nch == 0) return EOF;

        line[nch] = '\0';

        return nch;
}

void readfiles(char *ficheiros[20])
{
	struct dirent *de;  // Pointer for directory entry

    	// opendir() returns a pointer of DIR type.
	DIR *dr = opendir(".");
  	int dr_length, existe;
	char nome[256];

	if (dr == NULL)  // opendir returns NULL if couldn't open directory
    	{
		printf("Could not open current directory" );
		return;
    	}

   	// Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
  	// for readdir()

	int i;
	int it = 0;
	
	while ((de = readdir(dr)) != NULL)
    	{
		existe = 0;
		strcpy(nome,de->d_name);

		//printf("%s\n",nome);
		if(!strcmp(de->d_name,"projectsvr") || !strcmp(de->d_name,"project"))
			existe = 1;

		for(i = 0; i < strlen(de->d_name); i++)
		{
			if(nome[i] == '.')
				existe = 1;
		}

	       	if(!existe)
		{
			ficheiros[it] = (char*)malloc(20);
			strcpy(ficheiros[it],de->d_name);
			//printf("%s\n",ficheiros[it]);
			it++;
		}
    	}

	for(it; it < 20 ; it++)
		ficheiros[it] = "0";


    	closedir(dr);
}

int conta_bytes(char fnome[20])
{
	struct stat sb;

	if (stat(fnome, &sb) == -1)
       	{
    		perror("stat");
	}
	else 
	{
    		//printf("File size: %d bytes\n",(int) sb.st_size);
	}

	return (int)sb.st_size;
}

void ler_ficheiro(char ficheiro[MAXFNAME],char texto[1024])
{
	char *abre, line[150], text[1024] = "";
	FILE *fp;
	
	fp = fopen(ficheiro,"r");

	while(!feof(fp))
		if(abre = fgets(line, 150, fp))
			strcat(text,line);

	//printf("%s\n",text);
	strcpy(texto,text);
	fclose(fp);
}

int main()
{
	char *ficheiros[20];

	int pid,i = 0,fd[2], id[2], id_shm,id_fshm, chave[2], chave_shm, tamanho = 10,r;
	int de;
	long type,type1;

	struct s_msg msg;
	struct s_shm *shm;
	struct f_shm *fshm;
       	struct msg fmsg;
	char *nome_f;
	pipe(fd);
	
	chave[0] = 1;
	chave[1] = 2;
	chave_shm = 3;
	
	id[0] = msgget(chave[0], IPC_CREAT | PERMISSOES);
	id[1] = msgget(chave[1], IPC_CREAT | PERMISSOES);
	
	id_shm=shmget(3, 0, 0); /*Obtem id*/
	shm = (struct s_shm *) shmat(id_shm,NULL,0);

     	pid_t pai = getpid();	
	
	if(!fork())
	{
		int count = 0;
		
		readfiles(ficheiros);
		
		close(fd[0]);		

		pid = getpid();

		write(fd[1],&pid,sizeof(pid));
		close(fd[1]);
		printf("PID = %d PPID = %d\n",getpid(),getppid());
	
		type = pid;	

		while(1)
		{
			while(msgrcv(id[0],(struct msgbuf*)&msg,sizeof(msg)-sizeof(long),type,IPC_NOWAIT)!=-1)
			{
				//printf("%s\n", msg.opcao);
				if(!strcmp(msg.opcao,"pesquisar"))
					for(i = 0; i < 20; i++)
					{
						if(!strcmp(ficheiros[i],msg.ficheiro))
						{		
							msg.para = msg.pid;
							//printf("Mensagem para: %ld\n",msg.para);
							strcpy(msg.opcao,"resposta");
							strcpy(msg.ficheiro, msg.ficheiro);
							msg.pid = type; 
					
							r = msgsnd(id[0],(struct msgbuf *) &msg, sizeof(msg)-sizeof(long), 0);
	                       		         	exit_on_error(r,"ERROUU");
						}
					}

				if(!strcmp(msg.opcao,"requisitar"))
				{
					int bytes;
					char conteudo[1024];

					//printf("fr\n");
					bytes = conta_bytes(msg.ficheiro);	
					
					if(bytes <= 1024)
					{
						//printf("Nome: %s\n", msg.ficheiro);
						ler_ficheiro(msg.ficheiro,conteudo);
						//printf("%s\n",conteudo);

						fmsg.type = msg.pid;
						strcpy(fmsg.nome,msg.ficheiro);
						strcpy(fmsg.conteudo,conteudo);

						r = msgsnd(id[1],(struct fmsgbuf *) &fmsg, sizeof(fmsg)-sizeof(long), 0);
		                                exit_on_error(r,"ERROUU");

					}
					else
					{
						printf("Ficheiro muito grande!");

					}
				}
			}
		}
	}
	else
	{	
		close(fd[1]);
		read(fd[0],&pid,sizeof(pid));
		
		int fid,fid1, id_f, id_f1;
		fid = fork();
		fid1 = fork();

		if(!fid)
			while(1)
			{
			
				sleep(30);
				type = getppid();

				while(msgrcv(id[0],(struct msgbuf*)&msg,sizeof(msg)-sizeof(long),type,IPC_NOWAIT)!=-1)
                                	printf("%s %s %d\n",msg.opcao, msg.ficheiro,msg.pid);
				
				while(msgrcv(id[1],(struct fmsgbuf*)&fmsg,sizeof(fmsg)-sizeof(long),type,IPC_NOWAIT)!=-1)
                                        printf("%s\n%s\n",fmsg.nome, fmsg.conteudo);
			}

		if(!fid1)
			while(1)
			{	
				//printf("%d\n", id_f);
				//printf("%d\n",shmget(3, 0, 0));
				if(shmget(3,0,0) == -1)
				{
					printf("\nO servidor desligou-se ou encontra-se desligado!\n");
					printf("Por favor carregue ctrl+c para sair!\n");
					
					exit(0);
				}
					
			}

		while(1)
		{		
			type = getpid();
			//printf("type pai = %ldn\n", type);
			//while(msgrcv(id[0],(struct msgbuf*)&msg,sizeof(msg)-sizeof(long),type,IPC_NOWAIT)!=-1)
                         //       printf("%s %s %d\n",msg.opcao, msg.ficheiro,msg.pid);

			r=mygetline(msg.opcao,MAXMSG);
			
			char aux[MAXMSG], op[MAXMSG] = "0";
			int count = 0;

			strcpy(aux,msg.opcao);
			//printf("%s\n", aux);
			for(i = 0; i < MAXMSG;i++)
			{
				if(aux[i] == ' ')
					count++;

				if(aux[i] == '\0')
					break;
			}
			//printf("Count: %d\n",count);

			if(count)
				for(i = 0; i < MAXMSG; i++)
				{
					op[i] = aux[i];

					if(aux[i+1] == ' ')
					{
						op[i+1] = '\0';
						break;
					}
				}
			
			
		//	printf("op: %s\n", op);


			if(!strcmp(msg.opcao,"ligar") || !strcmp(msg.opcao,"desligar"))
			{
				msg.para = 1;
				//printf("brotha");
				//msg.para = 1;
				//msg.type = 1; /*destino = projectsvr*/	
				msg.pid = pid;
				//printf("PID %d\n",msg.pid);
				r = msgsnd(id[0],(struct msgbuf *) &msg, sizeof(msg)-sizeof(long), 0);
				exit_on_error(r,"ERROUU");
				//printf("sup boi");
			}
			
			if(count == 1)
				if(!strcmp(op,"pesquisar"))
				{//char fnome[MAXFNAME];
					//printf("%d\n", strlen(op));
					long to;
					char fnome[MAXMSG] = "0";

					strcpy(msg.opcao, op);
					msg.pid = getpid();
					
					for(i = 10; i < MAXMSG; i++)
					{
						fnome[i-10] = aux[i];
						//printf("%c ",aux[i]);

						if(aux[i+1] == '\0')
							break;
					}

					strcpy(msg.ficheiro,fnome);
					
					for(i = 0; i < MAXSHM; i++)
					{
						if(shm->tabela[i] && shm->tabela[i] != pid)
						{
							//printf("%d\n", shm->tabela[i]);
							msg.para = shm->tabela[i];
							//printf("C2: %ld\n",msg.para);

							r = msgsnd(id[0],(struct msgbuf *) &msg, sizeof(msg)-sizeof(long), 0);
                      		        		exit_on_error(r,"ERROUU");
						}
					}
				
				}

			if(count == 2)
				if(!strcmp(op,"requisitar"))
                        	{
					char para[MAXMSG]= "0";
					char f_nome[MAXMSG] = "0";
					long to1;
					int k, c = 0;
					
					strcpy(msg.opcao,op);
                                        msg.pid = getpid();

                                        for(i = 11; i < MAXMSG; i++)
                                        {
                                                f_nome[i-11] = aux[i];
                                                //printf("%c",aux[i]);

                                                if(aux[i+1] == ' ')
						{
							k = i+2;
                                                        break;
						}
                                        }

					//printf("fnome = %s\n",f_nome);

					i = 0;

					for(k; k < MAXMSG; k++)
					{
						para[i] = aux[k];
						i++;

						if(aux[k+1] == '\0')
						{
							para[i+1] = '\0';
							break;
						}
					}
					i=0;
					//printf("pars = %s\n",para);

					to1 = atoi(para);
					msg.para = to1;
					strcpy(msg.ficheiro,f_nome);

                                        for(i = 0; i < MAXSHM; i++)
					{
                                                if(shm->tabela[i] == msg.para)
                                                {       
	      						c = 1;								
                                                	//printf("C2: %ld\n",msg.para);

                                                        r = msgsnd(id[0],(struct msgbuf *) &msg, sizeof(msg)-sizeof(long), 0);
                                                        exit_on_error(r,"ERROUU");
                                                }
                                        }
					if(!c)
						printf("O cliente %ld ao qual requisitou o ficheiro não está connectado!\n", msg.para);
					
					
				}
			
			
		//	while(msgrcv(id[0],(struct msgbuf*)&msg,sizeof(msg)-sizeof(long),type,IPC_NOWAIT)!=-1)
                 //             printf("%s %s %d\n",msg.opcao, msg.ficheiro,msg.pid);
		//	printf("%ld\n",type);
		//
			//type = getpid();
			//printf("tipo = %ld\n", type);

			//r=msgrcv(id[0], (struct msgbuf *) &msg, sizeof(msg)-sizeof(long),type,0);
                        //exit_on_error(r,"Erro na leitura de mensagem");
	//		while(msgrcv(id[0],(struct msgbuf*)&msg,sizeof(msg)-sizeof(long),type,IPC_NOWAIT)!=-1)
	//			printf("Mensagem recebida: tipo=%ld texto=%s %s %dn",msg.para,msg.opcao, msg.ficheiro,msg.pid);
			
			//printf("Nao existem mais mensagens\n");
			

		//	printf("%s ",msg.opcao);
	//		printf("%s ", msg.ficheiro);
//			printf("%d", msg.pid);
			//exit_on_error(r,"Erro no envio da mensagem para a fila\n");
		}	
	}

	return 0;
}

