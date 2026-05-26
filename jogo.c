#include <stdio.h>
#include <string.h>
#include "jogo.h"
#include "dados/receitas.h"
#include "dados/pilha.h"

// estado global usado por outros modulos
EstadoJogo estado;
// lista global de receitas disponiveis no jogo
Receita *receitas_disponiveis = NULL;
// receita escolhida pelo jogador no menu de receitas
Receita *receita_selecionada = NULL;
// tela ativa no momento
EstadoTela tela_atual = TELA_MENU;

// reinicia a partida (preserva receitas_completadas)
void resetar_partida(void) {
    estado.pontuacao        = PONTUACAO_INICIAL;
    estado.rodada_atual     = 1;
    estado.em_execucao      = 0;
    estado.fase_final_ativa = 0;
    estado.passos_acertados = 0;
    estado.passos_total     = 0;
    estado.tempo_extra      = 0;
    estado.erro_passo       = 0;
    // NÃO reseta receitas_completadas - persiste por toda a sessão

    printf("[SISTEMA] Partida resetada. Pontuacao: %d\n", PONTUACAO_INICIAL);
}

void iniciar_jogo(void) {
    resetar_partida();
    estado.receitas_completadas = 0;  // inicializa contador na primeira execução
    tela_atual = TELA_MENU;
    printf("[SISTEMA] Jogo iniciado. Boa sorte!\n");
}

int verificar_vitoria(void) {
    return estado.pontuacao >= META_FASE_FINAL;
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
    inserir_ingrediente(tapioca, "Queijo coalho");
    inserir_ingrediente(tapioca, "Manteiga");

    // === ingredientes do Escondidinho de Carne de Sol ===
    Receita *escondidinho = buscar_receita(receitas_disponiveis, "Escondidinho");
    inserir_ingrediente(escondidinho, "Carne de sol");
    inserir_ingrediente(escondidinho, "Macaxeira");
    inserir_ingrediente(escondidinho, "Cebola");
    inserir_ingrediente(escondidinho, "Manteiga");
    inserir_ingrediente(escondidinho, "Queijo coalho");

    // === ingredientes do Bolo de Rolo (ordem de uso: manteiga, acucar, ovos, farinha, goiabada) ===
    Receita *bolo = buscar_receita(receitas_disponiveis, "Bolo de Rolo");
    inserir_ingrediente(bolo, "Manteiga");
    inserir_ingrediente(bolo, "Acucar");
    inserir_ingrediente(bolo, "Ovos");
    inserir_ingrediente(bolo, "Farinha de trigo");
    inserir_ingrediente(bolo, "Goiabada");

    // === passos jogaveis (push em ordem de execucao: passo 1 primeiro) ===

    // tapioca
    tapioca->passos = push_passo(tapioca->passos,
        "Hidrate a tapioca peneirando", "Tapioca granulada", "SSSS", "sprites/receitas/frigideira_vazia.png");
    tapioca->passos = push_passo(tapioca->passos,
        "Adicione o coco ralado", "Coco ralado", "ADAD", "sprites/receitas/frigideira_tapioca.png");
    tapioca->passos = push_passo(tapioca->passos,
        "Adicione o queijo", "Queijo coalho", "QUEIJO", "sprites/receitas/frigideira_coco.png");
    tapioca->passos = push_passo(tapioca->passos,
        "Derreta a manteiga por cima", "Manteiga", "MNTG", "sprites/receitas/frigideira_queijo.png");

    strncpy(tapioca->img_inicio, "sprites/receitas/frigideira_vazia.png", sizeof(tapioca->img_inicio) - 1);
    tapioca->img_inicio[sizeof(tapioca->img_inicio) - 1] = '\0';
    strncpy(tapioca->img_receita_pronta, "sprites/receitas/tapioca_pronta.png", sizeof(tapioca->img_receita_pronta) - 1);
    tapioca->img_receita_pronta[sizeof(tapioca->img_receita_pronta) - 1] = '\0';

    // escondidinho
    escondidinho->passos = push_passo(escondidinho->passos,
        "Corte a carne de sol", "Carne de sol", "WSWS", "sprites/receitas/tabua_carne.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Descasque a macaxeira", "Macaxeira", "CASCA", "sprites/receitas/macaxeira_tabua.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Corte a cebola", "Cebola", "QWER", "sprites/receitas/tabua_cebola.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Misture macaxeira com manteiga", "Manteiga", "ADAD", "sprites/receitas/macaxeira_cozida.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Coloque a carne na travessa", "Carne de sol", "TRAVESSA", "sprites/receitas/travessa_vazia.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Coloque a macaxeira", "Macaxeira", "DDD", "sprites/receitas/travessa_carne.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Coloque o queijo coalho", "Queijo coalho", "CCC", "sprites/receitas/travessa_macaxeira.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Leve ao forno", "", "FFF", "sprites/receitas/travessa_queijo.png");

    strncpy(escondidinho->img_inicio, "sprites/receitas/tabua_carne.png", sizeof(escondidinho->img_inicio) - 1);
    escondidinho->img_inicio[sizeof(escondidinho->img_inicio) - 1] = '\0';
    strncpy(escondidinho->img_receita_pronta, "sprites/receitas/escondidinho_pronto.png", sizeof(escondidinho->img_receita_pronta) - 1);
    escondidinho->img_receita_pronta[sizeof(escondidinho->img_receita_pronta) - 1] = '\0';

    // bolo de rolo
    bolo->passos = push_passo(bolo->passos,
        "Bata manteiga com acucar", "Manteiga", "WASDW", "sprites/receitas/bolo_de_rolo/ingredientes_bolo_rolo.png");
    bolo->passos = push_passo(bolo->passos,
        "Adicione o acucar e bata", "Acucar", "ASDF", "sprites/receitas/bolo_de_rolo/mistura_bolo_rolo.png");
    bolo->passos = push_passo(bolo->passos,
        "Acrescente os ovos um a um", "Ovos", "MNTG", "sprites/receitas/bolo_de_rolo/travessa_bolo_forno.png");
    bolo->passos = push_passo(bolo->passos,
        "Misture a farinha de trigo", "Farinha de trigo", "QWERTY", "sprites/receitas/bolo_de_rolo/massa_bolo_rolo.png");
    bolo->passos = push_passo(bolo->passos,
        "Espalhe a goiabada e enrole", "Goiabada", "DDDD", "sprites/receitas/bolo_de_rolo/enrolando_bolo.png");
    bolo->passos = push_passo(bolo->passos,
        "Leve ao forno", "", "FFF", "sprites/receitas/bolo_de_rolo/travessa_bolo_forno.png");

    strncpy(bolo->img_inicio, "sprites/receitas/bolo_de_rolo/ingredientes_bolo_rolo.png", sizeof(bolo->img_inicio) - 1);
    bolo->img_inicio[sizeof(bolo->img_inicio) - 1] = '\0';
    strncpy(bolo->img_receita_pronta, "sprites/receitas/bolo_de_rolo/bolo_rolo_pronto.png", sizeof(bolo->img_receita_pronta) - 1);
    bolo->img_receita_pronta[sizeof(bolo->img_receita_pronta) - 1] = '\0';

    // === ingredientes do Pirão de Carne (desbloqueavel - final do jogo no Marco Zero) ===
    Receita *pirao_carne = buscar_receita(receitas_disponiveis, "Pirão de Carne");
    inserir_ingrediente(pirao_carne, "Caldo de carne");
    inserir_ingrediente(pirao_carne, "Carne");
    inserir_ingrediente(pirao_carne, "Farinha de mandioca");
    inserir_ingrediente(pirao_carne, "Manteiga");
    inserir_ingrediente(pirao_carne, "Alho");

    // === passos do Pirão de Carne (7 passos) ===
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Desfie a carne", "Carne", "DDDD", "sprites/receitas/pirao/tabua_carne_crua.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Aqueca o caldo", "Caldo de carne", "SSSS", "sprites/receitas/pirao/tabua_carne_cortada.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Adicione a carne ao caldo", "Carne", "AAAA", "sprites/receitas/pirao/caldo_com_carne_pirao.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Derreta manteiga e frite alho", "Manteiga", "MMMM", "sprites/receitas/pirao/caldo_legumes_pirao.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Despeje o refogado na panela", "Alho", "RRRR", "sprites/receitas/pirao/caldo_sem_carne_pirao.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Adicione a farinha de mandioca", "Farinha de mandioca", "FFFF", "sprites/receitas/pirao/caldo_farinha_pirao.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Pirao pronto", "", "PPPP", "sprites/receitas/pirao/pirao_pronto.png");

    // Define imagens de inicio e pronta do Pirão
    strncpy(pirao_carne->img_inicio, "sprites/receitas/pirao/tabua_carne_crua.png", sizeof(pirao_carne->img_inicio) - 1);
    pirao_carne->img_inicio[sizeof(pirao_carne->img_inicio) - 1] = '\0';
    strncpy(pirao_carne->img_receita_pronta, "sprites/receitas/pirao/pirao_pronto.png", sizeof(pirao_carne->img_receita_pronta) - 1);
    pirao_carne->img_receita_pronta[sizeof(pirao_carne->img_receita_pronta) - 1] = '\0';

    printf("[SISTEMA] Modulos integrados. 3 receitas + 1 desbloque\u00e1vel (Pir\u00e3o de Carne).\n");
}
