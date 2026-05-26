#ifndef TELAS_H
#define TELAS_H

#include "../api/jurados.h"
#include "raylib.h"
#include "../dados/receitas.h"
#include "../jogo.h"

// tela menu principal
void tela_menu(void);

// tela de lista de receitas
void tela_receitas(Receita *lista);

// tela de ingredientes de uma receita
void tela_ingredientes(Receita *receita);

// desenha a tela de introducao final (indice 0..2)
void tela_final_intro_draw(int indice);

// tela de cozinhar (pilha de passos)
void tela_pilha(const char *passo_atual, int num_passo, int total_passos, double tempo);

// tela de feedback apos cada passo (1 = acertou, 0 = errou)
void tela_feedback(int acertou);

// tela de resultado final (1 = venceu, 0 = perdeu)
void tela_resultado(int venceu, ResultadoJurados *j);

// tela de creditos
void tela_creditos(void);

// carrega texturas das telas
void telas_carregar_sprites(void);

// descarrega texturas das telas
void telas_limpar(void);

#endif