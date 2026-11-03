/*
 * main.c - Programa de teste do gerenciador de memoria (mymemory)
 * Programacao de Software Basico - Trabalho 2
 *
 * Roteiro:
 *   1. init do pool
 *   2. 50 alocacoes de tamanhos variados, em ordem embaralhada
 *   3. libera 1 a cada 2 blocos  -> gera fragmentacao
 *   4. confere integridade dos blocos que sobraram
 *   5. novas alocacoes pequenas   -> First Fit reaproveita as lacunas
 *   6. free com ponteiro invalido -> nao faz nada
 *   7. libera tudo
 *   8. cleanup
 */
#include <stdio.h>
#include <string.h>
#include "mymemory.h"

#define POOL_SIZE   10000
#define NUM_BLOCOS  50

/* Tamanhos sugeridos no enunciado */
static const size_t TAMANHOS[] = {10, 25, 50, 100, 250, 500, 1000};
#define NUM_TAMANHOS (sizeof(TAMANHOS) / sizeof(TAMANHOS[0]))

/* Gerador pseudoaleatorio proprio (LCG) em vez de rand():
 * a sequencia e identica em qualquer maquina/compilador,
 * entao a saida do programa e sempre a mesma na apresentacao. */
static unsigned int semente = 12345u;

static unsigned int proximo_aleatorio(void){
    semente = semente * 1103515245u + 12345u;
    return (semente >> 16) & 0x7fffu;
}

static void embaralhar(size_t *v, int n){
    for (int i = n - 1; i > 0; i--) {
        int j = (int)(proximo_aleatorio() % (unsigned int)(i + 1));
        size_t tmp = v[i];
        v[i] = v[j];
        v[j] = tmp;
    }
}

static size_t offset_de(mymemory_t *memory, void *ptr){
    return (size_t)((char*)ptr - (char*)memory->pool);
}

int main(void)
{
    size_t tamanhos[NUM_BLOCOS];
    void  *blocos[NUM_BLOCOS];
    int alocados = 0, falhas = 0;

    for (int i = 0; i < NUM_BLOCOS; i++) {
        tamanhos[i] = TAMANHOS[i % NUM_TAMANHOS];
    }
    embaralhar(tamanhos, NUM_BLOCOS);

    /* ------------------------------------------------------------ 1 */
    printf("### 1. init: pool de %d bytes\n", POOL_SIZE);
    mymemory_t *memory = mymemory_init(POOL_SIZE);
    if (memory == NULL) {
        fprintf(stderr, "falha ao inicializar o pool\n");
        return 1;
    }
    mymemory_stats(memory);

    /* ------------------------------------------------------------ 2 */
    printf("\n### 2. %d alocacoes em ordem embaralhada\n", NUM_BLOCOS);
    printf("ordem dos tamanhos:");
    for (int i = 0; i < NUM_BLOCOS; i++) {
        printf(" %zu", tamanhos[i]);
    }
    printf("\n");

    for (int i = 0; i < NUM_BLOCOS; i++) {
        blocos[i] = mymemory_alloc(memory, tamanhos[i]);
        if (blocos[i] != NULL) {
            /* escreve no bloco inteiro: prova que o endereco devolvido e valido */
            memset(blocos[i], (unsigned char)(i + 1), tamanhos[i]);
            alocados++;
        } else {
            falhas++;
        }
    }
    printf("alocados: %d | falharam (sem lacuna suficiente): %d\n", alocados, falhas);
    mymemory_display(memory);
    mymemory_stats(memory);

    /* ------------------------------------------------------------ 3 */
    printf("\n### 3. libera 1 a cada 2 blocos (gera fragmentacao)\n");
    for (int i = 0; i < NUM_BLOCOS; i += 2) {
        if (blocos[i] != NULL) {
            mymemory_free(memory, blocos[i]);
            blocos[i] = NULL;
        }
    }
    mymemory_stats(memory);

    /* ------------------------------------------------------------ 4 */
    printf("\n### 4. integridade: blocos restantes ainda tem o conteudo original?\n");
    int erros = 0;
    for (int i = 0; i < NUM_BLOCOS; i++) {
        if (blocos[i] == NULL) continue;
        unsigned char *b = blocos[i];
        for (size_t k = 0; k < tamanhos[i]; k++) {
            if (b[k] != (unsigned char)(i + 1)) {
                erros++;
                break;
            }
        }
    }
    printf("%s (%d bloco(s) corrompido(s))\n", erros == 0 ? "OK" : "ERRO", erros);

    /* ------------------------------------------------------------ 5 */
    printf("\n### 5. novas alocacoes: First Fit reaproveita as lacunas\n");
    size_t novos_tam[] = {10, 25, 50, 100, 250, 500};
    int num_novos = (int)(sizeof(novos_tam) / sizeof(novos_tam[0]));
    void *novos[sizeof(novos_tam) / sizeof(novos_tam[0])];
    for (int i = 0; i < num_novos; i++) {
        novos[i] = mymemory_alloc(memory, novos_tam[i]);
        if (novos[i] != NULL) {
            printf("alloc(%4zu) -> offset %5zu\n", novos_tam[i], offset_de(memory, novos[i]));
        } else {
            printf("alloc(%4zu) -> NULL\n", novos_tam[i]);
        }
    }
    mymemory_display(memory);
    mymemory_stats(memory);

    /* ------------------------------------------------------------ 6 */
    printf("\n### 6. free com ponteiro invalido: nada deve mudar\n");
    int variavel_local = 0;
    mymemory_free(memory, NULL);                       /* NULL */
    mymemory_free(memory, &variavel_local);            /* fora do pool */
    mymemory_free(memory, (char*)memory->pool + 3);    /* dentro do pool, mas nao e inicio de bloco */
    mymemory_stats(memory);

    /* ------------------------------------------------------------ 7 */
    printf("\n### 7. libera tudo\n");
    for (int i = 0; i < NUM_BLOCOS; i++) {
        if (blocos[i] != NULL) {
            mymemory_free(memory, blocos[i]);
        }
    }
    for (int i = 0; i < num_novos; i++) {
        if (novos[i] != NULL) {
            mymemory_free(memory, novos[i]);
        }
    }
    mymemory_display(memory);
    mymemory_stats(memory);

    /* ------------------------------------------------------------ 8 */
    printf("\n### 8. cleanup\n");
    mymemory_cleanup(memory);
    printf("fim.\n");

    return 0;
}
