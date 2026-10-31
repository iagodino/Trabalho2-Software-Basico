#include <stdio.h>
#include <stdlib.h>
#include "mymemory.h"

mymemory_t* mymemory_init(size_t size);

void* mymemory_alloc(mymemory_t *memory, size_t size);

void mymemory_display(mymemory_t *memory);

void mymemory_cleanup(mymemory_t *memory);

mymemory_t* mymemory_init(size_t size){
    mymemory_t *aux = (mymemory_t*) malloc(sizeof(mymemory_t));
    if(aux == NULL){
        return NULL;
    }
    aux->pool = malloc(size);
    if(aux->pool == NULL){
        free(aux);
        return NULL;
    }

    aux->total_size = size;
    aux->head = NULL;
    
    return aux;
}

void* mymemory_alloc(mymemory_t *memory, size_t size){
    if (memory == NULL || size == 0) {
        return NULL;
    }
    
    void *endereco_disponivel = NULL;

    // Caso 1: A lista está vazia (nenhum bloco alocado ainda)
    if (memory->head == NULL) {
        // Verifica se o tamanho cabe no pool inteiro
        if (size <= memory->total_size) {
            endereco_disponivel = memory->pool;
        } else {
            return NULL; // Não cabe
        }
    } 
    else {
        // Estratégia First Fit: Procurar lacunas entre os blocos alocados
        // (Aqui você percorre os nós 'head', verifica o espaço entre o fim de um bloco
        // e o início do próximo, ou antes do primeiro bloco, ou após o último).
        
        // Exemplo simplificado de varredura (você precisará ajustar conforme sua lógica de ponteiros):
        allocation_t *atual = memory->head;
        
        // Verifique se há espaço entre o início do pool e o primeiro nó:
        size_t espaco_antes = (size_t)((char*)atual->start - (char*)memory->pool);
        if (espaco_antes >= size) {
            endereco_disponivel = memory->pool;
        } else {
            // Percorre a lista procurando lacuna entre os nós
            while (atual->next != NULL) {
                void *fim_atual = (char*)atual->start + atual->size;
                size_t espaco_entre = (size_t)((char*)atual->next->start - (char*)fim_atual);
                
                if (espaco_entre >= size) {
                    endereco_disponivel = fim_atual;
                    break;
                }
                atual = atual->next;
            }
            
            // Se ainda não achou, verifica se cabe no final do último nó até o fim do pool
            if (endereco_disponivel == NULL) {
                void *fim_ultimo = (char*)atual->start + atual->size;
                size_t espaco_depois = (size_t)((char*)memory->pool + memory->total_size - (char*)fim_ultimo);
                
                if (espaco_depois >= size) {
                    endereco_disponivel = fim_ultimo;
                }
            }
        }
    }

    // Se encontramos um espaço livre válido, criamos o nodo de controle para ele
    if (endereco_disponivel != NULL) {
        allocation_t *novo_nodo = (allocation_t *) malloc(sizeof(allocation_t));
        if (novo_nodo == NULL) {
            return NULL; // Falha no malloc do nodo de controle
        }
        
        novo_nodo->start = endereco_disponivel;
        novo_nodo->size = size;
        novo_nodo->next = NULL;

        // Insere o novo nodo na lista encadeada (mantendo a ordem ou inserindo no início/fim)
        // (Dica: ordenar por endereço de memória 'start' facilita muito na hora de buscar lacunas!)
        if (memory->head == NULL || (char*)novo_nodo->start < (char*)memory->head->start) {
            novo_nodo->next = memory->head;
            memory->head = novo_nodo;
        } else {
            allocation_t *temp = memory->head;
            while (temp->next != NULL && (char*)temp->next->start < (char*)novo_nodo->start) {
                temp = temp->next;
            }
            novo_nodo->next = temp->next;
            temp->next = novo_nodo;
        }

        return endereco_disponivel;
    }

    return NULL; // Retorna NULL se não houver espaço suficiente no pool

}

//void mymemory_display(mymemory_t *memory)
//{
//    printf("%d \n", aux->data);
//    if(aux!= next){
//        mymemory_display(aux->next,tail);
//    }
//}

void mymemory_cleanup(mymemory_t *memory)
{
    if(memory == NULL){
        return;
    }
    allocation_t *atual = memory->head;
    while(atual !=NULL){
        allocation_t *proximo = atual->next;
        free(atual);
        atual = proximo;
    }
    if(memory->pool != NULL){
        free(memory->pool);
    }
    free(memory);
}

int main()
{
    // aloca pool memoria
    mymemory_t *memory = mymemory_init(1000);
    //printf(" tamanho total: %zu \n",memory->total_size);
    //printf(" cabaça: %zu \n",memory->head);
    
    int *v = mymemory_alloc(memory, sizeof(int)*32);
    for (int i = 0; i < 32; i++)
        v[i] = i+1;


    int *v2 = mymemory_alloc(memory, sizeof(int)*16);
//
    //mymemory_free(memory, v);
//
    //char *str = mymemory_malloc(memory, sizeof(char)*16);
    //sprintf(str, "ola mundo");

    // libera o pool de memoria
    mymemory_cleanup(memory);
    //printf(" tamanho total: %zu \n",memory->total_size);
    //printf(" cabaça: %zu \n",memory->head);
    return 0;
}
