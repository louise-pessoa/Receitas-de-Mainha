#include <stdio.h>
#include <string.h>
#include "raylib.h"
#include "ordenacao.h"
#include "../jogo.h"
#include "../ui/fonte.h"

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
// AUXILIAR: CONVERTE NOME EM CAMINHO DE SPRITE
// ==========================================
static void construir_caminho_sprite(const char *nome, char *caminho, int tam_caminho) {
    // Mapeamentos explicitos fornecidos pelo usuario
    struct Map { const char *nome; const char *file; } map[] = {
        {"Tapioca granulada", "massa_tapioca.png"},
        {"Carne de sol", "carne.png"},
        {"Farinha de mandioca", "farinha.png"},
        {"Farinha de trigo", "farinha.png"},
        {"Queijo coalho", "queijo.png"},
        {"Ovos", "ovo.png"},
    };
    for (size_t i = 0; i < sizeof(map)/sizeof(map[0]); i++) {
        if (strcasecmp(nome, map[i].nome) == 0) {
            snprintf(caminho, tam_caminho, "sprites/coleta/%s", map[i].file);
            return;
        }
    }

    // Fallback: converte para lowercase e substitui espacos por underscore
    snprintf(caminho, tam_caminho, "sprites/coleta/");
    int len = strlen(caminho);
    for (int i = 0; i < (int)strlen(nome) && len < tam_caminho - 5; i++) {
        char c = nome[i];
        if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
        if (c == ' ') c = '_';
        caminho[len++] = c;
    }
    snprintf(caminho + len, tam_caminho - len, ".png");
}

// ==========================================
// INICIALIZACAO
// ==========================================

// inicia com os ingredientes na ordem de coleta do catcher.
// ordem_correta = indice do ingrediente na receita (posicao final desejada).
void ordenacao_iniciar(Receita *receita, int *ordem_coleta, int n_coletados) {
    // descarrega texturas anteriores se existirem
    if (ordenacao.bg_carregado) {
        UnloadTexture(ordenacao.bg_ordenar);
        ordenacao.bg_carregado = 0;
    }
    for (int ii = 0; ii < MAX_ORDENACAO; ii++) {
        if (ordenacao.itens[ii].sprite_carregado) {
            UnloadTexture(ordenacao.itens[ii].sprite);
            ordenacao.itens[ii].sprite_carregado = 0;
        }
    }

    memset(&ordenacao, 0, sizeof(ordenacao));
    ordenacao.receita = receita;

    if (receita == NULL || ordem_coleta == NULL || n_coletados == 0) return;

    int total = n_coletados < MAX_ORDENACAO ? n_coletados : MAX_ORDENACAO;
    for (int i = 0; i < total; i++) {
        int idx = ordem_coleta[i];
        strncpy(ordenacao.itens[i].nome, receita->ingredientes[idx],
                sizeof(ordenacao.itens[i].nome) - 1);
        ordenacao.itens[i].nome[sizeof(ordenacao.itens[i].nome) - 1] = '\0';
        ordenacao.itens[i].ordem_correta = idx;
    }
    ordenacao.n = total;

    ordenacao.i = 1;
    ordenacao.j = 1;
    ordenacao.passo = 0;
    ordenacao.concluido = (ordenacao.n <= 1);
    ordenacao.timer = 0.0f;
    ordenacao.intervalo = 0.55f;

    // carrega textura de fundo
    ordenacao.bg_ordenar = LoadTexture("sprites/fundos/bg_ordenar.png");
    if (ordenacao.bg_ordenar.id != 0) {
        SetTextureFilter(ordenacao.bg_ordenar, TEXTURE_FILTER_BILINEAR);
        ordenacao.bg_carregado = 1;
    }

    // carrega sprites dos itens
    for (int i = 0; i < ordenacao.n; i++) {
        char path[256] = {0};
        construir_caminho_sprite(ordenacao.itens[i].nome, path, sizeof(path));
        ordenacao.itens[i].sprite = LoadTexture(path);
        if (ordenacao.itens[i].sprite.id != 0) {
            SetTextureFilter(ordenacao.itens[i].sprite, TEXTURE_FILTER_BILINEAR);
            ordenacao.itens[i].sprite_carregado = 1;
        } else {
            // tentar remover espacos e substituir por underline
            for (int k = 0; k < (int)strlen(path); k++) if (path[k] == ' ') path[k] = '_';
            ordenacao.itens[i].sprite = LoadTexture(path);
            if (ordenacao.itens[i].sprite.id != 0) ordenacao.itens[i].sprite_carregado = 1;
        }
    }

    printf("[ORDENACAO] Iniciada com %d itens (ordem de coleta)\n", ordenacao.n);
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

    // desenha fundo
    if (ordenacao.bg_carregado) {
        DrawTexturePro(ordenacao.bg_ordenar,
                       (Rectangle){0, 0, ordenacao.bg_ordenar.width, ordenacao.bg_ordenar.height},
                       (Rectangle){0, 0, 800, 600},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        ClearBackground(COR_FUNDO_ORD);
    }

    // cabecalho
    DrawRectangle(0, 0, 800, 60, COR_AZUL_ORD);
    DrawRectangle(0, 55, 800, 5, COR_AMARELO_ORD);
    if (!ordenacao.concluido) {
        const char *hdr = "ORGANIZANDO...";
        int htw = medir_txt(hdr, 28);
        txt(hdr, (800 - htw) / 2, 16, 28, WHITE);
    } else {
        const char *hdr_done = "Tudo organizado pra receita!";
        int htw = medir_txt(hdr_done, 24);
        txt(hdr_done, (800 - htw) / 2, 16, 24, WHITE);
    }

    // subtitulo pós-ordenacao (apenas quando concluido)
    // sem subtitulo pós-ordenacao por request

    // cartas dos itens
    int total = ordenacao.n;
    int carta_w = 100;
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

        // sombra e corpo (cores mínimas)
        DrawRectangleRounded((Rectangle){x + 4, y + 4, carta_w, carta_h},
                             0.2f, 8, (Color){0, 0, 0, 50});
        int correto = (ordenacao.itens[k].ordem_correta == k);
        Color cor_corpo = correto && ordenacao.concluido ? COR_VERDE_ORD :
                          correto ? (Color){220, 240, 220, 255} :
                          (Color){250, 250, 240, 255};
        DrawRectangleRounded((Rectangle){x, y, carta_w, carta_h},
                             0.2f, 8, cor_corpo);
        // borda única, mais sutil — maior quando destacado
        if (destaque) {
            DrawRectangleRoundedLines((Rectangle){x - 2, y - 2,
                                                  carta_w + 4, carta_h + 4},
                                      0.2f, 8, 3.0f, borda);
        } else {
            DrawRectangleRoundedLines((Rectangle){x, y, carta_w, carta_h},
                                      0.2f, 8, 1.4f, COR_TEXTO_ORD);
        }

        // numero da posicao
        txt(TextFormat("%d", k + 1), x + 8, y + 6, 18, GRAY);

        // sprite do item (se carregado) ou fallback para nome
        if (ordenacao.itens[k].sprite_carregado) {
            Texture2D t = ordenacao.itens[k].sprite;
            float pad_x = 8;
            float pad_y = 30;
            float avail_w = carta_w - pad_x*2;
            float avail_h = carta_h - pad_y - 8;
            float scale_w = avail_w / t.width;
            float scale_h = avail_h / t.height;
            float scale = scale_w < scale_h ? scale_w : scale_h;
            if (scale > 1.25f) scale = 1.25f;
            float tx = x + (carta_w - t.width*scale) / 2.0f;
            float ty = y + pad_y;
            DrawTextureEx(t, (Vector2){tx, ty}, 0.0f, scale, WHITE);
        } else {
            const char *nome = ordenacao.itens[k].nome;
            int tw = medir_txt(nome, 16);
            if (tw <= carta_w - 16) {
                txt(nome, x + (carta_w - tw) / 2, y + 36, 16, COR_TEXTO_ORD);
            } else {
                // texto longo: diminui
                int tam = 12;
                int tw2 = medir_txt(nome, tam);
                txt(nome, x + (carta_w - tw2) / 2, y + 40, tam, COR_TEXTO_ORD);
            }
        }
    }

    if (!ordenacao.concluido) {
        // durante a ordenacao, removemos legenda/indicadores para focar apenas
        // nos cartões que se movem. Nada extra é desenhado aqui.
    } else {
        DrawRectangleRounded((Rectangle){80, 380, 640, 110}, 0.3f, 8,
                             COR_VERDE_ORD);
        txt("ORDENADO!", 320, 395, 32, WHITE);
        txt("Agora voce ja sabe a ordem de uso dos ingredientes.",
                 130, 440, 18, WHITE);
        txt("[ENTER] Comecar a cozinhar", 250, 465, 20, COR_AMARELO_ORD);
    }

    // rodape
    DrawRectangle(0, 560, 800, 40, (Color){30, 60, 140, 255});
    txt(ordenacao.concluido ?
             "[ENTER] Cozinhar  |  [ESC] Menu" :
             "Aguarde a ordenacao terminar...",
             220, 570, 20, COR_AMARELO_ORD);
}

int ordenacao_terminou(void) {
    return ordenacao.concluido;
}

// libera texturas carregadas pela ordenacao
void ordenacao_limpar(void) {
    if (ordenacao.bg_carregado) {
        UnloadTexture(ordenacao.bg_ordenar);
        ordenacao.bg_carregado = 0;
    }
    for (int i = 0; i < ordenacao.n; i++) {
        if (ordenacao.itens[i].sprite_carregado) {
            UnloadTexture(ordenacao.itens[i].sprite);
            ordenacao.itens[i].sprite_carregado = 0;
        }
    }
    ordenacao.n = 0;
}
