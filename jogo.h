#ifndef JOGO_H
#define JOGO_H

#include "dados/receitas.h"

// constantes do jogo
#define PONTUACAO_INICIAL  100
#define META_FASE_FINAL     70


// estado da partida
typedef struct {
    int pontuacao;
    int rodada_atual;
    int em_execucao;
    int fase_final_ativa;
    int passos_acertados;
    int passos_total;
    int tempo_extra;
    int erro_passo;
    int receitas_completadas;   // contador de receitas terminadas (para desbloquear Pirão)
} EstadoJogo;

// estados de tela do jogo (qual tela esta sendo exibida)
typedef enum {
    TELA_MENU,
    TELA_RECEITAS,
    TELA_INGREDIENTES,
    TELA_CATCHER,
    TELA_ORDENACAO,       // insertion sort animado dos ingredientes coletados
    TELA_PILHA,           // execucao da receita (cozinhar)
    TELA_FEEDBACK,
    TELA_RESULTADO,
    TELA_CREDITOS
} EstadoTela;

// estado global definido em jogo.c
extern EstadoJogo estado;
extern Receita *receitas_disponiveis;
extern Receita *receita_selecionada;  // receita escolhida pelo jogador
extern EstadoTela tela_atual;

// funcoes publicas
void   resetar_partida(void);
void   iniciar_jogo(void);
int    verificar_vitoria(void);
void   _avancar_progresso(void);
void   integrar_modulos(void);

#endif
