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
    int server_fifo_fd, client_fifo_fd;
    int read_res, write_res;
    char client_fifo[256];
    char *tmp_char_ptr;
	
	//Initialisation des pointeurs
	head = NULL;
	queue = NULL;
	nbThreadAMLSO = 0;
	sem_init(&semH, 0, 1);
	sem_init(&semQ, 0, 1);
	sem_init(&semConsole, 0, 1);
	sem_init(&semNBThreadAMLSO, 0, 1);

	//-------------------------------------------------------CRÉATION ET OUVERTURE FIFO_TRANSACTIONS
	//Création du FIFO_TRANSACTIONS
	if(access(FIFO_TRANSACTIONS, F_OK) == -1){
        res = mkfifo(FIFO_TRANSACTIONS, 0777);
        if (res != 0) {
            fprintf(stderr, "Could not create fifo %s\n", FIFO_TRANSACTIONS);
            exit(EXIT_FAILURE);
        }
	}

	//ouverture de fifo transaction en lecture seulement
	server_fifo_fd = open(FIFO_TRANSACTIONS, O_RDONLY);
    if(server_fifo_fd == -1){
        fprintf(stderr, "Server fifo failure\n");
        exit(EXIT_FAILURE);
    }

    //S'apprête à ouvrir le fifo du client en écriture
    client_fifo_fd = open(client_fifo, O_WRONLY);


    readTrans(server_fifo_fd);

    
	//Arrêt de l'utilisation du fifo_transactions
	close(server_fifo_fd);
    unlink(FIFO_TRANSACTIONS);
    exit(EXIT_SUCCESS);

	sem_destroy(&semH);
	sem_destroy(&semQ);
	sem_destroy(&semConsole);
	sem_destroy(&semNBThreadAMLSO);


	//Fin du programme
	exit( 0);
}

