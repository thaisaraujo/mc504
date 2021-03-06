#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include<stdbool.h>



struct filaDir{
	int id;
    char dir;
	struct filaDir *prox;
}; typedef struct filaDir filaDir;

struct filaInicios{
    filaDir *comeco;
}; typedef struct filaInicios filaInicios;

int k=2;

pthread_mutex_t cruzamentoMutex;
pthread_cond_t batN = PTHREAD_COND_INITIALIZER;
pthread_cond_t batE = PTHREAD_COND_INITIALIZER;
pthread_cond_t batS = PTHREAD_COND_INITIALIZER;
pthread_cond_t batW = PTHREAD_COND_INITIALIZER;



void enfileirar (int id, char dir, filaDir *fila);
void *arrive (void * arg);
void leave(filaDir *comeco); 
void * batman (void *arg);
bool filaVazia (filaInicios **fila);
void imprimir(filaInicios **fila);




int main(){

    int j=1;
    int i=0;



    char dirAtual;
    char direcoes[20];

    pthread_t gerenciador;
    pthread_t threadsDir[4];

    //Inicializar Mutex cruzamentoMutex
    pthread_mutex_init(&cruzamentoMutex, NULL);


    filaInicios *filaPri[3];
    for(i=0;i<4;i++){
        filaPri[i] = malloc(sizeof(filaInicios));
        filaPri[i]->comeco = NULL;
    }
   
    for(i=0;i<4;i++){
        filaDir *pont = malloc(sizeof(filaDir));
        pont->dir = 'z';
        pont->id = 0;
        pont->prox = NULL;


        filaPri[i]->comeco = pont;
    }


    while(1){
    
        
        scanf("%s", direcoes);
        i=0;
        dirAtual = direcoes[i];

        while(dirAtual != '\0'){
            
            if(dirAtual == 'n'){
                enfileirar(j,'N',filaPri[0]->comeco);
            }

             if(dirAtual == 'e'){
                enfileirar(j,'E',filaPri[1]->comeco);
            }

             if(dirAtual == 's'){
                enfileirar(j,'S',filaPri[2]->comeco);
            }

             if(dirAtual == 'w'){
                enfileirar(j,'W',filaPri[3]->comeco);
            }
       
            j++;
            i++;
            dirAtual=direcoes[i];
          //  printf("estou aquiiiiiiiiiiiiiiiii\n");

        }

        imprimir(filaPri);


        pthread_create(&gerenciador, NULL, batman, (void *) filaPri);

        for(i=0;i<4;i++){ 
            printf("Thread %d criada \n", i);
            if(filaPri[i]->comeco->prox != NULL){
                pthread_create(&threadsDir[i],NULL, arrive, (void *) filaPri[i]->comeco);
            }
                
        }






    }



}



void * batman (void *arg){    
    filaInicios **fila = (filaInicios **)arg;
    bool estaVazia = filaVazia(fila);
    char ultimoCeder = '\0';

    printf("BATMANNN \n");

    while(estaVazia=filaVazia(fila) == false){


        if(fila[0]->comeco->prox != NULL){
            if(fila[0]->comeco->id > k){
                ultimoCeder = 'N';
            }else{
                if(ultimoCeder == fila[0]->comeco->prox->dir || fila[0]->comeco->id <= k){
                    pthread_cond_signal(&batN);  
                }
            }

        }else if(fila[1]->comeco->prox != NULL){
            if(fila[1]->comeco->id > k){
                ultimoCeder = 'E';
            }else{
                if(ultimoCeder == fila[1]->comeco->prox->dir || fila[1]->comeco->id <= k){
                    pthread_cond_signal(&batE);  
                }
            }

        }else if(fila[2]->comeco->prox != NULL){
            if(fila[2]->comeco->id > k){
                ultimoCeder = 'S';
            }else{
                if(ultimoCeder == fila[2]->comeco->prox->dir || fila[2]->comeco->id <= k){
                    pthread_cond_signal(&batS);  
                }
            }

        }else if(fila[3]->comeco->prox != NULL){
            if(fila[3]->comeco->id > k){
                ultimoCeder = 'W';
            }else{
                if(ultimoCeder == fila[3]->comeco->prox->dir || fila[3]->comeco->id <= k){
                    pthread_cond_signal(&batW);  
                }
            }
        }

    }
  
    printf("SAIU DO BATMAN\n");
    

}


void *arrive (void *arg){
    filaDir *comeco = (filaDir *)arg;
    

    //printf("ARRIVE - %c\n", dir);

    while(comeco->prox != NULL){

        char dir = comeco->prox->dir;

        if(dir == 'N'){
          //  printf("ESTOU N\n");
            pthread_mutex_lock(&cruzamentoMutex);
            printf("LOCK NO N\n");
            pthread_cond_wait(&batN, &cruzamentoMutex);
            printf("entrar no sleep\n");
            sleep(1);
            leave(comeco);
            
        }else if(dir == 'E'){
            //printf("ESTOU E\n");
            pthread_mutex_lock(&cruzamentoMutex);
            pthread_cond_wait(&batE, &cruzamentoMutex);
            printf("LOCK E \n");
            sleep(1);
            leave(comeco);
        }else if(dir == 'S'){
            //printf("ESTOU S\n");
            pthread_mutex_lock(&cruzamentoMutex);
            pthread_cond_wait(&batS, &cruzamentoMutex);
            //printf("LOCK S \n");
            sleep(1);
            leave(comeco);
        }else if(dir == 'W'){
            //printf("ESTOU W\n");
            pthread_mutex_lock(&cruzamentoMutex);
            pthread_cond_wait(&batW, &cruzamentoMutex);
            printf("LOCK W \n");
            sleep(1);
            leave(comeco);
        }
    }
  
}


void leave(filaDir *comeco){
    printf("LEAVE - %c - %d\n", comeco->prox->dir, comeco->prox->id);
    pthread_mutex_unlock(&cruzamentoMutex);
    printf("BAT %d %c saiu do cruzamento",comeco->prox->id, comeco->prox->dir);

    comeco->prox = comeco->prox->prox;
    comeco->id--;


}



void enfileirar (int id, char dir, filaDir *comeco){
    filaDir *temp, *novo;
    temp = comeco;

    novo = malloc(sizeof(filaDir));
    novo->id = id;
    novo->dir = dir;
    novo->prox=NULL;


    if(temp->prox != NULL){

        while(temp->prox != NULL){
            temp = temp->prox;
        }
    
        temp->prox=novo;
    
    }else{

        temp->prox = novo;
    
    }   

    comeco->id++;

    printf("BAT %d %c chegou no cruzamento\n", novo->id, novo->dir); 

}


bool filaVazia (filaInicios **fila){
    
    if(fila[0]->comeco->prox == NULL && fila[1]->comeco->prox == NULL && fila[2]->comeco->prox == NULL && fila[3]->comeco->prox == NULL){
        return true;
    }else{
        return false;
    }
}


void imprimir(filaInicios **fila){
    int i=0;
    filaDir *temp;

    for(i=0;i<4;i++){
        temp = fila[i]->comeco->prox;
        
        while (temp != NULL){
            printf("id:%d - dir:%c\n", temp->id, temp->dir);
            temp = temp->prox;
        }

    }
    
}

/*
void iprimirImpasse(filaInicios **fila){
    filaDir *temp;
    char dir[4];
    int i=0;

   
    if(temp = fila[0]->comeco->prox != NULL){
        dir[0] = 'N';
    }else{
        dir[0] = '\0';
    }

    if(temp = fila[1]->comeco->prox != NULL){
        dir[1] = 'E';
    }else{
        dir[1] = '\0';
    }

      if(temp = fila[2]->comeco->prox != NULL){
        dir[2] = 'S';
    }else{
        dir[2] = '\0';
    }

      if(temp = fila[3]->comeco->prox != NULL){
        dir[3] = 'W';
    }else{
        dir[3] = '\0';
    }
    
    printf("Impasse: %c,%c,%c e %c, sinalizando %c para ir", )

}*/