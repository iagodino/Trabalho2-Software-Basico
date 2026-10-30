#include <stdio.h>
#include <stdlib.h>

typedef struct TNode {
    int data;
    struct TNode* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
    int size;
} List;

void add(List* l, int val);

void add(List* l, int val)
{
    Node* aux = malloc(sizeof(Node));
    aux->data = val;
    aux->next = NULL;
    
    if(l->tail != NULL){
        l->tail->next=aux;
        l->tail = aux;
    }else
        l->head = aux;
    l->tail = aux;
    l->size++;
}

void addStart(List* l, int val)
{
    Node* aux = malloc(sizeof(Node));
    aux->data = val;
    aux->next = NULL;
    
    if(l->head != NULL){
        aux->next=l->head;
        l->head = aux;
    }else
        l->tail = aux;
    l->head = aux;
    l->size++;
}

void removeByValue(List* l,Node* aux, int val)
{
    if(aux->data == val){
        Node* aux2 = malloc(sizeof(Node));
        aux2 = aux->next;
        free(aux);
        aux = aux2;
        
        l->size--;
    }else{
        if(aux!=l->tail){
            removeByValue(l, aux->next, val);
        }
    }
    //Não consegui encontrar uma solução a tempo
}

void show(Node* aux,Node* tail)
{
    printf("%d \n", aux->data);
    if(aux!=tail){
        show(aux->next,tail);
    }
}

void libera(Node* aux,List* l)
{
    if(aux!=l->tail){
        libera(aux->next,l);
    }
    free(aux);
    l->size--;
}


int main()
{
    
    List lista;
    lista.head = lista.tail = NULL;
    lista.size = 0;
    //printf("head | tail | size \n",lista.head, lista.tail, lista.size);
    //printf("%d| %d | %d \n",lista.head, lista.tail, lista.size);
    add(&lista, 1);
   // printf("%d| %d | %d \n",lista.head, lista.tail, lista.size);
    add(&lista, 2);
   // printf("%d| %d | %d \n",lista.head, lista.tail, lista.size);
    add(&lista, 3);
   //printf("%d| %d | %d \n",lista.head->data, lista.tail->data, lista.size);
    add(&lista, 4);
   //printf("%d| %d | %d \n",lista.head->data, lista.tail->data, lista.size);
    add(&lista, 5);
   //printf("%d| %d | %d \n",lista.head->data, lista.tail->data, lista.size);

   addStart(&lista, 6);
    
    show(lista.head,lista.tail);
    removeByValue(&lista, lista.head, 3);
    libera(lista.head,&lista);
    // TODO: uma função para exibir essa lista
    // e não esquecer de liberar os nodos no final!!!
    return 0;
}