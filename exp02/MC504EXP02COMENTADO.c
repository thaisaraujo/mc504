/* Caio Augusto Alves Nolasco - RA:195181
   Thais Araujo Bispo - RA:187386

    O código apresentado varre, linha por linha, o diretório /proc/pid/maps para um pid dado. Cada linha
    fornece um intervalo de páginas de memória virtual, que é percorrido, página por página, e cada endereço é enviado
    como parâmetro para um função específica. Esta função lê, a partir de um offset, entradas do diretório /proc/pid/pagemap
    , e de cada entrada tira o número de page frame correspondente. As páginas presentes e ausentes são salvas em listas ligadas
    diferentes, e então os resultados são impressos.

    O offset para o diretório usado é offset = enderecoVirtual / PAGE_SIZE * TAMANHO_PAGEMAP(bytes)

    O código assume que as permissões CAP_SYS_ADMIN estão devidamente atribuídas.
    precisava alterar o nome do arquivo para encaminhar para professor e receber nota

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#define TAMANHO_PAGEMAP 8 //tamanho, em bytes, de uma entrada de /proc/pid/pagemap
#define BIT_PRESENTE(X,Y) (X & ((uint64_t)1<<Y)) >> Y //definição global para averiguar valor do bit 63 se página está em memória ou não

typedef struct enderecosMemoria {
    unsigned long int numeroPageFrame;
    unsigned long int entradaPagemap;
    struct enderecosMemoria *prox;
} enderecosMemoria; //nó da lista ligada para salvar enderecos parseados

typedef struct listaEnderecos {
    enderecosMemoria *comeco;
    enderecosMemoria *fim;
} listaEnderecos; //lista ligada com ponteiro para fim para agilizar inserção de novos endereços

void calculaEnderecoFisico (unsigned long int enderecoVirt, FILE* fppagemaps, listaEnderecos *enderecosPresentes, listaEnderecos *enderecosAusentes);
void adicionarListaPresentes(listaEnderecos *lista, unsigned long int numeroPageFrame, unsigned long int entradaPagemap);
void adicionarListaAusentes(listaEnderecos *lista, unsigned long int enderecoVirtual);
void imprimirListaPresentes(listaEnderecos *lista);
void imprimirListaAusentes(listaEnderecos *lista);


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
    fppagemaps = fopen(nomeArquivoPM, "rb"); //abre arquivos "maps" e "pagemaps" para processo de id dado

    if (fp == NULL) {
        printf("Processo não existente\n");
    } else {
        while ((fscanf(fp,"%s%*[^\n]",word) == 1)) { //lê todas as linhas de /proc/pid/maps 
            buffer = word[0];
            while(buffer != '-'){
                enderecoInicio[i] = word[i];
                i++;
                buffer = word[i];
            } //percorre linha para ler inicio do intervalo de memoria virtual

            i++;
            buffer = word[i];

            while(buffer != ' '){
                enderecoFinal[j] = word[i];
                i++;
                j++;
                buffer = word[i];
            } //le endereco final do intervalo de memória virtual


            i = 0;
            j = 0;
            enderecoLongComeco = strtol(enderecoInicio, NULL, 16);
            enderecoLongFinal = strtol(enderecoFinal, NULL,16);
            //converte os enderecos em string para inteiros

            unsigned long int tamanhoBloco = enderecoLongFinal - enderecoLongComeco;
            int numPaginas = tamanhoBloco / getpagesize();
            //calcula o numero de paginas presentes em um dado intervalor calculado

            for(k = 0; k < numPaginas; k++){ // enquanto ainda há páginas no intervalo
                calculaEnderecoFisico(enderecoLongComeco, fppagemaps, &enderecosPresentes, &enderecosAusentes);
                enderecoLongComeco += getpagesize(); //soma o tamanho de uma página no endereço anterior
            } 
        }
    }

    printf("Imprimindo Presentes\n");
    printf("As entradas estão na forma: #numeroPageFrame #numeroEntradaPagemap\n");
    imprimirListaPresentes(&enderecosPresentes);
    printf("Imprimindo Ausentes\n");
    imprimirListaAusentes(&enderecosAusentes);  //impressão do endereços: primeiro os presentes em memória, e depois os ausentes

    fclose(fp);
    fclose(fppagemaps);

}


void calculaEnderecoFisico (unsigned long int enderecoVirt, FILE* fppagemaps, listaEnderecos *enderecosPresentes, listaEnderecos *enderecosAusentes){

    uint64_t offset = enderecoVirt / getpagesize() * TAMANHO_PAGEMAP;
    //offset para cada entrada de /proc/pid/pagemap, que depende do endereço virtual e do tamanho da página, multiplicados por 8 bytes
    

    if(fseek(fppagemaps, offset, SEEK_SET) != 0){
        printf("Pagemap não encontrado\n");
    };

    unsigned long int numeroPageFrame;
    uint64_t entradaPagemap;

    fread(&numeroPageFrame, 1, (TAMANHO_PAGEMAP - 1), fppagemaps); //le 7 bytes, que são os primeiros 56 bits da entrada de pagemap
    fseek(fppagemaps, offset, SEEK_SET);
    fread(&entradaPagemap, 1, TAMANHO_PAGEMAP, fppagemaps); //le 8 bytes, que corresponde a toda a entrada em pagemap para dado endereco virtual
    
    //os bits 0 a 55 são lidos em numeroPageFrame. O page frame corresponde aos bits 0 a 54, então é preciso zerar o bit 55

    numeroPageFrame = numeroPageFrame & 0x7fffffffffffff;

    if(BIT_PRESENTE(entradaPagemap, 63)){ //analisa o bit 63 e adiciona a lista correspondente
        adicionarListaPresentes(enderecosPresentes, numeroPageFrame, entradaPagemap);
    }else{
        adicionarListaAusentes(enderecosAusentes, enderecoVirt);
    }
    
    return;

} 

void adicionarListaPresentes(listaEnderecos *lista, unsigned long int numeroPageFrame, unsigned long int entradaPagemap){ //função para inserção em lista ligada

    enderecosMemoria *novoEndereco =  malloc(sizeof(enderecosMemoria));
    novoEndereco->numeroPageFrame = numeroPageFrame;
    novoEndereco->entradaPagemap = entradaPagemap;
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


void adicionarListaAusentes(listaEnderecos *lista, unsigned long int enderecoVirtual){ //função para inserção em lista ligada

    enderecosMemoria *novoEndereco =  malloc(sizeof(enderecosMemoria));
    novoEndereco->numeroPageFrame = enderecoVirtual;
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

void imprimirListaPresentes(listaEnderecos *lista){ //função para impressão de lista ligada
    enderecosMemoria *temp = lista->comeco;
    while(temp!=NULL){
        printf("0x%lx 0x%lx\n", temp->numeroPageFrame, temp->entradaPagemap);
        temp = temp->prox;
    }
}

void imprimirListaAusentes(listaEnderecos *lista){ //função para impressão de lista ligada
    enderecosMemoria *temp = lista->comeco;
    while(temp!=NULL){
        printf("0x%lx\n", temp->numeroPageFrame);
        temp = temp->prox;
    }
}

