/*
Universidade Federal de Santa Catarina - UFSC
Centro de Ciências, Tecnologias e Saúde
Departamento de Computação
Curso de Engenharia da Computação

Alunos: Matheus André Soares - 15103102 
        Patrick Davila Kochan - 15102827
*/

// Includes
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
#include <fcntl.h>
#include <stdbool.h>

// Defines
#define NBLOCKS 10
#define BLOCK_SIZE 36
#define MAX_ARQ 2

// Structs
    // Inode
    struct inode_{
        char nome[5];
        char tipo[3];
        int bloco; 
        bool estado;
    };
    typedef struct inode_ inode;

    // Dados
    struct dado_{
        char nome[5];
        int inode;
    };  
    typedef struct dado_ dado;

    // Diretorio
    struct dir_{
        int atual;
        int ant;
        dado arq[MAX_ARQ];
    };
    typedef struct dir_ dir;


// Variáveis globais
    // Mutex
    pthread_mutex_t cadeado;
    
    // Tamanho das particoes
    int mapa_tam = sizeof(bool) * NBLOCKS;
    int inode_tam = sizeof(inode) * NBLOCKS;
    int blocos_tam = BLOCK_SIZE * NBLOCKS;
    int dir_atual;

    FILE *file;

// Definição das funcoes
    // Socket
    int create_socket();

    // Thread
    void* comand(void* acceptR_);

    // Criar arquivo binario
    int create_fs();
    int initialize_fs();
    void root();
    int find_block();

    // Funcoes
    int mkdir_(char* recvBuffer);
    int rm_r(char* recvBuffer);
    //char*cd(char* recvBuffer);
    char* ls();
    int touch(char* recvBuffer);
    int rm(char* recvBuffer);
    char* echo(char* recvBuffer);
    char* cat(char* recvBuffer);



int main(){

    int socketR = create_socket();

    printf("Criou o socket\n");
    

    int initializeR = initialize_fs();
    if(initializeR != 0){
        printf("Erro criando o sistema de arquivos\n");
        exit(-1);
    }

    printf("Sistema de Arquivo criado com sucesso\n");

    while(1){
        // Aceita conexoes
        int *acceptR = calloc(sizeof(int), 1);

        printf("\nEsperando conexao\n");		
        *acceptR = accept(socketR, (struct sockaddr*)NULL, NULL);

        if(*acceptR == -1){
            printf("Erro ao aceitar a conexao\n");
            exit(-1);
        }
        printf("Conexao criada\n");

        pthread_t tid;
        pthread_create(&tid, NULL, comand, acceptR);
        printf("Thread criada.\n");

	}

	return 0;
}

int initialize_fs(){
    file = fopen("fs.bin", "w+b");

    rewind(file);
    bool estado = false;
    for(int i = 0; i < NBLOCKS; i++){
        fwrite(&estado, sizeof(bool), 1, file);
    }  

    inode temp;
    temp.estado = false;
    fseek(file, mapa_tam, SEEK_SET);
    for(int i = 0; i < NBLOCKS; i++){
        fwrite(&temp, sizeof(inode), 1, file);   
    }

    char *temp_ = malloc(BLOCK_SIZE);
    fseek(file, (mapa_tam + inode_tam), SEEK_SET);
    for(int i = 0; i < NBLOCKS; i++){
        fwrite(temp_, BLOCK_SIZE, 1, file);   
    }

    root();

    return 0;
}

void root(){
    inode temp;
    bool estado = 1;

    fseek(file, mapa_tam, SEEK_SET);
    fread(&temp, sizeof(inode), 1, file);

    strcpy(temp.nome, "Raiz");
    strcpy(temp.tipo, "Dir"); 
    temp.bloco = find_block();

    temp.estado = 1;

    fseek(file, mapa_tam, SEEK_SET);
    fwrite(&temp, sizeof(inode), 1, file);

    dir temp_;
    temp_.atual = 0;
    temp_.ant = 0;
    dir_atual = 0;

    fseek(file, (mapa_tam + inode_tam + temp.bloco * BLOCK_SIZE), SEEK_SET);    
    fwrite(&temp_, sizeof(temp_), 1, file);   

    return;
}

int find_block(){
    fseek(file, 0, SEEK_SET);

    bool estado;
    int i;
    for(i = 0; i < NBLOCKS; i++){
        fread(&estado, sizeof(bool), 1, file);
        if(estado == 0)
            break;
    }

    estado = 1;
    fseek(file, (i * sizeof(bool)), SEEK_SET);
    fwrite(&estado, sizeof(bool), 1, file);   

    return i;

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
		exit(-1);
	}

    int enable = 1;
    setsockopt(socketR, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	//Associa o socket criado com a porta do SO
	int bindR = bind(socketR, (struct sockaddr *)&socketAddr, sizeof(socketAddr));
	if(bindR == -1){
		printf("Erro na funcao bind.\n");
		exit(-1);
	}

    //Habilita o socket pra receber as conexoes atraves do descritor (socketR) do socket
	int listenR = listen(socketR, 10);
	if(listenR == -1){
		printf("Erro na funcao listen.\n");
		exit(-1);
	}

	return socketR;
}


void* comand(void* acceptR_){
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
        int status;

        //criar (sub)diretorio
        if (strncmp(recvBuffer, "mkdir ", 6) == 0){
            pthread_mutex_lock(&cadeado);

            status = mkdir_(recvBuffer);

            if(!status) printf("Pasta criada com sucesso\n");
            else printf ("Erro ao criar pasta\n");
        
            pthread_mutex_unlock(&cadeado);
        }

        //remover (sub)diretorio
        if (strncmp(recvBuffer, "rm -r ", 6) == 0){
            pthread_mutex_lock(&cadeado);

            //status = rm_r(recvBuffer);
            
            if(!status) printf("Pasta excluida com sucesso\n");
            else printf ("Erro ao excluir pasta\n");

            pthread_mutex_unlock(&cadeado);
        }

        //entrar em (sub)diretorio
        if (strncmp(recvBuffer, "cd ", 3) == 0){
            
            //--------------------------------------

            pthread_mutex_lock(&cadeado);
            memmove(recvBuffer, recvBuffer + 3, strlen(recvBuffer));

            chdir(recvBuffer);
            printf("Entrando no diretorio %s\n", recvBuffer);
            pthread_mutex_unlock(&cadeado);
            
            //--------------------------------------
        }

        //mostrar conteudo do diretorio
        if (strcmp(recvBuffer, "ls") == 0){

            char *buffer = ls();

            send(*conexao, buffer, strlen(buffer), 0);

        }

        //criar arquivo
        if (strncmp(recvBuffer, "touch ", 6) == 0){
            pthread_mutex_lock(&cadeado);

            status = touch(recvBuffer);

            if(!status) printf("Arquivo criado com sucesso\n");
            else printf ("Erro ao criar arquivo\n");

            pthread_mutex_unlock(&cadeado);
        }

        //remover arquivo
        if ((strncmp(recvBuffer, "rm ", 3) == 0) && (strncmp(recvBuffer, "rm -r ", 6) != 0) ){
            pthread_mutex_lock(&cadeado);
            
            //status = rm(recvBuffer);

            if(!status) printf("Arquivo removido com sucesso\n");
            else printf("Erro ao remover arquivo\n");

            pthread_mutex_unlock(&cadeado);
        }

        //escrever um sequencia de caracteres em um arquivo
        if (strncmp(recvBuffer, "echo ", 5) == 0){
            pthread_mutex_lock(&cadeado);

            //status = echo(recvBuffer);

            if(!status) printf("Caracteres inseridos com sucesso\n");
            else printf("Erro ao inserir caracteres\n");

            pthread_mutex_unlock(&cadeado);
        }

        //mostrar conteudo do arquivo
        if (strncmp(recvBuffer, "cat ", 4) == 0){
            pthread_mutex_lock(&cadeado);

            //sendBuffer = cat(recvBuffer);
            send(*conexao, sendBuffer, strlen(sendBuffer), 0);
            
            pthread_mutex_unlock(&cadeado);
        }

    }
    printf("Conexao fechada\n");
}


int find_inode(){

    inode temp;
    int i;

    for(i = 0; i < NBLOCKS; i++){
        fseek(file, (mapa_tam + i * sizeof(inode)), SEEK_SET);
        fread(&temp, sizeof(inode), 1, file);
        if(temp.estado == 0)
            break;
    }

    temp.estado = 1;

    fseek(file, (mapa_tam + i * sizeof(inode)), SEEK_SET);
    fwrite(&temp, sizeof(inode), 1, file);

    return i;
}


int mkdir_(char* recvBuffer){

    inode temp;
    int inode_pos, status;
    bool estado = 1;
    char *nome;

    inode_pos = find_inode();
    
    fseek(file, (mapa_tam + inode_pos * sizeof(inode)), SEEK_SET);
    fread(&temp, sizeof(inode), 1, file);

    nome = strtok(recvBuffer," ");
    nome = strtok(NULL, " ");

    strcpy(temp.nome, nome);
    strcpy(temp.tipo, "Dir"); 
    temp.bloco = find_block();

    dir temp_;
    inode_pos = 9;
    temp_.atual = inode_pos;
    temp_.ant = dir_atual;

    fseek(file, (mapa_tam + inode_tam + temp.bloco * BLOCK_SIZE), SEEK_SET);    
    fwrite(&temp_, sizeof(temp_), 1, file);   
    
    status = 0;
    
    return status;
}

int touch(char* recvBuffer){

    int bloco_livre, bloco_dir;
    inode temp;
    dir atual;
    char *nome;

    bloco_livre = find_block();

    fseek(file, (mapa_tam + dir_atual * sizeof(inode)), SEEK_SET);
    fread(&temp, sizeof(inode), 1, file);

    bloco_dir = temp.bloco;

    fseek(file, (mapa_tam + inode_tam + bloco_dir * BLOCK_SIZE), SEEK_SET);
    fread(&atual, BLOCK_SIZE, 1, file);

    nome = strtok(recvBuffer," ");
    nome = strtok(NULL, " ");

    //---------------------------------------------------------------------- Só cria um arquivo
    int i;
    for(i = 0; i < MAX_ARQ; i++){
        if(atual.arq[i].inode != 0)
            break;
    }

    if(i != MAX_ARQ){
        strcpy(atual.arq[i].nome, nome);
        atual.arq[i].inode = bloco_livre;
    }
    else{
        printf("Diretorio cheio\n");
        exit(0);
    }

    fseek(file, (mapa_tam + inode_tam + bloco_dir * BLOCK_SIZE), SEEK_SET);
    fwrite(&atual, sizeof(dir), 1, file);

    return 0;
    
}

char* ls(){
    
    inode temp;
    int bloco;
    dir atual;
    char *lista = (char*) malloc(sizeof(char));

    fseek(file, (mapa_tam + dir_atual * sizeof(inode)), SEEK_SET);
    fread(&temp, sizeof(inode), 1, file);

    bloco = temp.bloco;

    fseek(file, (mapa_tam + inode_tam + bloco * BLOCK_SIZE), SEEK_SET);
    fread(&atual, BLOCK_SIZE, 1, file);
        
    strcat(lista, "\t.\t..");

    for(int i = 0; i < MAX_ARQ; i++){
        if(atual.arq[i].nome != NULL){
            strcat(lista, "\t");
            strcat(lista, atual.arq[i].nome);
        }
        else break;
    }

    lista[strlen(lista) - 2] = '\n';
    lista[strlen(lista) - 1] = '\0';

    return lista;

}