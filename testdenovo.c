#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#define TAMANHO_PAGEMAP 8

typedef struct enderecosMemoria {
    unsigned long int valor;
    struct enderecosMemoria *prox;
} enderecosMemoria;

void calculaEnderecoFisico (unsigned long int enderecoVirt, FILE* fppagemaps, enderecosMemoria *enderecosPresentes, enderecosMemoria *enderecosAusentes);
void adicionarLista(enderecosMemoria *lista, unsigned long int valor);


int main(){

    enderecosMemoria enderecosPresentes;
    enderecosPresentes.valor = 0;
    enderecosPresentes.prox = NULL;

    enderecosMemoria enderecosAusentes;
    enderecosAusentes.valor = 0;
    enderecosAusentes.prox = NULL;

    FILE *fp, *fppagemaps;
    char word[1024];
    char buffer;
    char enderecoInicio[30], enderecoFinal[30];
    int i = 0, j = 0, k = 0;
    long unsigned int enderecoLongComeco, enderecoLongFinal;
    fp = fopen("/proc/20501/maps", "r");
    fppagemaps = fopen("/proc/20501/pagemap", "r");
    if (fp == NULL) {
        printf("Processo não existente\n");
    } else {
        while ((fscanf(fp,"%s%*[^\n]",word) == 1)) { 
            buffer = word[0];
            while(buffer != '-'){
                enderecoInicio[i] = word[i];
                i++;
                buffer = word[i];
            }

            i++;
            buffer = word[i];

            while(buffer != ' '){ //TA ERRADO
                //printf("oi\n");
                enderecoFinal[j] = word[i];
                i++;
                j++;
                buffer = word[i];
            } 


            i = 0;
            j = 0;
            //printf("word read is: %s\n", endereco);
            enderecoLongComeco = strtol(enderecoInicio, NULL, 16);
            enderecoLongFinal = strtol(enderecoFinal, NULL,16);

            printf("numero: %lu - %lu\n", enderecoLongComeco, enderecoLongFinal);

            unsigned long int tamanhoBloco = enderecoLongFinal - enderecoLongComeco;
            int numPaginas = tamanhoBloco / getpagesize();

            //printf("%d\n", numPaginas);

            for(k = 0; k < numPaginas; k++){
                calculaEnderecoFisico(enderecoLongComeco, fppagemaps, &enderecosPresentes, &enderecosAusentes);
                enderecoLongComeco += getpagesize();
            } 
        }
    }

fclose(fp);
fclose(fppagemaps);

}


void calculaEnderecoFisico (unsigned long int enderecoVirt, FILE* fppagemaps, enderecosMemoria *enderecosPresentes, enderecosMemoria *enderecosAusentes){

    uint64_t offset = enderecoVirt / getpagesize() * TAMANHO_PAGEMAP;

    if(fseek(fppagemaps, offset, SEEK_SET) != 0){
        printf("Pagemap não encontrado\n");
    };

    unsigned long int numeroPageFrame = 0;
    uint64_t bitsPagemap = 0;


    fread(&bitsPagemap, 1, TAMANHO_PAGEMAP, fppagemaps);
    fread(&numeroPageFrame, 1, TAMANHO_PAGEMAP - 1, fppagemaps);

    numeroPageFrame &= 0x7FFFFFFFFFFFFF;

    if(bitsPagemap & 1){
        adicionarLista(enderecosPresentes, numeroPageFrame);
    }else
    {
        adicionarLista(enderecosAusentes, numeroPageFrame);
    }
    

} 

void adicionarLista(enderecosMemoria *lista, unsigned long int valor){

    enderecosMemoria *novoEndereco =  malloc(sizeof(enderecosMemoria));
    novoEndereco->valor = valor;
    novoEndereco->prox = NULL;

    if(lista->prox == NULL){
        lista->prox = novoEndereco;
    }else{
        enderecosMemoria *temp = lista->prox;
        while(temp->prox != NULL){
            temp = temp->prox;
        }

        temp->prox = novoEndereco;
       
    }
}
