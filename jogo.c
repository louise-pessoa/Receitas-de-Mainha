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
    receitas_disponiveis = inserir_receita(receitas_disponiveis, "Bolo de Rolo", 4, 60, 100);
    receitas_disponiveis = inserir_receita(receitas_disponiveis, "Escondidinho", 5, 40, 80);
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

    // === ingredientes do Bolo de Rolo (ordem de uso: farinha, acucar, ovo, fermento, manteiga, goiabada) ===
    Receita *bolo = buscar_receita(receitas_disponiveis, "Bolo de Rolo");
    inserir_ingrediente(bolo, "Farinha");
    inserir_ingrediente(bolo, "Acucar");
    inserir_ingrediente(bolo, "Ovo");
    inserir_ingrediente(bolo, "Fermento");
    inserir_ingrediente(bolo, "Manteiga");
    inserir_ingrediente(bolo, "Goiabada");

    // === passos jogaveis (push em ordem de execucao: passo 1 primeiro) ===

    // tapioca
    tapioca->passos = push_passo(tapioca->passos,
        "Peneire a massa da tapioca", "Tapioca granulada", "ADAD", "sprites/receitas/tapioca/frigideira_vazia.png");
    tapioca->passos = push_passo(tapioca->passos,
        "Adicione o coco ralado", "Coco ralado", "KOKO", "sprites/receitas/tapioca/frigideira_tapioca.png");
    tapioca->passos = push_passo(tapioca->passos,
        "Adicione o queijo", "Queijo coalho", "UEIO", "sprites/receitas/tapioca/frigideira_coco.png");
    tapioca->passos = push_passo(tapioca->passos,
        "Derreta a manteiga por cima", "Manteiga", "MNTG", "sprites/receitas/tapioca/frigideira_queijo.png");

    strncpy(tapioca->img_inicio, "sprites/receitas/tapioca/frigideira_vazia.png", sizeof(tapioca->img_inicio) - 1);
    tapioca->img_inicio[sizeof(tapioca->img_inicio) - 1] = '\0';
    strncpy(tapioca->img_receita_pronta, "sprites/receitas/tapioca/tapioca_pronta.png", sizeof(tapioca->img_receita_pronta) - 1);
    tapioca->img_receita_pronta[sizeof(tapioca->img_receita_pronta) - 1] = '\0';

    // escondidinho
    escondidinho->passos = push_passo(escondidinho->passos,
        "Corte a carne de sol", "Carne de sol", "CRT", "sprites/receitas/escondidinho/tabua_carne.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Descasque a macaxeira", "Macaxeira", "CASCA", "sprites/receitas/escondidinho/macaxeira_tabua.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Corte a cebola", "Cebola", "CBLA", "sprites/receitas/escondidinho/tabua_cebola.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Misture macaxeira com manteiga", "Manteiga", "MNTG", "sprites/receitas/escondidinho/macaxeira_cozida.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Coloque a carne na travessa", "Carne de sol", "TRAVESSA", "sprites/receitas/escondidinho/travessa_vazia.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Coloque a macaxeira", "Macaxeira", "XXX", "sprites/receitas/escondidinho/travessa_carne.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Coloque o queijo coalho", "Queijo coalho", "UEIOC", "sprites/receitas/escondidinho/travessa_macaxeira.png");
    escondidinho->passos = push_passo(escondidinho->passos,
        "Leve ao forno", "", "FFF", "sprites/receitas/escondidinho/travessa_queijo.png");

    strncpy(escondidinho->img_inicio, "sprites/receitas/escondidinho/tabua_carne.png", sizeof(escondidinho->img_inicio) - 1);
    escondidinho->img_inicio[sizeof(escondidinho->img_inicio) - 1] = '\0';
    strncpy(escondidinho->img_receita_pronta, "sprites/receitas/escondidinho/escondidinho_pronto.png", sizeof(escondidinho->img_receita_pronta) - 1);
    escondidinho->img_receita_pronta[sizeof(escondidinho->img_receita_pronta) - 1] = '\0';

    // bolo de rolo
    bolo->passos = push_passo(bolo->passos,
        "Misture todos os ingredientes", "todos os ingredientes", "WASDWASDW", "sprites/receitas/bolo_de_rolo/ingredientes_bolo_rolo.png");
    bolo->passos = push_passo(bolo->passos,
        "Coloque a mistura numa travessa e leve ao forno", "Forno", "FFF", "sprites/receitas/bolo_de_rolo/mistura_bolo_rolo.png");
    bolo->passos = push_passo(bolo->passos,
        "Coloque a massa num papel manteiga", "Papel manteiga", "PPMNTG", "sprites/receitas/bolo_de_rolo/travessa_bolo_forno.png");
    bolo->passos = push_passo(bolo->passos,
        "Adicione a goiabada", "Goiabada", "GOIA", "sprites/receitas/bolo_de_rolo/massa_bolo_rolo.png");
    bolo->passos = push_passo(bolo->passos,
        "Agora enrole essa massa", "Enrolar", "ENRLR", "sprites/receitas/bolo_de_rolo/goiabada_bolo.png");
    bolo->passos = push_passo(bolo->passos,
        "Agora adicione açúcar para enfeitar", "Acucar", "ACUC", "sprites/receitas/bolo_de_rolo/bolo_sem_acucar.png");

    strncpy(bolo->img_inicio, "sprites/receitas/bolo_de_rolo/ingredientes_bolo_rolo.png", sizeof(bolo->img_inicio) - 1);
    bolo->img_inicio[sizeof(bolo->img_inicio) - 1] = '\0';
    strncpy(bolo->img_receita_pronta, "sprites/receitas/bolo_de_rolo/bolo_rolo_pronto.png", sizeof(bolo->img_receita_pronta) - 1);
    bolo->img_receita_pronta[sizeof(bolo->img_receita_pronta) - 1] = '\0';

    // === ingredientes do Pirão de Carne (desbloqueavel - final do jogo no Marco Zero) ===
    Receita *pirao_carne = buscar_receita(receitas_disponiveis, "Pirão de Carne");
    inserir_ingrediente(pirao_carne, "Agua");
    inserir_ingrediente(pirao_carne, "Legumes");
    inserir_ingrediente(pirao_carne, "Carne");
    inserir_ingrediente(pirao_carne, "Farinha de mandioca");
    inserir_ingrediente(pirao_carne, "Manteiga");
    inserir_ingrediente(pirao_carne, "Alho");

    // === passos do Pirão de Carne ===
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Corte a carne", "Carne", "CRTC", "sprites/receitas/pirao/tabua_carne_crua.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Corte os legumes", "Legumes", "CRTL", "sprites/receitas/pirao/legumes_inteiros_pirao.png");
    // Coloca pra ferver: 5 sub-passos (um clique por ingrediente)
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Adicione a agua na panela", "Agua", "AGUA", "sprites/receitas/pirao/caldo_sem_carne_pirao.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Adicione a manteiga", "Manteiga", "MNTG", "sprites/receitas/pirao/caldo_sem_carne_pirao.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Adicione o alho", "Alho", "ALHO", "sprites/receitas/pirao/caldo_sem_carne_pirao.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Adicione os legumes", "Legumes", "LGMS", "sprites/receitas/pirao/caldo_legumes_pirao.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Adicione a carne para ferver", "Carne", "FERV", "sprites/receitas/pirao/caldo_com_carne_pirao.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Deixa so o caldo (tira os legumes e a carne)", "", "", "sprites/receitas/pirao/caldo_sem_carne_pirao.png");
    pirao_carne->passos = push_passo(pirao_carne->passos,
        "Engrossa com farinha", "Farinha de mandioca", "FRNH", "sprites/receitas/pirao/caldo_farinha_pirao.png");

    // Define imagens de inicio e pronta do Pirão
    strncpy(pirao_carne->img_inicio, "sprites/receitas/pirao/tabua_carne_crua.png", sizeof(pirao_carne->img_inicio) - 1);
    pirao_carne->img_inicio[sizeof(pirao_carne->img_inicio) - 1] = '\0';
    strncpy(pirao_carne->img_receita_pronta, "sprites/receitas/pirao/pirao_pronto.png", sizeof(pirao_carne->img_receita_pronta) - 1);
    pirao_carne->img_receita_pronta[sizeof(pirao_carne->img_receita_pronta) - 1] = '\0';

    printf("[SISTEMA] Modulos integrados. 3 receitas + 1 desbloque\u00e1vel (Pir\u00e3o de Carne).\n");
}
