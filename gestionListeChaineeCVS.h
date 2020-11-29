#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#define FIFO_TRANSACTIONS "./FIFO_TRANSACTIONS"
#define CLIENT_FIFO_NAME "./cli_%d_fifo"


struct Info_FIFO_Transactions{
    int pid_client;
    char transaction[200];
};

struct infoligne{						
	int		noligne;				
	char	ptrligne[100];							
	};								 

struct noeud{			
	struct infoligne	ligne;
	sem_t 			sem;
	struct noeud		*suivant;	
	};

struct infoADD{	
    int noligne;
	char tligne[100];
	};

struct infoMODIFY{
	int noligne;
	char tligne[100];
	};
	
struct infoREMOVE{						
	int noligne;
	};

struct infoLIST{						
	int start;
	int end;
	};
	
struct infoSAVE{
	char nomFichier[100];
	};
struct infoLOAD{
	char nomFichier[100];
	};
struct infoEXE{
	char nomFichier[100];
	};
		
	
void cls(void);
void error(const int exitcode, const char * message);

struct noeud * findItem(const int no);
struct noeud * findPrev(const int no);

void addItemNONCONCURRENT(const int nl, const char* tl);
void addItem(struct infoADD* param);
void removeItem(struct infoREMOVE* param);
void modifyItem(struct infoMODIFY* param);
void listItems(struct infoLIST* param);
void saveItems(struct infoSAVE* param);
void executeFile(const char* sourcefname);

void loadFich(const char* sourcefname);
void readTrans(char* nomFichier);
void clientTransactions();