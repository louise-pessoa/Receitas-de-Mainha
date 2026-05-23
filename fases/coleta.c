#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "raylib.h"
#include "coleta.h"
#include "../jogo.h"

// ==========================================
// CORES
// ==========================================
#define COR_CEU       (Color){135, 206, 235, 255}   // azul claro
#define COR_CEU_BAIXO (Color){200, 235, 250, 255}
#define COR_CHAO      (Color){90,  140, 60,  255}
#define COR_CESTA     (Color){150, 90,  40,  255}
#define COR_CESTA_ESC (Color){110, 60,  20,  255}
#define COR_TEXTO     (Color){60,  30,  10,  255}
#define COR_BOM       (Color){50,  160, 80,  255}
#define COR_RUIM      (Color){220, 50,  50,  255}
#define COR_HUD       (Color){30,  60,  140, 230}

// ==========================================
// CONSTANTES DO MINIGAME
// ==========================================
#define LARG_TELA       800
#define ALT_TELA        600
#define ALT_HUD         70           // faixa superior reservada
#define ALT_CHAO        40

#define CESTA_LARGURA   220.0f
#define CESTA_ALTURA    95.0f
#define CESTA_VEL       420.0f

#define ITEM_RAIO       22.0f
#define ITEM_VEL_MIN    140.0f
#define ITEM_VEL_MAX    260.0f

#define SPAWN_BASE      0.85f
#define SPAWN_MIN       0.45f

// ==========================================
// ESTADO GLOBAL
// ==========================================
EstadoCatcher catcher;

// sprites carregadas uma vez no inicio do jogo
static Texture2D _spr_cesta;
static int _spr_cesta_carregado = 0;

static Texture2D _spr_massa_tapioca;
static int _spr_massa_tapioca_carregado = 0;

static Texture2D _spr_coco_ralado;
static int _spr_coco_ralado_carregado = 0;

static Texture2D _spr_queijo;
static int _spr_queijo_carregado = 0;

static Texture2D _spr_manteiga;
static int _spr_manteiga_carregado = 0;

static Texture2D _spr_carne;
static int _spr_carne_carregado = 0;

static Texture2D _spr_cebola;
static int _spr_cebola_carregado = 0;

static Texture2D _spr_mandioca;
static int _spr_mandioca_carregado = 0;

static Texture2D _spr_acucar;
static int _spr_acucar_carregado = 0;

static Texture2D _spr_baunilha;
static int _spr_baunilha_carregado = 0;

static Texture2D _spr_farinha;
static int _spr_farinha_carregado = 0;

static Texture2D _spr_fermento;
static int _spr_fermento_carregado = 0;

static Texture2D _spr_goiabada;
static int _spr_goiabada_carregado = 0;

static Texture2D _spr_leite;
static int _spr_leite_carregado = 0;

static Texture2D _spr_ovo;
static int _spr_ovo_carregado = 0;

static Texture2D _spr_alho;
static int _spr_alho_carregado = 0;

static Texture2D _spr_banana;
static int _spr_banana_carregado = 0;

static Texture2D _spr_limao;
static int _spr_limao_carregado = 0;

static Texture2D _spr_pao;
static int _spr_pao_carregado = 0;

static Texture2D _spr_pimenta;
static int _spr_pimenta_carregado = 0;

static Texture2D _spr_tomate;
static int _spr_tomate_carregado = 0;

static Texture2D _spr_arroz;
static int _spr_arroz_carregado = 0;

static Texture2D _spr_cebolinha;
static int _spr_cebolinha_carregado = 0;

static Texture2D _spr_mostarda;
static int _spr_mostarda_carregado = 0;

static Texture2D _spr_sal;
static int _spr_sal_carregado = 0;

static Texture2D _spr_pedra;
static int _spr_pedra_carregado = 0;

static Texture2D _spr_sapato;
static int _spr_sapato_carregado = 0;

static Texture2D _spr_feijao;
static int _spr_feijao_carregado = 0;

static Texture2D _spr_chocolate;
static int _spr_chocolate_carregado = 0;

void catcher_carregar_sprites(void) {
    _spr_cesta = LoadTexture("sprites/cesta.png");
    if (_spr_cesta.id != 0) {
        SetTextureFilter(_spr_cesta, TEXTURE_FILTER_BILINEAR);
        _spr_cesta_carregado = 1;
    }

    _spr_massa_tapioca = LoadTexture("sprites/massa_tapioca.png");
    if (_spr_massa_tapioca.id != 0) {
        SetTextureFilter(_spr_massa_tapioca, TEXTURE_FILTER_BILINEAR);
        _spr_massa_tapioca_carregado = 1;
    }

    _spr_coco_ralado = LoadTexture("sprites/coco_ralado.png");
    if (_spr_coco_ralado.id != 0) {
        SetTextureFilter(_spr_coco_ralado, TEXTURE_FILTER_BILINEAR);
        _spr_coco_ralado_carregado = 1;
    }

    _spr_queijo = LoadTexture("sprites/queijo.png");
    if (_spr_queijo.id != 0) {
        SetTextureFilter(_spr_queijo, TEXTURE_FILTER_BILINEAR);
        _spr_queijo_carregado = 1;
    }

    _spr_manteiga = LoadTexture("sprites/manteiga.png");
    if (_spr_manteiga.id != 0) {
        SetTextureFilter(_spr_manteiga, TEXTURE_FILTER_BILINEAR);
        _spr_manteiga_carregado = 1;
    }

    _spr_carne = LoadTexture("sprites/carne.png");
    if (_spr_carne.id != 0) {
        SetTextureFilter(_spr_carne, TEXTURE_FILTER_BILINEAR);
        _spr_carne_carregado = 1;
    }

    _spr_cebola = LoadTexture("sprites/cebola.png");
    if (_spr_cebola.id != 0) {
        SetTextureFilter(_spr_cebola, TEXTURE_FILTER_BILINEAR);
        _spr_cebola_carregado = 1;
    }

    _spr_mandioca = LoadTexture("sprites/mandioca.png");
    if (_spr_mandioca.id != 0) {
        SetTextureFilter(_spr_mandioca, TEXTURE_FILTER_BILINEAR);
        _spr_mandioca_carregado = 1;
    }

    _spr_acucar = LoadTexture("sprites/acucar.png");
    if (_spr_acucar.id != 0) {
        SetTextureFilter(_spr_acucar, TEXTURE_FILTER_BILINEAR);
        _spr_acucar_carregado = 1;
    }

    _spr_baunilha = LoadTexture("sprites/baunilha.png");
    if (_spr_baunilha.id != 0) {
        SetTextureFilter(_spr_baunilha, TEXTURE_FILTER_BILINEAR);
        _spr_baunilha_carregado = 1;
    }

    _spr_farinha = LoadTexture("sprites/farinha.png");
    if (_spr_farinha.id != 0) {
        SetTextureFilter(_spr_farinha, TEXTURE_FILTER_BILINEAR);
        _spr_farinha_carregado = 1;
    }

    _spr_fermento = LoadTexture("sprites/fermento.png");
    if (_spr_fermento.id != 0) {
        SetTextureFilter(_spr_fermento, TEXTURE_FILTER_BILINEAR);
        _spr_fermento_carregado = 1;
    }

    _spr_goiabada = LoadTexture("sprites/goiabada.png");
    if (_spr_goiabada.id != 0) {
        SetTextureFilter(_spr_goiabada, TEXTURE_FILTER_BILINEAR);
        _spr_goiabada_carregado = 1;
    }

    _spr_leite = LoadTexture("sprites/leite.png");
    if (_spr_leite.id != 0) {
        SetTextureFilter(_spr_leite, TEXTURE_FILTER_BILINEAR);
        _spr_leite_carregado = 1;
    }

    _spr_ovo = LoadTexture("sprites/ovo.png");
    if (_spr_ovo.id != 0) {
        SetTextureFilter(_spr_ovo, TEXTURE_FILTER_BILINEAR);
        _spr_ovo_carregado = 1;
    }

    _spr_alho = LoadTexture("sprites/alho.png");
    if (_spr_alho.id != 0) {
        SetTextureFilter(_spr_alho, TEXTURE_FILTER_BILINEAR);
        _spr_alho_carregado = 1;
    }

    _spr_banana = LoadTexture("sprites/banana.png");
    if (_spr_banana.id != 0) {
        SetTextureFilter(_spr_banana, TEXTURE_FILTER_BILINEAR);
        _spr_banana_carregado = 1;
    }

    _spr_limao = LoadTexture("sprites/limao.png");
    if (_spr_limao.id != 0) {
        SetTextureFilter(_spr_limao, TEXTURE_FILTER_BILINEAR);
        _spr_limao_carregado = 1;
    }

    _spr_pao = LoadTexture("sprites/pao.png");
    if (_spr_pao.id != 0) {
        SetTextureFilter(_spr_pao, TEXTURE_FILTER_BILINEAR);
        _spr_pao_carregado = 1;
    }

    _spr_pimenta = LoadTexture("sprites/pimenta.png");
    if (_spr_pimenta.id != 0) {
        SetTextureFilter(_spr_pimenta, TEXTURE_FILTER_BILINEAR);
        _spr_pimenta_carregado = 1;
    }

    _spr_tomate = LoadTexture("sprites/tomate.png");
    if (_spr_tomate.id != 0) {
        SetTextureFilter(_spr_tomate, TEXTURE_FILTER_BILINEAR);
        _spr_tomate_carregado = 1;
    }

    _spr_arroz = LoadTexture("sprites/arroz.png");
    if (_spr_arroz.id != 0) {
        SetTextureFilter(_spr_arroz, TEXTURE_FILTER_BILINEAR);
        _spr_arroz_carregado = 1;
    }

    _spr_cebolinha = LoadTexture("sprites/cebolinha.png");
    if (_spr_cebolinha.id != 0) {
        SetTextureFilter(_spr_cebolinha, TEXTURE_FILTER_BILINEAR);
        _spr_cebolinha_carregado = 1;
    }

    _spr_mostarda = LoadTexture("sprites/mostarda.png");
    if (_spr_mostarda.id != 0) {
        SetTextureFilter(_spr_mostarda, TEXTURE_FILTER_BILINEAR);
        _spr_mostarda_carregado = 1;
    }

    _spr_sal = LoadTexture("sprites/sal.png");
    if (_spr_sal.id != 0) {
        SetTextureFilter(_spr_sal, TEXTURE_FILTER_BILINEAR);
        _spr_sal_carregado = 1;
    }

    _spr_pedra = LoadTexture("sprites/pedra.png");
    if (_spr_pedra.id != 0) {
        SetTextureFilter(_spr_pedra, TEXTURE_FILTER_BILINEAR);
        _spr_pedra_carregado = 1;
    }

    _spr_sapato = LoadTexture("sprites/sapato.png");
    if (_spr_sapato.id != 0) {
        SetTextureFilter(_spr_sapato, TEXTURE_FILTER_BILINEAR);
        _spr_sapato_carregado = 1;
    }

    _spr_feijao = LoadTexture("sprites/feijao.png");
    if (_spr_feijao.id != 0) {
        SetTextureFilter(_spr_feijao, TEXTURE_FILTER_BILINEAR);
        _spr_feijao_carregado = 1;
    }

    _spr_chocolate = LoadTexture("sprites/chocolate.png");
    if (_spr_chocolate.id != 0) {
        SetTextureFilter(_spr_chocolate, TEXTURE_FILTER_BILINEAR);
        _spr_chocolate_carregado = 1;
    }
}

void catcher_descarregar_sprites(void) {
    if (_spr_cesta_carregado) {
        UnloadTexture(_spr_cesta);
        _spr_cesta_carregado = 0;
    }
    if (_spr_massa_tapioca_carregado) {
        UnloadTexture(_spr_massa_tapioca);
        _spr_massa_tapioca_carregado = 0;
    }
    if (_spr_coco_ralado_carregado) {
        UnloadTexture(_spr_coco_ralado);
        _spr_coco_ralado_carregado = 0;
    }
    if (_spr_queijo_carregado) {
        UnloadTexture(_spr_queijo);
        _spr_queijo_carregado = 0;
    }
    if (_spr_manteiga_carregado) {
        UnloadTexture(_spr_manteiga);
        _spr_manteiga_carregado = 0;
    }
    if (_spr_carne_carregado) {
        UnloadTexture(_spr_carne);
        _spr_carne_carregado = 0;
    }
    if (_spr_cebola_carregado) {
        UnloadTexture(_spr_cebola);
        _spr_cebola_carregado = 0;
    }
    if (_spr_mandioca_carregado) {
        UnloadTexture(_spr_mandioca);
        _spr_mandioca_carregado = 0;
    }
    if (_spr_acucar_carregado)   { UnloadTexture(_spr_acucar);   _spr_acucar_carregado   = 0; }
    if (_spr_baunilha_carregado) { UnloadTexture(_spr_baunilha); _spr_baunilha_carregado = 0; }
    if (_spr_farinha_carregado)  { UnloadTexture(_spr_farinha);  _spr_farinha_carregado  = 0; }
    if (_spr_fermento_carregado) { UnloadTexture(_spr_fermento); _spr_fermento_carregado = 0; }
    if (_spr_goiabada_carregado) { UnloadTexture(_spr_goiabada); _spr_goiabada_carregado = 0; }
    if (_spr_leite_carregado)    { UnloadTexture(_spr_leite);    _spr_leite_carregado    = 0; }
    if (_spr_ovo_carregado)      { UnloadTexture(_spr_ovo);      _spr_ovo_carregado      = 0; }
    if (_spr_alho_carregado)     { UnloadTexture(_spr_alho);     _spr_alho_carregado     = 0; }
    if (_spr_banana_carregado)   { UnloadTexture(_spr_banana);   _spr_banana_carregado   = 0; }
    if (_spr_limao_carregado)    { UnloadTexture(_spr_limao);    _spr_limao_carregado    = 0; }
    if (_spr_pao_carregado)      { UnloadTexture(_spr_pao);      _spr_pao_carregado      = 0; }
    if (_spr_pimenta_carregado)  { UnloadTexture(_spr_pimenta);  _spr_pimenta_carregado  = 0; }
    if (_spr_tomate_carregado)   { UnloadTexture(_spr_tomate);   _spr_tomate_carregado   = 0; }
    if (_spr_arroz_carregado)     { UnloadTexture(_spr_arroz);     _spr_arroz_carregado     = 0; }
    if (_spr_cebolinha_carregado) { UnloadTexture(_spr_cebolinha); _spr_cebolinha_carregado = 0; }
    if (_spr_mostarda_carregado)  { UnloadTexture(_spr_mostarda);  _spr_mostarda_carregado  = 0; }
    if (_spr_sal_carregado)       { UnloadTexture(_spr_sal);       _spr_sal_carregado       = 0; }
    if (_spr_pedra_carregado)     { UnloadTexture(_spr_pedra);     _spr_pedra_carregado     = 0; }
    if (_spr_sapato_carregado)    { UnloadTexture(_spr_sapato);    _spr_sapato_carregado    = 0; }
    if (_spr_feijao_carregado)    { UnloadTexture(_spr_feijao);    _spr_feijao_carregado    = 0; }
    if (_spr_chocolate_carregado) { UnloadTexture(_spr_chocolate); _spr_chocolate_carregado = 0; }
}

// retorna o sprite correspondente ao nome do ingrediente, ou NULL se nao houver
static const Texture2D* _sprite_para_ingrediente(const char *nome) {
    if (_spr_massa_tapioca_carregado && strstr(nome, "Tapioca") != NULL) {
        return &_spr_massa_tapioca;
    }
    if (_spr_coco_ralado_carregado && strstr(nome, "Coco") != NULL) {
        return &_spr_coco_ralado;
    }
    if (_spr_queijo_carregado && strstr(nome, "Queijo") != NULL) {
        return &_spr_queijo;
    }
    if (_spr_manteiga_carregado && strstr(nome, "Manteiga") != NULL) {
        return &_spr_manteiga;
    }
    if (_spr_carne_carregado && strstr(nome, "Carne") != NULL) {
        return &_spr_carne;
    }
    // exato pra Cebola (evita conflito com "Cebolinha" que e distrator)
    if (_spr_cebola_carregado && strcmp(nome, "Cebola") == 0) {
        return &_spr_cebola;
    }
    if (_spr_mandioca_carregado && strstr(nome, "Mandioca") != NULL) {
        return &_spr_mandioca;
    }
    if (_spr_acucar_carregado   && strstr(nome, "Acucar")   != NULL) return &_spr_acucar;
    if (_spr_baunilha_carregado && strstr(nome, "Baunilha") != NULL) return &_spr_baunilha;
    if (_spr_farinha_carregado  && strstr(nome, "Farinha")  != NULL) return &_spr_farinha;
    if (_spr_fermento_carregado && strstr(nome, "Fermento") != NULL) return &_spr_fermento;
    if (_spr_goiabada_carregado && strstr(nome, "Goiabada") != NULL) return &_spr_goiabada;
    if (_spr_leite_carregado    && strstr(nome, "Leite")    != NULL) return &_spr_leite;
    if (_spr_ovo_carregado      && strstr(nome, "Ovo")      != NULL) return &_spr_ovo;
    if (_spr_alho_carregado     && strstr(nome, "Alho")     != NULL) return &_spr_alho;
    if (_spr_banana_carregado   && strstr(nome, "Banana")   != NULL) return &_spr_banana;
    if (_spr_limao_carregado    && strstr(nome, "Limao")    != NULL) return &_spr_limao;
    if (_spr_pao_carregado      && strstr(nome, "Pao")      != NULL) return &_spr_pao;
    if (_spr_pimenta_carregado  && strstr(nome, "Pimenta")  != NULL) return &_spr_pimenta;
    if (_spr_tomate_carregado   && strstr(nome, "Tomate")   != NULL) return &_spr_tomate;
    if (_spr_arroz_carregado     && strstr(nome, "Arroz")     != NULL) return &_spr_arroz;
    // Cebolinha precisa vir ANTES de qualquer matching de "Cebola"
    if (_spr_cebolinha_carregado && strstr(nome, "Cebolinha") != NULL) return &_spr_cebolinha;
    if (_spr_mostarda_carregado  && strstr(nome, "Mostarda")  != NULL) return &_spr_mostarda;
    if (_spr_sal_carregado       && strcmp(nome, "Sal")       == 0)   return &_spr_sal;
    if (_spr_pedra_carregado     && strstr(nome, "Pedra")     != NULL) return &_spr_pedra;
    if (_spr_sapato_carregado    && strstr(nome, "Sapato")    != NULL) return &_spr_sapato;
    if (_spr_feijao_carregado    && strstr(nome, "Feijao")    != NULL) return &_spr_feijao;
    if (_spr_chocolate_carregado && strstr(nome, "Chocolate") != NULL) return &_spr_chocolate;
    return NULL;
}

// lista de distratores: ingredientes que NAO devem entrar na receita.
// (mistura de coisas comestiveis e bobas pra atrapalhar)
static const char *DISTRATORES_GERAIS[] = {
    "Pimenta", "Alho", "Tomate", "Limao",
    "Pao", "Banana", "Arroz", "Feijao",
    "Cebolinha", "Mostarda", "Chocolate",
    "Sapato", "Pedra"
};
static const int N_DISTRATORES_GERAIS =
    (int)(sizeof(DISTRATORES_GERAIS) / sizeof(DISTRATORES_GERAIS[0]));

static const Color CORES_ITEM[] = {
    {220, 90,  90,  255},
    {90,  170, 90,  255},
    {240, 200, 80,  255},
    {200, 130, 220, 255},
    {80,  170, 220, 255},
    {240, 150, 70,  255},
    {180, 100, 50,  255}
};
static const int N_CORES_ITEM = (int)(sizeof(CORES_ITEM) / sizeof(CORES_ITEM[0]));

// ==========================================
// UTILITARIOS
// ==========================================

// retorna valor aleatorio entre min e max
static float frand(float min, float max) {
    float t = (float)GetRandomValue(0, 10000) / 10000.0f;
    return min + (max - min) * t;
}

// verifica se um nome ja existe na lista de alvos
static int eh_alvo(const char *nome) {
    for (int i = 0; i < catcher.n_alvos; i++) {
        if (strcmp(catcher.alvos[i].nome, nome) == 0) return 1;
    }
    return 0;
}

// ==========================================
// SPAWN DE ITENS
// ==========================================
static void spawnar_item(void) {
    // procura slot vazio
    int slot = -1;
    for (int i = 0; i < CATCHER_MAX_ITENS; i++) {
        if (!catcher.itens[i].ativo) { slot = i; break; }
    }
    if (slot < 0) return;

    ItemCaindo *it = &catcher.itens[slot];
    it->ativo = 1;
    it->x = frand(40.0f, (float)LARG_TELA - 40.0f);
    it->y = (float)ALT_HUD - ITEM_RAIO;
    it->velocidade = frand(ITEM_VEL_MIN, ITEM_VEL_MAX);
    it->cor = CORES_ITEM[GetRandomValue(0, N_CORES_ITEM - 1)];

    // escolhe se cai ingrediente da receita ou distrator
    // chance de ~55% pra item da receita (60% se ainda falta muito)
    int faltam = catcher.n_alvos - catcher.coletados;
    int chance_receita = (faltam > 0) ? 55 : 15;
    if (faltam >= 3) chance_receita = 65;

    int sortear_receita = (GetRandomValue(1, 100) <= chance_receita);

    if (sortear_receita && faltam > 0) {
        // escolhe um alvo ainda nao coletado (prioriza pendentes mas pode repetir)
        int candidatos[CATCHER_MAX_ALVOS];
        int n = 0;
        for (int i = 0; i < catcher.n_alvos; i++) {
            if (!catcher.alvos[i].coletado) candidatos[n++] = i;
        }
        int escolha;
        if (n > 0) escolha = candidatos[GetRandomValue(0, n - 1)];
        else       escolha = GetRandomValue(0, catcher.n_alvos - 1);

        catcher.spawnou[escolha] = 1;
        it->eh_da_receita = 1;
        it->idx_alvo = escolha;
        strncpy(it->nome, catcher.alvos[escolha].nome, sizeof(it->nome) - 1);
        it->nome[sizeof(it->nome) - 1] = '\0';
    } else {
        // escolhe distrator que nao seja alvo
        const char *nome = NULL;
        for (int tentativas = 0; tentativas < 8; tentativas++) {
            const char *cand = DISTRATORES_GERAIS[GetRandomValue(0, N_DISTRATORES_GERAIS - 1)];
            if (!eh_alvo(cand)) { nome = cand; break; }
        }
        if (nome == NULL) nome = "Sapato"; // fallback improvavel

        it->eh_da_receita = 0;
        it->idx_alvo = -1;
        strncpy(it->nome, nome, sizeof(it->nome) - 1);
        it->nome[sizeof(it->nome) - 1] = '\0';
    }
}

// força o spawn de um ingrediente específico (pelo índice em alvos)
static void spawnar_forcado(int idx) {
    int slot = -1;
    for (int i = 0; i < CATCHER_MAX_ITENS; i++) {
        if (!catcher.itens[i].ativo) { slot = i; break; }
    }
    if (slot < 0) return;

    ItemCaindo *it = &catcher.itens[slot];
    it->ativo = 1;
    it->x = frand(60.0f, (float)LARG_TELA - 60.0f);
    it->y = (float)ALT_HUD - ITEM_RAIO;
    it->velocidade = frand(ITEM_VEL_MIN, ITEM_VEL_MIN + 40.0f);
    it->cor = CORES_ITEM[GetRandomValue(0, N_CORES_ITEM - 1)];
    it->eh_da_receita = 1;
    it->idx_alvo = idx;
    catcher.spawnou[idx] = 1;
    strncpy(it->nome, catcher.alvos[idx].nome, sizeof(it->nome) - 1);
    it->nome[sizeof(it->nome) - 1] = '\0';
}

// ==========================================
// INICIALIZACAO
// ==========================================
void catcher_iniciar(Receita *receita) {
    // descarrega textura anterior se existir
    if (catcher.bg_carregado) {
        UnloadTexture(catcher.bg_coletar);
    }

    memset(&catcher, 0, sizeof(catcher));
    catcher.receita = receita;

    // copia ingredientes da receita para alvos
    if (receita != NULL) {
        for (int i = 0; i < receita->n_ingredientes && i < CATCHER_MAX_ALVOS; i++) {
            strncpy(catcher.alvos[i].nome, receita->ingredientes[i],
                    sizeof(catcher.alvos[i].nome) - 1);
            catcher.alvos[i].nome[sizeof(catcher.alvos[i].nome) - 1] = '\0';
            catcher.alvos[i].coletado = 0;
            catcher.n_alvos++;
        }
    }

    // cesta comeca centralizada na base
    catcher.cesta_w = CESTA_LARGURA;
    catcher.cesta_h = CESTA_ALTURA;
    catcher.cesta_x = (LARG_TELA - CESTA_LARGURA) / 2.0f;
    catcher.cesta_y = ALT_TELA - ALT_CHAO - CESTA_ALTURA - 10;
    catcher.cesta_velocidade = CESTA_VEL;

    catcher.tempo_inicio = GetTime();
    catcher.tempo_decorrido = 0.0f;
    catcher.periodo_garantido = (receita != NULL && receita->dificuldade <= 3) ? 30 : 20;
    catcher.ultimo_segundo_penalizado = catcher.periodo_garantido;

    catcher.pontos = estado.pontuacao;   // herda pontuacao atual
    catcher.erros_distrator = 0;
    catcher.perdidos = 0;
    catcher.coletados = 0;

    catcher.iniciado = 1;
    catcher.terminou = 0;
    catcher.venceu   = 0;
    catcher.pausado  = 0;

    catcher.spawn_timer = 0.0f;
    catcher.spawn_intervalo = SPAWN_BASE;

    // carrega textura de fundo
    catcher.bg_coletar = LoadTexture("sprites/bg_coletar.png");
    if (catcher.bg_coletar.id != 0) {
        SetTextureFilter(catcher.bg_coletar, TEXTURE_FILTER_BILINEAR);
        catcher.bg_carregado = 1;
    }

    printf("[CATCHER] Iniciado para '%s' com %d ingredientes\n",
           receita ? receita->nome : "(sem receita)", catcher.n_alvos);
}

// ==========================================
// COLISAO
// ==========================================
// AABB entre circulo do item e retangulo da cesta
static int item_pegou_cesta(const ItemCaindo *it) {
    Rectangle r = { catcher.cesta_x, catcher.cesta_y,
                    catcher.cesta_w, catcher.cesta_h };
    return CheckCollisionCircleRec((Vector2){ it->x, it->y }, ITEM_RAIO, r);
}

// ==========================================
// ATUALIZACAO
// ==========================================
void catcher_atualizar(void) {
    if (!catcher.iniciado || catcher.terminou) return;

    if (IsKeyPressed(KEY_P)) catcher.pausado = !catcher.pausado;
    if (catcher.pausado) return;

    float dt = GetFrameTime();

    // ---- timer ----
    catcher.tempo_decorrido = (float)(GetTime() - catcher.tempo_inicio);

    // penalidade por segundo somente apos o periodo garantido
    int seg_inteiro = (int)catcher.tempo_decorrido;
    if (catcher.coletados < catcher.n_alvos &&
        seg_inteiro > catcher.ultimo_segundo_penalizado) {
        int delta = seg_inteiro - catcher.ultimo_segundo_penalizado;
        estado.tempo_extra += delta;
        catcher.pontos -= delta;
        if (catcher.pontos < 0) catcher.pontos = 0;
        catcher.ultimo_segundo_penalizado = seg_inteiro;
    }

    // ---- movimento da cesta ----
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        catcher.cesta_x -= catcher.cesta_velocidade * dt;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        catcher.cesta_x += catcher.cesta_velocidade * dt;
    }
    if (catcher.cesta_x < 0) catcher.cesta_x = 0;
    if (catcher.cesta_x + catcher.cesta_w > LARG_TELA) {
        catcher.cesta_x = LARG_TELA - catcher.cesta_w;
    }

    // ---- garantia: forca spawn de ingredientes que nunca apareceram ----
    // dispara 5 segundos antes do periodo garantido expirar
    float limite_spawn = (float)(catcher.periodo_garantido - 5);
    if (catcher.tempo_decorrido >= limite_spawn) {
        for (int i = 0; i < catcher.n_alvos; i++) {
            if (!catcher.alvos[i].coletado && !catcher.spawnou[i]) {
                spawnar_forcado(i);
            }
        }
    }

    // ---- spawn de itens ----
    catcher.spawn_timer += dt;
    // intervalo encurta ate o fim do periodo garantido, depois fica no minimo
    float ref = (float)catcher.periodo_garantido;
    float prog = catcher.tempo_decorrido < ref ? catcher.tempo_decorrido / ref : 1.0f;
    catcher.spawn_intervalo = SPAWN_BASE - (SPAWN_BASE - SPAWN_MIN) * prog;
    if (catcher.spawn_timer >= catcher.spawn_intervalo) {
        catcher.spawn_timer = 0.0f;
        spawnar_item();
    }

    // ---- atualiza itens ----
    for (int i = 0; i < CATCHER_MAX_ITENS; i++) {
        ItemCaindo *it = &catcher.itens[i];
        if (!it->ativo) continue;

        it->velocidade += 120.0f * dt;  // aceleracao gravitacional
        it->y += it->velocidade * dt;

        // pegou na cesta?
        if (item_pegou_cesta(it)) {
            if (it->eh_da_receita) {
                int idx = it->idx_alvo;
                if (idx >= 0 && idx < catcher.n_alvos &&
                    !catcher.alvos[idx].coletado) {
                    catcher.alvos[idx].coletado = 1;
                    catcher.ordem_coleta[catcher.n_coletados_em_ordem++] = idx;
                    catcher.coletados++;
                    catcher.pontos += 5; // bonus por acerto
                }
            } else {
                catcher.erros_distrator++;
                catcher.pontos -= 3; // penalidade por pegar errado
                if (catcher.pontos < 0) catcher.pontos = 0;
            }
            it->ativo = 0;
            continue;
        }

        // saiu pela base?
        if (it->y - ITEM_RAIO > ALT_TELA - ALT_CHAO) {
            if (it->eh_da_receita) {
                catcher.perdidos++;
            }
            it->ativo = 0;
        }
    }

    // ---- vitoria: todos os ingredientes coletados ----
    if (catcher.coletados >= catcher.n_alvos) {
        catcher.terminou = 1;
        catcher.venceu   = 1;
        estado.pontuacao = catcher.pontos;
    }
}

// ==========================================
// DESENHO
// ==========================================
static void desenhar_cesta(void) {
    float x = catcher.cesta_x;
    float y = catcher.cesta_y;
    float w = catcher.cesta_w;
    float h = catcher.cesta_h;

    if (_spr_cesta_carregado) {
        // desenha o sprite escalonado pro tamanho da cesta
        Rectangle src = { 0, 0, (float)_spr_cesta.width, (float)_spr_cesta.height };
        Rectangle dst = { x, y, w, h };
        DrawTexturePro(_spr_cesta, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);
    } else {
        // fallback: desenho geometrico se o sprite nao carregou
        // borda superior (alca)
        DrawRectangleRounded((Rectangle){x - 6, y - 8, w + 12, 14}, 0.6f, 8, COR_CESTA_ESC);

        // corpo da cesta (trapezoide simulado por dois retangulos)
        DrawRectangleRounded((Rectangle){x, y, w, h}, 0.35f, 8, COR_CESTA);
        DrawRectangleRoundedLines((Rectangle){x, y, w, h}, 0.35f, 8, COR_CESTA_ESC);

        // tramas horizontais
        for (int i = 1; i < 4; i++) {
            float ly = y + (h / 4.0f) * i;
            DrawLine((int)(x + 6), (int)ly, (int)(x + w - 6), (int)ly, COR_CESTA_ESC);
        }
        // tramas verticais
        for (int i = 1; i < 6; i++) {
            float lx = x + (w / 6.0f) * i;
            DrawLine((int)lx, (int)(y + 4), (int)lx, (int)(y + h - 4), COR_CESTA_ESC);
        }
    }
}

static void desenhar_item(const ItemCaindo *it) {
    if (!it->ativo) return;

    const Texture2D *spr = _sprite_para_ingrediente(it->nome);

    if (spr != NULL) {
        // tamanho de exibicao mantendo o aspect ratio original do sprite
        float aspect = (float)spr->width / (float)spr->height;
        float tam_alvo = ITEM_RAIO * 5.0f;  // ~110px
        float dw, dh;
        if (aspect >= 1.0f) {
            dw = tam_alvo;
            dh = tam_alvo / aspect;
        } else {
            dh = tam_alvo;
            dw = tam_alvo * aspect;
        }

        // sombra
        DrawTextureEx(*spr, (Vector2){ it->x - dw/2 + 3, it->y - dh/2 + 3 },
                      0.0f, dw / (float)spr->width, (Color){0, 0, 0, 80});

        // desenha o sprite centralizado
        Rectangle src = { 0, 0, (float)spr->width, (float)spr->height };
        Rectangle dst = { it->x - dw/2, it->y - dh/2, dw, dh };
        DrawTexturePro(*spr, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);

        // borda colorida indicando se eh da receita
        DrawCircleLines((int)it->x, (int)it->y, ITEM_RAIO + 2,
                        it->eh_da_receita ? COR_BOM : COR_RUIM);
    } else {
        // fallback: desenho circular original
        // sombra
        DrawCircle((int)it->x + 3, (int)it->y + 3, ITEM_RAIO, (Color){0, 0, 0, 60});
        // corpo
        DrawCircle((int)it->x, (int)it->y, ITEM_RAIO, it->cor);
        DrawCircleLines((int)it->x, (int)it->y, ITEM_RAIO,
                        it->eh_da_receita ? COR_BOM : COR_RUIM);

        // letra inicial centralizada
        char letra[2] = { (char)toupper((unsigned char)it->nome[0]), '\0' };
        int tam = 22;
        int lw = MeasureText(letra, tam);
        DrawText(letra, (int)it->x - lw / 2, (int)it->y - tam / 2, tam, WHITE);
    }

    // rotulo curto abaixo (sempre desenha pra dar identidade ao item)
    int tw = MeasureText(it->nome, 12);
    DrawText(it->nome, (int)it->x - tw / 2, (int)it->y + ITEM_RAIO + 2,
             12, COR_TEXTO);
}

static void desenhar_hud(void) {
    // faixa superior
    DrawRectangle(0, 0, LARG_TELA, ALT_HUD, COR_HUD);
    DrawRectangle(0, ALT_HUD - 4, LARG_TELA, 4, (Color){255, 200, 0, 255});

    // indicador de periodo garantido / tempo extra
    int elapsed = (int)catcher.tempo_decorrido;
    if (elapsed < catcher.periodo_garantido) {
        int restante = catcher.periodo_garantido - elapsed;
        DrawText(TextFormat("Garantido: %02ds", restante), 20, 18, 28, WHITE);
    } else {
        int extra = elapsed - catcher.periodo_garantido;
        DrawText(TextFormat("+%02ds", extra), 20, 18, 28, (Color){255, 80, 80, 255});
    }

    // pontos
    DrawText(TextFormat("Pontos: %d", catcher.pontos), 240, 18, 28,
             (Color){255, 230, 120, 255});

    // progresso
    DrawText(TextFormat("Coletados: %d / %d",
                        catcher.coletados, catcher.n_alvos),
             480, 18, 24, WHITE);
}

static void desenhar_checklist(void) {
    // painel a direita com os ingredientes que faltam
    int x = LARG_TELA - 220;
    int y = ALT_HUD + 16;
    int w = 200;
    int h = 28 + catcher.n_alvos * 24 + 12;

    DrawRectangleRounded((Rectangle){x, y, w, h}, 0.1f, 8,
                         (Color){255, 255, 255, 220});
    DrawRectangleRoundedLines((Rectangle){x, y, w, h}, 0.1f, 8, COR_HUD);
    DrawText("Receita", x + 12, y + 8, 18, COR_TEXTO);

    for (int i = 0; i < catcher.n_alvos; i++) {
        int ly = y + 32 + i * 24;
        // bolinha de status
        if (catcher.alvos[i].coletado) {
            DrawCircle(x + 22, ly + 8, 7, COR_BOM);
            DrawText("v", x + 18, ly + 1, 14, WHITE);
        } else {
            DrawCircleLines(x + 22, ly + 8, 7, COR_TEXTO);
        }

        Color cor = catcher.alvos[i].coletado ? GRAY : COR_TEXTO;
        // trunca nome longo
        char buf[24];
        snprintf(buf, sizeof(buf), "%.20s", catcher.alvos[i].nome);
        DrawText(buf, x + 38, ly, 16, cor);
    }
}

static void desenhar_fundo(void) {
    // desenha textura de fundo se carregada
    if (catcher.bg_carregado) {
        DrawTexturePro(catcher.bg_coletar,
                       (Rectangle){0, 0, catcher.bg_coletar.width, catcher.bg_coletar.height},
                       (Rectangle){0, ALT_HUD, LARG_TELA, ALT_TELA - ALT_HUD},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        // fallback para fundo com gradiente
        DrawRectangleGradientV(0, ALT_HUD, LARG_TELA, ALT_TELA - ALT_HUD - ALT_CHAO,
                               COR_CEU, COR_CEU_BAIXO);

        // nuvens decorativas
        DrawCircle(120, 130, 22, (Color){255, 255, 255, 200});
        DrawCircle(150, 125, 26, (Color){255, 255, 255, 200});
        DrawCircle(178, 132, 20, (Color){255, 255, 255, 200});

        DrawCircle(560, 100, 18, (Color){255, 255, 255, 180});
        DrawCircle(585, 95,  24, (Color){255, 255, 255, 180});
        DrawCircle(615, 105, 20, (Color){255, 255, 255, 180});
    }

    // chao
    DrawRectangle(0, ALT_TELA - ALT_CHAO, LARG_TELA, ALT_CHAO, COR_CHAO);
    DrawRectangle(0, ALT_TELA - ALT_CHAO, LARG_TELA, 6, (Color){60, 110, 40, 255});
}

static void desenhar_tela_fim(void) {
    DrawRectangle(0, 0, LARG_TELA, ALT_TELA, (Color){0, 0, 0, 170});

    const char *titulo = "RECEITA COMPLETA!";
    Color cor_titulo = COR_BOM;
    int tam = 48;
    int tw = MeasureText(titulo, tam);
    DrawText(titulo, (LARG_TELA - tw) / 2, 180, tam, cor_titulo);

    DrawText(TextFormat("Pontuacao final: %d", catcher.pontos),
             260, 260, 26, WHITE);
    DrawText(TextFormat("Coletados: %d / %d", catcher.coletados, catcher.n_alvos),
             260, 295, 22, WHITE);
    DrawText(TextFormat("Erros (item errado): %d", catcher.erros_distrator),
             260, 325, 20, (Color){255, 200, 200, 255});
    DrawText(TextFormat("Ingredientes perdidos: %d", catcher.perdidos),
             260, 350, 20, (Color){255, 200, 200, 255});

    DrawText("[ENTER] Voltar ao menu  |  [R] Jogar de novo",
             180, 470, 20, (Color){255, 230, 120, 255});
}

void tela_catcher(void) {
    catcher_atualizar();

    ClearBackground(COR_CEU);
    desenhar_fundo();

    // itens
    for (int i = 0; i < CATCHER_MAX_ITENS; i++) {
        desenhar_item(&catcher.itens[i]);
    }

    desenhar_cesta();
    desenhar_hud();
    desenhar_checklist();

    // instrucoes na base
    DrawText("Use [<-] [->] ou [A] [D] para mover a cesta  |  [P] Pausa",
             20, ALT_TELA - ALT_CHAO + 12, 16, WHITE);

    if (catcher.pausado && !catcher.terminou) {
        DrawRectangle(0, 0, LARG_TELA, ALT_TELA, (Color){0, 0, 0, 140});
        const char *t = "PAUSADO";
        int tw = MeasureText(t, 56);
        DrawText(t, (LARG_TELA - tw) / 2, 260, 56, WHITE);
        DrawText("[P] retomar", 340, 330, 22, (Color){255, 230, 120, 255});
    }

    if (catcher.terminou) {
        desenhar_tela_fim();
    }
}

int catcher_terminou(void) {
    return catcher.terminou;
}

int catcher_venceu(void) {
    return catcher.venceu;
}
