//
// Created by philippe on 2020-11-28.
//
#include "gestionListeChaineeCVS.h"
#include <ncurses.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>


int main(){
    //int server_fifo_fd, client_fifo_fd;
    int server_sockfd, client_sockfd;
    int sockfd;
    int res;
    char buffer[1000];
    struct Info_FIFO_Transactions data;
    struct sockaddr_in address;
    char *tok, *sp;
	char client_fifo[256];
	int read_res;
	char str[400];
	int sorti = 1;
	int ligneEntree = 1;
	int result;
	int len;
	int ligneSorti = 1;
	char quit[] = "quit";
	data.nbLignes = 0;


	WINDOW *transmission;
    WINDOW *reception;

    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = 6734;
    len = sizeof(address);

    result =  connect(sockfd, (struct sockaddr *)&address, len);
    if(result == -1){
        perror("erreur de connexion");
        exit(1);
    }

    initscr();

	//-------------------------------------Création des deux fenêtres avec bordures
    transmission = newwin(20, 80, 0, 0);
    reception = newwin(20, 80, 0, 81);
    scrollok(transmission, true);
    scrollok(reception, true);
	box(transmission, '|', '-');
    box(reception, '|', '-');
    mvwprintw(transmission, 0, 10, "%s", "TRANSMISSION");
    mvwprintw(reception, 0, 10, "%s", "RECEPTION");

	//--------------------------------------------------Réception de l'info du serveur

		//Il va falloir lire client_fifo en passant par notre client_fifo_fd
		//Affiché le contenu du client_fifo

    do {

        //--------------------------------------------------INPUT envoyé au serveur
        //Input de l'utilisateur
        mvwgetstr(transmission, ligneEntree, 2, buffer);
        if(strcmp(buffer, quit) != 0){
            ligneEntree++;
            //Le contenu du buffer dans ma transaction, ensuite j'envoie ma structure. La structure permettra au serveur de savoir quel client et quelle transaction.
            sprintf(data.transaction, "%s", buffer);
            res = write(sockfd, &data, sizeof(data));

            sleep(1);

            wrefresh(transmission);

            char c = data.transaction[0];
            char l = 76;//"L"

            wrefresh(transmission);

            tok = strtok_r(buffer, "  ", &sp);

            if(tok[0] == 76 || tok[0] == 108){
                read_res = read(sockfd, &data, sizeof(data));
                mvwprintw(transmission, ligneSorti, 2, "111");
                if (read_res > 0) {
                    if(ligneSorti + data.nbLignes > 20)
                        ligneSorti = 20 - 1;
                    mvwprintw(reception, ligneSorti, 2, "%s", data.transaction);
                    ligneSorti += data.nbLignes;
                    wrefresh(reception);
                }
                else{
                    //mvwprintw(reception, ligneSorti, 2, "read function failed");
                    //mvwprintw(reception, ligneSorti+1, 2, data.transaction);
                    //wrefresh(reception);
                }
            }

            //Refresh des écrans (Tant qu'il n'y a pas de refresh, rien ne s'affiche)
            wrefresh(reception);
            wrefresh(transmission);
        }
        else{
            sorti = 0;
        }


    } while (sorti == 1);

	//--------------------------------------------------



    sleep(1);
    refresh();

	endwin();

	//Fermeture des tubes serveurs et clients. Destruction du fifo client
	close(sockfd);
	//close(server_fifo_fd);
    //unlink(client_fifo);
    exit(EXIT_SUCCESS);

}
