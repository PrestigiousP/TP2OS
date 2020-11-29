//
// Created by philippe on 2020-11-28.
//
#include "gestionListeChaineeCVS.h"
#include <ncurses.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(){
    int server_fifo_fd, client_fifo_fd;
    int res;
    char buffer[100];
    struct Info_FIFO_Transactions data;
	char client_fifo[256];
	int read_res;
	
	WINDOW *transmission;
    WINDOW *reception;
	
	//--------------------------------------Section FIFO/Tubes
	//Création FIFO client
    data.pid_client = getpid();
	sprintf(client_fifo, CLIENT_FIFO_NAME, data.pid_client);
	if (mkfifo(client_fifo, 0777) == -1) {
        fprintf(stderr, "Sorry, can't make %s\n", client_fifo);
        exit(EXIT_FAILURE);
    }

    //Tube ouvre client_fifo pour éventuellement lire ce que le serveur va avoir écrit dedans
    client_fifo_fd = open(client_fifo, O_RDWR);
    if(client_fifo_fd == -1){
        fprintf(stderr, "Client fifo failure\n");
        exit(EXIT_FAILURE);
    }
	
	//Tube ouvre FIFO_TRANSACTIONS pour éventuellement écrire dedans
	server_fifo_fd = open(FIFO_TRANSACTIONS, O_WRONLY);
	if(server_fifo_fd == -1){
        fprintf(stderr, "Server fifo failure\n");
        exit(EXIT_FAILURE);
    }
    
    initscr();
    
	//-------------------------------------Création des deux fenêtres avec bordures
    transmission = newwin(20, 50, 0, 0);
    reception = newwin(20, 50, 0, 51);
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
        mvwgetstr(transmission, 2, 2, buffer);

        //Le contenu du buffer dans ma transaction, ensuite j'envoie ma structure. La structure permettra au serveur de savoir quel client et quelle transaction.
        sprintf(data.transaction, "%s", buffer);
        res = write(server_fifo_fd, &data, sizeof(data));

        sleep(2);

        read_res = read(client_fifo_fd, &data, sizeof(data));
        if (read_res > 0) {

            mvwprintw(reception, 1, 10, "%s", data.transaction);
            wrefresh(reception);
        }
    } while (read_res > 0);
	
	//--------------------------------------------------
    
	
	//Refresh des écrans (Tant qu'il n'y a pas de refresh, rien ne s'affiche)
	wrefresh(reception);
    wrefresh(transmission);
    sleep(1);
    refresh();

	endwin();

	//Fermeture des tubes serveurs et clients. Destruction du fifo client
	close(client_fifo_fd);
	close(server_fifo_fd);
    unlink(client_fifo);
    exit(EXIT_SUCCESS);

}
