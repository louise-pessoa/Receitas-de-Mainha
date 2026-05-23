#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "raylib.h"
#include "telas.h"
#include "../jogo.h"
#include "../dados/receitas.h"
#include "../dados/pilha.h"

// ==========================================
// CORES DO PROJETO
// ==========================================
#define COR_FUNDO (Color){255, 243, 210, 255}  // amarelo claro
#define COR_AZUL (Color){30, 100, 200, 255}    // azul vibrante
#define COR_VERMELHO (Color){220, 50, 50, 255} // vermelho
#define COR_VERDE (Color){50, 160, 80, 255}    // verde
#define COR_AMARELO (Color){255, 200, 0, 255}  // amarelo botao
#define COR_LARANJA (Color){255, 140, 0, 255}  // laranja
#define COR_TEXTO (Color){60, 30, 10, 255}     // marrom escuro
#define COR_BRANCO WHITE
#define COR_BARRA_FUNDO (Color){30, 60, 140, 255} // azul escuro barra

// ==========================================
// ESTADO GLOBAL (texturas)
// ==========================================
typedef struct {
    Texture2D bg_jurados;
    int bg_carregado;
} EstadoTelas;

static EstadoTelas telas_estado = {0};

// cache simples de sprites (nome -> texture)
#define TELAS_SPRITE_CACHE 64
typedef struct {
    char nome[64];
    Texture2D tex;
    int carregado;
} SpriteEntry;

static SpriteEntry sprite_cache[TELAS_SPRITE_CACHE];

static void construir_caminho_sprite_telas(const char *nome, char *caminho, int tam) {
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
            snprintf(caminho, tam, "sprites/%s", map[i].file);
            return;
        }
    }

    // Fallback: lowercase + underscores
    snprintf(caminho, tam, "sprites/");
    int len = strlen(caminho);
    for (int i = 0; i < (int)strlen(nome) && len < tam - 5; i++) {
        char c = nome[i];
        if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
        if (c == ' ') c = '_';
        caminho[len++] = c;
    }
    snprintf(caminho + len, tam - len, ".png");
}

// retorna Texture2D; se nao encontrado, tex.id == 0
static Texture2D obter_sprite_telas(const char *nome) {
    if (nome == NULL) return (Texture2D){0};
    // procurar no cache
    for (int i = 0; i < TELAS_SPRITE_CACHE; i++) {
        if (sprite_cache[i].carregado && strcmp(sprite_cache[i].nome, nome) == 0) {
            return sprite_cache[i].tex;
        }
    }
    // encontrar posicao livre
    int idx = -1;
    for (int i = 0; i < TELAS_SPRITE_CACHE; i++) if (!sprite_cache[i].carregado) { idx = i; break; }
    if (idx == -1) idx = 0; // sobrescreve o primeiro se cheio

    char path[256] = {0};
    construir_caminho_sprite_telas(nome, path, sizeof(path));
    Texture2D t = LoadTexture(path);
    if (t.id == 0) {
        // tentar sem underline
        for (int k = 0; k < (int)strlen(path); k++) if (path[k] == '_') path[k] = ' ';
        t = LoadTexture(path);
    }
    if (t.id != 0) {
        SetTextureFilter(t, TEXTURE_FILTER_BILINEAR);
        strncpy(sprite_cache[idx].nome, nome, sizeof(sprite_cache[idx].nome)-1);
        sprite_cache[idx].tex = t;
        sprite_cache[idx].carregado = 1;
        return t;
    }
    return (Texture2D){0};
}

// carrega texturas das telas
void telas_carregar_sprites(void) {
    telas_estado.bg_jurados = LoadTexture("sprites/jurados_nota.png");
    if (telas_estado.bg_jurados.id != 0) {
        SetTextureFilter(telas_estado.bg_jurados, TEXTURE_FILTER_BILINEAR);
        telas_estado.bg_carregado = 1;
    }
}

// descarrega texturas das telas
void telas_limpar(void) {
    if (telas_estado.bg_carregado) {
        UnloadTexture(telas_estado.bg_jurados);
        telas_estado.bg_carregado = 0;
    }

    // limpar cache de sprites
    for (int i = 0; i < TELAS_SPRITE_CACHE; i++) {
        if (sprite_cache[i].carregado) {
            UnloadTexture(sprite_cache[i].tex);
            sprite_cache[i].carregado = 0;
        }
    }
}

// ==========================================
// UTILITARIOS
// ==========================================

// desenha botao colorido com texto centralizado
static void desenhar_botao(int x, int y, int w, int h, Color cor, const char *texto) {
    DrawRectangleRounded((Rectangle){x, y, w, h}, 0.4f, 8, cor);
    DrawRectangleRoundedLines((Rectangle){x, y, w, h}, 0.4f, 8, WHITE);
    int tam = 22;
    int tw = MeasureText(texto, tam);
    DrawText(texto, x + (w - tw) / 2, y + (h - tam) / 2, tam, WHITE);
}

// desenha barra de progresso
static void desenhar_barra_progresso(int x, int y, int w, int h, float progresso) {
    DrawRectangleRounded((Rectangle){x, y, w, h}, 0.5f, 8, LIGHTGRAY);
    int preenchido = (int)(w * progresso);
    if (preenchido > 0) {
        Color cor = progresso > 0.7f ? COR_VERDE : progresso > 0.4f ? COR_LARANJA : COR_VERMELHO;
        DrawRectangleRounded((Rectangle){x, y, preenchido, h}, 0.5f, 8, cor);
    }
    // estrela no meio
    DrawText("*", x + w/2 - 6, y - 2, 24, COR_AMARELO);
}

// desenha cabecalho com titulo do jogo
static void desenhar_cabecalho(void) {
    // barra azul no topo
    DrawRectangle(0, 0, 800, 60, COR_AZUL);
    DrawRectangle(0, 55, 800, 5, COR_AMARELO);
    // titulo
    DrawText("COMIDA DE MAINHA", 220, 15, 32, WHITE);
}

// desenha rodape azul
static void desenhar_rodape(const char *texto) {
    DrawRectangle(0, 560, 800, 40, COR_BARRA_FUNDO);
    int tw = MeasureText(texto, 20);
    DrawText(texto, (800 - tw) / 2, 570, 20, COR_AMARELO);
}

// ==========================================
// TELA: MENU PRINCIPAL
// ==========================================
void tela_menu(void) {
    // fundo cozinha (cor quente)
    ClearBackground(COR_FUNDO);

    // cabecalho
    DrawRectangle(0, 0, 800, 70, COR_AZUL);
    DrawRectangle(0, 65, 800, 6, COR_AMARELO);

    // titulo principal
    DrawText("COMIDA", 260, 80, 64, COR_VERMELHO);
    DrawText("DE", 340, 140, 48, COR_AZUL);
    DrawText("MAINHA", 240, 185, 64, COR_AZUL);

    // estrelinhas decorativas
    DrawText("*", 210, 90, 28, COR_AMARELO);
    DrawText("*", 570, 90, 28, COR_VERDE);
    DrawText("*", 190, 200, 20, COR_VERMELHO);
    DrawText("*", 590, 200, 20, COR_LARANJA);

    // botoes principais
    desenhar_botao(200, 290, 180, 55, COR_VERMELHO, "Receitas");
    desenhar_botao(430, 290, 160, 55, COR_AZUL,     "Colecao");

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();
        if (CheckCollisionPointRec(m, (Rectangle){200, 290, 180, 55})) tela_atual = TELA_RECEITAS;
        if (CheckCollisionPointRec(m, (Rectangle){430, 290, 160, 55})) tela_atual = TELA_CREDITOS;
    }

    // instrucao
    DrawText("[2] Comecar (escolher receita)    [8] Creditos    [F11] Tela cheia",
             80, 370, 17, COR_TEXTO);

    // pontuacao atual
    DrawText(TextFormat("Pontuacao: %d", estado.pontuacao), 50, 420, 22, COR_TEXTO);

    // rodape
    DrawRectangle(0, 560, 800, 40, COR_BARRA_FUNDO);
    int tw = MeasureText("* Toque para comecar *", 22);
    DrawText("* Toque para comecar *", (800 - tw)/2, 568, 22, COR_AMARELO);
}

// ==========================================
// TELA: LISTA DE RECEITAS
// ==========================================
void tela_receitas(Receita *lista) {
    ClearBackground(COR_FUNDO);
    desenhar_cabecalho();

    // painel de madeira (simulado)
    DrawRectangleRounded((Rectangle){30, 70, 460, 470}, 0.05f, 8, (Color){210, 170, 100, 255});
    DrawRectangleRounded((Rectangle){38, 78, 444, 454}, 0.05f, 8, (Color){255, 248, 220, 255});

    DrawText("RECEITAS DISPONIVEIS", 60, 90, 22, COR_TEXTO);

    if (lista == NULL) {
        DrawText("Nenhuma receita cadastrada.", 80, 200, 20, GRAY);
    } else {
        Color cores[] = {COR_VERMELHO, COR_VERDE, COR_AZUL, COR_LARANJA, COR_VERMELHO, COR_VERDE, COR_AZUL, COR_LARANJA};
        Vector2 mouse = GetMousePosition();
        Receita *aux = lista;
        int i = 0, y = 120;
        while (aux != NULL && i < 8) {
            Rectangle card = {50, y, 420, 48};
            int eh_sel   = (receita_selecionada == aux);
            int hovered  = CheckCollisionPointRec(mouse, card);

            // borda amarela se selecionado
            if (eh_sel)
                DrawRectangleRounded((Rectangle){46, y - 4, 428, 56}, 0.3f, 8, COR_AMARELO);

            // card — um pouco mais escuro se hover
            Color cor = cores[i];
            if (hovered && !eh_sel) {
                cor.r = (unsigned char)(cor.r > 30 ? cor.r - 30 : 0);
                cor.g = (unsigned char)(cor.g > 30 ? cor.g - 30 : 0);
                cor.b = (unsigned char)(cor.b > 30 ? cor.b - 30 : 0);
            }
            DrawRectangleRounded(card, 0.3f, 8, cor);
            DrawText(TextFormat("[%d] %s", i + 1, aux->nome), 70, y + 8, 20, WHITE);
            DrawText(TextFormat("Dif: %d", aux->dificuldade), 370, y + 8, 18, WHITE);
            if (hovered)
                DrawText("Clique para selecionar", 70, y + 28, 13, COR_AMARELO);
            else {
                int np = 0;
                No *tp = aux->passos;
                while (tp) { np++; tp = tp->prox; }
                DrawText(TextFormat("%d passos", np), 70, y + 28, 14, COR_AMARELO);
            }

            // clique: seleciona e avanca direto para ingredientes
            if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                receita_selecionada = aux;
                tela_atual = TELA_INGREDIENTES;
            }

            aux = aux->prox;
            y += 56;
            i++;
        }
    }

    // lado direito - vovo
    DrawRectangleRounded((Rectangle){510, 70, 260, 470}, 0.05f, 8, (Color){255, 235, 180, 255});
    DrawText("Escolha uma", 530, 100, 20, COR_TEXTO);
    DrawText("receita para", 530, 125, 20, COR_TEXTO);
    DrawText("cozinhar!", 530, 150, 20, COR_TEXTO);

    if (receita_selecionada != NULL) {
        DrawRectangleRounded((Rectangle){520, 200, 240, 110}, 0.2f, 8, COR_VERDE);
        DrawText("Selecionada:", 535, 210, 16, WHITE);
        // tentar desenhar sprite do nome da receita
        Texture2D rtex = obter_sprite_telas(receita_selecionada->nome);
        if (rtex.id != 0) {
            float rw = 200;
            float rh = 70;
            float scale = fminf(rw / rtex.width, rh / rtex.height);
            float tx = 520 + (240 - rtex.width*scale)/2.0f;
            float ty = 200 + (110 - rtex.height*scale)/2.0f;
            DrawTextureEx(rtex, (Vector2){tx, ty}, 0.0f, scale, WHITE);
        } else {
            DrawText(receita_selecionada->nome, 535, 235, 18, WHITE);
        }
        DrawText(TextFormat("Dif: %d | %d min",
                            receita_selecionada->dificuldade,
                            receita_selecionada->tempo),
                 535, 262, 14, WHITE);
        DrawText("[ENTER] Continuar", 535, 285, 14, COR_AMARELO);
    } else {
        DrawText("(nenhuma selecionada)", 530, 215, 16, GRAY);
    }

    // instrucoes
    DrawText("[Setas] Navegar", 530, 380, 16, COR_TEXTO);
    DrawText("[1-3] Selecionar", 530, 400, 16, COR_TEXTO);
    DrawText("[ENTER] Continuar", 530, 420, 16, COR_TEXTO);
    DrawText("[1] Menu", 530, 440, 16, COR_TEXTO);

    desenhar_rodape("[Setas/1-3] Selecionar  |  [ENTER] ou Clique para continuar");
}

// ==========================================
// TELA: INGREDIENTES DA RECEITA
// ==========================================
void tela_ingredientes(Receita *receita) {
    ClearBackground(COR_FUNDO);
    desenhar_cabecalho();

    if (receita == NULL) {
        DrawText("Nenhuma receita selecionada.", 220, 280, 24, COR_VERMELHO);
        DrawText("Volte ao menu de receitas e escolha uma.",
                 170, 320, 20, COR_TEXTO);
        desenhar_rodape("[2] Voltar para receitas");
        return;
    }

    // nome da receita
    DrawRectangleRounded((Rectangle){50, 75, 700, 50}, 0.3f, 8, COR_LARANJA);
    // tentar desenhar sprite do nome da receita
    Texture2D rtex2 = obter_sprite_telas(receita->nome);
    if (rtex2.id != 0) {
        float rw = 600;
        float rh = 40;
        float scale = fminf(rw / rtex2.width, rh / rtex2.height);
        float tx = (800 - rtex2.width*scale)/2.0f;
        float ty = 85 - rtex2.height*scale/2.0f;
        DrawTextureEx(rtex2, (Vector2){tx, ty}, 0.0f, scale, WHITE);
    } else {
        int tw = MeasureText(receita->nome, 28);
        DrawText(receita->nome, (800 - tw)/2, 85, 28, WHITE);
    }

    // balao de instrucao
    DrawRectangleRounded((Rectangle){100, 140, 600, 110}, 0.1f, 8, WHITE);
    DrawRectangleRoundedLines((Rectangle){100, 140, 600, 110}, 0.1f, 8, COR_AZUL);
    DrawText("Memorize os ingredientes abaixo!", 120, 155, 20, COR_VERDE);
    DrawText("Eles vao cair do ceu em 25 segundos — use [<-][->] para", 120, 182, 17, COR_TEXTO);
    DrawText("mover a cesta e coletar so os certos. Errados custam pontos!", 120, 207, 17, COR_VERMELHO);

    // lista de ingredientes
    DrawText("Ingredientes:", 50, 270, 22, COR_TEXTO);
    if (receita->n_ingredientes == 0) {
        DrawText("(nenhum ingrediente cadastrado)", 70, 305, 18, GRAY);
    } else {
        Color cores[] = {COR_VERMELHO, COR_VERDE, COR_AZUL, COR_LARANJA};
        int y = 305;
        for (int i = 0; i < receita->n_ingredientes && i < 8; i++) {
            DrawCircle(65, y + 10, 14, cores[i % 4]);
            DrawText(TextFormat("%d", i + 1), 60, y + 3, 16, WHITE);
            // desenhar sprite do ingrediente se existir
            Texture2D itex = obter_sprite_telas(receita->ingredientes[i]);
            if (itex.id != 0) {
                float sz = 28;
                float scale = sz / itex.width;
                float tx = 85;
                float ty = y - 6;
                DrawTextureEx(itex, (Vector2){tx, ty}, 0.0f, scale, WHITE);
            } else {
                DrawText(receita->ingredientes[i], 85, y, 20, COR_TEXTO);
            }
            y += 30;
        }
    }

    // dica
    DrawRectangleRounded((Rectangle){50, 480, 700, 60}, 0.3f, 8, COR_AMARELO);
    DrawText("Aperte [ENTER] para entrar no minigame da cesta!",
             80, 498, 22, COR_TEXTO);

    desenhar_rodape("[ENTER] Ir para o catcher  |  [2] Voltar para receitas");
}

// ==========================================
// TELA: COZINHANDO (PILHA DE PASSOS)
// ==========================================
void tela_pilha(const char *passo_atual, int num_passo, int total_passos, double tempo) {
    ClearBackground((Color){255, 245, 200, 255});

    // fundo da cozinha (tiles simulados)
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            DrawRectangleLines(i*100, j*80, 100, 80, (Color){200, 220, 255, 100});
        }
    }

    // instrucao no topo
    DrawRectangleRounded((Rectangle){100, 20, 600, 60}, 0.3f, 8, (Color){255, 245, 200, 240});
    DrawRectangleRoundedLines((Rectangle){100, 20, 600, 60}, 0.3f, 8, COR_LARANJA);
    DrawText("*", 115, 35, 22, COR_AMARELO);
    int tw = MeasureText(passo_atual, 22);
    DrawText(passo_atual, (800 - tw)/2, 35, 22, COR_TEXTO);
    DrawText("*", 665, 35, 22, COR_AMARELO);

    // relogio / timer
    DrawCircle(100, 200, 55, WHITE);
    DrawCircleLines(100, 200, 55, COR_VERMELHO);
    DrawCircleLines(100, 200, 50, (Color){200,200,200,255});
    DrawText(TextFormat("%.0fs", tempo), 75, 185, 26, COR_VERMELHO);
    DrawText("TEMPO", 68, 215, 16, GRAY);

    // balao da vovo com dica
    DrawRectangleRounded((Rectangle){30, 320, 220, 80}, 0.2f, 8, (Color){255,220,230,255});
    DrawRectangleRoundedLines((Rectangle){30, 320, 220, 80}, 0.2f, 8, (Color){255,150,180,255});
    DrawText("Siga a ordem!", 45, 335, 18, COR_TEXTO);
    DrawText("Voce consegue!", 45, 358, 17, COR_TEXTO);

    // barra de progresso na base
    DrawRectangle(0, 470, 800, 10, LIGHTGRAY);
    float prog = total_passos > 0 ? (float)num_passo / total_passos : 0;
    desenhar_barra_progresso(30, 460, 740, 20, prog);

    // numero do passo
    DrawText(TextFormat("Passo %d de %d", num_passo, total_passos), 310, 430, 20, COR_TEXTO);

    // pontuacao
    DrawText(TextFormat("Pontos: %d", estado.pontuacao), 600, 200, 22, COR_TEXTO);

    // instrucao
    DrawText("[ENTER] Confirmar passo  [ESC] Desistir", 200, 495, 18, COR_TEXTO);
}

// ==========================================
// TELA: FEEDBACK (ACERTO / ERRO)
// ==========================================
void tela_feedback(int acertou) {
    if (acertou) {
        ClearBackground((Color){200, 255, 200, 255});
        DrawText("ACERTOU!", 250, 180, 64, COR_VERDE);
        DrawText(":)", 360, 260, 48, COR_VERDE);
        DrawText(TextFormat("Passo %d/%d concluido!", estado.passos_acertados, estado.passos_total), 200, 340, 24, COR_TEXTO);
    } else {
        ClearBackground((Color){255, 210, 210, 255});
        DrawText("ERROU!", 270, 180, 64, COR_VERMELHO);
        DrawText(":(", 365, 260, 48, COR_VERMELHO);
        DrawText(TextFormat("-20 pontos | Pontuacao: %d", estado.pontuacao), 180, 340, 24, COR_TEXTO);
    }
    DrawText("Pontuacao atual:", 280, 400, 22, COR_TEXTO);
    DrawText(TextFormat("%d", estado.pontuacao), 370, 430, 36, acertou ? COR_VERDE : COR_VERMELHO);
    DrawText("[ENTER] Continuar", 300, 510, 20, GRAY);
}

// ==========================================
// TELA: RESULTADO FINAL
// ==========================================
void tela_resultado(int venceu, ResultadoJurados *j)
{
    // desenha fundo
    if (telas_estado.bg_carregado) {
        DrawTexturePro(telas_estado.bg_jurados,
                       (Rectangle){0, 0, telas_estado.bg_jurados.width, telas_estado.bg_jurados.height},
                       (Rectangle){0, 0, 800, 600},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        ClearBackground(COR_FUNDO);
        DrawRectangle(0, 0, 800, 600, venceu ? (Color){220,255,220,255} : (Color){255,220,220,255});
    }

    for (int i = 0; i < 8; i++) {
        DrawRectangle(i*100, 0, 100, 50, (Color){200,230,255,180});
        DrawRectangleLines(i*100, 0, 100, 50, (Color){150,200,255,200});
    }

    if (venceu) {
        DrawText("*", 200, 20, 80, COR_AMARELO);
        DrawText("*", 340, 10, 100, COR_AMARELO);
        DrawText("*", 490, 20, 80, COR_AMARELO);
        DrawText("MUITO BOM, MEU FILHO!", 100, 60, 32, COR_TEXTO);
        DrawText(TextFormat("Pontuacao: %d", estado.pontuacao), 270, 95, 24, COR_VERDE);
    } else {
        DrawText(":(", 360, 20, 60, COR_VERMELHO);
        DrawText("Fim de jogo. Tente de novo!", 140, 80, 30, COR_TEXTO);
        DrawText(TextFormat("Pontuacao: %d  (meta: %d)", estado.pontuacao, META_FASE_FINAL),
                 160, 115, 22, COR_VERMELHO);
    }

    /* Painel dos jurados */
    if (j != NULL) {
        /* Fundo do painel */
        DrawRectangleRounded((Rectangle){20, 135, 760, 355}, 0.05f, 8, (Color){255,255,255,220});
        DrawText("JURI DA FASE FINAL", 280, 145, 24, COR_TEXTO);
        DrawText(TextFormat("Media: %.1f / 10", j->media_final), 320, 172, 20,
                 j->media_final >= 7 ? COR_VERDE : COR_VERMELHO);

        /* Ariano Suassuna */
        DrawRectangleRounded((Rectangle){25, 200, 750, 85}, 0.1f, 8, (Color){255,230,180,255});
        DrawText(TextFormat("Ariano Suassuna   %.1f/10", j->nota_ariano), 35, 208, 20, COR_TEXTO);
        DrawText(j->comentario_ariano, 35, 232, 17, DARKGRAY);

        /* Clarice Lispector */
        DrawRectangleRounded((Rectangle){25, 292, 750, 85}, 0.1f, 8, (Color){210,230,255,255});
        DrawText(TextFormat("Clarice Lispector   %.1f/10", j->nota_clarice), 35, 300, 20, COR_TEXTO);
        DrawText(j->comentario_clarice, 35, 324, 17, DARKGRAY);

        /* Chico Science */
        DrawRectangleRounded((Rectangle){25, 384, 750, 85}, 0.1f, 8, (Color){210,255,220,255});
        DrawText(TextFormat("Chico Science   %.1f/10", j->nota_chico), 35, 392, 20, COR_TEXTO);
        DrawText(j->comentario_chico, 35, 416, 17, DARKGRAY);
    }

    DrawRectangle(0, 498, 800, 102, COR_BARRA_FUNDO);
    int tw = MeasureText(venceu ? "Parabens! Voce e o melhor cozinheiro!" : "Continue treinando com mainha!", 20);
    DrawText(venceu ? "Parabens! Voce e o melhor cozinheiro!" : "Continue treinando com mainha!",
             (800-tw)/2, 508, 20, COR_AMARELO);
    DrawText("[ESC] Voltar ao menu", 300, 540, 18, WHITE);
}

// ==========================================
// TELA: CREDITOS
// ==========================================
void tela_creditos(void) {
    ClearBackground(COR_FUNDO);
    desenhar_cabecalho();

    DrawText("EQUIPE", 340, 80, 28, COR_TEXTO);

    const char *nomes[] = {"Louise Pessoa", "Marilia Liz Alves", "Mateus Lins", "Pedro David Oliveira", "Victor Martins"};
    const char *funcoes[] = {"Estruturas de Dados", "IA dos Jurados", "Gameplay / Pilha", "Interface / UI", "Integracao / Timer"};
    Color cores[] = {COR_VERMELHO, COR_VERDE, COR_AZUL, COR_LARANJA, COR_VERDE};

    for (int i = 0; i < 5; i++) {
        DrawRectangleRounded((Rectangle){80, 130 + i*70, 640, 55}, 0.3f, 8, cores[i]);
        DrawText(nomes[i], 110, 143 + i*70, 22, WHITE);
        DrawText(funcoes[i], 420, 143 + i*70, 18, WHITE);
    }

    DrawText("Disciplina: Algoritmos e Estruturas de Dados", 150, 500, 18, COR_TEXTO);
    DrawText("Tema: Na Vibe do Recife - Comida de Mainha", 155, 525, 18, COR_TEXTO);

    desenhar_rodape("[ESC] Voltar ao menu");
}