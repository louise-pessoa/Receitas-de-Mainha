#include "receitas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
Receita *_criar_no_receita(const char *nome, int dificuldade,int tempo, int pontuacao) {
    Receita *novo = (Receita *) malloc(sizeof(Receita));
    if (novo == NULL) return NULL;

    strncpy(novo->nome, nome, sizeof(novo->nome) - 1);
    novo->nome[sizeof(novo->nome) - 1] = '\0';
    novo->dificuldade = dificuldade;
    novo->tempo = tempo;
    novo->pontuacao = pontuacao;
    novo->desbloqueada = 1;  // inicialmente desbloqueada
    novo->ingredientes = NULL;

    novo->passo_acao[0]  = '\0';
    novo->ingrediente[0] = '\0';

    novo->n_passos_jog = 0;
    novo->img_receita_pronta[0] = '\0';
    novo->img_inicio[0] = '\0';

    novo->passos       = NULL;
    novo->prox         = NULL;
    return novo;
}

Ingrediente *_criar_no_ingrediente(const char *nome, const char *quantidade) {
    Ingrediente *novo = (Ingrediente *) malloc(sizeof(Ingrediente));
    if (novo == NULL) {
        return NULL;
    }

    strncpy(novo->nome, nome, sizeof(novo->nome) - 1);
    novo->nome[sizeof(novo->nome) - 1] = '\0';

    strncpy(novo->quantidade, quantidade, sizeof(novo->quantidade) - 1);
    novo->quantidade[sizeof(novo->quantidade) - 1] = '\0';

    novo->prox = NULL;
    return novo;
}

// Lista de receitas

// Insere no final da lista e retorna o inicio da lista (pode ser o novo no se a lista tava vazia).
Receita *inserir_receita(Receita *lista, const char *nome,int dificuldade, int tempo, int pontuacao) {
    Receita *novo = _criar_no_receita(nome, dificuldade, tempo, pontuacao);
    if (novo == NULL) return lista;

    if (lista == NULL) {
        return novo;
    }

    Receita *aux = lista;
    while (aux->prox != NULL) {
        aux = aux->prox;
    }
    aux->prox = novo;
    return lista;
}

void listar_receitas(Receita *lista) {
    if (lista == NULL) {
        printf("Nenhuma receita cadastrada.\n");
        return;
    }

    printf("\n=== Receitas Disponiveis ===\n");
    int i = 1;
    Receita *aux = lista;
    while (aux != NULL) {
        printf("%d. %-20s | dificuldade: %d | tempo: %3d min | pontos: %d\n",i, aux->nome, aux->dificuldade, aux->tempo, aux->pontuacao);
        aux = aux->prox;
        i++;
    }
    printf("============================\n");
}

// Busca por nome 

Receita *buscar_receita(Receita *lista, const char *nome) {
    Receita *aux = lista;
    while (aux != NULL) {
        if (strcmp(aux->nome, nome) == 0) {
            return aux;
        }
        aux = aux->prox;
    }
    return NULL;
}

// Busca por indice 
Receita *buscar_receita_idx(Receita *lista, int indice) {
    if (indice < 1) return NULL;
    int i = 1;
    Receita *aux = lista;
    while (aux != NULL) {
        if (i == indice) return aux;
        aux = aux->prox;
        i++;
    }
    return NULL;
}

void definir_passo(Receita *no, const char *acao, const char *ing) {
    if (no == NULL) return;
    strncpy(no->passo_acao, acao, sizeof(no->passo_acao) - 1);
    no->passo_acao[sizeof(no->passo_acao) - 1] = '\0';
    strncpy(no->ingrediente, ing, sizeof(no->ingrediente) - 1);
    no->ingrediente[sizeof(no->ingrediente) - 1] = '\0';
}

void adicionar_passo_jogavel(Receita *receita, const char *acao,
                             const char *ingrediente, const char *teclas,
                             int tempo_limite) {
    if (receita == NULL) return;
    if (receita->n_passos_jog >= MAX_PASSOS_JOGAVEIS) return;

    PassoJogavel *p = &receita->passos_jog[receita->n_passos_jog++];
    strncpy(p->acao, acao, sizeof(p->acao) - 1);
    p->acao[sizeof(p->acao) - 1] = '\0';
    strncpy(p->ingrediente, ingrediente, sizeof(p->ingrediente) - 1);
    p->ingrediente[sizeof(p->ingrediente) - 1] = '\0';
    strncpy(p->teclas, teclas, sizeof(p->teclas) - 1);
    p->teclas[sizeof(p->teclas) - 1] = '\0';
    p->tempo_limite = tempo_limite;
    p->img_passo[0] = '\0';  // sem imagem por padrão
}

void adicionar_passo_jogavel_com_img(Receita *receita, const char *acao,
                                     const char *ingrediente, const char *teclas,
                                     int tempo_limite, const char *img_passo) {
    if (receita == NULL) return;
    if (receita->n_passos_jog >= MAX_PASSOS_JOGAVEIS) return;

    PassoJogavel *p = &receita->passos_jog[receita->n_passos_jog++];
    strncpy(p->acao, acao, sizeof(p->acao) - 1);
    p->acao[sizeof(p->acao) - 1] = '\0';
    strncpy(p->ingrediente, ingrediente, sizeof(p->ingrediente) - 1);
    p->ingrediente[sizeof(p->ingrediente) - 1] = '\0';
    strncpy(p->teclas, teclas, sizeof(p->teclas) - 1);
    p->teclas[sizeof(p->teclas) - 1] = '\0';
    p->tempo_limite = tempo_limite;
    strncpy(p->img_passo, img_passo, sizeof(p->img_passo) - 1);
    p->img_passo[sizeof(p->img_passo) - 1] = '\0';
}

// Lista de ingredientes por receita

// Adiciona um ingrediente no final da lista de ingredientes da receita.
Receita *inserir_ingrediente(Receita *receita, const char *nome,const char *quantidade) {
    if (receita == NULL) {
        fprintf(stderr, "Erro: receita nula em inserir_ingrediente.\n");
        return NULL;
    }

    Ingrediente *novo = _criar_no_ingrediente(nome, quantidade);
    if (novo == NULL) return receita;

    if (receita->ingredientes == NULL) {
        receita->ingredientes = novo;
    } else {
        Ingrediente *aux = receita->ingredientes;
        while (aux->prox != NULL) {
            aux = aux->prox;
        }
        aux->prox = novo;
    }
    return receita;
}

void listar_ingredientes(const Receita *receita) {
    if (receita == NULL) {
        printf("Receita invalida.\n");
        return;
    }

    printf("\n--- Ingredientes de %s ---\n", receita->nome);
    if (receita->ingredientes == NULL) {
        printf("  (nenhum ingrediente cadastrado)\n");
        return;
    }

    Ingrediente *aux = receita->ingredientes;
    int i = 1;
    while (aux != NULL) {
        printf("  %d. %-25s - %s\n", i, aux->nome, aux->quantidade);
        aux = aux->prox;
        i++;
    }
}

//(NP) Adicionei funções de liberação de memória 

static void liberar_ingredientes(Ingrediente *lista) {
    while (lista != NULL) {
        Ingrediente *tmp = lista;
        lista = lista->prox;
        free(tmp);
    }
}

void liberar_receitas(Receita *lista) {
    while (lista != NULL) {
        Receita *tmp = lista;
        lista = lista->prox;
        liberar_ingredientes(tmp->ingredientes);
        free(tmp);
    }
}