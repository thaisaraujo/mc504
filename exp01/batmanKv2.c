/* MC504 Sistemas Operacionais
 * Experimento 01 - Gerenciamento de Thread
 * Autores: 
 * Autores: Thais Araujo Bispo - RA:187386
 * Data: 6/10/2019
 */



#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include<stdbool.h>



//Estrutura para BAT
struct filaDir{
	int id;
    char dir;
	struct filaDir *prox;
}; typedef struct filaDir filaDir;


//Estrtura que armazena o vetor para começa da fila de BAT  
struct filaInicios{
    filaDir *comeco;
}; typedef struct filaInicios filaInicios;


//Variaveis Globais
int k=2;
pthread_mutex_t cruzamentoMutex;
pthread_cond_t batN = PTHREAD_COND_INITIALIZER;
pthread_cond_t batE = PTHREAD_COND_INITIALIZER;
pthread_cond_t batS = PTHREAD_COND_INITIALIZER;
pthread_cond_t batW = PTHREAD_COND_INITIALIZER;


//Prototipos das funcoes
void enfileirar (int id, char dir, filaDir *fila);
void *chegada (void * arg);
void saida(filaDir *comeco); 
void * batman (void *arg);
bool filaVazia (filaInicios **fila);
void imprimir(filaInicios **fila);
void iprimirImpasse(filaInicios **fila, char ultimoCeder);
char batPassar(filaInicios **fila, char ultimoCeder);




int main(){

    int j=1;
    int i=0;

    char dirAtual;
    char direcoes[20];

    pthread_t gerenciador;
    pthread_t threadsDir[4];

    //Inicializar Mutex cruzamentoMutex
    pthread_mutex_init(&cruzamentoMutex, NULL);

    //Inicializar vetor filaPri que armazena ponteiro para inicio da fila de direcoes
    filaInicios *filaPri[3];
    for(i=0;i<4;i++){
        filaPri[i] = malloc(sizeof(filaInicios));
        filaPri[i]->comeco = NULL;
    }
   
   //Inicializar a cabeca de cada fila
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

        }

        //imprimir(filaPri);

        //criar thread gerenciador 
        pthread_create(&gerenciador, NULL, batman, (void *) filaPri);

        //criar thread para cada fila, caso nao vazia
        for(i=0;i<4;i++){ 
            if(filaPri[i]->comeco->prox != NULL){
                pthread_create(&threadsDir[i],NULL, chegada, (void *) filaPri[i]->comeco);
            }       
        }
    }
}



/* 
 * Rotina da Thread Gerenciador
 * Definir qual fila irá para cruzamento
 * Respeitando as regras do enunciado: 
 * prioridades das direções, verificar se n>k, e se BAT já cedeu
 * param: vetor filaPri
 */
void * batman (void *arg){    
    filaInicios **fila = (filaInicios **)arg;
    bool estaVazia = filaVazia(fila);
    char ultimoCeder = '\0';   //inicialziar a variavel 
   

    while(estaVazia=filaVazia(fila) == false){

        if(fila[0]->comeco->prox != NULL){
            if(fila[0]->comeco->id > k && ultimoCeder != 'N'){
                ultimoCeder = 'N';
            }else{
                if(ultimoCeder == 'N'){
                    ultimoCeder = '\0';
                }
                pthread_cond_signal(&batN);  
                }
        } 
        
        if(fila[1]->comeco->prox != NULL){
            if(fila[1]->comeco->id > k && ultimoCeder != 'E'){
                ultimoCeder = 'E';
            }else{
                if(ultimoCeder == 'E'){
                    ultimoCeder = '\0';
                }
                    pthread_cond_signal(&batE);  
            }
        } 
        
        if(fila[2]->comeco->prox != NULL){
            if(fila[2]->comeco->id > k && ultimoCeder != 'S'){
                ultimoCeder = 'S';
            }else{
                if(ultimoCeder == 'S'){
                    ultimoCeder = '\0';
                }
                pthread_cond_signal(&batS);  
            }
        }
        
        if(fila[3]->comeco->prox != NULL){
            if(fila[3]->comeco->id > k && ultimoCeder != 'W'){
                ultimoCeder = 'W';
            }else{
                if(ultimoCeder == 'W'){
                    ultimoCeder = '\0'; 
                }
                pthread_cond_signal(&batW); 
            }
        }
    } 
}



/* 
 * Rotina para cada BAT
 * Realizar cruzamento
 * BAT realiza lock no cruzamento
 * param: ponteiro da filaDir
 */
void *chegada (void *arg){
    filaDir *comeco = (filaDir *)arg;

    while(comeco->prox != NULL){

        char dir = comeco->prox->dir;

        if(dir == 'N'){
            pthread_mutex_lock(&cruzamentoMutex);
            pthread_cond_wait(&batN, &cruzamentoMutex);
            sleep(1);
            saida(comeco);
            
        }else if(dir == 'E'){
            pthread_mutex_lock(&cruzamentoMutex);
            pthread_cond_wait(&batE, &cruzamentoMutex);
            sleep(1);
            saida(comeco);
        }else if(dir == 'S'){
            pthread_mutex_lock(&cruzamentoMutex);
            pthread_cond_wait(&batS, &cruzamentoMutex);
            sleep(1);
            saida(comeco);
        }else if(dir == 'W'){
            pthread_mutex_lock(&cruzamentoMutex);
            pthread_cond_wait(&batW, &cruzamentoMutex);
            sleep(1);
            saida(comeco);
        }
    }
  
}



/* 
 * Saída do BAT no cruzamento
 * Print da saida do cruzamento
 * BAT realiza unlock e sai da fila (atualizar ponteiro da cabeca)
 * param: filaDir
 */
void saida(filaDir *comeco){
    pthread_mutex_unlock(&cruzamentoMutex);
    printf("BAT %d %c saiu do cruzamento",comeco->prox->id, comeco->prox->dir);

    comeco->prox = comeco->prox->prox;
    comeco->id--;
    filaDir *temp = comeco->prox;

}



/* 
 * Enfileira BAT na fila
 * Print que o BAT chegou no cruzamento
 * enfileira o BAT na fila de sua direção
 * param: id do BAT, direcao e ponteiro da cabeca da fila
 */
void enfileirar (int id, char dir, filaDir *comeco){
    filaDir *temp, *novo;
    temp = comeco;

    novo = malloc(sizeof(filaDir));
    novo->id = id;
    novo->dir = dir;
    novo->prox=NULL;


    //Verificar se nao eh o primeiro elemento
    //Procurar o ultima posicao inserida 
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



/* 
 * Verificar se nao existe mas BAT
 * param: filaDir
 * saida: bool true para fila vazia, false caso contrario
 */
bool filaVazia (filaInicios **fila){
    
    if(fila[0]->comeco->prox == NULL && fila[1]->comeco->prox == NULL && fila[2]->comeco->prox == NULL && fila[3]->comeco->prox == NULL){
        return true;
    }else{
        return false;
    }
}



/* 
 * Imprimir todas as filas de direcao
 * param: vetor de filaPri
 */
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
 * Imprimir Impasse
 * Verificar quais direcoes possui BAT que quer cruzar
 * Verificar qual BAT vai cruzar (ultimoCeder ou segui ordem de prioridade)
 * param: vetor de filaPri, char ultomoCeder
 */
void iprimirImpasse(filaInicios **fila, char ultimoCeder){
    char dir[4], passar;
    int i=0;

   
    if(fila[0]->comeco->prox != NULL){
        dir[0] = 'N';
    }else{
        dir[0] = '\0';
    }

    if(fila[1]->comeco->prox != NULL){
        if(passar )
        dir[1] = 'E';
    }else{
        dir[1] = '\0';
    }

    if(fila[2]->comeco->prox != NULL){
        dir[2] = 'S';
    }else{
        dir[2] = '\0';
    }

      if(fila[3]->comeco->prox != NULL){
        dir[3] = 'W';
    }else{
        dir[3] = '\0';
    }
    
    passar = batPassar(fila, ultimoCeder);

    printf("Impasse: %c,%c,%c e %c, sinalizando %c para ir\n",dir[0],dir[1], dir[2], dir[3], passar);


}



/* 
 * Funcao auxliar para Imprimir Impasse
 * Verificar qual BAT foi o ultimo a ceder 
 * param: vetor de filaPri, char ultomoCeder
 */
char batPassar(filaInicios **fila, char ultimoCeder){
    char batPassa;

    if(ultimoCeder != '\0'){
        return ultimoCeder;
    }else{
        if(fila[0]->comeco->id > 0){
            return 'N';
        }else if(fila[1]->comeco->id > 0){
            return'E';
        }else if(fila[2]->comeco->id > 0){
            return 'S';
        }else if(fila[3]->comeco->id > 0){
            return 'W';
        }
    }
}