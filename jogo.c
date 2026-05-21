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
    inserir_ingrediente(tapioca, "Tapioca granulada", "200g");
    inserir_ingrediente(tapioca, "Coco ralado",       "50g");
    inserir_ingrediente(tapioca, "Queijo",            "80g");
    inserir_ingrediente(tapioca, "Manteiga",          "1 colher");

    // === ingredientes do Escondidinho de Carne de Sol ===
    Receita *escondidinho = buscar_receita(receitas_disponiveis, "Escondidinho");
    inserir_ingrediente(escondidinho, "Carne de sol",  "400g");
    inserir_ingrediente(escondidinho, "Mandioca",      "600g");
    inserir_ingrediente(escondidinho, "Cebola",        "2 unidades");
    inserir_ingrediente(escondidinho, "Manteiga",      "2 colheres");
    inserir_ingrediente(escondidinho, "Queijo coalho", "200g");

    // === ingredientes do Bolo de Rolo (ordem de uso: manteiga, acucar, ovos, farinha, goiabada) ===
    Receita *bolo = buscar_receita(receitas_disponiveis, "Bolo de Rolo");
    inserir_ingrediente(bolo, "Manteiga",         "200g");
    inserir_ingrediente(bolo, "Acucar",           "200g");
    inserir_ingrediente(bolo, "Ovos",             "6 unidades");
    inserir_ingrediente(bolo, "Farinha de trigo", "300g");
    inserir_ingrediente(bolo, "Goiabada",         "300g");
    inserir_ingrediente(bolo, "Baunilha",         "1 colher");
    inserir_ingrediente(bolo, "Fermento",         "1 colher");

    // === passos jogaveis ===
    // tapioca: hidrata, adiciona coco, adiciona queijo, derrete manteiga
    adicionar_passo_jogavel_com_img(tapioca, "Hidrate a tapioca peneirando",
                            "Tapioca granulada", "SSSS", 12, "sprites/frigideira_vazia.png");
    adicionar_passo_jogavel_com_img(tapioca, "Adicione o coco ralado",
                            "Coco ralado", "ADAD", 10, "sprites/frigideira_tapioca.png");
    adicionar_passo_jogavel_com_img(tapioca, "Adicione o queijo",
                            "Queijo", "QWER", 10, "sprites/frigideira_coco.png");
    adicionar_passo_jogavel_com_img(tapioca, "Derreta a manteiga por cima",
                            "Manteiga", "SPACE", 8, "sprites/frigideira_queijo.png");

    // Define imagens de inicio e pronta da tapioca
    strncpy(tapioca->img_inicio, "sprites/frigideira_vazia.png", sizeof(tapioca->img_inicio) - 1);
    tapioca->img_inicio[sizeof(tapioca->img_inicio) - 1] = '\0';
    strncpy(tapioca->img_receita_pronta, "sprites/tapioca_pronta.png", sizeof(tapioca->img_receita_pronta) - 1);
    tapioca->img_receita_pronta[sizeof(tapioca->img_receita_pronta) - 1] = '\0';

    // escondidinho: 8 passos com progressão visual (antes e depois das ações)
    adicionar_passo_jogavel_com_img(escondidinho, "Corte a carne de sol",
                            "Carne de sol", "WSWS", 12, "sprites/tabua_carne.png");
    adicionar_passo_jogavel_com_img(escondidinho, "Descasque a macaxeira",
                            "Mandioca", "SSSS", 10, "sprites/macaxeira_tabua.png");
    adicionar_passo_jogavel_com_img(escondidinho, "Corte a cebola",
                            "Cebola", "QWER", 10, "sprites/tabua_cebola.png");
    adicionar_passo_jogavel_com_img(escondidinho, "Misture macaxeira com manteiga",
                            "Manteiga", "ADAD", 10, "sprites/macaxeira_cozida.png");
    adicionar_passo_jogavel_com_img(escondidinho, "Coloque a carne na travessa",
                            "Carne de sol", "SPACE", 8, "sprites/travessa_vazia.png");
    adicionar_passo_jogavel_com_img(escondidinho, "Coloque a macaxeira",
                            "Mandioca", "DDD", 8, "sprites/travessa_carne.png");
    adicionar_passo_jogavel_com_img(escondidinho, "Coloque o queijo coalho",
                            "Queijo coalho", "CCC", 8, "sprites/travessa_macaxeira.png");
    adicionar_passo_jogavel_com_img(escondidinho, "Leve ao forno",
                            "Forno", "FFF", 12, "sprites/travessa_forno.png");

    // Define imagens de inicio e pronta do escondidinho
    strncpy(escondidinho->img_inicio, "sprites/tabua_carne.png", sizeof(escondidinho->img_inicio) - 1);
    escondidinho->img_inicio[sizeof(escondidinho->img_inicio) - 1] = '\0';
    strncpy(escondidinho->img_receita_pronta, "sprites/escondidinho_pronto.png", sizeof(escondidinho->img_receita_pronta) - 1);
    escondidinho->img_receita_pronta[sizeof(escondidinho->img_receita_pronta) - 1] = '\0';

    // bolo de rolo: cremeia manteiga+acucar, ovos, farinha, recheia
    adicionar_passo_jogavel(bolo, "Bata manteiga com acucar",
                            "Manteiga", "WASDW", 12);
    adicionar_passo_jogavel(bolo, "Adicione o acucar e bata",
                            "Acucar", "ASDF", 10);
    adicionar_passo_jogavel(bolo, "Acrescente os ovos um a um",
                            "Ovos", "SPACE", 8);
    adicionar_passo_jogavel(bolo, "Misture a farinha de trigo",
                            "Farinha de trigo", "QWERTY", 14);
    adicionar_passo_jogavel(bolo, "Espalhe a goiabada e enrole",
                            "Goiabada", "DDDD", 12);

    // === ingredientes do Pirão de Carne (desbloqueavel - final do jogo no Marco Zero) ===
    Receita *pirao_carne = buscar_receita(receitas_disponiveis, "Pirão de Carne");
    inserir_ingrediente(pirao_carne, "Legumes",           "cenoura, cebola, batata");
    inserir_ingrediente(pirao_carne, "Coentro",           "1 maço");
    inserir_ingrediente(pirao_carne, "Carne",             "600g");
    inserir_ingrediente(pirao_carne, "Água",              "2 litros");
    inserir_ingrediente(pirao_carne, "Temperado",         "à gosto");
    inserir_ingrediente(pirao_carne, "Farinha de mandioca", "400g");

    // === ETAPA 1: Cortar Ingredientes ===
    adicionar_passo_jogavel_com_img(pirao_carne, "Corte os legumes",
                            "Legumes", "CCCC", 8, "sprites/pirao_legumes_cortados.png");
    adicionar_passo_jogavel_com_img(pirao_carne, "Pique o coentro",
                            "Coentro", "PPPP", 8, "sprites/pirao_coentro_picado.png");
    adicionar_passo_jogavel_com_img(pirao_carne, "Corte a carne",
                            "Carne", "DDDD", 10, "sprites/pirao_carne_cortada.png");

    // === ETAPA 2: Adicionar na Panela ===
    adicionar_passo_jogavel_com_img(pirao_carne, "Coloque água na panela",
                            "Água", "AAAA", 10, "sprites/pirao_agua_panela.png");
    adicionar_passo_jogavel_com_img(pirao_carne, "Adicione a carne",
                            "Carne", "CCCC", 10, "sprites/pirao_carne_panela.png");
    adicionar_passo_jogavel_com_img(pirao_carne, "Adicione os legumes",
                            "Legumes", "LLLL", 10, "sprites/pirao_legumes_panela.png");
    adicionar_passo_jogavel_com_img(pirao_carne, "Tempere a panela",
                            "Temperado", "TTTT", 8, "sprites/pirao_temperado.png");

    // === ETAPA 3: Retirar Ingredientes (Minigame de Peneiramento) ===
    adicionar_passo_jogavel_com_img(pirao_carne, "Retire a carne com peneira",
                            "Carne", "MMMM", 12, "sprites/pirao_carne_retirada.png");
    adicionar_passo_jogavel_com_img(pirao_carne, "Retire os legumes",
                            "Legumes", "VVVV", 12, "sprites/pirao_legumes_retirados.png");

    // === ETAPA 4: O Pirão (Clímax) ===
    adicionar_passo_jogavel_com_img(pirao_carne, "Adicione farinha aos poucos",
                            "Farinha de mandioca", "FFFF", 10, "sprites/pirao_farinha_adiciona.png");
    adicionar_passo_jogavel_com_img(pirao_carne, "Mexa bem o pirão",
                            "Farinha de mandioca", "EEEE", 12, "sprites/pirao_mexendo.png");
    adicionar_passo_jogavel_com_img(pirao_carne, "Finalize com coentro",
                            "Coentro", "FFFF", 8, "sprites/pirao_coentro_final.png");

    // Define imagens de inicio e pronta do Pirão
    strncpy(pirao_carne->img_inicio, "sprites/pirao_ingredientes_mesa.png", sizeof(pirao_carne->img_inicio) - 1);
    pirao_carne->img_inicio[sizeof(pirao_carne->img_inicio) - 1] = '\0';
    strncpy(pirao_carne->img_receita_pronta, "sprites/pirao_completo.png", sizeof(pirao_carne->img_receita_pronta) - 1);
    pirao_carne->img_receita_pronta[sizeof(pirao_carne->img_receita_pronta) - 1] = '\0';

    printf("[SISTEMA] Modulos integrados. 3 receitas + 1 desbloque\u00e1vel (Pir\u00e3o de Carne).\n");
}
