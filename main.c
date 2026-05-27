#include <stdio.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include "raylib.h"
#include "api/jurados.h"
#include "jogo.h"
#include "ui/telas.h"
#include "ui/fonte.h"
#include "dados/receitas.h"
#include "fases/coleta.h"
#include "fases/ordenacao.h"
#include "fases/cozinhar.h"
#include <unistd.h>


#define LARG_VIRTUAL 800
#define ALT_VIRTUAL  600

typedef enum {
    MUSICA_NENHUMA = 0,
    MUSICA_PRAIEIRA,
    MUSICA_VOLTEI_RECIFE,
    MUSICA_MARACATU,
} TipoMusica;

static Music musica_a_praieira = {0};
static Music musica_voltei_recife = {0};
static Music musica_maracatu = {0};
static TipoMusica musica_ativa = MUSICA_NENHUMA;
static int musicas_carregadas = 0;
static int musica_a_praieira_ok = 0;
static int musica_voltei_recife_ok = 0;
static int musica_maracatu_ok = 0;

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

static void carregar_musicas(void) {
    if (musicas_carregadas) return;
    // Load only existing music files to avoid invalid internal pointers
    if (access("musicas/a_praieira.mp3", F_OK) == 0) {
        musica_a_praieira = LoadMusicStream("musicas/a_praieira.mp3");
        musica_a_praieira.looping = true;
        SetMusicVolume(musica_a_praieira, 0.55f);
        musica_a_praieira_ok = 1;
    }
    if (access("musicas/voltei_recife.mp3", F_OK) == 0) {
        musica_voltei_recife = LoadMusicStream("musicas/voltei_recife.mp3");
        musica_voltei_recife.looping = true;
        SetMusicVolume(musica_voltei_recife, 0.55f);
        musica_voltei_recife_ok = 1;
    }
    if (access("musicas/maracatu_atomico.mp3", F_OK) == 0) {
        musica_maracatu = LoadMusicStream("musicas/maracatu_atomico.mp3");
        musica_maracatu.looping = true;
        SetMusicVolume(musica_maracatu, 0.55f);
        musica_maracatu_ok = 1;
    }

    musicas_carregadas = (musica_a_praieira_ok || musica_voltei_recife_ok || musica_maracatu_ok);
}

static void liberar_musicas(void) {
    if (!musicas_carregadas) return;

    // Pare apenas a música que estiver ativa para evitar asserts internos
    if (musica_ativa == MUSICA_PRAIEIRA && musica_a_praieira_ok) StopMusicStream(musica_a_praieira);
    else if (musica_ativa == MUSICA_VOLTEI_RECIFE && musica_voltei_recife_ok) StopMusicStream(musica_voltei_recife);
    else if (musica_ativa == MUSICA_MARACATU && musica_maracatu_ok) StopMusicStream(musica_maracatu);

    // Descarregar apenas se o arquivo existir (carregamento pode ter falhado)
    if (access("musicas/a_praieira.mp3", F_OK) == 0) UnloadMusicStream(musica_a_praieira);
    if (access("musicas/voltei_recife.mp3", F_OK) == 0) UnloadMusicStream(musica_voltei_recife);
    if (access("musicas/maracatu_atomico.mp3", F_OK) == 0) UnloadMusicStream(musica_maracatu);

    musicas_carregadas = 0;
    musica_ativa = MUSICA_NENHUMA;
    musica_a_praieira_ok = musica_voltei_recife_ok = musica_maracatu_ok = 0;
}

static TipoMusica musica_para_tela(void) {
    switch (tela_atual) {
        case TELA_MENU:
        case TELA_RECEITAS:
        case TELA_INGREDIENTES:
            return MUSICA_PRAIEIRA;
        case TELA_CATCHER:
        case TELA_ORDENACAO:
            return MUSICA_VOLTEI_RECIFE;
        case TELA_PILHA:
        case TELA_FEEDBACK:
        case TELA_RESULTADO:
            return MUSICA_MARACATU;
        default:
            return MUSICA_NENHUMA;
    }
}

// índice atual do slideshow de introdução (0..2)
static int intro_idx_global = 0;
static int intro_block_until_release = 0;

static void atualizar_musica(void) {
    if (!musicas_carregadas) return;

    TipoMusica desejada = musica_para_tela();
    if (desejada != musica_ativa) {
        if (musica_ativa == MUSICA_PRAIEIRA) StopMusicStream(musica_a_praieira);
        else if (musica_ativa == MUSICA_VOLTEI_RECIFE) StopMusicStream(musica_voltei_recife);
        else if (musica_ativa == MUSICA_MARACATU) StopMusicStream(musica_maracatu);

        musica_ativa = desejada;
        if (musica_ativa == MUSICA_PRAIEIRA) PlayMusicStream(musica_a_praieira);
        else if (musica_ativa == MUSICA_VOLTEI_RECIFE) PlayMusicStream(musica_voltei_recife);
        else if (musica_ativa == MUSICA_MARACATU && musica_maracatu_ok) {
            // iniciar a música a partir de 29 segundos
            SeekMusicStream(musica_maracatu, 29.0f);
            PlayMusicStream(musica_maracatu);
        }
    }

    if (musica_ativa == MUSICA_PRAIEIRA && musica_a_praieira_ok) UpdateMusicStream(musica_a_praieira);
    else if (musica_ativa == MUSICA_VOLTEI_RECIFE && musica_voltei_recife_ok) UpdateMusicStream(musica_voltei_recife);
    else if (musica_ativa == MUSICA_MARACATU && musica_maracatu_ok) UpdateMusicStream(musica_maracatu);
}

int main(void) {
    iniciar_jogo();
    integrar_modulos();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(LARG_VIRTUAL, ALT_VIRTUAL, "Receitas de Mainha");
    InitAudioDevice();
    SetTargetFPS(60);

    fonte_carregar("fontes/fonte.ttf", 96);

    // carrega sprites depois que a janela existe (textura precisa de contexto OpenGL)
    catcher_carregar_sprites();
    telas_carregar_sprites();
    carregar_musicas();

    RenderTexture2D alvo = LoadRenderTexture(LARG_VIRTUAL, ALT_VIRTUAL);
    SetTextureFilter(alvo.texture, TEXTURE_FILTER_BILINEAR);
    SetExitKey(0);

    while (!WindowShouldClose()) {
        atualizar_musica();

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
                        // Se for a fase final (Pirão), mostra a introducao em slides
                        if (strcmp(receita_selecionada->nome, "Pirão de Carne") == 0) {
                            intro_idx_global = 0; // garante que o primeiro slide seja mostrado
                            // bloqueia avanço por Enter caso a tecla já esteja sendo segurada
                            intro_block_until_release = IsKeyDown(KEY_ENTER) ? 1 : 0;
                            tela_atual = TELA_FINAL_INTRO;
                        } else {
                            catcher_iniciar(receita_selecionada);
                            tela_atual = TELA_CATCHER;
                        }
                    }
                }
                if (IsKeyPressed(KEY_B)) tela_atual = TELA_RECEITAS;
                break;

            case TELA_FINAL_INTRO:
                break; // input/logic handled above in the main loop

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
                if (cozinhar.terminou && IsKeyPressed(KEY_ENTER) && !IsKeyDown(KEY_LEFT_ALT)) {
                    int eh_pirao = (receita_selecionada != NULL &&
                                    strcmp(receita_selecionada->nome, "Pirão de Carne") == 0);

                    cozinhar_limpar();
                    if (receita_selecionada != NULL) {
                        estado.receitas_completadas++;
                        printf("[PROGRESSO] Receita '%s' completada! Total: %d/4\n",
                               receita_selecionada->nome, estado.receitas_completadas);
                        if (estado.receitas_completadas == 3) {
                            Receita *pirao = buscar_receita(receitas_disponiveis, "Pirão de Carne");
                            if (pirao) {
                                pirao->desbloqueada = 1;
                                printf("[DESBLOQUEIO] Pirão de Carne desbloqueado! Vá ao Marco Zero (TELA_RESULTADO).\n");
                            }
                        }
                    }

                    if (eh_pirao) {
                        // Pirão: dispara jurados e mostra resultado
                        disparar_jurados();
                        tela_atual = TELA_RESULTADO;
                    } else {
                        // Receitas normais: volta ao menu sem jurados
                        receita_selecionada = NULL;
                        tela_atual = TELA_RECEITAS;
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
            case TELA_FINAL_INTRO: {
                // draw current slide; manage advance with Enter
                tela_final_intro_draw(intro_idx_global);
                // if we blocked input on entry, wait until Enter is released
                if (intro_block_until_release) {
                    if (!IsKeyDown(KEY_ENTER)) intro_block_until_release = 0;
                } else if (IsKeyPressed(KEY_ENTER) && !IsKeyDown(KEY_LEFT_ALT)) {
                    intro_idx_global++;
                    if (intro_idx_global >= 3) {
                        intro_idx_global = 0;
                        // start catcher after intro
                        catcher_iniciar(receita_selecionada);
                        tela_atual = TELA_CATCHER;
                    }
                }
                break;
            }
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
            txt("[F11] Tela cheia", 10, GetScreenHeight() - 22, 14,
                     (Color){255, 255, 255, 160});
        }
        EndDrawing();
    }

    UnloadRenderTexture(alvo);
    liberar_musicas();
    catcher_descarregar_sprites();
    liberar_receitas(receitas_disponiveis);
    fonte_descarregar();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
