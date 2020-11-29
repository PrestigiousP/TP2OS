
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
	sem_wait(&head->sem);
	struct noeud * ptr = head;
	sem_post(&semQ);
	sem_post(&semH);
	
	if(ptr->ligne.noligne==no) // premier noeud verrouille
		return ptr;
	//Tant qu'un item suivant existe
	if(ptr->suivant!=NULL){
		sem_wait(&ptr->suivant->sem); // noeud suivant verreouille
	}
	while (ptr->suivant!=NULL){
		//Déplacement du pointeur de navigation
		struct noeud* optr = ptr;
	
		ptr=ptr->suivant;
		sem_post(&optr->sem);
		//Est-ce l'item recherché?
		if (ptr->ligne.noligne==no){
			//On retourne un pointeur sur l'item (toujours verrouillé)
			return ptr;
		}
		if(ptr->suivant!=NULL){
			sem_wait(&ptr->suivant->sem);
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

