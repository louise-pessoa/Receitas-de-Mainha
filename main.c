#include <stdio.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include "raylib.h"
#include "api/jurados.h"
#include "jogo.h"
#include "ui/telas.h"
#include "dados/receitas.h"
#include "fases/coleta.h"
#include "fases/ordenacao.h"
#include "fases/cozinhar.h"


#define LARG_VIRTUAL 800
#define ALT_VIRTUAL  600

static void alternar_fullscreen(void) {
    int monitor = GetCurrentMonitor();
    int mw = GetMonitorWidth(monitor);
    int mh = GetMonitorHeight(monitor);

    if (!IsWindowFullscreen()) {
        SetWindowSize(mw, mh);
        ToggleFullscreen();
    } else {
        ToggleFullscreen();
        SetWindowSize(LARG_VIRTUAL, ALT_VIRTUAL);
        SetWindowPosition((mw - LARG_VIRTUAL) / 2, (mh - ALT_VIRTUAL) / 2);
    }
}

static void atualizar_selecao_receitas(void) {
    if (tela_atual != TELA_RECEITAS) return;

    // contar total de receitas DESBLOQUEADAS
    int total = 0;
    { Receita *r = receitas_disponiveis; while (r) { if (r->desbloqueada) total++; r = r->prox; } }
    if (total == 0) return;

    // descobrir indice atual entre desbloqueadas (1-based); 0 = nenhuma
    int idx = 0;
    { int k = 1; Receita *r = receitas_disponiveis;
      while (r) { if (r->desbloqueada && r == receita_selecionada) { idx = k; break; } 
                  if (r->desbloqueada) k++; 
                  r = r->prox; } }
    if (idx == 0) {
        // seleciona primeira desbloqueada se nada selecionado
        Receita *r = receitas_disponiveis;
        while (r) { if (r->desbloqueada) { receita_selecionada = r; break; } r = r->prox; }
        idx = 1;
    }

    if (IsKeyPressed(KEY_ONE))   idx = 1;
    if (IsKeyPressed(KEY_TWO))   idx = 2;
    if (IsKeyPressed(KEY_THREE)) idx = 3;
    if (IsKeyPressed(KEY_DOWN))  idx = (idx % total) + 1;
    if (IsKeyPressed(KEY_UP))    idx = (idx - 2 + total) % total + 1;

    // busca a idx-esima receita DESBLOQUEADA
    receita_selecionada = NULL;
    { int k = 1; Receita *r = receitas_disponiveis;
      while (r) { if (r->desbloqueada) {
                      if (k == idx) { receita_selecionada = r; break; }
                      k++;
                  } r = r->prox; } }

    if (IsKeyPressed(KEY_ENTER) && !IsKeyDown(KEY_LEFT_ALT) &&
        receita_selecionada != NULL) {
        tela_atual = TELA_INGREDIENTES;
    }
}

// --- avaliacao dos jurados em thread separada ---
static ResultadoJurados jurados;
static int        jurados_prontos     = 0;
static int        jurados_solicitados = 0;
static pthread_t  _thread_jurados_id;
static EstadoJogo _estado_snapshot;   // copia do estado no momento do disparo

static void *_thread_jurados(void *arg) {
    EstadoJogo *snap = (EstadoJogo *)arg;
    jurados          = avaliar_com_jurados(*snap);
    jurados_prontos  = 1;
    return NULL;
}

static void disparar_jurados(void) {
    if (jurados_solicitados) return;
    jurados_solicitados = 1;
    jurados_prontos     = 0;
    _estado_snapshot    = estado;  // captura o estado AGORA, antes do thread rodar
    pthread_create(&_thread_jurados_id, NULL, _thread_jurados, &_estado_snapshot);
    pthread_detach(_thread_jurados_id);
}

int main(void) {
    iniciar_jogo();
    integrar_modulos();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(LARG_VIRTUAL, ALT_VIRTUAL, "Receitas de Mainha");
    SetTargetFPS(60);

    // carrega sprites depois que a janela existe (textura precisa de contexto OpenGL)
    catcher_carregar_sprites();

    RenderTexture2D alvo = LoadRenderTexture(LARG_VIRTUAL, ALT_VIRTUAL);
    SetTextureFilter(alvo.texture, TEXTURE_FILTER_BILINEAR);
    SetExitKey(0);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (tela_atual == TELA_MENU) break;
            if (tela_atual == TELA_RESULTADO) {
                resetar_partida();
                receita_selecionada = NULL;
                jurados_prontos     = 0;
                jurados_solicitados = 0;
            }
            tela_atual = TELA_MENU;
        }
        // ---- alterna fullscreen ----
        if (IsKeyPressed(KEY_F11) ||
            (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER))) {
            alternar_fullscreen();
        }

        // ---- atalhos de debug (bloqueados nas telas jogaveis) ----
        if (tela_atual != TELA_CATCHER &&
            tela_atual != TELA_ORDENACAO &&
            tela_atual != TELA_PILHA) {

            if (IsKeyPressed(KEY_ONE)   && tela_atual != TELA_RECEITAS) tela_atual = TELA_MENU;
            if (IsKeyPressed(KEY_TWO)   && tela_atual != TELA_RECEITAS) tela_atual = TELA_RECEITAS;
            if (IsKeyPressed(KEY_THREE) && tela_atual != TELA_RECEITAS) tela_atual = TELA_INGREDIENTES;
            if (IsKeyPressed(KEY_SIX))   tela_atual = TELA_FEEDBACK;
            if (IsKeyPressed(KEY_SEVEN)) {
                disparar_jurados();
                tela_atual = TELA_RESULTADO;
            }
            if (IsKeyPressed(KEY_EIGHT)) tela_atual = TELA_CREDITOS;
        }
        if (IsKeyPressed(KEY_FIVE) &&
            tela_atual != TELA_CATCHER &&
            tela_atual != TELA_ORDENACAO) {
            tela_atual = TELA_PILHA;
        }

        // ---- CHEAT: Ctrl+P para ir direto para o Pirão (DEBUG) ----
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_P)) {
            // Desbloqueia e seleciona Pirão
            Receita *pirao = buscar_receita(receitas_disponiveis, "Pirão de Carne");
            if (pirao) {
                pirao->desbloqueada = 1;
                receita_selecionada = pirao;
                estado.receitas_completadas = 3;  // simula já ter completado 3 receitas
                cozinhar_iniciar(receita_selecionada);
                tela_atual = TELA_PILHA;
                printf("[CHEAT] Ctrl+P: Pulando para o PIRÃO DE CARNE!\n");
            }
        }

        // ---- transforma coordenadas do mouse para o canvas virtual ----
        float sw = (float)GetScreenWidth();
        float sh = (float)GetScreenHeight();
        float escala = fminf(sw / LARG_VIRTUAL, sh / ALT_VIRTUAL);
        float vw = LARG_VIRTUAL * escala;
        float vh = ALT_VIRTUAL  * escala;
        float vx = (sw - vw) * 0.5f;
        float vy = (sh - vh) * 0.5f;
        SetMouseOffset((int)(-vx), (int)(-vy));
        SetMouseScale(1.0f / escala, 1.0f / escala);

        // ---- logica de cada tela ----
        switch (tela_atual) {
            case TELA_RECEITAS:
                atualizar_selecao_receitas();
                break;

            case TELA_INGREDIENTES:
                if (IsKeyPressed(KEY_ENTER) && !IsKeyDown(KEY_LEFT_ALT)) {
                    if (receita_selecionada != NULL) {
                        // Verifica se é Pirão e se as 3 primeiras foram completadas
                        if (strcmp(receita_selecionada->nome, "Pirão de Carne") == 0) {
                            if (estado.receitas_completadas < 3) {
                                printf("[BLOQUEIO] Pirão de Carne está bloqueado! Complete as 3 outras receitas primeiro.\n");
                                printf("[PROGRESSO] Completadas: %d/3\n", estado.receitas_completadas);
                                // Volta ao menu de receitas para escolher outra
                                tela_atual = TELA_RECEITAS;
                                break;
                            }
                        }
                        
                        catcher_iniciar(receita_selecionada);
                        tela_atual = TELA_CATCHER;
                    }
                }
                if (IsKeyPressed(KEY_B)) tela_atual = TELA_RECEITAS;
                break;

            case TELA_CATCHER:
                if (catcher.terminou) {
                    if (IsKeyPressed(KEY_R)) {
                        catcher_iniciar(receita_selecionada);
                    } else if (IsKeyPressed(KEY_ENTER) &&
                               !IsKeyDown(KEY_LEFT_ALT)) {
                        if (catcher.venceu) {
                            ordenacao_iniciar(receita_selecionada, catcher.ordem_coleta, catcher.n_coletados_em_ordem);
                            tela_atual = TELA_ORDENACAO;
                        } else {
                            disparar_jurados();
                            tela_atual = TELA_RESULTADO;
                        }
                    }
                }
                break;

            case TELA_ORDENACAO:
                if (ordenacao_terminou() &&
                    IsKeyPressed(KEY_ENTER) && !IsKeyDown(KEY_LEFT_ALT)) {
                    cozinhar_iniciar(receita_selecionada);
                    tela_atual = TELA_PILHA;
                }
                break;

            case TELA_PILHA:
                if (cozinhar.terminou &&
                    IsKeyPressed(KEY_ENTER) && !IsKeyDown(KEY_LEFT_ALT)) {
                    cozinhar_limpar();
                    
                    // Incrementa contador de receitas completadas
                    if (receita_selecionada != NULL) {
                        estado.receitas_completadas++;
                        printf("[PROGRESSO] Receita '%s' completada! Total: %d/4\n",
                               receita_selecionada->nome, estado.receitas_completadas);
                        
                        // Se completou as 3 primeiras, desbloqueia Pirão
                        if (estado.receitas_completadas == 3) {
                            Receita *pirao = buscar_receita(receitas_disponiveis, "Pirão de Carne");
                            if (pirao) {
                                pirao->desbloqueada = 1;
                                printf("[DESBLOQUEIO] Pirão de Carne desbloqueado! Vá ao Marco Zero (TELA_RESULTADO).\n");
                            }
                        }
                        
                        // JURADOS SÓ NA FASE FINAL (PIRÃO)
                        if (strcmp(receita_selecionada->nome, "Pirão de Carne") == 0) {
                            // Fase final: chama jurados
                            disparar_jurados();
                            tela_atual = TELA_RESULTADO;
                        } else {
                            // Receitas normais: volta ao menu sem jurados
                            receita_selecionada = NULL;
                            tela_atual = TELA_RECEITAS;
                        }
                    }
                }
                break;

            default:
                break;
        }

        // ---- desenha no canvas virtual ----
        BeginTextureMode(alvo);
        switch (tela_atual) {
            case TELA_MENU:
                tela_menu();
                break;
            case TELA_RECEITAS:
                tela_receitas(receitas_disponiveis);
                break;
            case TELA_INGREDIENTES:
                tela_ingredientes(receita_selecionada);
                break;
            case TELA_CATCHER:
                tela_catcher();
                break;
            case TELA_ORDENACAO:
                tela_ordenacao();
                break;
            case TELA_PILHA:
                tela_cozinhar();
                break;
            case TELA_FEEDBACK:
                tela_feedback(1);
                break;
            case TELA_RESULTADO:
                tela_resultado(verificar_vitoria(),
                               jurados_prontos ? &jurados : NULL);
                break;
            case TELA_CREDITOS:
                tela_creditos();
                break;
            default:
                ClearBackground(RAYWHITE);
                break;
        }
        EndTextureMode();

        // ---- escala o canvas pra janela mantendo aspect ratio ----
        BeginDrawing();
        ClearBackground(BLACK);

        DrawTexturePro(
            alvo.texture,
            (Rectangle){ 0, 0, (float)LARG_VIRTUAL, -(float)ALT_VIRTUAL },
            (Rectangle){ vx, vy, vw, vh },
            (Vector2){ 0, 0 }, 0.0f, WHITE
        );

        if (!IsWindowFullscreen()) {
            DrawText("[F11] Tela cheia", 10, GetScreenHeight() - 22, 14,
                     (Color){255, 255, 255, 160});
        }
        EndDrawing();
    }

    UnloadRenderTexture(alvo);
    catcher_descarregar_sprites();
    liberar_receitas(receitas_disponiveis);
    CloseWindow();
    return 0;
}
