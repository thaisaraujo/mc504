#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#define TAMANHO_PAGEMAP 8
#define GET_BIT(X,Y) (X & ((uint64_t)1<<Y)) >> Y

typedef struct enderecosMemoria {
    unsigned long int valor;
    struct enderecosMemoria *prox;
} enderecosMemoria;

typedef struct listaEnderecos {
    enderecosMemoria *comeco;
    enderecosMemoria *fim;
} listaEnderecos;

void calculaEnderecoFisico (unsigned long int enderecoVirt, FILE* fppagemaps, listaEnderecos *enderecosPresentes, listaEnderecos *enderecosAusentes);
void adicionarLista(listaEnderecos *lista, unsigned long int valor);
void imprimirLista(listaEnderecos *lista);


int main(int argc, char **argv){

    listaEnderecos enderecosPresentes;
    enderecosPresentes.comeco = NULL;
    enderecosPresentes.fim = NULL;

    listaEnderecos enderecosAusentes;
    enderecosAusentes.comeco = NULL;
    enderecosAusentes.fim = NULL;

    FILE *fp, *fppagemaps;
    char word[1024];
    char nomeArquivoMaps [1024] = {}; char nomeArquivoPM [1024] = {};
    char buffer;
    char enderecoInicio[30], enderecoFinal[30];
    char *endptr;
    int i = 0, j = 0, k = 0;
    unsigned long pid = strtol(argv[1], &endptr, 10);
    long unsigned int enderecoLongComeco, enderecoLongFinal;

    sprintf(nomeArquivoMaps, "/proc/%ld/maps", pid);
    sprintf(nomeArquivoPM, "/proc/%ld/pagemap", pid);

    fp = fopen(nomeArquivoMaps, "rb");
    fppagemaps = fopen(nomeArquivoPM, "rb");

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

            while(buffer != ' '){
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

            unsigned long int tamanhoBloco = enderecoLongFinal - enderecoLongComeco;
            int numPaginas = tamanhoBloco / getpagesize();

            //printf("%lx %lx %d\n", enderecoLongComeco, enderecoLongFinal, numPaginas);

            for(k = 0; k < numPaginas; k++){
                calculaEnderecoFisico(enderecoLongComeco, fppagemaps, &enderecosPresentes, &enderecosAusentes);
                enderecoLongComeco += getpagesize();
            } 
        }
    }

    printf("Imprimindo Presentes\n");
    imprimirLista(&enderecosPresentes);
    printf("Imprimindo Ausentes\n");
    imprimirLista (&enderecosAusentes); 

    fclose(fp);
    fclose(fppagemaps);

}


void calculaEnderecoFisico (unsigned long int enderecoVirt, FILE* fppagemaps, listaEnderecos *enderecosPresentes, listaEnderecos *enderecosAusentes){

    uint64_t offset = enderecoVirt / getpagesize() * TAMANHO_PAGEMAP;
    

    if(fseek(fppagemaps, offset, SEEK_SET) != 0){
        printf("Pagemap não encontrado\n");
    };

    unsigned long int numeroPageFrame;
    uint64_t entradaPagemap;


    fread(&entradaPagemap, 1, TAMANHO_PAGEMAP, fppagemaps);
    fread(&numeroPageFrame, 1, TAMANHO_PAGEMAP - 1, fppagemaps); 

    //printf("0x%lx 0x%lx \n", entradaPagemap, numeroPageFrame);


    if(GET_BIT(entradaPagemap, 63)){
        adicionarLista(enderecosPresentes, numeroPageFrame);
    }else
    {
       adicionarLista(enderecosAusentes, enderecoVirt);
       return;
    }
    
    

} 

void adicionarLista(listaEnderecos *lista, unsigned long int valor){

    enderecosMemoria *novoEndereco =  malloc(sizeof(enderecosMemoria));
    novoEndereco->valor = valor;
    novoEndereco->prox = NULL;

    if(lista->comeco == NULL){
        lista->comeco = novoEndereco;
        lista->fim = novoEndereco;
    }else{
        enderecosMemoria *temp = lista->fim;
        temp->prox = novoEndereco;

        lista->fim = novoEndereco;    
    }
}


void imprimirLista(listaEnderecos *lista){
    enderecosMemoria *temp = lista->comeco;
    while(temp!=NULL){
        printf("0x%lx\n", temp->valor);
        temp = temp->prox;
    }
}

