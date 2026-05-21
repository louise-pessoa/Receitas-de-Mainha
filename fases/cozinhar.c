#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "raylib.h"
#include "cozinhar.h"
#include "../jogo.h"
#include "../dados/pilha.h"

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
    if (c == ' ' || c == '_') return "SPACE";
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
    cozinhar.n_grid = 0;
    Receita *r = cozinhar.receita;
    int total = r->n_ingredientes < COZ_MAX_ING_GRID ? r->n_ingredientes : COZ_MAX_ING_GRID;
    for (int i = 0; i < total; i++) {
        strncpy(cozinhar.grid[i].nome, r->ingredientes[i],
                sizeof(cozinhar.grid[i].nome) - 1);
        cozinhar.grid[i].nome[sizeof(cozinhar.grid[i].nome) - 1] = '\0';
        cozinhar.grid[i].destacado = 0;
        cozinhar.grid[i].usado = 0;

        int col = i % 5;
        float w = 125;
        float h = 65;
        float gx = 20 + col * (w + 8);
        float gy = 488;
        cozinhar.grid[i].area = (Rectangle){ gx, gy, w, h };
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
    Texture2D tex_frigideira = LoadTexture("sprites/frigideira_pronta.png");
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

    montar_grid();
    cozinhar.passo_idx = 0;
    cozinhar.fase = COZ_FASE_CLICAR;
    cozinhar.pos_tecla = 0;
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

static void fase_clicar(void) {
    cozinhar.tempo_passo += GetFrameTime();

    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;

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
                avancar_passo(1);
                return;
            }
        } else if (tecla != KEY_F11 && tecla != KEY_LEFT_ALT &&
                   tecla != KEY_RIGHT_ALT) {
            // tecla errada zera progresso e penaliza pouco
            cozinhar.pos_tecla = 0;
            cozinhar.pontos -= 1;
            if (cozinhar.pontos < 0) cozinhar.pontos = 0;
        }
        tecla = GetKeyPressed();
    }
}

static void fase_feedback(void) {
    cozinhar.feedback_timer += GetFrameTime();
    if (cozinhar.feedback_timer >= 1.0f) {
        if (cozinhar.feedback_acerto) {
            // pop: remove passo concluido do topo da pilha
            cozinhar.pilha = pop_passo(cozinhar.pilha);
            cozinhar.passo_idx++;
            if (pilha_vazia(cozinhar.pilha)) {
                cozinhar.venceu = 1;
                cozinhar.fase = COZ_FASE_EMPRATAR;
                estado.pontuacao = cozinhar.pontos;
                return;
            }
        }
        // se errou, reinicia o mesmo passo; se acertou, ja avancou o idx acima
        cozinhar.fase = COZ_FASE_CLICAR;
        cozinhar.tempo_passo = 0.0f;
        cozinhar.pos_tecla = 0;
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
    
    // desenha frigideira pronta
    if (cozinhar.textura_frigideira_pronta_carregada) {
        Texture2D tex = cozinhar.textura_frigideira_pronta;
        float max_altura = ALT - 100;
        float escala_altura = max_altura / tex.height;
        float escala_largura = (LARG - 40) / tex.width;
        float escala = (escala_altura < escala_largura) ? escala_altura : escala_largura;
        
        float largura = tex.width * escala;
        float altura = tex.height * escala;
        float x = (LARG - largura) / 2;
        float y = 80;
        DrawTextureEx(tex, (Vector2){x, y}, 0, escala, WHITE);
    }
    
    // desenha botão EMPRATAR no topo
    Rectangle btn_empratar = {300, 10, 200, 40};
    DrawRectangleRec(btn_empratar, COR_VERDE_COZ);
    DrawRectangleRoundedLines(btn_empratar, 0.1f, 10, 2, COR_AMA_COZ);
    
    int txt_width = MeasureText("EMPRATAR", 24);
    DrawText("EMPRATAR", 400 - txt_width/2, 18, 24, WHITE);
}

// ==========================================
// DESENHO - FIM
// ==========================================
static void desenhar_cabecalho(void) {
    DrawRectangle(0, 0, LARG, 60, COR_AZUL_COZ);
    DrawRectangle(0, 55, LARG, 5, COR_AMA_COZ);
    DrawText(TextFormat("COZINHANDO: %s", cozinhar.receita->nome),
             40, 16, 24, WHITE);

    DrawText(TextFormat("Passo %d / %d",
                        cozinhar.passo_idx + 1,
                        cozinhar.n_passos),
             580, 20, 22, WHITE);

    // pontos
    DrawRectangle(0, 540, LARG, 60, (Color){30, 60, 140, 255});
    DrawText(TextFormat("Pontos: %d", cozinhar.pontos),
             40, 558, 22, COR_AMA_COZ);
    DrawText(TextFormat("Acertos: %d  Erros: %d",
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
    DrawText("Instrucao:", 60, 80, 16, COR_AZUL_COZ);
    DrawText(p->acao, 60, 98, 18, COR_TEXTO_COZ);

    if (cozinhar.fase == COZ_FASE_CLICAR) {
        DrawText(TextFormat("1) Clique: %s", p->ingrediente),
                 60, 120, 14, COR_LARA_COZ);
    } else if (cozinhar.fase == COZ_FASE_TECLAS) {
        DrawText("2) Teclas:",
                 60, 120, 14, COR_VERDE_COZ);
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
        DrawRectangleRoundedLines((Rectangle){x, y0, box, box}, 0.25f, 8, 2.0f,
                                  borda);
        const char *nome = nome_tecla(p->teclas[k]);
        int tam = (strlen(nome) > 1) ? 18 : 32;
        int tw = MeasureText(nome, tam);
        DrawText(nome, x + (box - tw) / 2, y0 + (box - tam) / 2, tam,
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
        DrawRectangleRoundedLines(it->area, 0.25f, 8, 2.0f,
                                  it->destacado ? COR_LARA_COZ
                                                : (Color){180,180,180,255});
        if (it->destacado && cozinhar.fase == COZ_FASE_CLICAR) {
            // contorno externo para chamar atencao
            DrawRectangleRoundedLines(
                (Rectangle){ it->area.x - 3, it->area.y - 3,
                             it->area.width + 6, it->area.height + 6 },
                0.25f, 8, 2.0f, COR_LARA_COZ);
        }

        int tw = MeasureText(it->nome, 16);
        if (tw > (int)it->area.width - 14) {
            DrawText(it->nome, (int)(it->area.x + 8),
                     (int)(it->area.y + it->area.height / 2 - 8), 14,
                     COR_TEXTO_COZ);
        } else {
            DrawText(it->nome,
                     (int)(it->area.x + (it->area.width - tw) / 2),
                     (int)(it->area.y + it->area.height / 2 - 8),
                     16, COR_TEXTO_COZ);
        }

        if (it->usado) {
            DrawText("ok", (int)(it->area.x + it->area.width - 30),
                     (int)(it->area.y + 6), 16, COR_VERDE_COZ);
        }
    }
}

static void desenhar_feedback(void) {
    if (cozinhar.fase != COZ_FASE_FEEDBACK) return;
    Color cor = cozinhar.feedback_acerto ? COR_VERDE_COZ : COR_VERM_COZ;
    const char *txt = cozinhar.feedback_acerto ? "ACERTOU!" : "PERDEU O TEMPO!";
    int tam = 56;
    int tw = MeasureText(txt, tam);
    DrawRectangle(0, 220, LARG, 100, (Color){0, 0, 0, 80});
    DrawText(txt, (LARG - tw) / 2, 240, tam, cor);
}

static void desenhar_imagem_passo(void) {
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
    // desenha a imagem pronta (pré-carregada)
    if (cozinhar.venceu && cozinhar.textura_pronta_carregada) {
        Texture2D tex_pronta = cozinhar.textura_pronta;
        float max_altura = ALT - 150;
        float escala_altura = max_altura / tex_pronta.height;
        float escala_largura = (LARG - 40) / tex_pronta.width;
        float escala = (escala_altura < escala_largura) ? escala_altura : escala_largura;
        
        float largura = tex_pronta.width * escala;
        float altura = tex_pronta.height * escala;
        float x = (LARG - largura) / 2;
        float y = 20;
        DrawTextureEx(tex_pronta, (Vector2){x, y}, 0, escala, WHITE);
    }
    
    // pontuação embaixo da imagem
    DrawText(TextFormat("Acertos: %d  Erros: %d",
                        cozinhar.acertos, cozinhar.erros),
             280, 470, 22, WHITE);
    DrawText(TextFormat("Pontos finais: %d", cozinhar.pontos),
             290, 510, 22, COR_AMA_COZ);

    DrawText("[ENTER] Resultado final", 270, 550, 22, COR_AMA_COZ);
}

void tela_cozinhar(void) {
    if (cozinhar.receita == NULL) {
        ClearBackground(COR_FUNDO_COZ);
        DrawText("Nenhuma receita carregada.", 220, 280, 22, COR_VERM_COZ);
        return;
    }

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
    
    // libera textura frigideira pronta
    if (cozinhar.textura_frigideira_pronta_carregada) {
        UnloadTexture(cozinhar.textura_frigideira_pronta);
        cozinhar.textura_frigideira_pronta_carregada = 0;
    }
    
    // libera pilha
    while (!pilha_vazia(cozinhar.pilha))
        cozinhar.pilha = pop_passo(cozinhar.pilha);
}
