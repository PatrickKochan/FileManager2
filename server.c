/*
Universidade Federal de Santa Catarina - UFSC
Centro de Ciências, Tecnologias e Saúde
Departamento de Computação
Curso de Engenharia da Computação

Alunos: Matheus André Soares - 15103102 
        Patrick Davila Kochan - 15102827
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>

//Criar variavel do mutex. Deve ser global.
pthread_mutex_t cadeado;

//Definição das funcoes
int create_socket();
void* comando(void* acceptR_);

int main(){

    int socketR = create_socket();
    printf("Criou o socket\n");

	while(1){

        // Aceita conexoes
        int *acceptR = calloc(sizeof(int), 1);

        printf("\nEsperando conexao\n");		
        *acceptR = accept(socketR, (struct sockaddr*)NULL, NULL);

        if(*acceptR == -1){
            printf("Error in function accept\n");
            exit(0);
        }
        printf("Conexao criada\n");

        pthread_t tid;
        pthread_create(&tid, NULL, comando, acceptR);
        printf("Thread criada.\n");

	}

	return 0;
}



int create_socket(){
    struct sockaddr_in socketAddr;

    //Configuracao do socket
	socketAddr.sin_family = AF_INET;						// Familia do endereço
	socketAddr.sin_port = htons(7000);						// Porta escolhida (htons converte o valor na ordem de bytes da rede)
	socketAddr.sin_addr.s_addr = inet_addr("127.0.0.1");	// Endereço IP, localhost (inet_addr converte o endereço para binario em bytes)
	bzero(&(socketAddr.sin_zero), 8);						// Prevencao de bug para arquiteturas diferentes


	/* Inicializa o socket:
    Dominio da comunicacao (AF_NET p/ TCP/IP),
    Tipo de comunicacao (TCP/UDP) (SOCK_STREAM p/ TCP)
    Protocolo (0) */
	int socketR = socket(AF_INET, SOCK_STREAM, 0);
	if(socketR == -1){
		printf ("Erro ao criar o socket.\n");
		exit(0);
	}

	//Associa o socket criado com a porta do SO
	int bindR = bind(socketR, (struct sockaddr *)&socketAddr, sizeof(socketAddr));
	if(bindR == -1){
		printf("Erro na funcao bind.\n");
		exit(0);
	}

    //Habilita o socket pra receber as conexoes atraves do descritor (socketR) do socket
	int listenR = listen(socketR, 10);
	if(listenR == -1){
		printf("Erro na funcao listen.\n");
		exit(0);
	}

	return socketR;
}


void* comando(void* acceptR_){
    int* conexao = (int*)acceptR_;
    char sendBuffer[1024], recvBuffer[1024];

    memset(sendBuffer, 0, sizeof(sendBuffer));

    strcpy(sendBuffer, "Conexao realizada com sucesso!\n");
    send(*conexao, sendBuffer, strlen(sendBuffer), 0);

    while (strcmp(recvBuffer, "exit") != 0){

        //Retorna o tamanho da String que o cliente escreveu
        memset(recvBuffer, 0, sizeof(recvBuffer));
        int recvR = recv(*conexao, recvBuffer, 100, 0);

        //Adiciona um \0 ao final da string para indicar seu termino
        recvBuffer[recvR - 1] = '\0'; 

        //criar (sub)diretorio
        if (strncmp(recvBuffer, "mkdir ", 6) == 0){
            pthread_mutex_lock(&cadeado);
            system(recvBuffer);
            printf("Pasta criada com sucesso\n");
            pthread_mutex_unlock(&cadeado);
        }

        //remover (sub)diretorio
        if (strncmp(recvBuffer, "rm -r ", 6) == 0){
            pthread_mutex_lock(&cadeado);
            system(recvBuffer);
            printf("Pasta excluida com sucesso\n");
            pthread_mutex_unlock(&cadeado);
        }

        //entrar em (sub)diretorio
        if (strncmp(recvBuffer, "cd ", 3) == 0){
            pthread_mutex_lock(&cadeado);
            memmove(recvBuffer, recvBuffer + 3, strlen(recvBuffer));
            chdir(recvBuffer);
            printf("Entrando no diretorio %s\n", recvBuffer);
            pthread_mutex_unlock(&cadeado);
        }

        //mostrar conteudo do diretorio
        if (strcmp(recvBuffer, "ls") == 0){

            DIR *diretorioAtual = opendir(".");
            struct dirent *diretorio;
            
            if(!diretorioAtual){
                printf("Erro ao abrir diretorio\n");
                break;
            }

            memset(sendBuffer, 0, sizeof(sendBuffer));
             
            /*readdir() Retorna um ponteiro do tipo dirent apontando para o 
            proximo diretorio na lista*/
            while(diretorio = readdir(diretorioAtual)){
                strcat(sendBuffer, diretorio->d_name);
                strcat(sendBuffer, "\t");
            }
            strcat(sendBuffer, "\n");
            send(*conexao, sendBuffer, strlen(sendBuffer), 0);
        }

        //criar arquivo
        if (strncmp(recvBuffer, "touch ", 6) == 0){
            pthread_mutex_lock(&cadeado);
            system(recvBuffer);
            printf("Arquivo criado com sucesso\n");
            pthread_mutex_unlock(&cadeado);
        }

        //remover arquivo
        if ((strncmp(recvBuffer, "rm ", 3) == 0) && (strncmp(recvBuffer, "rm -r ", 6) != 0) ){
            pthread_mutex_lock(&cadeado);
            system(recvBuffer);            
            printf("Arquivo removido com sucesso\n");
            pthread_mutex_unlock(&cadeado);
        }

        //escrever um sequencia de caracteres em um arquivo
        if (strncmp(recvBuffer, "echo ", 5) == 0){
            pthread_mutex_lock(&cadeado);
            system(recvBuffer);
            printf("Caracteres inseridos com sucesso\n");
            pthread_mutex_unlock(&cadeado);
        }

        //mostrar conteudo do arquivo
        if (strncmp(recvBuffer, "cat ", 4) == 0){
            pthread_mutex_lock(&cadeado);

            FILE *fn;
            
            memmove(recvBuffer, recvBuffer + 4, strlen(recvBuffer));
            fn = fopen(recvBuffer, "r");

            if(fn == NULL){
                printf("Erro lendo o arquivo\n");
                break;
            }

            if(fgets(sendBuffer, 1024, fn) != NULL){
                printf("Lendo o arquivo\n");
            }

            printf("Arquivo lido\n");

            fclose(fn);
            free(fn);

            send(*conexao, sendBuffer, strlen(sendBuffer), 0);
            pthread_mutex_unlock(&cadeado);
        }

    }
    printf("Conexao fechada\n");
}
