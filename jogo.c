#include <stdio.h>
#include <time.h>
#include <string.h>
#include "jogo.h"
#include "dados/receitas.h"
#include "dados/pilha.h"

// estado interno do timer
static clock_t _inicio_timer;

// estado global usado por outros modulos
EstadoJogo estado;
// lista global de receitas disponiveis no jogo
Receita *receitas_disponiveis = NULL;
// receita escolhida pelo jogador no menu de receitas
Receita *receita_selecionada = NULL;
// tela ativa no momento
EstadoTela tela_atual = TELA_MENU;

// inicia o cronometro do passo atual
void iniciar_timer(void) {
    _inicio_timer = clock();
}

// retorna segundos desde o ultimo iniciar_timer()
double tempo_decorrido(void) {
    clock_t agora = clock();
    return (double)(agora - _inicio_timer) / CLOCKS_PER_SEC;
}

// verifica se o tempo passou do limite
int tempo_esgotado(int limite) {
    if (limite <= 0) {
        // limite invalido, trata como esgotado
        fprintf(stderr, "[AVISO] tempo_esgotado: limite invalido (%d)\n", limite);
        return 1;
    }

    return tempo_decorrido() >= (double)limite;
}

// calcula o desconto por atraso (uso interno)
static int _calcular_penalidade_tempo(int tempo_gasto, int tempo_limite) {
    if (tempo_gasto <= tempo_limite) return 0;

    int atraso   = tempo_gasto - tempo_limite;
    int desconto = atraso * PENALIDADE_POR_SEGUNDO;

    return desconto;
}

// reinicia a partida e limpa a pilha de passos
void resetar_partida(void) {
    estado.pontuacao        = PONTUACAO_INICIAL;
    estado.rodada_atual     = 1;
    estado.em_execucao      = 0;
    estado.fase_final_ativa = 0;
    estado.passos_acertados  = 0;
    estado.passos_total      = 0;
    estado.tempo_extra       = 0;
    estado.erro_passo        = 0;
    // NÃO reseta receitas_completadas - persiste por toda a sessão
    _inicio_timer           = 0;
    //jurados_prontos = 0;

    //pilha_esvaziar();

    printf("[SISTEMA] Partida resetada. Pontuacao: %d\n", PONTUACAO_INICIAL);
}

void iniciar_jogo(void) {
    resetar_partida();
    estado.receitas_completadas = 0;  // inicializa contador na primeira execução
    tela_atual = TELA_MENU;     // <-- ADICIONAR
    printf("[SISTEMA] Jogo iniciado. Boa sorte!\n");
}
static void _aplicar_penalidade_error(int valor){
    if (valor <= 0) return;
    estado.pontuacao -= valor;
    if(estado.pontuacao < 0){
        estado.pontuacao = 0;
    }
    printf("[PENALIDADE] -%d pontos. Pontuacao atual: %d\n", valor, estado.pontuacao);

    }
int verificar_vitoria(void) {
    return estado.pontuacao >= META_FASE_FINAL;
}
void calcular_pontuacao(int acertou, int tempo_gasto, int tempo_limite) {
    if (!acertou) {
        _aplicar_penalidade_error(PENALIDADE_POR_ERROR);
    }
    int desconto_tempo = _calcular_penalidade_tempo(tempo_gasto, tempo_limite);
    if (desconto_tempo > 0) {
        _aplicar_penalidade_error(desconto_tempo);
    }
    if (acertou && desconto_tempo == 0) {
        printf("[PONTUACAO] Acerto perfeito! Pontuacao: %d\n", estado.pontuacao);
    }
}
void _avancar_progresso(void) {
    estado.passos_acertados++;
    printf("[PROGRESSO] Passo concluido (%d/%d)\n",
           estado.passos_acertados, estado.passos_total);
}

void integrar_modulos(void) {
    // limpa lista anterior se existir
    if (receitas_disponiveis != NULL) {
        liberar_receitas(receitas_disponiveis);
        receitas_disponiveis = NULL;
    }

    // === cria as 3 receitas iniciais + Pirão como desbloqueável ===
    receitas_disponiveis = inserir_receita(receitas_disponiveis, "Tapioca",      2, 15, 50);
    receitas_disponiveis = inserir_receita(receitas_disponiveis, "Escondidinho", 4, 40, 80);
    receitas_disponiveis = inserir_receita(receitas_disponiveis, "Bolo de Rolo", 5, 60, 100);
    receitas_disponiveis = inserir_receita(receitas_disponiveis, "Pirão de Carne", 6, 80, 150);
    
    // Pirão de Carne começa bloqueado
    Receita *pirao = buscar_receita(receitas_disponiveis, "Pirão de Carne");
    if (pirao) pirao->desbloqueada = 0;

    // === ingredientes da Tapioca ===
    Receita *tapioca = buscar_receita(receitas_disponiveis, "Tapioca");
    inserir_ingrediente(tapioca, "Tapioca granulada");
    inserir_ingrediente(tapioca, "Coco ralado");
    inserir_ingrediente(tapioca, "Queijo");
    inserir_ingrediente(tapioca, "Manteiga");

    // === ingredientes do Escondidinho de Carne de Sol ===
    Receita *escondidinho = buscar_receita(receitas_disponiveis, "Escondidinho");
    inserir_ingrediente(escondidinho, "Carne de sol");
    inserir_ingrediente(escondidinho, "Mandioca");
    inserir_ingrediente(escondidinho, "Cebola");
    inserir_ingrediente(escondidinho, "Manteiga");
    inserir_ingrediente(escondidinho, "Queijo coalho");
    inserir_ingrediente(escondidinho, "Forno");

    // === ingredientes do Bolo de Rolo (ordem de uso: manteiga, acucar, ovos, farinha, goiabada) ===
    Receita *bolo = buscar_receita(receitas_disponiveis, "Bolo de Rolo");
    inserir_ingrediente(bolo, "Manteiga");
    inserir_ingrediente(bolo, "Acucar");
    inserir_ingrediente(bolo, "Ovos");
    inserir_ingrediente(bolo, "Farinha de trigo");
    inserir_ingrediente(bolo, "Goiabada");
    inserir_ingrediente(bolo, "Baunilha");
    inserir_ingrediente(bolo, "Fermento");

    // === passos jogaveis (push em ordem de execucao: passo 1 primeiro) ===

    // tapioca
    tapioca->passos = push_passo(tapioca->passos,
        "Hidrate a tapioca peneirando", "Tapioca granulada", "SSSS", "sprites/frigideira_vazia.png");
    tapioca->passos = push_passo(tapioca->passos,
        "Adicione o coco ralado", "Coco ralado", "ADAD", "sprites/frigideira_tapioca.png");
    tapioca->passos = push_passo(tapioca->passos,
        "Adicione o queijo", "Queijo", "QWER", "sprites/frigideira_coco.png");
    tapioca->passos = push_passo(tapioca->passos,
        "Derreta a manteiga por cima", "Manteiga", "SPACE", "sprites/frigideira_queijo.png");

    strncpy(tapioca->img_inicio, "sprites/frigideira_vazia.png", sizeof(tapioca->img_inicio) - 1);
    tapioca->img_inicio[sizeof(tapioca->img_inicio) - 1] = '\0';
    strncpy(tapioca->img_receita_pronta, "sprites/tapioca_pronta.png", sizeof(tapioca->img_receita_pronta) - 1);
    tapioca->img_receita_pronta[sizeof(tapioca->img_receita_pronta) - 1] = '\0';

    // escondidinho
    escondidinho->passos = push_passo(escondidinho->passos,
        "Corte a carne de sol", "Carne de sol", "WSWS", "sprites/tabua_carne.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Descasque a macaxeira", "Mandioca", "SSSS", "sprites/macaxeira_tabua.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Corte a cebola", "Cebola", "QWER", "sprites/tabua_cebola.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Misture macaxeira com manteiga", "Manteiga", "ADAD", "sprites/macaxeira_cozida.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Coloque a carne na travessa", "Carne de sol", "SPACE", "sprites/travessa_vazia.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Coloque a macaxeira", "Mandioca", "DDD", "sprites/travessa_carne.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Coloque o queijo coalho", "Queijo coalho", "CCC", "sprites/travessa_macaxeira.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Leve ao forno", "Forno", "FFF", "sprites/travessa_queijo.png");

    strncpy(escondidinho->img_inicio, "sprites/tabua_carne.png", sizeof(escondidinho->img_inicio) - 1);
    escondidinho->img_inicio[sizeof(escondidinho->img_inicio) - 1] = '\0';
    strncpy(escondidinho->img_receita_pronta, "sprites/escondidinho_pronto.png", sizeof(escondidinho->img_receita_pronta) - 1);
    escondidinho->img_receita_pronta[sizeof(escondidinho->img_receita_pronta) - 1] = '\0';

    // bolo de rolo
    bolo->passos = push_passo(bolo->passos,
        "Bata manteiga com acucar", "Manteiga", "WASDW", "");
    bolo->passos = push_passo(bolo->passos,
        "Adicione o acucar e bata", "Acucar", "ASDF", "");
    bolo->passos = push_passo(bolo->passos,
        "Acrescente os ovos um a um", "Ovos", "SPACE", "");
    bolo->passos = push_passo(bolo->passos,
        "Misture a farinha de trigo", "Farinha de trigo", "QWERTY", "");
    bolo->passos = push_passo(bolo->passos,
        "Espalhe a goiabada e enrole", "Goiabada", "DDDD", "");

    // === ingredientes do Pirão de Carne (desbloqueavel - final do jogo no Marco Zero) ===
    Receita *pirao_carne = buscar_receita(receitas_disponiveis, "Pirão de Carne");
    inserir_ingrediente(pirao_carne, "Caldo de carne");
    inserir_ingrediente(pirao_carne, "Carne desfiada");
    inserir_ingrediente(pirao_carne, "Farinha de mandioca");
    inserir_ingrediente(pirao_carne, "Manteiga");
    inserir_ingrediente(pirao_carne, "Alho");

    // === passos do Pirão de Carne (6 passos) ===
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Desfie a carne", "Carne desfiada", "DDDD", "sprites/prato_carne.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Aqueca o caldo", "Caldo de carne", "SSSS", "sprites/prato_caldo.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Adicione a carne ao caldo", "Carne desfiada", "AAAA", "sprites/prato_carne_caldo.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Derreta manteiga e frite alho", "Manteiga", "MMMM", "sprites/prato_alho.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Despeje o refogado na panela", "Alho", "RRRR", "sprites/prato_refogado.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Adicione a farinha de mandioca", "Farinha de mandioca", "FFFF", "sprites/prato_pirao.png");

    // Define imagens de inicio e pronta do Pirão
    strncpy(pirao_carne->img_inicio, "sprites/pirao_ingredientes_mesa.png", sizeof(pirao_carne->img_inicio) - 1);
    pirao_carne->img_inicio[sizeof(pirao_carne->img_inicio) - 1] = '\0';
    strncpy(pirao_carne->img_receita_pronta, "sprites/pirao_completo.png", sizeof(pirao_carne->img_receita_pronta) - 1);
    pirao_carne->img_receita_pronta[sizeof(pirao_carne->img_receita_pronta) - 1] = '\0';

    printf("[SISTEMA] Modulos integrados. 3 receitas + 1 desbloque\u00e1vel (Pir\u00e3o de Carne).\n");
}
