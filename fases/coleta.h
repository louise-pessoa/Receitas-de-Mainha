#ifndef COLETA_H
#define COLETA_H

#include "raylib.h"
#include "../dados/receitas.h"

#define CATCHER_TEMPO_LIMITE    25
#define CATCHER_MAX_ITENS       16
#define CATCHER_MAX_ALVOS       10
#define CATCHER_MAX_DISTRATORES 16

// um ingrediente caindo na tela
typedef struct {
    int   ativo;
    int   eh_da_receita;     // 1 se faz parte da receita escolhida
    int   idx_alvo;          // indice em EstadoCatcher.alvos quando eh_da_receita = 1
    char  nome[60];
    float x;
    float y;
    float velocidade;
    Color cor;
} ItemCaindo;

// ingrediente que o jogador precisa coletar
typedef struct {
    char nome[60];
    int  coletado;
} IngredienteAlvo;

typedef struct {
    Receita *receita;

    IngredienteAlvo alvos[CATCHER_MAX_ALVOS];
    int n_alvos;
    int coletados;
    int spawnou[CATCHER_MAX_ALVOS];        // 1 se o ingrediente ja apareceu na tela
    int ordem_coleta[CATCHER_MAX_ALVOS];   // indices de alvos na ordem de coleta
    int n_coletados_em_ordem;

    ItemCaindo itens[CATCHER_MAX_ITENS];

    float cesta_x;
    float cesta_y;
    float cesta_w;
    float cesta_h;
    float cesta_velocidade;

    double tempo_inicio;
    float  tempo_decorrido;
    int    periodo_garantido;        // segundos de spawn garantido por dificuldade
    int    ultimo_segundo_penalizado;

    int    pontos;
    int    erros_distrator;        // pegou item errado
    int    perdidos;               // ingrediente da receita escapou

    int    iniciado;
    int    terminou;
    int    venceu;
    int    pausado;

    float  spawn_timer;
    float  spawn_intervalo;
} EstadoCatcher;

extern EstadoCatcher catcher;

// inicia uma rodada do catcher com a receita fornecida
void catcher_iniciar(Receita *receita);

// chamado todo frame: atualiza fisica, spawn, colisoes e timer
void catcher_atualizar(void);

// desenha a tela do minigame
void tela_catcher(void);

// retorna 1 quando a rodada acabou (vitoria ou tempo esgotado)
int  catcher_terminou(void);

// retorna 1 se o jogador coletou todos os ingredientes
int  catcher_venceu(void);

// carrega sprites do catcher (chamar uma vez depois do InitWindow)
void catcher_carregar_sprites(void);

// libera sprites do catcher (chamar uma vez antes do CloseWindow)
void catcher_descarregar_sprites(void);

#endif
