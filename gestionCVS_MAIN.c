//#########################################################
//#
//# Titre : 	UTILITAIRES (MAIN) TP1 LINUX Automne 19
//#				VERSION CONCURRENTE
//#				SIF-1015 - Système d'exploitation
//#				Université du Québec à Trois-Rivières
//#
//# Auteur : 	Francois Meunier
//#	Date :		Septembre 2019
//#
//# Langage : 	ANSI C on LINUX
//#
//#######################################
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "gestionListeChaineeCVS.h"
#include <ncurses.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <errno.h>

#define BUFFER_SIZE PIPE_BUF
#define TEN_MEG (10)

//Pointeur de tête de liste
struct noeud* head;
//Pointeur de queue de liste pour ajout rapide
struct noeud* queue;

int nbThreadAMLSO;


sem_t semH;
sem_t semQ;
sem_t semConsole;
sem_t semNBThreadAMLSO;


int main(int argc, char* argv[]){

    struct Info_FIFO_Transactions data;
    int res;
    //int server_fifo_fd, client_fifo_fd;
    int server_sockfd, client_sockfd;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int client_len;
    int read_res, write_res;
    char client_fifo[256];
    char buffer[256];
    char *tmp_char_ptr;
    int len;
	
	//Initialisation des pointeurs
	head = NULL;
	queue = NULL;
	nbThreadAMLSO = 0;
	sem_init(&semH, 0, 1);
	sem_init(&semQ, 0, 1);
	sem_init(&semConsole, 0, 1);
	sem_init(&semNBThreadAMLSO, 0, 1);

    /* Remove any old socket and create an unamed socket for the server */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sockfd < 0){
        perror("Error at socket");
    }

    /* Name the socket. */
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = 6734;
    //server_len = sizeof(server_address);

    if(bind(server_sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0){
        perror("Error at bind");
    }

    listen(server_sockfd, 5);

    int sortir = 0;
    while(sortir == 0){
        /* Accept a connection */
        len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, (socklen_t*)&len);
        if(client_sockfd == -1){
            perror("erreur de la fonction accept: ");
        }
        else{
            //meme pas sur qui soit useful
           // read(client_sockfd, &buffer, 1);
            readTrans(client_sockfd);
        }
    }


//	//-------------------------------------------------------CRÉATION ET OUVERTURE FIFO_TRANSACTIONS
//	//Création du FIFO_TRANSACTIONS
//	if(access(FIFO_TRANSACTIONS, F_OK) == -1){
//        res = mkfifo(FIFO_TRANSACTIONS, 0777);
//        if (res != 0) {
//            fprintf(stderr, "Could not create fifo %s\n", FIFO_TRANSACTIONS);
//            exit(EXIT_FAILURE);
//        }
//	}
//
//	//ouverture de fifo transaction en lecture seulement
//	server_fifo_fd = open(FIFO_TRANSACTIONS, O_RDONLY);
//    if(server_fifo_fd == -1){
//        fprintf(stderr, "Server fifo failure\n");
//        exit(EXIT_FAILURE);
//    }

    //S'apprête à ouvrir le fifo du client en écriture
    //client_fifo_fd = open(client_fifo, O_WRONLY);


    //readTrans(server_fifo_fd);

    
	//Arrêt de l'utilisation du fifo_transactions
	close(server_sockfd);
    //unlink(FIFO_TRANSACTIONS);
    exit(EXIT_SUCCESS);

	sem_destroy(&semH);
	sem_destroy(&semQ);
	sem_destroy(&semConsole);
	sem_destroy(&semNBThreadAMLSO);


	//Fin du programme
	exit( 0);
}

