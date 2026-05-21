#include <stdio.h>
#include <string.h>
#include "raylib.h"
#include "ordenacao.h"
#include "../jogo.h"

// ==========================================
// CORES
// ==========================================
#define COR_FUNDO_ORD  (Color){255, 243, 210, 255}
#define COR_AZUL_ORD   (Color){30,  100, 200, 255}
#define COR_VERDE_ORD  (Color){50,  160, 80,  255}
#define COR_VERMELHO_ORD (Color){220, 50, 50, 255}
#define COR_AMARELO_ORD (Color){255, 200, 0, 255}
#define COR_LARANJA_ORD (Color){255, 140, 0, 255}
#define COR_TEXTO_ORD  (Color){60,  30,  10, 255}

EstadoOrdenacao ordenacao;

// ==========================================
// INICIALIZACAO
// ==========================================
static void embaralhar(void) {
    // shuffle simples Fisher-Yates
    for (int i = ordenacao.n - 1; i > 0; i--) {
        int k = GetRandomValue(0, i);
        ItemOrdenacao tmp = ordenacao.itens[i];
        ordenacao.itens[i] = ordenacao.itens[k];
        ordenacao.itens[k] = tmp;
    }
}

void ordenacao_iniciar(Receita *receita) {
    memset(&ordenacao, 0, sizeof(ordenacao));
    ordenacao.receita = receita;

    if (receita == NULL) return;

    // carrega ingredientes na ordem original (que e a ordem de uso)
    Ingrediente *ing = receita->ingredientes;
    int idx = 0;
    while (ing != NULL && idx < MAX_ORDENACAO) {
        strncpy(ordenacao.itens[idx].nome, ing->nome,
                sizeof(ordenacao.itens[idx].nome) - 1);
        ordenacao.itens[idx].nome[sizeof(ordenacao.itens[idx].nome) - 1] = '\0';
        ordenacao.itens[idx].ordem_correta = idx;
        idx++;
        ing = ing->prox;
    }
    ordenacao.n = idx;

    embaralhar();

    ordenacao.i = 1;
    ordenacao.j = 1;
    ordenacao.passo = 0;
    ordenacao.concluido = (ordenacao.n <= 1);
    ordenacao.timer = 0.0f;
    ordenacao.intervalo = 0.55f;

    printf("[ORDENACAO] Iniciada com %d itens (embaralhados)\n", ordenacao.n);
}

// ==========================================
// PASSO DO INSERTION SORT
// ==========================================
// faz UMA acao por chamada: ou avanca i, ou troca itens[j-1] e itens[j]
static void avancar_um_passo(void) {
    if (ordenacao.concluido) return;

    if (ordenacao.i >= ordenacao.n) {
        ordenacao.concluido = 1;
        return;
    }

    if (ordenacao.j > 0 &&
        ordenacao.itens[ordenacao.j - 1].ordem_correta >
        ordenacao.itens[ordenacao.j].ordem_correta) {
        // troca
        ItemOrdenacao tmp = ordenacao.itens[ordenacao.j - 1];
        ordenacao.itens[ordenacao.j - 1] = ordenacao.itens[ordenacao.j];
        ordenacao.itens[ordenacao.j] = tmp;
        ordenacao.j--;
    } else {
        // proxima iteracao externa
        ordenacao.i++;
        ordenacao.j = ordenacao.i;
        if (ordenacao.i >= ordenacao.n) {
            ordenacao.concluido = 1;
        }
    }
}

void ordenacao_atualizar(void) {
    if (ordenacao.concluido) return;
    ordenacao.timer += GetFrameTime();
    if (ordenacao.timer >= ordenacao.intervalo) {
        ordenacao.timer = 0.0f;
        avancar_um_passo();
    }
}

// ==========================================
// DESENHO
// ==========================================
void tela_ordenacao(void) {
    ordenacao_atualizar();

    ClearBackground(COR_FUNDO_ORD);

    // cabecalho
    DrawRectangle(0, 0, 800, 60, COR_AZUL_ORD);
    DrawRectangle(0, 55, 800, 5, COR_AMARELO_ORD);
    DrawText("ORGANIZANDO INGREDIENTES (Insertion Sort)",
             80, 16, 24, WHITE);

    // subtitulo
    DrawText("Voce coletou os ingredientes fora de ordem!",
             140, 80, 22, COR_TEXTO_ORD);
    DrawText("Vamos coloca-los na ordem certa de uso da receita:",
             100, 110, 18, COR_TEXTO_ORD);

    // cartas dos itens
    int total = ordenacao.n;
    int carta_w = 130;
    int carta_h = 80;
    int espaco = 10;
    int largura_total = total * (carta_w + espaco) - espaco;
    int x0 = (800 - largura_total) / 2;
    int y0 = 230;

    for (int k = 0; k < total; k++) {
        int x = x0 + k * (carta_w + espaco);
        int y = y0;

        // destacar i, j-1, j durante a animacao
        Color borda = COR_TEXTO_ORD;
        int destaque = 0;
        if (!ordenacao.concluido) {
            if (k == ordenacao.j)     { borda = COR_VERMELHO_ORD; destaque = 1; }
            if (k == ordenacao.j - 1) { borda = COR_LARANJA_ORD;  destaque = 1; }
        }

        // sombra
        DrawRectangleRounded((Rectangle){x + 4, y + 4, carta_w, carta_h},
                             0.2f, 8, (Color){0, 0, 0, 80});
        // corpo (verde se ja em posicao correta)
        int correto = (ordenacao.itens[k].ordem_correta == k);
        Color cor_corpo = correto && ordenacao.concluido ? COR_VERDE_ORD :
                          correto ? (Color){200, 240, 200, 255} :
                          (Color){255, 250, 220, 255};
        DrawRectangleRounded((Rectangle){x, y, carta_w, carta_h},
                             0.2f, 8, cor_corpo);
        // borda destacada
        if (destaque) {
            DrawRectangleRoundedLinesEx((Rectangle){x - 2, y - 2,
                                                  carta_w + 4, carta_h + 4},
                                      0.2f, 8, 2.0f, borda);
            DrawRectangleRoundedLinesEx((Rectangle){x - 1, y - 1,
                                                  carta_w + 2, carta_h + 2},
                                      0.2f, 8, 2.0f, borda);
        } else {
            DrawRectangleRoundedLinesEx((Rectangle){x, y, carta_w, carta_h},
                                      0.2f, 8, 2.0f, COR_TEXTO_ORD);
        }

        // numero da posicao
        DrawText(TextFormat("%d", k + 1), x + 8, y + 6, 18, GRAY);

        // nome (com quebra simples)
        const char *nome = ordenacao.itens[k].nome;
        int tw = MeasureText(nome, 16);
        if (tw <= carta_w - 16) {
            DrawText(nome, x + (carta_w - tw) / 2, y + 36, 16, COR_TEXTO_ORD);
        } else {
            // texto longo: diminui
            int tam = 12;
            int tw2 = MeasureText(nome, tam);
            DrawText(nome, x + (carta_w - tw2) / 2, y + 40, tam, COR_TEXTO_ORD);
        }
    }

    // legenda
    if (!ordenacao.concluido) {
        DrawText("Comparando vizinhos e trocando se estiverem fora de ordem...",
                 80, 370, 18, COR_TEXTO_ORD);
        DrawText(TextFormat("i = %d   j = %d", ordenacao.i, ordenacao.j),
                 320, 400, 18, COR_AZUL_ORD);

        DrawRectangle(80, 440, 30, 20, COR_VERMELHO_ORD);
        DrawText("j (atual)", 120, 440, 18, COR_TEXTO_ORD);
        DrawRectangle(280, 440, 30, 20, COR_LARANJA_ORD);
        DrawText("j-1 (vizinho)", 320, 440, 18, COR_TEXTO_ORD);
    } else {
        DrawRectangleRounded((Rectangle){80, 380, 640, 110}, 0.3f, 8,
                             COR_VERDE_ORD);
        DrawText("ORDENADO!", 320, 395, 32, WHITE);
        DrawText("Agora voce ja sabe a ordem de uso dos ingredientes.",
                 130, 440, 18, WHITE);
        DrawText("[ENTER] Comecar a cozinhar", 250, 465, 20, COR_AMARELO_ORD);
    }

    // rodape
    DrawRectangle(0, 560, 800, 40, (Color){30, 60, 140, 255});
    DrawText(ordenacao.concluido ?
             "[ENTER] Cozinhar  |  [ESC] Menu" :
             "Aguarde a ordenacao terminar...",
             220, 570, 20, COR_AMARELO_ORD);
}

int ordenacao_terminou(void) {
    return ordenacao.concluido;
}
