#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "raylib.h"
#include "cozinhar.h"
#include "../jogo.h"
#include "../dados/pilha.h"
#include "../ui/fonte.h"

// ==========================================
// CORES
// ==========================================
#define COR_FUNDO_COZ  (Color){255, 243, 210, 255}
#define COR_AZUL_COZ   (Color){30,  100, 200, 255}
#define COR_VERDE_COZ  (Color){50,  160, 80,  255}
#define COR_VERM_COZ   (Color){220, 50,  50,  255}
#define COR_AMA_COZ    (Color){255, 200, 0,   255}
#define COR_LARA_COZ   (Color){255, 140, 0,   255}
#define COR_TEXTO_COZ  (Color){60,  30,  10,  255}

#define LARG  800
#define ALT   600

EstadoCozinhar cozinhar;

// ==========================================
// AUX: nome amigavel da tecla
// ==========================================
static const char *nome_tecla(char c) {
    static char buf[12];
    c = (char)toupper((unsigned char)c);
    if (c == ' ' || c == '_') return "BARRA";
    buf[0] = c; buf[1] = '\0';
    return buf;
}

// retorna o KeyboardKey da raylib pra um caractere da sequencia
static int char_para_keycode(char c) {
    c = (char)toupper((unsigned char)c);
    if (c >= 'A' && c <= 'Z') return KEY_A + (c - 'A');
    if (c >= '0' && c <= '9') return KEY_ZERO + (c - '0');
    if (c == ' ' || c == '_') return KEY_SPACE;
    return -1;
}

// ==========================================
// INICIALIZACAO
// ==========================================
static void montar_grid(void) {
    // liberar sprites antigos do grid
    for (int ii = 0; ii < cozinhar.n_grid; ii++) {
        if (cozinhar.grid[ii].sprite_carregado) {
            UnloadTexture(cozinhar.grid[ii].sprite);
            cozinhar.grid[ii].sprite_carregado = 0;
        }
    }
    cozinhar.n_grid = 0;
    Receita *r = cozinhar.receita;
    
    // Verifica se o passo atual tem ingrediente vazio (passo especial como forno)
    if (!pilha_vazia(cozinhar.pilha) && cozinhar.pilha->dado.ingrediente[0] == '\0') {
        // Passo especial - mostra um botão para o tipo de passo
        const char *nome_especial = "Forno";  // poderia ser parametrizável
        
        strncpy(cozinhar.grid[0].nome, nome_especial,
                sizeof(cozinhar.grid[0].nome) - 1);
        cozinhar.grid[0].nome[sizeof(cozinhar.grid[0].nome) - 1] = '\0';
        cozinhar.grid[0].destacado = 1;  // sempre destacado
        cozinhar.grid[0].usado = 0;
        
        float w = 130;
        float h = 70;
        float gx = 30 + 2 * (w + 14);  // centralizar
        float gy = 410;
        cozinhar.grid[0].area = (Rectangle){ gx, gy, w, h };
        // tentar carregar sprite do forno
        char path[256] = {0};
        // mapeamento explicito
        snprintf(path, sizeof(path), "sprites/receitas/travessa_forno.png");
        cozinhar.grid[0].sprite = LoadTexture(path);
        if (cozinhar.grid[0].sprite.id != 0) {
            SetTextureFilter(cozinhar.grid[0].sprite, TEXTURE_FILTER_BILINEAR);
            cozinhar.grid[0].sprite_carregado = 1;
        }
        cozinhar.n_grid = 1;
        return;
    }
    
    int total = r->n_ingredientes < COZ_MAX_ING_GRID ? r->n_ingredientes : COZ_MAX_ING_GRID;
    for (int i = 0; i < total; i++) {
        strncpy(cozinhar.grid[i].nome, r->ingredientes[i],
                sizeof(cozinhar.grid[i].nome) - 1);
        cozinhar.grid[i].nome[sizeof(cozinhar.grid[i].nome) - 1] = '\0';
        cozinhar.grid[i].destacado = 0;
        cozinhar.grid[i].usado = 0;
        cozinhar.grid[i].sprite_carregado = 0;

        // grid 5 colunas em 2 linhas
        int col = i % 5;
        int row = i / 5;
        float w = 130;
        float h = 70;
        float gap = 14;
        // compute how many columns in this row (last row may have fewer)
        int cols_in_row = ((row + 1) * 5 <= total) ? 5 : (total - row * 5);
        if (cols_in_row <= 0) cols_in_row = 1;
        int col_index = i - row * 5; // position within the row
        float start_x = (LARG - (cols_in_row * w + (cols_in_row - 1) * gap)) / 2.0f;
        float gx = start_x + col_index * (w + gap);
        float gy = 410 + row * (h + 10);
        cozinhar.grid[i].area = (Rectangle){ gx, gy, w, h };
        // carregar sprite do ingrediente (mapear nomes conhecidos)
        char path[256] = {0};
        // mapeamentos do jogo (mesmos que telas/ordenacao)
        if (strcasecmp(cozinhar.grid[i].nome, "Tapioca granulada") == 0) snprintf(path, sizeof(path), "sprites/coleta/massa_tapioca.png");
        else if (strcasecmp(cozinhar.grid[i].nome, "Carne de sol") == 0) snprintf(path, sizeof(path), "sprites/coleta/carne.png");
        else if (strcasecmp(cozinhar.grid[i].nome, "Farinha de mandioca") == 0) snprintf(path, sizeof(path), "sprites/coleta/farinha.png");
        else if (strcasecmp(cozinhar.grid[i].nome, "Farinha de trigo") == 0) snprintf(path, sizeof(path), "sprites/coleta/farinha.png");
        else if (strcasecmp(cozinhar.grid[i].nome, "Queijo coalho") == 0) snprintf(path, sizeof(path), "sprites/coleta/queijo.png");
        else if (strcasecmp(cozinhar.grid[i].nome, "Ovos") == 0) snprintf(path, sizeof(path), "sprites/coleta/ovo.png");
        else {
            // fallback: lowercase + underscore
            char namebuf[128] = {0};
            int p = 0;
            for (int k = 0; k < (int)strlen(cozinhar.grid[i].nome) && p < (int)sizeof(namebuf)-1; k++) {
                char c = cozinhar.grid[i].nome[k];
                if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
                if (c == ' ') c = '_';
                namebuf[p++] = c;
            }
            namebuf[p] = '\0';
            snprintf(path, sizeof(path), "sprites/coleta/%s.png", namebuf);
        }
        if (path[0] != '\0') {
            cozinhar.grid[i].sprite = LoadTexture(path);
            if (cozinhar.grid[i].sprite.id != 0) {
                SetTextureFilter(cozinhar.grid[i].sprite, TEXTURE_FILTER_BILINEAR);
                cozinhar.grid[i].sprite_carregado = 1;
            }
        }
    }
    cozinhar.n_grid = total;
}

static void marcar_destacado(void) {
    if (pilha_vazia(cozinhar.pilha)) return;
    const char *alvo = cozinhar.pilha->dado.ingrediente;
    for (int k = 0; k < cozinhar.n_grid; k++) {
        cozinhar.grid[k].destacado = (strcmp(cozinhar.grid[k].nome, alvo) == 0);
    }
}

void cozinhar_iniciar(Receita *receita) {
    // libera pilha de trabalho anterior
    while (!pilha_vazia(cozinhar.pilha))
        cozinhar.pilha = pop_passo(cozinhar.pilha);

    // libera texturas anteriores
    if (cozinhar.texturas_passos != NULL) {
        for (int i = 0; i < cozinhar.n_passos; i++) {
            if (cozinhar.texturas_carregadas[i])
                UnloadTexture(cozinhar.texturas_passos[i]);
        }
        free(cozinhar.texturas_passos);
        free(cozinhar.texturas_carregadas);
    }

    memset(&cozinhar, 0, sizeof(cozinhar));
    cozinhar.receita = receita;
    if (receita == NULL || pilha_vazia(receita->passos)) {
        cozinhar.terminou = 1;
        cozinhar.venceu = 0;
        return;
    }

    // conta passos e constroi pilha de trabalho a partir de receita->passos.
    // receita->passos tem o ultimo passo no topo (push em ordem 1..N).
    // ao iterar e re-empilhar, invertemos: passo 1 fica no topo de cozinhar.pilha.
    int n = 0;
    No *src = receita->passos;
    while (src != NULL) { n++; src = src->prox; }
    cozinhar.n_passos = n;

    src = receita->passos;
    while (src != NULL) {
        cozinhar.pilha = push_passo(cozinhar.pilha,
            src->dado.acao, src->dado.ingrediente,
            src->dado.teclas, src->dado.img_passo);
        src = src->prox;
    }

    // aloca e carrega texturas iterando cozinhar.pilha (topo = passo 1)
    cozinhar.texturas_passos    = (Texture2D *)malloc(sizeof(Texture2D) * n);
    cozinhar.texturas_carregadas = (int *)malloc(sizeof(int) * n);
    if (cozinhar.texturas_passos && cozinhar.texturas_carregadas) {
        memset(cozinhar.texturas_carregadas, 0, sizeof(int) * n);
        No *node = cozinhar.pilha;
        int idx = 0;
        while (node != NULL) {
            if (node->dado.img_passo[0] != '\0') {
                cozinhar.texturas_passos[idx] = LoadTexture(node->dado.img_passo);
                if (cozinhar.texturas_passos[idx].id != 0) {
                    SetTextureFilter(cozinhar.texturas_passos[idx], TEXTURE_FILTER_BILINEAR);
                    cozinhar.texturas_carregadas[idx] = 1;
                }
            }
            node = node->prox;
            idx++;
        }
    }

    // carrega textura frigideira pronta (para empratar)
    Texture2D tex_frigideira = LoadTexture("sprites/receitas/frigideira_vazia.png");
    if (tex_frigideira.id != 0) {
        cozinhar.textura_frigideira_pronta = tex_frigideira;
        SetTextureFilter(cozinhar.textura_frigideira_pronta, TEXTURE_FILTER_BILINEAR);
        cozinhar.textura_frigideira_pronta_carregada = 1;
    }

    // carrega textura da receita pronta
    if (receita->img_receita_pronta[0] != '\0') {
        cozinhar.textura_pronta = LoadTexture(receita->img_receita_pronta);
        if (cozinhar.textura_pronta.id != 0) {
            SetTextureFilter(cozinhar.textura_pronta, TEXTURE_FILTER_BILINEAR);
            cozinhar.textura_pronta_carregada = 1;
        }
    }

    // carrega textura inicial da receita
    cozinhar.textura_inicio_carregada = 0;
    if (receita->img_inicio[0] != '\0') {
        cozinhar.textura_inicio = LoadTexture(receita->img_inicio);
        if (cozinhar.textura_inicio.id != 0) {
            SetTextureFilter(cozinhar.textura_inicio, TEXTURE_FILTER_BILINEAR);
            cozinhar.textura_inicio_carregada = 1;
        }
    }

    // carrega sticker de bom trabalho (mainha animada)
    cozinhar.textura_mainha_animada_carregada = 0;
    Texture2D tex_mainha = LoadTexture("sprites/mainhas/mainha_animada.png");
    if (tex_mainha.id != 0) {
        cozinhar.textura_mainha_animada = tex_mainha;
        SetTextureFilter(cozinhar.textura_mainha_animada, TEXTURE_FILTER_BILINEAR);
        cozinhar.textura_mainha_animada_carregada = 1;
    }
    cozinhar.mostrando_mainha = 0;
    cozinhar.timer_mainha = 0.0f;
    
    // carrega sticker de erro (mainha brava)
    cozinhar.textura_mainha_brava_carregada = 0;
    Texture2D tex_brava = LoadTexture("sprites/mainhas/mainha_brava.png");
    if (tex_brava.id != 0) {
        cozinhar.textura_mainha_brava = tex_brava;
        SetTextureFilter(cozinhar.textura_mainha_brava, TEXTURE_FILTER_BILINEAR);
        cozinhar.textura_mainha_brava_carregada = 1;
    }
    cozinhar.mostrando_mainha_brava = 0;
    cozinhar.timer_mainha_brava = 0.0f;
    
    // carrega sprite de vitória (mainha vitória)
    cozinhar.textura_mainha_vitoria_carregada = 0;
    Texture2D tex_vitoria = LoadTexture("sprites/mainhas/mainha_vitoria.png");
    if (tex_vitoria.id != 0) {
        cozinhar.textura_mainha_vitoria = tex_vitoria;
        SetTextureFilter(cozinhar.textura_mainha_vitoria, TEXTURE_FILTER_BILINEAR);
        cozinhar.textura_mainha_vitoria_carregada = 1;
    }

    montar_grid();
    cozinhar.passo_idx = 0;
    cozinhar.fase = COZ_FASE_CLICAR;
    cozinhar.pos_tecla = 0;
    cozinhar.pontos_anterior = 0;
    cozinhar.tempo_passo = 0.0f;
    cozinhar.feedback_timer = 0.0f;
    cozinhar.feedback_acerto = 0;
    cozinhar.erros = 0;
    cozinhar.acertos = 0;
    cozinhar.pontos         = estado.pontuacao;
    estado.passos_total     = n;
    estado.passos_acertados = 0;
    cozinhar.terminou = 0;
    cozinhar.venceu = 0;
    marcar_destacado();

    printf("[COZINHAR] Iniciado para '%s' com %d passos (pilha carregada)\n",
           receita->nome, n);
}

// ==========================================
// LOGICA POR FASE
// ==========================================
static void avancar_passo(int acertou) {
    cozinhar.feedback_acerto = acertou;
    cozinhar.fase = COZ_FASE_FEEDBACK;
    cozinhar.feedback_timer = 0.0f;
    if (acertou) {
        cozinhar.acertos++;
        cozinhar.pontos += 10;
        _avancar_progresso();
    } else {
        cozinhar.erros++;
        cozinhar.pontos -= 5;
        if (cozinhar.pontos < 0) cozinhar.pontos = 0;
    }
}

static void ativar_feedback_erro(void) {
    cozinhar.feedback_acerto = 0;
    cozinhar.fase = COZ_FASE_FEEDBACK;
    cozinhar.feedback_timer = 0.0f;
}

static void fase_clicar(void) {
    cozinhar.tempo_passo += GetFrameTime();

    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;

    Passo *p = &cozinhar.pilha->dado;
    
    // Se é um passo especial (ingrediente vazio), qualquer clique no botão avança
    if (p->ingrediente[0] == '\0') {
        Vector2 m = GetMousePosition();
        if (cozinhar.n_grid > 0 && CheckCollisionPointRec(m, cozinhar.grid[0].area)) {
            cozinhar.fase = COZ_FASE_TECLAS;
            cozinhar.pos_tecla = 0;
        }
        return;
    }

    Vector2 m = GetMousePosition();
    for (int k = 0; k < cozinhar.n_grid; k++) {
        if (CheckCollisionPointRec(m, cozinhar.grid[k].area)) {
            if (cozinhar.grid[k].destacado) {
                // acertou o ingrediente: avanca pra fase de teclas
                cozinhar.fase = COZ_FASE_TECLAS;
                cozinhar.pos_tecla = 0;
            } else {
                cozinhar.erros++;
                cozinhar.pontos -= 2;
                if (cozinhar.pontos < 0) cozinhar.pontos = 0;
                estado.erro_passo++;
                ativar_feedback_erro();
            }
            return;
        }
    }
}

static void fase_teclas(void) {
    Passo *p = &cozinhar.pilha->dado;

    // qualquer tecla pressionada?
    int tecla = GetKeyPressed();
    while (tecla != 0) {
        int esperada = char_para_keycode(p->teclas[cozinhar.pos_tecla]);
        if (tecla == esperada) {
            cozinhar.pos_tecla++;
            if (cozinhar.pos_tecla >= (int)strlen(p->teclas)) {
                // marca ingrediente como usado
                for (int k = 0; k < cozinhar.n_grid; k++) {
                    if (strcmp(cozinhar.grid[k].nome, p->ingrediente) == 0) {
                        cozinhar.grid[k].usado = 1;
                    }
                }
                // Adiciona delay visual para forno
                if (strcmp(p->ingrediente, "Forno") == 0) {
                    cozinhar.delay_forno = 3.0f;  // 3 segundos de delay
                    cozinhar.fase = COZ_FASE_FEEDBACK;
                    cozinhar.feedback_acerto = 1;
                    cozinhar.feedback_timer = 0.0f;
                    cozinhar.acertos++;
                    cozinhar.pontos += 10;
                } else {
                    avancar_passo(1);
                }
                return;
            }
        } else if (tecla != KEY_F11 && tecla != KEY_LEFT_ALT &&
                   tecla != KEY_RIGHT_ALT) {
            // tecla errada zera progresso e penaliza pouco
            cozinhar.pos_tecla = 0;
            cozinhar.erros++;
            cozinhar.pontos -= 1;
            if (cozinhar.pontos < 0) cozinhar.pontos = 0;
            estado.erro_passo++;
            ativar_feedback_erro();
        }
        tecla = GetKeyPressed();
    }
}

static void fase_feedback(void) {
    cozinhar.feedback_timer += GetFrameTime();
    
    // Se está no forno, usa delay maior
    float tempo_minimo = (cozinhar.delay_forno > 0)
                             ? cozinhar.delay_forno
                             : (cozinhar.feedback_acerto ? 1.0f : 0.15f);
    
    if (cozinhar.feedback_timer >= tempo_minimo) {
        if (cozinhar.feedback_acerto) {
            // Ativa sticker de bom trabalho (mainha animada)
            cozinhar.mostrando_mainha = 1;
            cozinhar.timer_mainha = 1.5f;  // 1.5 segundos de exibição
            
            // pop: remove passo concluido do topo da pilha
            cozinhar.pilha = pop_passo(cozinhar.pilha);
            cozinhar.passo_idx++;
            if (pilha_vazia(cozinhar.pilha)) {
                cozinhar.venceu = 1;
                cozinhar.fase = COZ_FASE_EMPRATAR;
                estado.pontuacao = cozinhar.pontos;
                return;
            }
        } else {
            // Ativa sticker de erro (mainha brava)
            cozinhar.mostrando_mainha_brava = 1;
            cozinhar.timer_mainha_brava = 0.9f;  // exibição mais rápida
        }
        // se errou, reinicia o mesmo passo; se acertou, ja avancou o idx acima
        cozinhar.fase = COZ_FASE_CLICAR;
        cozinhar.tempo_passo = 0.0f;
        cozinhar.pos_tecla = 0;
        cozinhar.delay_forno = 0.0f;  // Reset delay
        montar_grid();  // reconstrói grid para o novo passo
        marcar_destacado();
    }
}

// ==========================================
// FASE EMPRATAR
// ==========================================
static void fase_empratar(void) {
    // verifica clique no botão EMPRATAR
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();
        // botão no topo: 300-500, 10-50
        if (mouse.x >= 300 && mouse.x <= 500 && mouse.y >= 10 && mouse.y <= 50) {
            cozinhar.terminou = 1;
            cozinhar.fase = COZ_FASE_FIM;
        }
    }
    
    // fallback: ENTER também funciona
    if (IsKeyPressed(KEY_ENTER)) {
        cozinhar.terminou = 1;
        cozinhar.fase = COZ_FASE_FIM;
    }
}

static void desenhar_empratar(void) {
    ClearBackground(COR_FUNDO_COZ);

    // carrega sob demanda se ainda nao carregou
    if (!cozinhar.textura_pronta_carregada &&
        cozinhar.receita && cozinhar.receita->img_receita_pronta[0] != '\0') {
        cozinhar.textura_pronta = LoadTexture(cozinhar.receita->img_receita_pronta);
        printf("[EMPRATAR] carregando '%s' id=%u\n",
               cozinhar.receita->img_receita_pronta, cozinhar.textura_pronta.id);
        if (cozinhar.textura_pronta.id != 0)
            cozinhar.textura_pronta_carregada = 1;
    }

    Texture2D tex = {0};
    int tem_tex = 0;
    if (cozinhar.textura_pronta_carregada) {
        tex = cozinhar.textura_pronta; tem_tex = 1;
    } else if (cozinhar.textura_frigideira_pronta_carregada) {
        tex = cozinhar.textura_frigideira_pronta; tem_tex = 1;
    }
    if (tem_tex && tex.width > 0 && tex.height > 0) {
        float max_altura = (float)(ALT - 100);
        float escala_altura = max_altura / (float)tex.height;
        float escala_largura = (float)(LARG - 40) / (float)tex.width;
        float escala = (escala_altura < escala_largura) ? escala_altura : escala_largura;
        float largura = (float)tex.width * escala;
        float altura = (float)tex.height * escala;
        float x = ((float)LARG - largura) / 2.0f;
        float y = 80;
        DrawTextureEx(tex, (Vector2){x, y}, 0, escala, WHITE);
    }
    
    // desenha botão EMPRATAR no topo
    Rectangle btn_empratar = {300, 10, 200, 40};
    DrawRectangleRec(btn_empratar, COR_VERDE_COZ);
        DrawRectangleRoundedLines(btn_empratar, 0.1f, 10, 2.0f, COR_AMA_COZ);
    
    int txt_width = medir_txt("EMPRATAR", 24);
    txt("EMPRATAR", 400 - txt_width/2, 18, 24, WHITE);
}

// ==========================================
// DESENHO - FIM
// ==========================================
static void desenhar_cabecalho(void) {
    DrawRectangle(0, 0, LARG, 60, COR_AZUL_COZ);
    DrawRectangle(0, 55, LARG, 5, COR_AMA_COZ);
    txt(TextFormat("COZINHANDO: %s", cozinhar.receita->nome),
             40, 16, 24, WHITE);

    txt(TextFormat("Passo %d / %d",
                        cozinhar.passo_idx + 1,
                        cozinhar.n_passos),
             580, 20, 22, WHITE);

    // pontos
    DrawRectangle(0, 540, LARG, 60, (Color){30, 60, 140, 255});
    txt(TextFormat("Pontos: %d", cozinhar.pontos),
             40, 558, 22, COR_AMA_COZ);
    txt(TextFormat("Acertos: %d  Erros: %d",
                        cozinhar.acertos, cozinhar.erros),
             280, 558, 20, WHITE);
}

static void desenhar_instrucao(void) {
    if (pilha_vazia(cozinhar.pilha)) return;
    Passo *p = &cozinhar.pilha->dado;

    // balao de instrucao no topo
    DrawRectangleRounded((Rectangle){40, 70, 720, 90}, 0.2f, 8, WHITE);
        DrawRectangleRoundedLines((Rectangle){40, 70, 720, 90}, 0.2f, 8, 2.0f,
                                  COR_AZUL_COZ);
    txt("Instrucao:", 60, 80, 16, COR_AZUL_COZ);
    txt(p->acao, 60, 98, 18, COR_TEXTO_COZ);

    if (cozinhar.fase == COZ_FASE_CLICAR) {
        txt(TextFormat("1) Clique: %s", p->ingrediente),
                 60, 120, 14, COR_LARA_COZ);
    } else if (cozinhar.fase == COZ_FASE_TECLAS) {
        txt("2) Teclas:",
                 60, 120, 14, COR_VERDE_COZ);
    }

    // desenha mainha animada no canto direito do balao de instrucao
    // NÃO desenha aqui se a reação central (mostrando_mainha/mostrando_mainha_brava) estiver ativa,
    // para evitar duplicação da mesma sprite.
    if (!cozinhar.mostrando_mainha && !cozinhar.mostrando_mainha_brava && cozinhar.textura_mainha_animada_carregada) {
        Texture2D tex = cozinhar.textura_mainha_animada;
        // escala para caber no balao (altura max ~70)
        float max_h = 70.0f;
        float sc = 1.0f;
        if (tex.height > 0) sc = (max_h / (float)tex.height);
        if (sc > 1.0f) sc = 1.0f;
        float draw_w = tex.width * sc;
        float draw_h = tex.height * sc;
        Rectangle src = (Rectangle){0.0f, 0.0f, (float)tex.width, (float)tex.height};
        Rectangle dst = (Rectangle){ 40 + 720 - 10 - draw_w, 70 + (90 - draw_h) / 2.0f, draw_w, draw_h };
        DrawTexturePro(tex, src, dst, (Vector2){0,0}, 0.0f, WHITE);
    }

}

static void desenhar_sequencia(void) {
    if (cozinhar.fase != COZ_FASE_TECLAS) return;
    if (pilha_vazia(cozinhar.pilha)) return;
    Passo *p = &cozinhar.pilha->dado;
    int len = (int)strlen(p->teclas);
    if (len == 0) return;

    // calcula largura dos blocos
    int box = 50;
    int gap = 8;
    int total = len * box + (len - 1) * gap;
    int x0 = (LARG - total) / 2;
    int y0 = 175;  // no topo, abaixo das instruções

    for (int k = 0; k < len; k++) {
        int x = x0 + k * (box + gap);
        Color fundo, borda;
        if (k < cozinhar.pos_tecla) {
            fundo = COR_VERDE_COZ;
            borda = (Color){30, 90, 40, 255};
        } else if (k == cozinhar.pos_tecla) {
            // pulsante
            float t = (float)GetTime();
            int pulso = (int)(((t - (int)t) * 255));
            if (pulso > 255) pulso = 510 - pulso;
            fundo = COR_AMA_COZ;
            fundo.a = (unsigned char)(180 + pulso / 4);
            borda = COR_LARA_COZ;
        } else {
            fundo = (Color){240, 240, 240, 255};
            borda = (Color){180, 180, 180, 255};
        }
        DrawRectangleRounded((Rectangle){x, y0, box, box}, 0.25f, 8, fundo);
        DrawRectangleRoundedLines((Rectangle){x, y0, box, box}, 0.25f, 8,
                      2.0f, borda);
        const char *nome = nome_tecla(p->teclas[k]);
        int tam = (strlen(nome) > 1) ? 18 : 32;
        int tw = medir_txt(nome, tam);
        txt(nome, x + (box - tw) / 2, y0 + (box - tam) / 2, tam,
                 COR_TEXTO_COZ);
    }
}

static void desenhar_grid(void) {
    for (int k = 0; k < cozinhar.n_grid; k++) {
        IngredienteIcone *it = &cozinhar.grid[k];
        Color fundo;
        if (it->usado)            fundo = (Color){200, 230, 200, 255};
        else if (it->destacado)   fundo = COR_AMA_COZ;
        else                       fundo = WHITE;

        DrawRectangleRounded(it->area, 0.25f, 8, fundo);
        DrawRectangleRoundedLines(it->area, 0.25f, 8,
                                      2.0f, it->destacado ? COR_LARA_COZ
                                                    : (Color){180,180,180,255});
        if (it->destacado && cozinhar.fase == COZ_FASE_CLICAR) {
            // contorno externo para chamar atencao
            DrawRectangleRoundedLines(
                (Rectangle){ it->area.x - 3, it->area.y - 3,
                             it->area.width + 6, it->area.height + 6 },
                0.25f, 8, 2.0f, COR_LARA_COZ);
        }

        // desenhar sprite do ingrediente se carregado
        if (it->sprite_carregado) {
            Texture2D t = it->sprite;
            float pad = 8;
            float avail_w = it->area.width - pad*2;
            float avail_h = it->area.height - pad*2;
            float scale_w = avail_w / t.width;
            float scale_h = avail_h / t.height;
            float scale = scale_w < scale_h ? scale_w : scale_h;
            if (scale > 1.25f) scale = 1.25f;
            float tx = it->area.x + (it->area.width - t.width*scale)/2.0f;
            float ty = it->area.y + (it->area.height - t.height*scale)/2.0f;
            DrawTextureEx(t, (Vector2){tx, ty}, 0.0f, scale, WHITE);
        } else {
            int tw = medir_txt(it->nome, 16);
            if (tw > (int)it->area.width - 14) {
                txt(it->nome, (int)(it->area.x + 8),
                         (int)(it->area.y + it->area.height / 2 - 8), 14,
                         COR_TEXTO_COZ);
            } else {
                txt(it->nome,
                         (int)(it->area.x + (it->area.width - tw) / 2),
                         (int)(it->area.y + it->area.height / 2 - 8),
                         16, COR_TEXTO_COZ);
            }
        }

        if (it->usado) {
            txt("ok", (int)(it->area.x + it->area.width - 30),
                     (int)(it->area.y + 6), 16, COR_VERDE_COZ);
        }
    }
}

static void desenhar_feedback(void) {
    if (cozinhar.fase != COZ_FASE_FEEDBACK) return;
    (void)cozinhar;
}

static void desenhar_imagem_passo(void) {
    // Mostra imagem inicial no primeiro passo
    if (cozinhar.passo_idx == 0 && cozinhar.textura_inicio_carregada) {
        Texture2D tex = cozinhar.textura_inicio;

        float max_altura = ALT - 100 - 80;
        float escala_altura = max_altura / tex.height;
        float max_largura = LARG;
        float escala_largura = max_largura / tex.width;
        float escala = escala_altura < escala_largura ? escala_altura : escala_largura;

        float largura = tex.width * escala;
        float altura = tex.height * escala;
        float x = (LARG - largura) / 2;
        float y = 80;
        DrawTextureEx(tex, (Vector2){x, y}, 0, escala, WHITE);
        return;
    }

    if (pilha_vazia(cozinhar.pilha)) return;
    if (cozinhar.texturas_passos == NULL || cozinhar.texturas_carregadas == NULL) return;
    
    if (cozinhar.texturas_carregadas[cozinhar.passo_idx]) {
        Texture2D tex = cozinhar.texturas_passos[cozinhar.passo_idx];
        
        // calcula escala para ocupar quase toda a tela mantendo aspecto ratio
        // deixa espaço de ~100px no topo e ~80px na base para elementos da UI
        float max_altura = ALT - 100 - 80;  // 420px disponíveis
        float escala_altura = max_altura / tex.height;
        float max_largura = LARG;
        float escala_largura = max_largura / tex.width;
        float escala = escala_altura < escala_largura ? escala_altura : escala_largura;
        
        float largura = tex.width * escala;
        float altura = tex.height * escala;
        float x = (LARG - largura) / 2;
        float y = 80;  // começa logo após o cabeçalho
        DrawTextureEx(tex, (Vector2){x, y}, 0, escala, WHITE);
    }
}

static void desenhar_fim(void) {
    // imagem pronta ocupa a tela toda
    if (cozinhar.venceu && cozinhar.textura_pronta_carregada) {
        Texture2D tex_pronta = cozinhar.textura_pronta;
        float escala_h = (float)ALT / (float)tex_pronta.height;
        float escala_w = (float)LARG / (float)tex_pronta.width;
        float escala = (escala_h < escala_w) ? escala_h : escala_w;
        float largura = (float)tex_pronta.width  * escala;
        float altura  = (float)tex_pronta.height * escala;
        float x = ((float)LARG - largura) / 2.0f;
        float y = ((float)ALT  - altura)  / 2.0f;
        DrawTextureEx(tex_pronta, (Vector2){x, y}, 0, escala, WHITE);
    }

    // sprite mainha vitoria no canto inferior esquerdo (acima da barra)
    if (cozinhar.textura_mainha_vitoria_carregada) {
        Texture2D tex = cozinhar.textura_mainha_vitoria;
        // Desenha grande e centralizada na tela de fim
        float base_h = (float)ALT * 0.6f;
        float sc = 1.0f;
        if (tex.height > 0) sc = base_h / (float)tex.height;
        if (sc > 1.0f) sc = 1.0f;
        float larg = tex.width  * sc;
        float alt  = tex.height * sc;
        Rectangle src = (Rectangle){0.0f, 0.0f, (float)tex.width, (float)tex.height};
        Rectangle dst = (Rectangle){ (LARG - larg) / 2.0f, (ALT - alt) / 2.0f, larg, alt };
        DrawTexturePro(tex, src, dst, (Vector2){0,0}, 0.0f, WHITE);
    }

    // barra superior com estatísticas
    DrawRectangle(0, 0, LARG, 52, (Color){0, 0, 0, 170});
    txt(TextFormat("Acertos: %d   Erros: %d   Pontos: %d",
                   cozinhar.acertos, cozinhar.erros, cozinhar.pontos),
        40, 14, 22, WHITE);

}

// ==========================================
// DESENHA STICKER DE BOM TRABALHO
// ==========================================
static void desenhar_mainha_animada(void) {
    if (!cozinhar.mostrando_mainha || cozinhar.timer_mainha <= 0.0f) {
        cozinhar.mostrando_mainha = 0;
        return;
    }

    if (!cozinhar.textura_mainha_animada_carregada) return;

    Texture2D tex = cozinhar.textura_mainha_animada;
    
    // Anima tamanho e opacidade com base no tempo restante
    // No início (1.5s): tamanho 1.0, opacidade 1.0
    // No final (0s): tamanho 0.7, opacidade 0.3
    float progresso = cozinhar.timer_mainha / 1.5f;  // 0 a 1
    float escala = 0.7f + (progresso * 0.3f);       // 0.7 a 1.0
    float opacidade = 0.3f + (progresso * 0.7f);    // 0.3 a 1.0 
    
    Color cor = (Color){255, 255, 255, (unsigned char)(opacidade * 255)};
    
    // Desenha grande e centralizada (reação)
    float base_h = (float)ALT * 0.6f; // ocupa até 60% da altura
    float desired_h = base_h * escala; // anima tamanho com o progresso
    float sc = 1.0f;
    if (tex.height > 0) sc = desired_h / (float)tex.height;
    float draw_w = tex.width * sc;
    float draw_h = tex.height * sc;
    float x = ((float)LARG - draw_w) / 2.0f;
    float y = ((float)ALT  - draw_h) / 2.0f;
    Rectangle src = (Rectangle){0.0f, 0.0f, (float)tex.width, (float)tex.height};
    Rectangle dst = (Rectangle){ x, y, draw_w, draw_h };
    DrawTexturePro(tex, src, dst, (Vector2){0,0}, 0.0f, cor);
}

// ==========================================
// DESENHA STICKER DE ERRO
// ==========================================
static void desenhar_mainha_brava(void) {
    if (!cozinhar.mostrando_mainha_brava || cozinhar.timer_mainha_brava <= 0.0f) {
        cozinhar.mostrando_mainha_brava = 0;
        return;
    }

    if (!cozinhar.textura_mainha_brava_carregada) {
        return;
    }

    Texture2D tex = cozinhar.textura_mainha_brava;
    
    // Anima tamanho e opacidade com base no tempo restante
    // Mesmo efeito que mainha_animada
    float progresso = cozinhar.timer_mainha_brava / 0.9f;   // 0 a 1
    float escala = 0.7f + (progresso * 0.3f);             // 0.7 a 1.0
    float opacidade = 0.3f + (progresso * 0.7f);          // 0.3 a 1.0 
    
    Color cor = (Color){255, 255, 255, (unsigned char)(opacidade * 255)};
    
    // Desenha grande e centralizada (reação de erro)
    float base_h = (float)ALT * 0.55f; // um pouco menor que a animada
    float desired_h = base_h * escala; // anima tamanho com o progresso
    float sc = 1.0f;
    if (tex.height > 0) sc = desired_h / (float)tex.height;
    float draw_w = tex.width * sc;
    float draw_h = tex.height * sc;
    float x = ((float)LARG - draw_w) / 2.0f;
    float y = ((float)ALT  - draw_h) / 2.0f;
    Rectangle src = (Rectangle){0.0f, 0.0f, (float)tex.width, (float)tex.height};
    Rectangle dst = (Rectangle){ x, y, draw_w, draw_h };
    DrawTexturePro(tex, src, dst, (Vector2){0,0}, 0.0f, cor);
}

void tela_cozinhar(void) {
    if (cozinhar.receita == NULL) {
        ClearBackground(COR_FUNDO_COZ);
        txt("Nenhuma receita carregada.", 220, 280, 22, COR_VERM_COZ);
        return;
    }

    // Gerencia tempo dos stickers
    if (cozinhar.mostrando_mainha && cozinhar.timer_mainha > 0.0f) {
        cozinhar.timer_mainha -= GetFrameTime();
        if (cozinhar.timer_mainha <= 0.0f) {
            cozinhar.mostrando_mainha = 0;
        }
    }
    if (cozinhar.mostrando_mainha_brava && cozinhar.timer_mainha_brava > 0.0f) {
        cozinhar.timer_mainha_brava -= GetFrameTime();
        if (cozinhar.timer_mainha_brava <= 0.0f) {
            cozinhar.mostrando_mainha_brava = 0;
        }
    }

    // Sincroniza pontos_anterior sem ativar reações automáticas aqui
    cozinhar.pontos_anterior = cozinhar.pontos;

    if (!cozinhar.terminou) {
        switch (cozinhar.fase) {
            case COZ_FASE_CLICAR:    fase_clicar();    break;
            case COZ_FASE_TECLAS:    fase_teclas();    break;
            case COZ_FASE_FEEDBACK:  fase_feedback();  break;
            case COZ_FASE_EMPRATAR:  fase_empratar();  break;
            default: break;
        }
    }

    if (cozinhar.fase == COZ_FASE_EMPRATAR) {
        desenhar_empratar();
        return;
    }

    if (cozinhar.fase == COZ_FASE_FIM) {
        ClearBackground(COR_FUNDO_COZ);
        desenhar_fim();
        return;
    }

    ClearBackground(COR_FUNDO_COZ);
    desenhar_cabecalho();
    desenhar_imagem_passo();
    desenhar_grid();
    desenhar_instrucao();
    desenhar_sequencia();
    desenhar_feedback();
    desenhar_mainha_animada();  // Renderiza sticker de bom trabalho
    desenhar_mainha_brava();    // Renderiza sticker de erro
}

int cozinhar_terminou(void) { return cozinhar.terminou; }
int cozinhar_venceu(void)   { return cozinhar.venceu; }

void cozinhar_limpar(void) {
    // libera texturas
    if (cozinhar.texturas_passos != NULL) {
        for (int i = 0; i < cozinhar.n_passos; i++) {
            if (cozinhar.texturas_carregadas[i])
                UnloadTexture(cozinhar.texturas_passos[i]);
        }
        free(cozinhar.texturas_passos);
        free(cozinhar.texturas_carregadas);
        cozinhar.texturas_passos = NULL;
        cozinhar.texturas_carregadas = NULL;
    }
    
    // libera textura pronta
    if (cozinhar.textura_pronta_carregada) {
        UnloadTexture(cozinhar.textura_pronta);
        cozinhar.textura_pronta_carregada = 0;
    }
    
    // libera textura inicial
    if (cozinhar.textura_inicio_carregada) {
        UnloadTexture(cozinhar.textura_inicio);
        cozinhar.textura_inicio_carregada = 0;
    }
    
    // libera textura frigideira pronta
    if (cozinhar.textura_frigideira_pronta_carregada) {
        UnloadTexture(cozinhar.textura_frigideira_pronta);
        cozinhar.textura_frigideira_pronta_carregada = 0;
    }
    
    // libera textura mainha animada (sticker de bom trabalho)
    if (cozinhar.textura_mainha_animada_carregada) {
        UnloadTexture(cozinhar.textura_mainha_animada);
        cozinhar.textura_mainha_animada_carregada = 0;
    }
    
    // libera textura mainha brava (sticker de erro)
    if (cozinhar.textura_mainha_brava_carregada) {
        UnloadTexture(cozinhar.textura_mainha_brava);
        cozinhar.textura_mainha_brava_carregada = 0;
    }
    
    // libera textura mainha vitória
    if (cozinhar.textura_mainha_vitoria_carregada) {
        UnloadTexture(cozinhar.textura_mainha_vitoria);
        cozinhar.textura_mainha_vitoria_carregada = 0;
    }
    
    // libera pilha
    while (!pilha_vazia(cozinhar.pilha))
        cozinhar.pilha = pop_passo(cozinhar.pilha);

    // libera sprites do grid
    for (int ii = 0; ii < COZ_MAX_ING_GRID; ii++) {
        if (cozinhar.grid[ii].sprite_carregado) {
            UnloadTexture(cozinhar.grid[ii].sprite);
            cozinhar.grid[ii].sprite_carregado = 0;
        }
    }
}
