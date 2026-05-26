#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "raylib.h"
#include "telas.h"
#include "../jogo.h"
#include "../dados/receitas.h"
#include "../dados/pilha.h"
#include "fonte.h"

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
    Texture2D bg_receitas;
    int bg_receitas_carregado;
    Texture2D jurado_ariano;
    Texture2D jurado_clarice;
    Texture2D jurado_chico;
    int jurados_carregados;
    Texture2D tela_inicial;
    int tela_inicial_carregada;
    Texture2D final_img[3];
    int final_img_carregada[3];
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
            {"Carne", "carne.png"},
        {"Farinha de mandioca", "farinha.png"},
        {"Farinha de trigo", "farinha.png"},
        {"Queijo coalho", "queijo.png"},
        {"Ovos", "ovo.png"},
    };
    for (size_t i = 0; i < sizeof(map)/sizeof(map[0]); i++) {
        if (strcasecmp(nome, map[i].nome) == 0) {
            snprintf(caminho, tam, "sprites/coleta/%s", map[i].file);
            return;
        }
    }

    // Fallback: lowercase + underscores
    snprintf(caminho, tam, "sprites/coleta/");
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
    telas_estado.tela_inicial = LoadTexture("sprites/fundos/tela_inicial_titulo.png");
    if (telas_estado.tela_inicial.id != 0) {
        SetTextureFilter(telas_estado.tela_inicial, TEXTURE_FILTER_BILINEAR);
        telas_estado.tela_inicial_carregada = 1;
    }

    telas_estado.bg_receitas = LoadTexture("sprites/fundos/fundo_receitas.png");
    if (telas_estado.bg_receitas.id != 0) {
        SetTextureFilter(telas_estado.bg_receitas, TEXTURE_FILTER_BILINEAR);
        telas_estado.bg_receitas_carregado = 1;
    }

    telas_estado.bg_jurados = LoadTexture("sprites/jurados/jurados_nota.png");
    if (telas_estado.bg_jurados.id != 0) {
        SetTextureFilter(telas_estado.bg_jurados, TEXTURE_FILTER_BILINEAR);
        telas_estado.bg_carregado = 1;
    }
    // carregar sprites dos jurados (se existirem)
    telas_estado.jurado_ariano = LoadTexture("sprites/jurados/jurado_ariano.png");
    telas_estado.jurado_clarice = LoadTexture("sprites/jurados/jurado_clarice.png");
    telas_estado.jurado_chico   = LoadTexture("sprites/jurados/jurado_chico.png");
    if (telas_estado.jurado_ariano.id) SetTextureFilter(telas_estado.jurado_ariano, TEXTURE_FILTER_BILINEAR);
    if (telas_estado.jurado_clarice.id) SetTextureFilter(telas_estado.jurado_clarice, TEXTURE_FILTER_BILINEAR);
    if (telas_estado.jurado_chico.id)   SetTextureFilter(telas_estado.jurado_chico, TEXTURE_FILTER_BILINEAR);
    telas_estado.jurados_carregados = (telas_estado.jurado_ariano.id != 0) || (telas_estado.jurado_clarice.id != 0) || (telas_estado.jurado_chico.id != 0);
    // carregar imagens finais (opcionais)
    const char *finais[3] = { "sprites/final/final_1.png", "sprites/final/final_2.png", "sprites/final/final_3.png" };
    for (int i = 0; i < 3; i++) {
        telas_estado.final_img[i] = LoadTexture(finais[i]);
        telas_estado.final_img_carregada[i] = (telas_estado.final_img[i].id != 0);
        if (telas_estado.final_img_carregada[i]) {
            SetTextureFilter(telas_estado.final_img[i], TEXTURE_FILTER_BILINEAR);
            printf("[TELAS] final_%d carregada: id=%u (%s)\n", i+1, telas_estado.final_img[i].id, finais[i]);
        } else {
            printf("[TELAS] falha ao carregar final_%d (%s)\n", i+1, finais[i]);
        }
    }
}

// descarrega texturas das telas
void telas_limpar(void) {
    if (telas_estado.tela_inicial_carregada) {
        UnloadTexture(telas_estado.tela_inicial);
        telas_estado.tela_inicial_carregada = 0;
    }
    if (telas_estado.bg_receitas_carregado) {
        UnloadTexture(telas_estado.bg_receitas);
        telas_estado.bg_receitas_carregado = 0;
    }
    if (telas_estado.bg_carregado) {
        UnloadTexture(telas_estado.bg_jurados);
        telas_estado.bg_carregado = 0;
    }
    if (telas_estado.jurado_ariano.id) { UnloadTexture(telas_estado.jurado_ariano); telas_estado.jurado_ariano.id = 0; }
    if (telas_estado.jurado_clarice.id) { UnloadTexture(telas_estado.jurado_clarice); telas_estado.jurado_clarice.id = 0; }
    if (telas_estado.jurado_chico.id)   { UnloadTexture(telas_estado.jurado_chico);   telas_estado.jurado_chico.id = 0; }
    telas_estado.jurados_carregados = 0;

    for (int i = 0; i < 3; i++) {
        if (telas_estado.final_img_carregada[i]) {
            UnloadTexture(telas_estado.final_img[i]);
            telas_estado.final_img_carregada[i] = 0;
        }
    }

    // limpar cache de sprites
    for (int i = 0; i < TELAS_SPRITE_CACHE; i++) {
        if (sprite_cache[i].carregado) {
            UnloadTexture(sprite_cache[i].tex);
            sprite_cache[i].carregado = 0;
        }
    }
}

// desenha a imagem de introducao final (indice 0..2). Se nao existir a imagem, desenha fundo simples.
void tela_final_intro_draw(int indice) {
    if (indice < 0) indice = 0;
    if (indice > 2) indice = 2;
    ClearBackground(COR_FUNDO);
    if (telas_estado.final_img_carregada[indice]) {
        Texture2D t = telas_estado.final_img[indice];
        DrawTexturePro(t,
            (Rectangle){0, 0, (float)t.width, (float)t.height},
            (Rectangle){0, 0, 800, 600},
            (Vector2){0,0}, 0.0f, WHITE);
    } else {
        // fallback: mostra nome do slide
        char buf[64];
        snprintf(buf, sizeof(buf), "Introducao %d", indice + 1);
        int tw = medir_txt(buf, 36);
        txt(buf, (800 - tw) / 2, 260, 36, COR_TEXTO);
    }
    // footer hint removed per UX request
}

// ==========================================
// UTILITARIOS
// ==========================================

// desenha botao colorido com texto centralizado
static void desenhar_botao(int x, int y, int w, int h, Color cor, const char *texto) {
    // sombra
    DrawRectangleRounded((Rectangle){x + 4, y + 5, w, h}, 0.4f, 8, (Color){0, 0, 0, 100});
    // corpo
    DrawRectangleRounded((Rectangle){x, y, w, h}, 0.4f, 8, cor);
    // borda branca fina
    DrawRectangleRoundedLines((Rectangle){x, y, w, h}, 0.4f, 8, 1.5f, (Color){255, 255, 255, 180});
    // texto
    int tam = 24;
    int tw = medir_txt(texto, tam);
    // sombra do texto
    txt(texto, x + (w - tw) / 2 + 1, y + (h - tam) / 2 + 2, tam, (Color){0, 0, 0, 120});
    txt(texto, x + (w - tw) / 2, y + (h - tam) / 2, tam, WHITE);
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
    txt("*", x + w/2 - 6, y - 2, 24, COR_AMARELO);
}

// desenha cabecalho com titulo do jogo
static void desenhar_cabecalho(void) {
    // barra azul no topo
    DrawRectangle(0, 0, 800, 60, COR_AZUL);
    DrawRectangle(0, 55, 800, 5, COR_AMARELO);
    // titulo
    txt("RECEITAS DE MAINHA", 220, 15, 32, WHITE);
}

// desenha rodape azul
static void desenhar_rodape(const char *texto) {
    DrawRectangle(0, 560, 800, 40, COR_BARRA_FUNDO);
    int tw = medir_txt(texto, 20);
    txt(texto, (800 - tw) / 2, 570, 20, COR_AMARELO);
}

// ==========================================
// TELA: MENU PRINCIPAL
// ==========================================
void tela_menu(void) {
    ClearBackground(COR_FUNDO);

    // imagem de capa (ocupa a tela inteira)
    if (telas_estado.tela_inicial_carregada) {
        Texture2D t = telas_estado.tela_inicial;
        DrawTexturePro(t,
            (Rectangle){0, 0, (float)t.width, (float)t.height},
            (Rectangle){0, 0, 800, 600},
            (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        // fallback: fundo colorido original
        DrawRectangle(0, 0, 800, 70, COR_AZUL);
        DrawRectangle(0, 65, 800, 6, COR_AMARELO);
        txt("COMIDA", 260, 80, 64, COR_VERMELHO);
        txt("DE", 340, 140, 48, COR_AZUL);
        txt("MAINHA", 240, 185, 64, COR_AZUL);
        txt("*", 210, 90, 28, COR_AMARELO);
        txt("*", 570, 90, 28, COR_VERDE);
        txt("*", 190, 200, 20, COR_VERMELHO);
        txt("*", 590, 200, 20, COR_LARANJA);
    }

    // botoes principais (parte inferior, centralizados e com largura dinamica)
    int botao_h = 55;
    int botao_y = 490;
    int botao_tam = 24;
    int botao_padding = 30;
    int botao_gap = 30;
    int botao_w_receitas = medir_txt("Receitas", botao_tam) + botao_padding * 2;
    int botao_w_creditos = medir_txt("Créditos", botao_tam) + botao_padding * 2;
    int botoes_total = botao_w_receitas + botao_w_creditos + botao_gap;
    int botoes_x0 = (800 - botoes_total) / 2;
    int botao_x_receitas = botoes_x0;
    int botao_x_creditos = botoes_x0 + botao_w_receitas + botao_gap;

    desenhar_botao(botao_x_receitas, botao_y, botao_w_receitas, botao_h, COR_VERMELHO, "Receitas");
    desenhar_botao(botao_x_creditos, botao_y, botao_w_creditos, botao_h, COR_AZUL,     "Créditos");

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();
        if (CheckCollisionPointRec(m, (Rectangle){botao_x_receitas, botao_y, botao_w_receitas, botao_h})) tela_atual = TELA_RECEITAS;
        if (CheckCollisionPointRec(m, (Rectangle){botao_x_creditos, botao_y, botao_w_creditos, botao_h})) tela_atual = TELA_CREDITOS;
    }

    // rodape removido
}

// ==========================================
// TELA: LISTA DE RECEITAS
// ==========================================
void tela_receitas(Receita *lista) {
    ClearBackground(COR_FUNDO);
    desenhar_cabecalho();

    if (telas_estado.bg_receitas_carregado) {
        DrawTexturePro(
            telas_estado.bg_receitas,
            (Rectangle){0, 0, (float)telas_estado.bg_receitas.width, (float)telas_estado.bg_receitas.height},
            (Rectangle){0, 0, 800, 600},
            (Vector2){0, 0},
            0.0f,
            WHITE);
    }

    // painel de madeira (simulado)
    DrawRectangleRounded((Rectangle){30, 70, 460, 470}, 0.05f, 8, (Color){210, 170, 100, 255});
    DrawRectangleRounded((Rectangle){38, 78, 444, 454}, 0.05f, 8, (Color){255, 248, 220, 255});

    txt("RECEITAS DISPONIVEIS", 60, 90, 22, COR_TEXTO);

    // mensagem de bloqueio temporaria
    static float timer_bloqueio = 0.0f;
    if (timer_bloqueio > 0.0f) timer_bloqueio -= GetFrameTime();

    if (lista == NULL) {
        txt("Nenhuma receita cadastrada.", 80, 200, 20, GRAY);
    } else {
        Color cores[] = {COR_VERMELHO, COR_VERDE, COR_AZUL, COR_LARANJA, COR_VERMELHO, COR_VERDE, COR_AZUL, COR_LARANJA};
        Vector2 mouse = GetMousePosition();
        Receita *aux = lista;
        int i = 0, y = 120;
        while (aux != NULL && i < 8) {
            Rectangle card = {50, y, 420, 48};
            int bloqueada = !aux->desbloqueada;
            int eh_sel    = (!bloqueada && receita_selecionada == aux);
            int hovered   = CheckCollisionPointRec(mouse, card);

            // borda amarela se selecionado
            if (eh_sel)
                DrawRectangleRounded((Rectangle){46, y - 4, 428, 56}, 0.3f, 8, COR_AMARELO);

            // card bloqueado: cinza escuro; desbloqueado: cor normal com hover
            Color cor;
            if (bloqueada) {
                cor = (Color){80, 80, 80, 255};
            } else {
                cor = cores[i];
                if (hovered && !eh_sel) {
                    cor.r = (unsigned char)(cor.r > 30 ? cor.r - 30 : 0);
                    cor.g = (unsigned char)(cor.g > 30 ? cor.g - 30 : 0);
                    cor.b = (unsigned char)(cor.b > 30 ? cor.b - 30 : 0);
                }
            }
            DrawRectangleRounded(card, 0.3f, 8, cor);

            // nome e dificuldade
            Color cor_txt = bloqueada ? (Color){180, 180, 180, 255} : WHITE;
            txt(TextFormat("[%d] %s", i + 1, aux->nome), 70, y + 8, 20, cor_txt);
            txt(TextFormat("Dif: %d", aux->dificuldade), 370, y + 8, 18, cor_txt);

            if (bloqueada) {
                // cadeado trancado
                float px = 53, sh = (float)y + 26;  // sh = topo do corpo
                float bw = 20, bh = 14;
                float cx = px + bw / 2.0f;
                // centro da argola EM sh: corpo cobre metade inferior → arco D visível
                float ring_cy = sh;

                // sombra
                Color sombra = (Color){0, 0, 0, 90};
                DrawRing((Vector2){cx+2, ring_cy+2}, 5.5f, 10.0f, 0, 360, 30, sombra);
                DrawRectangleRounded((Rectangle){px+2, sh+2, bw, bh}, 0.35f, 8, sombra);

                // argola - ouro escuro (metade superior visível acima do corpo)
                DrawRing((Vector2){cx, ring_cy}, 5.5f, 10.0f, 0, 360, 30, (Color){160,115,10,255});

                // corpo cobre metade inferior da argola
                DrawRectangleRounded((Rectangle){px, sh, bw, bh}, 0.35f, 8, (Color){230,180,30,255});

                // destaque no topo do corpo
                DrawRectangleRounded((Rectangle){px+3, sh+2, bw-6, 4}, 0.6f, 4, (Color){255,225,90,150});

                // buraco de fechadura
                DrawCircleV((Vector2){cx, sh + bh * 0.40f}, 2.5f, (Color){100,65,5,230});
                DrawRectangle((int)(cx - 1.5f), (int)(sh + bh * 0.40f), 3, 5, (Color){100,65,5,230});

                // texto bloqueado
                txt("Complete as 3 receitas para desbloquear", 82, y + 28, 12, (Color){200, 200, 100, 255});
            } else if (hovered) {
                txt("Clique para selecionar", 70, y + 28, 13, COR_AMARELO);
            } else {
                int np = 0;
                No *tp = aux->passos;
                while (tp) { np++; tp = tp->prox; }
                txt(TextFormat("%d passos", np), 70, y + 28, 14, COR_AMARELO);
            }

            // clique
            if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (bloqueada) {
                    timer_bloqueio = 3.0f;
                } else {
                    receita_selecionada = aux;
                    tela_atual = TELA_INGREDIENTES;
                }
            }

            aux = aux->prox;
            y += 56;
            i++;
        }

        // mensagem de bloqueio no topo do painel
        if (timer_bloqueio > 0.0f) {
            DrawRectangleRounded((Rectangle){38, 78, 444, 36}, 0.2f, 8, (Color){180, 30, 30, 230});
            txt("Complete as 3 receitas primeiro para desbloquear o Pirao!", 48, 88, 13, WHITE);
        }
    }

    // lado direito - vovo
    DrawRectangleRounded((Rectangle){510, 70, 260, 470}, 0.05f, 8, (Color){255, 235, 180, 255});
    txt("Escolha uma", 530, 100, 20, COR_TEXTO);
    txt("receita para", 530, 125, 20, COR_TEXTO);
    txt("cozinhar!", 530, 150, 20, COR_TEXTO);

    if (receita_selecionada != NULL) {
        DrawRectangleRounded((Rectangle){520, 200, 240, 110}, 0.2f, 8, COR_VERDE);
        txt("Selecionada:", 535, 210, 16, WHITE);
        // tentar desenhar sprite do nome da receita
        Texture2D rtex = obter_sprite_telas(receita_selecionada->nome);
        if (rtex.id != 0) {
            float rw = 200;
            float rh = 70;
            float scale = fminf(rw / rtex.width, rh / rtex.height) * 1.25f;
            float tx = 520 + (240 - rtex.width*scale)/2.0f;
            float ty = 200 + (110 - rtex.height*scale)/2.0f;
            DrawTextureEx(rtex, (Vector2){tx, ty}, 0.0f, scale, WHITE);
        } else {
            txt(receita_selecionada->nome, 535, 235, 18, WHITE);
        }
        txt(TextFormat("Dif: %d | %d min",
                            receita_selecionada->dificuldade,
                            receita_selecionada->tempo),
                 535, 262, 14, WHITE);
        txt("[ENTER] Continuar", 535, 285, 14, COR_AMARELO);
    } else {
        txt("(nenhuma selecionada)", 530, 215, 16, GRAY);
    }

    // instrucoes
    txt("[Setas] Navegar", 530, 380, 16, COR_TEXTO);
    txt("[1-3] Selecionar", 530, 400, 16, COR_TEXTO);
    txt("[ENTER] Continuar", 530, 420, 16, COR_TEXTO);
    txt("[1] Menu", 530, 440, 16, COR_TEXTO);

    desenhar_rodape("[Setas/1-3] Selecionar  |  [ENTER] ou Clique para continuar");
}

// ==========================================
// TELA: INGREDIENTES DA RECEITA
// ==========================================
void tela_ingredientes(Receita *receita) {
    ClearBackground(COR_FUNDO);

    if (telas_estado.bg_receitas_carregado) {
        Texture2D t = telas_estado.bg_receitas;
        DrawTexturePro(t,
            (Rectangle){0, 0, (float)t.width, (float)t.height},
            (Rectangle){0, 0, 800, 600},
            (Vector2){0, 0}, 0.0f, WHITE);
    }

    if (receita == NULL) {
        txt("Nenhuma receita selecionada.", 220, 280, 24, COR_VERMELHO);
        txt("Volte ao menu de receitas e escolha uma.",
                 170, 320, 20, COR_TEXTO);
        desenhar_rodape("[2] Voltar para receitas");
        return;
    }

    // nome da receita
    int card_x = 50;
    int card_y = 40;
    int card_w = 700;
    int card_h = 50;
    // card sem preenchimento (transparente)
    int tw = medir_txt(receita->nome, 40);
    txt(receita->nome, (800 - tw)/2, card_y + 40, 40, COR_TEXTO);

    // balao de instrucao
    txt("Memorize os ingredientes abaixo!", 120, 145, 28, COR_VERMELHO);
    txt("Eles vão cair do céu em 25 segundos, use <- e -> ou A e D para", 120, 185, 17, COR_TEXTO);
    txt("mover a cesta e coletar apenas os ingredientes listados.", 120, 210, 17, COR_TEXTO);
    txt("Deixar passar os certos depois do tempo custa pontos!", 120, 237, 17, COR_TEXTO);

    // lista de ingredientes
    int lista_x = 150;
    txt("Ingredientes:", lista_x, 270, 22, COR_TEXTO);
    if (receita->n_ingredientes == 0) {
        txt("(nenhum ingrediente cadastrado)", lista_x + 20, 305, 18, GRAY);
    } else {
        Color cores[] = {COR_VERMELHO, COR_VERDE, COR_AZUL, COR_LARANJA};
        int y = 305;
        for (int i = 0; i < receita->n_ingredientes && i < 8; i++) {
            DrawCircle(lista_x + 15, y + 10, 14, cores[i % 4]);
            txt(TextFormat("%d", i + 1), lista_x + 10, y + 3, 16, WHITE);
            // desenha apenas o nome do ingrediente (sprites removidos nesta tela)
            txt(receita->ingredientes[i], lista_x + 35, y, 20, COR_TEXTO);
            y += 30;
        }
    }

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
    DrawRectangleRoundedLines((Rectangle){100, 20, 600, 60}, 0.3f, 8, 2.0f, COR_LARANJA);
    txt("*", 115, 35, 22, COR_AMARELO);
    int tw = medir_txt(passo_atual, 22);
    txt(passo_atual, (800 - tw)/2, 35, 22, COR_TEXTO);
    txt("*", 665, 35, 22, COR_AMARELO);

    // relogio / timer
    DrawCircle(100, 200, 55, WHITE);
    DrawCircleLines(100, 200, 55, COR_VERMELHO);
    DrawCircleLines(100, 200, 50, (Color){200,200,200,255});
    txt(TextFormat("%.0fs", tempo), 75, 185, 26, COR_VERMELHO);
    txt("TEMPO", 68, 215, 16, GRAY);

    // balao da vovo com dica
    DrawRectangleRounded((Rectangle){30, 320, 220, 80}, 0.2f, 8, (Color){255,220,230,255});
    DrawRectangleRoundedLines((Rectangle){30, 320, 220, 80}, 0.2f, 8, 2.0f, (Color){255,150,180,255});
    txt("Siga a ordem!", 45, 335, 18, COR_TEXTO);
    txt("Voce consegue!", 45, 358, 17, COR_TEXTO);

    // barra de progresso na base
    DrawRectangle(0, 470, 800, 10, LIGHTGRAY);
    float prog = total_passos > 0 ? (float)num_passo / total_passos : 0;
    desenhar_barra_progresso(30, 460, 740, 20, prog);

    // numero do passo
    txt(TextFormat("Passo %d de %d", num_passo, total_passos), 310, 430, 20, COR_TEXTO);

    // pontuacao
    txt(TextFormat("Pontos: %d", estado.pontuacao), 600, 200, 22, COR_TEXTO);

    // instrucao
    txt("[ENTER] Confirmar passo  [ESC] Desistir", 200, 495, 18, COR_TEXTO);
}

// ==========================================
// TELA: FEEDBACK (ACERTO / ERRO)
// ==========================================
void tela_feedback(int acertou) {
    if (acertou) {
        ClearBackground((Color){200, 255, 200, 255});
        txt("ACERTOU!", 250, 180, 64, COR_VERDE);
        txt(":)", 360, 260, 48, COR_VERDE);
        txt(TextFormat("Passo %d/%d concluido!", estado.passos_acertados, estado.passos_total), 200, 340, 24, COR_TEXTO);
    } else {
        ClearBackground((Color){255, 210, 210, 255});
        txt("ERROU!", 270, 180, 64, COR_VERMELHO);
        txt(":(", 365, 260, 48, COR_VERMELHO);
        txt(TextFormat("-20 pontos | Pontuacao: %d", estado.pontuacao), 180, 340, 24, COR_TEXTO);
    }
    txt("Pontuacao atual:", 280, 400, 22, COR_TEXTO);
    txt(TextFormat("%d", estado.pontuacao), 370, 430, 36, acertou ? COR_VERDE : COR_VERMELHO);
    txt("[ENTER] Continuar", 300, 510, 20, GRAY);
}

// ==========================================
// TELA: RESULTADO FINAL
// ==========================================
void tela_resultado(int venceu, ResultadoJurados *j)
{
    // fundo
    if (telas_estado.bg_carregado) {
        DrawTexturePro(telas_estado.bg_jurados,
                       (Rectangle){0, 0, telas_estado.bg_jurados.width, telas_estado.bg_jurados.height},
                       (Rectangle){0, 0, 800, 600},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        ClearBackground((Color){240, 235, 220, 255});
    }
    // overlay escuro para garantir legibilidade
    DrawRectangle(0, 0, 800, 600, (Color){0, 0, 0, 120});

    // cabecalho
    DrawRectangle(0, 0, 800, 60, (Color){30, 60, 140, 240});
    DrawRectangle(0, 55, 800, 5, COR_AMARELO);
    // Sempre mostrar título genérico (remover mensagem de parabéns)
    const char *titulo = "RESULTADO FINAL";
    int ttw = medir_txt(titulo, 26);
    txt(titulo, (800 - ttw) / 2, 14, 26, WHITE);

    if (j == NULL) {
        // aguardando jurados
        txt("Avaliando sua performance...", 240, 280, 24, WHITE);
        txt("[ESC] Voltar ao menu", 290, 540, 18, (Color){255, 220, 80, 255});
        return;
    }

    // media e pontuacao em destaque
    char buf_media[64], buf_pts[64];
    snprintf(buf_media, sizeof(buf_media), "Media: %.1f / 10", j->media_final);
    snprintf(buf_pts,   sizeof(buf_pts),   "Pontuacao: %d",    estado.pontuacao);
    Color cor_media  = j->media_final   >= 7.0f            ? COR_VERDE : COR_VERMELHO;
    Color cor_pontos = estado.pontuacao >= META_FASE_FINAL  ? COR_VERDE : COR_VERMELHO;

    DrawRectangleRounded((Rectangle){20, 68, 370, 52}, 0.2f, 8, (Color){0,0,0,160});
    DrawRectangleRounded((Rectangle){410, 68, 370, 52}, 0.2f, 8, (Color){0,0,0,160});
    int mw = medir_txt(buf_media, 26);
    int pw = medir_txt(buf_pts,   26);
    txt(buf_media, 20  + (370 - mw) / 2, 80, 26, cor_media);
    txt(buf_pts,   410 + (370 - pw) / 2, 80, 26, cor_pontos);

    // cards dos jurados
    const int card_x = 20;
    const int card_w = 760;
    const int card_h = 88;
    const int sprite_size = 68;

    struct { int y; Color cor; Texture2D *spr; const char *nome; float nota; const char *comentario; } jurados[] = {
        { 130, (Color){255, 243, 210, 230}, &telas_estado.jurado_ariano,  "Ariano Suassuna",  j->nota_ariano,  j->comentario_ariano  },
        { 226, (Color){220, 235, 255, 230}, &telas_estado.jurado_clarice, "Clarice Lispector",j->nota_clarice, j->comentario_clarice },
        { 322, (Color){215, 255, 220, 230}, &telas_estado.jurado_chico,   "Chico Science",    j->nota_chico,   j->comentario_chico   },
    };

    for (int i = 0; i < 3; i++) {
        int cy = jurados[i].y;
        DrawRectangleRounded((Rectangle){card_x, cy, card_w, card_h}, 0.12f, 8, jurados[i].cor);
        DrawRectangleRoundedLines((Rectangle){card_x, cy, card_w, card_h}, 0.12f, 8, 1.5f, (Color){0,0,0,60});

        int text_x = card_x + 12;
        if (jurados[i].spr->id) {
            Texture2D t = *jurados[i].spr;
            float sc = (float)sprite_size / (float)(t.height > 0 ? t.height : 1);
            float draw_w = t.width * sc;
            float draw_h = t.height * sc;
            Rectangle src = (Rectangle){0.0f, 0.0f, (float)t.width, (float)t.height};
            Rectangle dst = (Rectangle){ (float)(card_x + 8), (float)(cy + (card_h - draw_h)/2), draw_w, draw_h };
            DrawTexturePro(t, src, dst, (Vector2){0,0}, 0.0f, WHITE);
            text_x = card_x + 8 + (int)ceilf(draw_w) + 14;
        }

        // nome + nota
        char buf_nome[64];
        snprintf(buf_nome, sizeof(buf_nome), "%s", jurados[i].nome);
        txt(buf_nome, text_x, cy + 10, 20, COR_TEXTO);

        char buf_nota[16];
        snprintf(buf_nota, sizeof(buf_nota), "%.1f/10", jurados[i].nota);
        Color cor_nota = jurados[i].nota >= 7.0f ? (Color){30,130,60,255} : (Color){180,30,30,255};
        int nw = medir_txt(buf_nota, 22);
        txt(buf_nota, card_x + card_w - nw - 14, cy + 10, 22, cor_nota);

        // comentario com texto escuro sobre fundo claro
        txt(jurados[i].comentario, text_x, cy + 36, 16, (Color){40, 20, 10, 255});
    }

    // rodape
    DrawRectangle(0, 418, 800, 182, (Color){20, 45, 110, 220});
    DrawRectangle(0, 418, 800, 4, COR_AMARELO);

    // resumo final
    // Remover texto de vitória; manter mensagem neutra
    const char *msg = "Continue praticando com Mainha!";
    int mw2 = medir_txt(msg, 22);
    txt(msg, (800 - mw2) / 2, 432, 22, COR_AMARELO);

    char buf_r[128];
    snprintf(buf_r, sizeof(buf_r), "Pontos: %d   |   Media: %.1f/10", estado.pontuacao, j->media_final);
    int rw = medir_txt(buf_r, 19);
    txt(buf_r, (800 - rw) / 2, 462, 19, WHITE);

    txt("[ESC] Voltar ao menu", 300, 500, 18, (Color){200, 200, 200, 255});
}

// ==========================================
// TELA: CREDITOS
// ==========================================
void tela_creditos(void) {
    ClearBackground(COR_FUNDO);
    desenhar_cabecalho();

    int largura_virtual = 800;
    const char *titulo = "EQUIPE";
    int tw_titulo = medir_txt(titulo, 28);
    txt(titulo, (largura_virtual - tw_titulo) / 2, 80, 28, COR_TEXTO);

    const char *nomes[] = {"Louise Pessoa", "Marilia Liz Alves", "Mateus Lins", "Pedro David Oliveira", "Victor Martins"};
    Color cores[] = {COR_VERMELHO, COR_VERDE, COR_AZUL, COR_LARANJA, COR_VERDE};

    for (int i = 0; i < 5; i++) {
        int card_x = 80;
        int card_y = 130 + i * 70;
        int card_w = 640;
        int card_h = 55;
        int nome_tam = 22;
        int tw_nome = medir_txt(nomes[i], nome_tam);
        int nome_x = card_x + (card_w - tw_nome) / 2;
        int nome_y = card_y + (card_h - nome_tam) / 2 + 2;
        DrawRectangleRounded((Rectangle){card_x, card_y, card_w, card_h}, 0.3f, 8, cores[i]);
        txt(nomes[i], nome_x, nome_y, nome_tam, WHITE);
    }

    const char *linha_disciplina = "Disciplina: Algoritmos e Estruturas de Dados";
    const char *linha_tema = "Tema: Na Vibe do Recife - Receitas de Mainha";
    int tw_disc = medir_txt(linha_disciplina, 18);
    int tw_tema = medir_txt(linha_tema, 18);
    txt(linha_disciplina, (largura_virtual - tw_disc) / 2, 500, 18, COR_TEXTO);
    txt(linha_tema, (largura_virtual - tw_tema) / 2, 525, 18, COR_TEXTO);

    desenhar_rodape("[ESC] Voltar ao menu");
}