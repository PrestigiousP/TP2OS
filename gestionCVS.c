//#########################################################
//#
//# Titre : 	Utilitaires CVS LINUX Automne 19
//#				SIF-1015 - Système d'exploitation
//#				Université du Québec à Trois-Rivières
//#
//# Auteur : 	Francois Meunier
//#	Date :		Septembre 2019
//#
//# Langage : 	ANSI C on LINUX 
//#
//#######################################
#include "gestionListeChaineeCVS.h"

//Pointeur de tête de liste
extern struct noeud* head;
//Pointeur de queue de liste pour ajout rapide
extern struct noeud* queue;

extern int nbThreadAMLSO;

extern sem_t semH;
extern sem_t semQ;
extern sem_t semConsole;
extern sem_t semNBThreadAMLSO;

//#######################################
//#
//# Affiche une série de retour de ligne pour "nettoyer" la console
//#
void cls(void){
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	}

//#######################################
//#
//# Affiche un messgae et quitte le programme
//#
void error(const int exitcode, const char * message){
	printf("\n-------------------------\n%s\n",message);
	exit(exitcode);
	}

//#######################################
//#
//# Chargement de la BD binaire
//#
void loadFich(const char* sourcefname){
	
	//Initialisation des pointeurs
	head = NULL;
	queue = NULL;


	const char tligne[100];
	int noligne=0;

	//Ouverture du fichier source en mode "rt" : [r]ead [t]ext
	FILE *f;
	f = fopen(sourcefname, "rt");
	if (f==NULL)
		return;

	//Ajout des éléments extraits du fichier source
	
	while(!feof(f)){
		fgets(tligne, 100, f);
		addItemNONCONCURRENT(noligne++, tligne);
		}

 }

//#######################################
//#
//# Enregistre le contenu de la liste chaînée dans un fichier texte
//#
void saveItems(struct infoSAVE* param){
	
	char sourcefname[100];
	struct noeud * ptr = NULL;
	
	strcpy(sourcefname,(const char*)param->nomFichier);
	free(param);
	
	sem_wait(&semNBThreadAMLSO);
	    nbThreadAMLSO++;
	sem_post(&semNBThreadAMLSO);
	
	sem_wait(&semH);
	sem_wait(&semQ);

	if(head != NULL)
		ptr = head;	// premier element
	else{ // liste vide 
		sem_wait(&semNBThreadAMLSO);
		    nbThreadAMLSO--;
		sem_post(&semNBThreadAMLSO);
		sem_post(&semQ);
		sem_post(&semH);
		return;	
	}
	FILE *f;
	char nomC[100];
	char nomTXT[100];
	char command[100];
	char *ptrC;

	strcpy(nomC,sourcefname);
	strcpy(nomTXT, sourcefname);
	ptrC = strrchr(nomTXT,'.');
	strcpy(ptrC,".txt");
	ptrC = strrchr(nomC,'.');
	strcpy(ptrC,".c");

	//Ouverture du fichier en mode "wt" : [w]rite [t]ext

 	f= fopen(nomTXT, "wt");

	
	if (f==NULL)
		error(2, "saveItems: Erreur lors de l'ouverture du fichier pour écriture en mode texte.");

	while (ptr!=NULL){

		//Écriture des données
		fprintf(f,"%s\n",ptr->ligne.ptrligne);
		
		//Déplacement du pointeur
		ptr = ptr->suivant;
		}
	
	//Fermeture du fichier
	fclose(f);

	// Passage de .txt a .c
	sprintf(command,"mv %s %s",nomTXT,nomC);
	system(command);
	
	sem_wait(&semNBThreadAMLSO);
	    nbThreadAMLSO--;
	sem_post(&semNBThreadAMLSO);
	sem_post(&semQ);
	sem_post(&semH);
	
}

//#######################################
//#
//# Execute le fichier source .c 
//#
void executeFile(const char* sourcefname){


	char command[100];
	char nameC[100];

	FILE *f;

	//Ouverture du fichier MakeCVS en mode "wt" : [w]rite [t]ext
	f = fopen("MakeCVS", "wt");
	if (f==NULL)
		error(2, "ExecuteFile: Erreur lors de l'ouverture du fichier pour écriture en mode texte.");

	
	strcpy(nameC,sourcefname);

	fprintf(f,"fichCVSEXE: %s\n",sourcefname);
	fprintf(f,"\tgcc -o fichCVSEXE %s\n",sourcefname);

	//Fermeture du fichier
	fclose(f);

	// make du fichier MakeCVS
	sprintf(command, "make -f MakeCVS");
	system(command);

	// execution du fichier fichCVSEXE
	sprintf(command, "./fichCVSEXE");
	system(command);
	


}


//#######################################
//#
//# fonction utilisée par les threads de transactions
//#
void readTrans(char* nomFichier){
	FILE *f;
	char buffer[100];
	pthread_t tid[1000];
	int i, nbThread = 0;
	char *tok, *sp;


    //--------------------------------------------------------------------------------------------------------------------
    //Lecture (tentative) d'une ligne de texte



	//Ouverture du fichier en mode "r" (equiv. "rt") : [r]ead [t]ext
	f = fopen(nomFichier, "rt");
	if (f==NULL)
		error(2, "readTrans: Erreur lors de l'ouverture du fichier.");

	//Lecture (tentative) d'une ligne de texte
	fgets(buffer, 100, f);

	//Pour chacune des lignes lues
	while(!feof(f)){
	  
		sleep(1);

		//Extraction du type de transaction
		tok = strtok_r(buffer, "  ", &sp);
		
		//printf(" \n tok = %s ", tok);

		//Branchement selon le type de transaction
		switch(tok[0]){
			case 'A':
			case 'a':{
				//Extraction des paramètres
				int noligne = atoi(strtok_r(NULL, " ", &sp));
				char *tligne = strtok_r(NULL, "\n", &sp);
				
				struct infoADD *ptr = (struct infoADD*) malloc(sizeof(struct infoADD));
				ptr->noligne = noligne;
				strcpy(ptr->tligne,(const char *)tligne);
				
				//Appel de la fonction associée
				//printf("\nDANS READTRANS nl = %d tl = %s ", noligne, tligne);
				pthread_create(&tid[nbThread++], NULL, (void *)addItem, ptr);
				break;
				}
			case 'E':
			case 'e':{
				//Extraction du paramètre
				int noligne = atoi(strtok_r(NULL, " ", &sp));
				
				struct infoREMOVE *ptr = (struct infoREMOVE*) malloc(sizeof(struct infoREMOVE));
				ptr->noligne = noligne;
				
				//Appel de la fonction associee
				pthread_create(&tid[nbThread++], NULL, (void *)removeItem, ptr);
				break;
				}
			case 'M':
			case 'm':{
				//Extraction des paramètres
				int noligne = atoi(strtok_r(NULL, " ", &sp));
				char *tligne = strtok_r(NULL, "\n", &sp);
				
				struct infoMODIFY *ptr = (struct infoMODIFY*) malloc(sizeof(struct infoMODIFY));
				ptr->noligne = noligne;
				strcpy(ptr->tligne,(const char *)tligne);
				
				//Appel de la fonction associee
				pthread_create(&tid[nbThread++], NULL, (void *)modifyItem, ptr);
				break;
				}
			case 'L':
			case 'l':{
				//Extraction des paramètres
				int nstart = atoi(strtok_r(NULL, "-", &sp));
				int nend = atoi(strtok_r(NULL, " ", &sp));
				
				struct infoLIST *ptr = (struct infoLIST*) malloc(sizeof(struct infoLIST));
				ptr->start = nstart;
				ptr->end = nend;
				//Appel de la fonction associee
				pthread_create(&tid[nbThread++], NULL, (void *)listItems, ptr);
				break;
				}
			case 'S':
			case 's':{
				//Appel de la fonction associée
				char *nomfich = strtok_r(NULL, " ", &sp);
				
				struct infoSAVE *ptr = (struct infoSAVE*) malloc(sizeof(struct infoSAVE));
				strcpy(ptr->nomFichier,(const char *)nomfich);

				//Appel de la fonction associee
				pthread_create(&tid[nbThread++], NULL, (void *)saveItems, ptr);
				break;
				}
			case 'O':
			case 'o':{
				//Appel de la fonction associée
				char *nomfich = strtok_r(NULL, " ", &sp);
				
				struct infoLOAD *ptr = (struct infoLOAD*) malloc(sizeof(struct infoLOAD));
				strcpy(ptr->nomFichier,(const char *)nomfich);

				//Appel de la fonction associee
				pthread_create(&tid[nbThread++], NULL, (void *)loadFich, ptr);
				break;
				}
			case 'X':
			case 'x':{
				//Appel de la fonction associée
				char *nomfich = strtok_r(NULL, " ", &sp);
				
				struct infoEXE *ptr = (struct infoEXE*) malloc(sizeof(struct infoEXE));
				strcpy(ptr->nomFichier,(const char *)nomfich);

				//Appel de la fonction associee
				pthread_create(&tid[nbThread++], NULL, (void *)executeFile, ptr);
				break;
				}
		}

		//Lecture (tentative) de la prochaine ligne de texte
		fgets(buffer, 100, f);
	}
	
	for(i=0; i<nbThread;i++)
	  pthread_join(tid[i], NULL);
	//Fermeture du fichier
	fclose(f);
}


