//#########################################################
//#
//# Titre : 	Utilitaires Liste Chainee et CVS LINUX Automne 11
//#				SIF-1015 - Système d'exploitation
//#				Université du Québec à Trois-Rivières
//#
//# Auteur : 	Francois Meunier
//#	Date :		Septembre 2011
//#
//# Langage : 	ANSI C on LINUX 
//#
//#######################################

#include "gestionListeChaineeCVS.h"
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>

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
//# Recherche un item dans la liste chaînée
//# ENTREE: Numéro de la ligne
//# RETOUR:	Un pointeur (verrouille) vers l'item recherché		
//# 		Retourne NULL dans le cas où l'item
//#			est introuvable
//#
struct noeud * findItem(const int no){
    //La liste est vide
    sem_wait(&semH);
    sem_wait(&semQ);
    if ((head==NULL)&&(queue==NULL)){
        sem_post(&semQ);
        sem_post(&semH);
        return NULL;
    }
    //Pointeur de navigation
    sem_wait(&(head->sem));
    struct noeud * ptr = head;
    sem_post(&semQ);
    sem_post(&semH);

    if(ptr->ligne.noligne==no) // premier noeud verrouille
        return ptr;
    //Tant qu'un item suivant existe
    if(ptr->suivant!=NULL){
        sem_wait(&(ptr->suivant->sem)); // noeud suivant verreouille
    }
    while (ptr->suivant!=NULL){
        //Déplacement du pointeur de navigation
        struct noeud* optr = ptr;

        ptr=ptr->suivant;
        sem_post(&(optr->sem));
        //Est-ce l'item recherché?
        if (ptr->ligne.noligne==no){
            //On retourne un pointeur sur l'item (toujours verrouillé)
            return ptr;
        }
        if(ptr->suivant!=NULL){
            sem_wait(&(ptr->suivant->sem));
        }
        else // suivant NULL
        {
            sem_post(&(ptr->sem));
            return NULL;
        }

    }
    //On retourne un pointeur NULL
    sem_post(&(ptr->sem));
    return NULL;
}

//#######################################
//#
//# Recherche le PRÉDÉCESSEUR d'un item dans la liste chaînée
//# ENTREE: Numéro de la ligne a supprimer
//# RETOUR:	Le pointeur vers le prédécesseur (verrouille) est retourné		
//# 		Retourne NULL dans le cas où l'item est introuvable
//#
struct noeud * findPrev(const int no){
    //La liste est vide
    sem_wait(&semH);
    sem_wait(&semQ);
    if ((head==NULL)&&(queue==NULL)){
        sem_post(&semQ);
        sem_post(&semH);
        return NULL;
    }

    //Pointeur de navigation
    sem_wait(&(head->sem));
    struct noeud * ptr = head;
    sem_post(&semQ);
    sem_post(&semH);

    //Tant qu'un item suivant existe
    if(ptr->suivant!=NULL){
        sem_wait(&(ptr->suivant->sem));
    }
    while (ptr->suivant!=NULL){

        //Est-ce le prédécesseur de l'item recherché?
        if (ptr->suivant->ligne.noligne==no){
            //On retourne un pointeur (verrouille) sur l'item précédent
            sem_post(&(ptr->suivant->sem));
            return ptr;
        }
        //Déplacement du pointeur de navigation
        struct noeud* optr = ptr;

        ptr=ptr->suivant;
        sem_post(&(optr->sem));
        if(ptr->suivant!=NULL){
            sem_wait(&(ptr->suivant->sem));
        }
        else // suivant NULL
        {
            sem_post(&(ptr->sem));
            return NULL;
        }
    }
    //On retourne un pointeur NULL
    sem_post(&(ptr->sem));
    return NULL;
}

//#######################################
//# Ajoute un item dans la liste chaînée (Version non concurrente)
//# ENTREE: nl: numéro de ligne 
//#			tl: ligne de code 
void addItemNONCONCURRENT(const int nl, const char* tl){
    //Création de l'enregistrement en mémoire
    struct noeud* ni = (struct noeud*)malloc(sizeof(struct noeud));
    struct noeud* ptrINS = findItem(nl);
    //Affectation des valeurs des champs
    ni->ligne.noligne	= nl;
    strcpy(ni->ligne.ptrligne, tl);
    sem_init(&ni->sem, 0, 1);


    if((ptrINS == NULL) && (nl == 1)) // ajout au debut de la liste vide
    {// premiere ligne premier noeud
        ni->suivant= NULL;
        queue = head = ni;
    }
    else if ((ptrINS == NULL) && (nl > 1)) // ajout a la fin de la liste
    {
        struct noeud* tptr = queue;
        ni->suivant= NULL;
        queue = ni;
        tptr->suivant = ni;
    }
    else
    {
        struct noeud* tptr = ptrINS;
        if(tptr == head) // ajou a la tete de la liste
            head = ni;
        else
        {
            struct noeud* ptrPREV = findPrev(nl);
            ptrPREV->suivant = ni;
        }
        ni->suivant = tptr;

        while (tptr!=NULL){

            //Est-ce le prédécesseur de l'item recherché?
            tptr->ligne.noligne++;
            //On retourne un pointeur sur l'item précédent

            //Déplacement du pointeur de navigation
            tptr=tptr->suivant;
        }
    }

}



//#######################################
//# Ajoute un item dans la liste chaînée
//# ENTREE: nl: numéro de ligne 
//#			tl: ligne de code 
void  addItem(struct infoADD* param ){

    char tl[100];
    int nl;

    nl = param->noligne;
    strcpy(tl,(const char*)param->tligne);
    free(param);

    sem_wait(&semNBThreadAMLSO);
    nbThreadAMLSO++;
    sem_post(&semNBThreadAMLSO);

    //Création de l'enregistrement en mémoire
    struct noeud* ni = (struct noeud*)malloc(sizeof(struct noeud));
    struct noeud* ptrINS = findItem(nl); // ptrINS est verrouille si non NULL
    //Affectation des valeurs des champs
    ni->ligne.noligne	= nl;
    strcpy(ni->ligne.ptrligne, tl);
    sem_init(&(ni->sem), 0, 1);

    sem_wait(&semH);
    sem_wait(&semQ);


    if((ptrINS == NULL) && (nl == 1)) // ajout au debut de la liste vide
    {// premiere ligne premier noeud
        ni->suivant= NULL;
        queue = head = ni;
    }
    else if ((ptrINS == NULL) && (nl > 1)) // ajout a la fin de la liste
    {
        sem_wait(&(queue->sem));

        struct noeud* tptr = queue;
        ni->suivant= NULL;
        queue = ni;
        tptr->suivant = ni;
        sem_post(&(tptr->sem));
    }
    else
    {
        struct noeud* tptr = ptrINS; // noeud pointe par ptrINS deja verrouille

        if(tptr == head) // ajout a la tete de la liste
        {
            head = ni;
            sem_wait(&(ni->sem)); // verrouille le nouveau noeud insere dans la liste
        }
        else
        {
            struct noeud* ptrPREV = findPrev(nl); // noeud pointe par ptrPREV deja verrouille
            ptrPREV->suivant = ni;
            sem_wait(&(ni->sem));
        }
        ni->suivant = tptr;
        sem_post(&(ni->sem));

        while (tptr!=NULL){
            printf("allo");
            //Est-ce le prédécesseur de l'item recherché?
            tptr->ligne.noligne++;
            //On retourne un pointeur sur l'item précédent

            struct noeud* otptr = tptr;
            if(tptr->suivant != NULL)
            {
                sem_wait(&(tptr->suivant->sem));
                //Déplacement du pointeur de navigation
                tptr=tptr->suivant;
                sem_post(&(otptr->sem));
            }
            else // tptr->suivant == NULL
            {
                //Déplacement du pointeur de navigation
                tptr=NULL;
                sem_post(&(otptr->sem));
            }
        }
    }

    sem_post(&semQ);
    sem_post(&semH);

    sem_wait(&semNBThreadAMLSO);
    nbThreadAMLSO--;
    sem_post(&semNBThreadAMLSO);
    printf("%s", "fin de additemL");

}

//#######################################
//# Retire un item de la liste chaînée
//# ENTREE: nl: numéro de ligne 
void removeItem(struct infoREMOVE* param){

    printf("allo hihi");
    int noline;

    noline = param->noligne;
    free(param);

    struct noeud * ptr;
    struct noeud * tptr;
    struct noeud * optr;

    // Attendre la terminaison des threads A M L S et O

    while(1) {

        sem_wait(&semNBThreadAMLSO);
        if(nbThreadAMLSO == 0){
            sem_wait(&semH);
            sem_wait(&semQ); // verrou sur la tete et queue pour toute la suppression
            sem_post(&semNBThreadAMLSO);
            break;
        }
        sem_post(&semNBThreadAMLSO);
    }



    //Vérification sommaire (no>0 et liste non vide)
    if ((noline<1)||((head==NULL)&&(queue==NULL))){	// liste vide
        sem_post(&semQ);
        sem_post(&semH);
        return;

    }

    //Pointeur de recherche
    if(noline==1){
        ptr = head; // suppression du premier element de la liste
    }
    else{
        sem_post(&semQ);
        sem_post(&semH);
        ptr = findPrev(noline); // noeud pointe par ptr deja verrouille si != NULL
        sem_wait(&semH);
        sem_wait(&semQ);
        if (ptr!=NULL)
            sem_post(&(ptr->sem));

    }
    //L'item a été trouvé
    if (ptr!=NULL){

        // Memorisation du pointeur de l'item en cours de suppression
        // Ajustement des pointeurs
        if((head == ptr) && (noline==1)) // suppression de l'element de tete
        {
            if(head==queue) // un seul element dans la liste
            {
                sem_destroy(&(ptr->sem));
                free(ptr);
                queue = head = NULL;
                sem_post(&semQ);
                sem_post(&semH);
                return;
            }
            tptr = ptr->suivant;
            sem_destroy(&(ptr->sem));
            free(ptr);

        }
        else if (queue==ptr->suivant) // suppression de l'element de queue
        {
            queue=ptr;
            sem_destroy(&(ptr->suivant->sem));
            free(ptr->suivant);
            ptr->suivant=NULL;
            sem_post(&semQ);
            sem_post(&semH);
            return;
        }
        else // suppression d'un element dans la liste
        {
            optr = ptr->suivant;
            ptr->suivant = ptr->suivant->suivant;
            tptr = ptr->suivant;
            sem_destroy(&(optr->sem));
            free(optr);

        }


        while (tptr!=NULL){ // ajustement des numeros de ligne

            //Est-ce le prédécesseur de l'item recherché?
            tptr->ligne.noligne--;
            //On retourne un pointeur sur l'item précédent


            //Déplacement du pointeur de navigation
            tptr=tptr->suivant;
        }
        sem_post(&semQ);
        sem_post(&semH);
    }
    sem_post(&semQ);
    sem_post(&semH);
}

//#######################################
//# Modifie un item de la liste chaînée
//# ENTREE: noline: numéro de ligne 
//#			tline: ligne de code 
void modifyItem(struct infoMODIFY* param){

    char tline[100];
    int noline;

    noline = param->noligne;
    strcpy(tline,(const char*)param->tligne);
    free(param);

    sem_wait(&semNBThreadAMLSO);
    nbThreadAMLSO++;
    sem_post(&semNBThreadAMLSO);

    sem_wait(&semH);
    sem_wait(&semQ);


    //Vérification sommaire (no>0 et liste non vide)
    if ((noline<1)||((head==NULL)&&(queue==NULL))){
        sem_post(&semQ);
        sem_post(&semH);

        sem_wait(&semNBThreadAMLSO);
        nbThreadAMLSO--;
        sem_post(&semNBThreadAMLSO);
        return;
    }

    sem_post(&semQ);
    sem_post(&semH);

    //Recherche de l'élément à modifier (verrouille)
    struct noeud * ptr = findItem(noline);

    //L'élément à été trouvé
    if (ptr!=NULL){
        //Modification des champs de l'élément trouvé
        strcpy(ptr->ligne.ptrligne, tline);
        sem_post(&ptr->sem);

    }
    sem_wait(&semNBThreadAMLSO);
    nbThreadAMLSO--;
    sem_post(&semNBThreadAMLSO);
}

//#######################################
//#
//# Affiche les items dont le numéro séquentiel est compris dans une plage
//#
void listItems(struct infoLIST* param){

    char client_fifo[100];
    int client_fifo_fd;
    struct Info_FIFO_Transactions data;
    int start, end;
    char str[400];
    char temp[100];
    int client_sockfd;

    client_sockfd = param->client_sockfd;
    data = param->data;
    data.nbLignes = 0;
    start =param->start;
    end = param->end;
    free(param);

    //sprintf(client_fifo, CLIENT_FIFO_NAME, data.pid_client);		//Trouve le client associé

    //client_fifo_fd = open(client_fifo, O_WRONLY);

    sem_wait(&semNBThreadAMLSO);
    nbThreadAMLSO++;
    sem_post(&semNBThreadAMLSO);

    sem_wait(&semH);
    sem_wait(&semQ);

    sem_wait(&semConsole);

    //Affichage des entêtes de colonnes
    strcpy(data.transaction, "noligne  texte                                          \n");
    data.nbLignes++;
    strcpy(temp, "======= ========================================\n");
    data.nbLignes++;
    strcat(data.transaction, temp);

    if(head != NULL){ // si liste non vide
        sem_wait(&(head->sem));
    }
    else{

        sem_post(&semConsole);
        sem_post(&semQ);
        sem_post(&semH);
        sem_wait(&semNBThreadAMLSO);
        nbThreadAMLSO--;
        sem_post(&semNBThreadAMLSO);
    }

    struct noeud * ptr = head;	//premier element


    sem_post(&semQ);
    sem_post(&semH);

    while (ptr!=NULL){
        //L'item a un numéro séquentiel dans l'interval défini
        if ((ptr->ligne.noligne>=start)&&(ptr->ligne.noligne<=end)){
            sprintf(temp, "%d ", ptr->ligne.noligne);
            strcat(data.transaction, temp);
            //write(client_fifo_fd, &data, sizeof(data));

            sprintf(temp, "%s  \n ", ptr->ligne.ptrligne);
            data.nbLignes++;
            strcat(data.transaction, temp);
            //write(client_fifo_fd, &data, sizeof(data));
//            sprintf(temp, "%d \t %s\n",
//                    ptr->ligne.noligne,
//                    ptr->ligne.ptrligne);
//            strcpy(data.transaction, temp);
//            write(client_fifo_fd, &data, sizeof(data));
        }
        if (ptr->ligne.noligne>end){
            //L'ensemble des items potentiels sont maintenant passés
            //Déplacement immédiatement à la FIN de la liste
            sem_post(&(ptr->sem));
            ptr=NULL;
        }
        else{
            if(ptr->suivant != NULL){
                sem_wait(&(ptr->suivant->sem));
                struct noeud* optr;
                optr = ptr;
                ptr = ptr->suivant;
                sem_post(&(optr->sem));
            }
            else{
                sem_post(&(ptr->sem));
                ptr = NULL;
            }
        }

    }

    //Affichage des pieds de colonnes
    strcpy(temp, "======= ================================================\n");
    data.nbLignes++;
    strcat(data.transaction, temp);
    write(client_sockfd, &data, sizeof(data));

    //close(client_sockfd);
    sem_post(&semConsole);
    sem_wait(&semNBThreadAMLSO);
    nbThreadAMLSO--;
    sem_post(&semNBThreadAMLSO);
}

