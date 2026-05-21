#include <stdio.h>
#include <string.h>
#include <ctype.h>
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
    Ingrediente *ing = cozinhar.receita->ingredientes;
    int i = 0;
    while (ing != NULL && i < COZ_MAX_ING_GRID) {
        strncpy(cozinhar.grid[i].nome, ing->nome,
                sizeof(cozinhar.grid[i].nome) - 1);
        cozinhar.grid[i].nome[sizeof(cozinhar.grid[i].nome) - 1] = '\0';
        cozinhar.grid[i].destacado = 0;
        cozinhar.grid[i].usado = 0;

        // grid 4x2 abaixo do balao
        int col = i % 4;
        int row = i / 4;
        float w = 160;
        float h = 80;
        float gx = 60 + col * (w + 20);
        float gy = 350 + row * (h + 16);
        cozinhar.grid[i].area = (Rectangle){ gx, gy, w, h };

        ing = ing->prox;
        i++;
    }
    cozinhar.n_grid = i;
}

static void marcar_destacado(void) {
    if (cozinhar.passo_idx >= cozinhar.receita->n_passos_jog) return;
    const char *alvo = cozinhar.receita->passos_jog[cozinhar.passo_idx].ingrediente;
    for (int k = 0; k < cozinhar.n_grid; k++) {
        cozinhar.grid[k].destacado = (strcmp(cozinhar.grid[k].nome, alvo) == 0);
    }
}

void cozinhar_iniciar(Receita *receita) {
    // libera pilha anterior se existir
    while (!pilha_vazia(cozinhar.pilha))
        cozinhar.pilha = pop_passo(cozinhar.pilha);

    memset(&cozinhar, 0, sizeof(cozinhar));
    cozinhar.receita = receita;
    if (receita == NULL || receita->n_passos_jog == 0) {
        cozinhar.terminou = 1;
        cozinhar.venceu = 0;
        return;
    }

    // empilha os passos em ordem reversa: ultimo passo entra primeiro,
    // assim o topo da pilha (LIFO) corresponde ao primeiro passo da receita
    int i = receita->n_passos_jog - 1;
    while (i >= 0) {
        cozinhar.pilha = push_passo(cozinhar.pilha,
                                    receita->passos_jog[i].acao,
                                    receita->passos_jog[i].ingrediente);
        i--;
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
    estado.passos_total     = receita->n_passos_jog;
    estado.passos_acertados = 0;
    cozinhar.terminou = 0;
    cozinhar.venceu = 0;
    marcar_destacado();

    printf("[COZINHAR] Iniciado para '%s' com %d passos (pilha carregada)\n",
           receita->nome, receita->n_passos_jog);
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
    PassoJogavel *p = &cozinhar.receita->passos_jog[cozinhar.passo_idx];

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
                cozinhar.terminou = 1;
                cozinhar.venceu = 1;
                cozinhar.fase = COZ_FASE_FIM;
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
// DESENHO
// ==========================================
static void desenhar_cabecalho(void) {
    DrawRectangle(0, 0, LARG, 60, COR_AZUL_COZ);
    DrawRectangle(0, 55, LARG, 5, COR_AMA_COZ);
    DrawText(TextFormat("COZINHANDO: %s", cozinhar.receita->nome),
             40, 16, 24, WHITE);

    DrawText(TextFormat("Passo %d / %d",
                        cozinhar.passo_idx + 1,
                        cozinhar.receita->n_passos_jog),
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
    if (cozinhar.passo_idx >= cozinhar.receita->n_passos_jog) return;
    PassoJogavel *p = &cozinhar.receita->passos_jog[cozinhar.passo_idx];

    // balao de instrucao
    DrawRectangleRounded((Rectangle){40, 80, 720, 110}, 0.2f, 8, WHITE);
    DrawRectangleRoundedLinesEx((Rectangle){40, 80, 720, 110}, 0.2f, 8, 2.0f,
                              COR_AZUL_COZ);
    DrawText("Instrucao:", 60, 92, 18, COR_AZUL_COZ);
    DrawText(p->acao, 60, 116, 22, COR_TEXTO_COZ);

    if (cozinhar.fase == COZ_FASE_CLICAR) {
        DrawText(TextFormat("1) Clique no ingrediente: %s", p->ingrediente),
                 60, 150, 18, COR_LARA_COZ);
    } else if (cozinhar.fase == COZ_FASE_TECLAS) {
        DrawText("2) Digite a sequencia de teclas:",
                 60, 150, 18, COR_VERDE_COZ);
    }

}

static void desenhar_sequencia(void) {
    if (cozinhar.fase != COZ_FASE_TECLAS) return;
    PassoJogavel *p = &cozinhar.receita->passos_jog[cozinhar.passo_idx];
    int len = (int)strlen(p->teclas);
    if (len == 0) return;

    // calcula largura dos blocos
    int box = 60;
    int gap = 12;
    int total = len * box + (len - 1) * gap;
    int x0 = (LARG - total) / 2;
    int y0 = 240;

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
        DrawRectangleRoundedLinesEx((Rectangle){x, y0, box, box}, 0.25f, 8, 2.0f,
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
        DrawRectangleRoundedLinesEx(it->area, 0.25f, 8, 2.0f,
                                  it->destacado ? COR_LARA_COZ
                                                : (Color){180,180,180,255});
        if (it->destacado && cozinhar.fase == COZ_FASE_CLICAR) {
            // contorno externo para chamar atencao
            DrawRectangleRoundedLinesEx(
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

static void desenhar_fim(void) {
    DrawRectangle(0, 0, LARG, ALT, (Color){0, 0, 0, 170});
    const char *titulo = cozinhar.venceu ? "RECEITA PRONTA!" : "FIM DE COZINHA";
    Color cor = cozinhar.venceu ? COR_VERDE_COZ : COR_VERM_COZ;
    int tam = 56;
    int tw = MeasureText(titulo, tam);
    DrawText(titulo, (LARG - tw) / 2, 180, tam, cor);

    DrawText(TextFormat("Acertos: %d  Erros: %d",
                        cozinhar.acertos, cozinhar.erros),
             280, 270, 22, WHITE);
    DrawText(TextFormat("Pontos finais: %d", cozinhar.pontos),
             290, 310, 22, COR_AMA_COZ);

    DrawText("[ENTER] Resultado final", 270, 420, 22, COR_AMA_COZ);
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
            default: break;
        }
    }

    ClearBackground(COR_FUNDO_COZ);
    desenhar_cabecalho();
    desenhar_instrucao();
    desenhar_sequencia();
    desenhar_grid();
    desenhar_feedback();
    if (cozinhar.terminou) desenhar_fim();
}

int cozinhar_terminou(void) { return cozinhar.terminou; }
int cozinhar_venceu(void)   { return cozinhar.venceu; }
