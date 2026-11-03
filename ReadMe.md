# Trabalho 2 - Programação de Software Básico

Biblioteca de gerenciamento de memória em C: aloca e libera blocos dentro de
um **pool pré-alocado**, controlado por uma lista encadeada de alocações.

## Arquivos

| Arquivo       | O que é |
|---------------|---------|
| `mymemory.h`  | Estruturas (`allocation_t`, `mymemory_t`) e protótipos |
| `mymemory.c`  | Implementação da biblioteca |
| `main.c`      | Programa de teste (50 blocos de tamanhos variados, ordem embaralhada) |
| `Makefile`    | `make` compila, `make run` executa, `make clean` limpa |

## Compilar e rodar

```sh
make && ./trabalho
```

ou, sem `make`:

```sh
gcc -Wall -Wextra -g -o trabalho mymemory.c main.c
./trabalho
```

## Como funciona

```
+-----------------------------------------------------------------------------------------+
|                                    ESTRUTURA mymemory_t                                 |
|  total_size = 1024 bytes                                                                |
|  pool = 0x7FFF00 (Início do Bloco Gigante)                                              |
|  head --------+                                                                         |
+---------------+-------------------------------------------------------------------------+
                |
                v
       [NODO 1: allocation_t]                        [NODO 2: allocation_t]
       +-----------------------+                     +-----------------------+
       | start = 0x7FFF00      |                     | start = 0x7FFF80      |
       | size  = 64 bytes      |                     | size  = 128 bytes     |
       | next  ----------------+-------------------->| next  = NULL          |
       +-----------------------+                     +-----------------------+
                   |                                             |
                   | (Aponta para a                              | (Aponta para a
                   |  fatia no pool)                             |  fatia no pool)
                   v                                             v
+-----------------------------------------------------------------------------------------+
|                                  BLOCO DE MEMÓRIA (POOL)                                |
|                                                                                         |
|  [ Bloco Alocado 1 ]   [ Espaço Livre (Lacuna) ]   [ Bloco Alocado 2 ]  [ Livre Até o Fim]|
|  (64 bytes)            (  Bloco de Saturação  )    (128 bytes)          ( Restante )      |
|  0x7FFF00              0x7FFF40                    0x7FFF80             0x7FFFF0          |
+-----------------------------------------------------------------------------------------+
```

- O **pool** é um único `malloc` grande feito no `init`. Os ponteiros devolvidos
  por `mymemory_alloc` apontam para dentro dele.
- A **lista encadeada** guarda, para cada alocação, onde começa (`start`) e
  quanto ocupa (`size`). Ela é mantida **ordenada por endereço**, então a
  lacuna livre entre dois nodos é `prox->start - (atual->start + atual->size)`.
- **Estratégia: First Fit.** O `alloc` percorre as lacunas (antes do primeiro
  bloco, entre blocos, depois do último) e usa a primeira em que o tamanho
  pedido cabe.
- **`free`** não mexe no pool: só remove o nodo da lista. A região passa a ser
  enxergada como lacuna pelo `alloc` e pelo `stats`.
- **`stats`** percorre a lista uma vez e conta: nº de alocações, bytes
  alocados, bytes livres, maior lacuna contígua e nº de fragmentos (cada
  lacuna contígua conta como um fragmento).
- `malloc`/`free` da libc só são usados para o pool e para as estruturas de
  controle (`mymemory_t` e nodos `allocation_t`), como o enunciado permite.
