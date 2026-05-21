#ifndef ORDENACAO_H
#define ORDENACAO_H

#include "../dados/receitas.h"

#define MAX_ORDENACAO 12

typedef struct {
    char nome[60];
    int  ordem_correta;  // posicao final desejada
} ItemOrdenacao;

typedef struct {
    ItemOrdenacao itens[MAX_ORDENACAO];
    int n;

    int i;            // indice externo do insertion sort
    int j;            // indice interno (compara com j-1)
    int passo;        // 0=preparar comparacao, 1=animando troca
    int concluido;

    float timer;
    float intervalo;  // segundos entre cada passo da animacao

    Receita *receita;
} EstadoOrdenacao;

extern EstadoOrdenacao ordenacao;

// inicia a ordenacao com os ingredientes na ordem em que foram coletados no catcher.
// o insertion sort os coloca na ordem correta de uso da receita.
void ordenacao_iniciar(Receita *receita, int *ordem_coleta, int n_coletados);

// avanca um passo da animacao (chamado automaticamente por timer)
void ordenacao_atualizar(void);

// desenha a tela
void tela_ordenacao(void);

int  ordenacao_terminou(void);

#endif
