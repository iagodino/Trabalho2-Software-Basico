/*
 * mymemory.c - Biblioteca de gerenciamento de memoria em pool
 * Programacao de Software Basico - Trabalho 2
 *
 * Estrategia de alocacao: First Fit (primeira lacuna que couber).
 *
 * A lista encadeada de allocation_t e mantida ORDENADA por endereco de
 * inicio. Isso permite descobrir as lacunas livres percorrendo a lista
 * uma unica vez: a lacuna entre dois nodos e simplesmente
 *     inicio_do_proximo - (inicio_do_atual + tamanho_do_atual)
 *
 * malloc/free so sao usados para as estruturas de controle
 * (mymemory_t e os nodos allocation_t) e para o proprio pool,
 * conforme a restricao do enunciado.
 */
#include <stdio.h>
#include <stdlib.h>
#include "mymemory.h"

/* ================================================================ init */
mymemory_t *mymemory_init(size_t size)
{
    mymemory_t *aux = (mymemory_t *)malloc(sizeof(mymemory_t));
    if (aux == NULL)
    {
        return NULL;
    }
    aux->pool = malloc(size);
    if (aux->pool == NULL)
    {
        free(aux);
        return NULL;
    }

    aux->total_size = size;
    aux->head = NULL;

    return aux;
}

/* =============================================================== alloc */
void *mymemory_alloc(mymemory_t *memory, size_t size)
{
    if (memory == NULL || size == 0)
    {
        return NULL;
    }

    void *endereco_disponivel = NULL;

    if (memory->head == NULL)
    {
        if (size <= memory->total_size)
        {
            endereco_disponivel = memory->pool;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        allocation_t *atual = memory->head;

        size_t espaco_antes = (size_t)((char *)atual->start - (char *)memory->pool);
        if (espaco_antes >= size)
        {
            endereco_disponivel = memory->pool;
        }
        else
        {
            while (atual->next != NULL)
            {
                void *fim_atual = (char *)atual->start + atual->size;
                size_t espaco_entre = (size_t)((char *)atual->next->start - (char *)fim_atual);

                if (espaco_entre >= size)
                {
                    endereco_disponivel = fim_atual;
                    break;
                }
                atual = atual->next;
            }

            if (endereco_disponivel == NULL)
            {
                void *fim_ultimo = (char *)atual->start + atual->size;
                size_t espaco_depois = (size_t)((char *)memory->pool + memory->total_size - (char *)fim_ultimo);

                if (espaco_depois >= size)
                {
                    endereco_disponivel = fim_ultimo;
                }
            }
        }
    }

    if (endereco_disponivel != NULL)
    {
        /* Cria o nodo de controle da nova alocacao */
        allocation_t *novo_nodo = (allocation_t *)malloc(sizeof(allocation_t));
        if (novo_nodo == NULL)
        {
            return NULL;
        }

        novo_nodo->start = endereco_disponivel;
        novo_nodo->size = size;
        novo_nodo->next = NULL;

        /* Insere na lista mantendo a ordem crescente de endereco */
        if (memory->head == NULL || (char *)novo_nodo->start < (char *)memory->head->start)
        {
            novo_nodo->next = memory->head;
            memory->head = novo_nodo;
        }
        else
        {
            allocation_t *temp = memory->head;
            while (temp->next != NULL && (char *)temp->next->start < (char *)novo_nodo->start)
            {
                temp = temp->next;
            }
            novo_nodo->next = temp->next;
            temp->next = novo_nodo;
        }

        return endereco_disponivel;
    }

    return NULL; /* nenhuma lacuna comporta o tamanho pedido */
}

/* ================================================================ free */
/* Libera a alocacao cujo inicio e exatamente ptr.
 * Se ptr nao for o inicio de nenhuma alocacao, nao faz nada.
 * Observacao: o pool nao e tocado; "liberar" = remover o nodo de controle
 * da lista. A regiao volta a ser vista como lacuna pelo alloc/stats. */
void mymemory_free(mymemory_t *memory, void *ptr)
{
    if (memory == NULL || ptr == NULL)
    {
        return;
    }

    allocation_t *anterior = NULL;
    allocation_t *atual = memory->head;

    /* Procura o nodo cujo start == ptr, guardando o anterior para religar a lista */
    while (atual != NULL && atual->start != ptr)
    {
        anterior = atual;
        atual = atual->next;
    }

    if (atual == NULL)
    {
        return; /* ptr nao e uma alocacao valida */
    }

    if (anterior == NULL)
    {
        memory->head = atual->next; /* era o primeiro da lista */
    }
    else
    {
        anterior->next = atual->next; /* estava no meio ou no fim */
    }

    free(atual);
}

/* ============================================================= display */
/* Lista todas as alocacoes atuais, em ordem de endereco.
 * Alem do endereco real, mostra o offset (distancia do inicio do pool),
 * que e bem mais legivel para enxergar as lacunas. */
void mymemory_display(mymemory_t *memory)
{
    if (memory == NULL)
    {
        return;
    }

    if (memory->head == NULL)
    {
        printf("(nenhuma alocacao)\n");
        return;
    }

    printf("--- Alocacoes atuais (pool em %p, %zu bytes) ---\n",
           memory->pool, memory->total_size);

    int i = 0;
    for (allocation_t *a = memory->head; a != NULL; a = a->next, i++)
    {
        size_t offset = (size_t)((char *)a->start - (char *)memory->pool);
        printf("[%2d] inicio=%p | offset %5zu | tamanho %5zu | fim %5zu\n",
               i, a->start, offset, a->size, offset + a->size);
    }
}

/* =============================================================== stats */
/* Percorre o pool do inicio ao fim acompanhando a lista (ordenada):
 * tudo que esta entre o "cursor" e o inicio do proximo bloco e lacuna livre.
 * Cada lacuna contigua conta como um fragmento. */
void mymemory_stats(mymemory_t *memory)
{
    if (memory == NULL)
    {
        return;
    }

    size_t num_alocacoes = 0;
    size_t total_alocado = 0;
    size_t maior_livre = 0;
    size_t num_fragmentos = 0;

    char *cursor = (char *)memory->pool; /* inicio da proxima lacuna possivel */
    char *fim_pool = (char *)memory->pool + memory->total_size;

    for (allocation_t *a = memory->head; a != NULL; a = a->next)
    {
        num_alocacoes++;
        total_alocado += a->size;

        size_t lacuna = (size_t)((char *)a->start - cursor);
        if (lacuna > 0)
        {
            num_fragmentos++;
            if (lacuna > maior_livre)
            {
                maior_livre = lacuna;
            }
        }
        cursor = (char *)a->start + a->size;
    }

    /* Lacuna final: do fim do ultimo bloco ate o fim do pool */
    size_t lacuna_final = (size_t)(fim_pool - cursor);
    if (lacuna_final > 0)
    {
        num_fragmentos++;
        if (lacuna_final > maior_livre)
        {
            maior_livre = lacuna_final;
        }
    }

    size_t total_livre = memory->total_size - total_alocado;

    printf("--- Estatisticas ---\n");
    printf("Alocacoes:           %6zu\n", num_alocacoes);
    printf("Memoria alocada:     %6zu bytes\n", total_alocado);
    printf("Memoria livre:       %6zu bytes\n", total_livre);
    printf("Maior bloco livre:   %6zu bytes\n", maior_livre);
    printf("Fragmentos livres:   %6zu\n", num_fragmentos);
}

/* ============================================================= cleanup */
void mymemory_cleanup(mymemory_t *memory)
{
    if (memory == NULL)
    {
        return;
    }
    allocation_t *atual = memory->head;
    while (atual != NULL)
    {
        allocation_t *proximo = atual->next;
        free(atual);
        atual = proximo;
    }
    if (memory->pool != NULL)
    {
        free(memory->pool);
    }
    free(memory);
}
