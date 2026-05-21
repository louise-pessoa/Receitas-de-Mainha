#include "receitas.h"
#include "pilha.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Receita *_criar_no_receita(const char *nome, int dificuldade, int tempo, int pontuacao) {
    Receita *novo = (Receita *) malloc(sizeof(Receita));
    if (novo == NULL) return NULL;

    strncpy(novo->nome, nome, sizeof(novo->nome) - 1);
    novo->nome[sizeof(novo->nome) - 1] = '\0';
    novo->dificuldade    = dificuldade;
    novo->tempo          = tempo;
    novo->pontuacao      = pontuacao;
    novo->desbloqueada   = 1;  // inicialmente desbloqueada
    novo->n_ingredientes = 0;
    novo->img_receita_pronta[0] = '\0';
    novo->img_inicio[0]         = '\0';
    novo->passos = NULL;
    novo->prox   = NULL;
    return novo;
}

// Lista de receitas

Receita *inserir_receita(Receita *lista, const char *nome, int dificuldade, int tempo, int pontuacao) {
    Receita *novo = _criar_no_receita(nome, dificuldade, tempo, pontuacao);
    if (novo == NULL) return lista;

    if (lista == NULL) return novo;

    Receita *aux = lista;
    while (aux->prox != NULL) aux = aux->prox;
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
        printf("%d. %-20s | dificuldade: %d | tempo: %3d min | pontos: %d\n",
               i, aux->nome, aux->dificuldade, aux->tempo, aux->pontuacao);
        aux = aux->prox;
        i++;
    }
    printf("============================\n");
}

Receita *buscar_receita(Receita *lista, const char *nome) {
    Receita *aux = lista;
    while (aux != NULL) {
        if (strcmp(aux->nome, nome) == 0) return aux;
        aux = aux->prox;
    }
    return NULL;
}

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

// Ingredientes

Receita *inserir_ingrediente(Receita *receita, const char *nome) {
    if (receita == NULL) return NULL;
    if (receita->n_ingredientes >= MAX_INGREDIENTES) return receita;

    strncpy(receita->ingredientes[receita->n_ingredientes], nome, 59);
    receita->ingredientes[receita->n_ingredientes][59] = '\0';
    receita->n_ingredientes++;
    return receita;
}

void listar_ingredientes(const Receita *receita) {
    if (receita == NULL) { printf("Receita invalida.\n"); return; }

    printf("\n--- Ingredientes de %s ---\n", receita->nome);
    if (receita->n_ingredientes == 0) {
        printf("  (nenhum ingrediente cadastrado)\n");
        return;
    }
    for (int i = 0; i < receita->n_ingredientes; i++) {
        printf("  %d. %s\n", i + 1, receita->ingredientes[i]);
    }
}

void liberar_receitas(Receita *lista) {
    while (lista != NULL) {
        Receita *tmp = lista;
        lista = lista->prox;
        while (!pilha_vazia(tmp->passos))
            tmp->passos = pop_passo(tmp->passos);
        free(tmp);
    }
}
