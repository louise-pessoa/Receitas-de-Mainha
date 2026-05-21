#include "pilha.h"
#include <stdlib.h>
#include <string.h>

// cria um no com o passo informado
No* _criar_no_passo(const char *acao, const char *ingrediente,
                    const char *teclas, const char *img_passo) {
    No* novo = (No*)malloc(sizeof(No));

    if (novo == NULL) return NULL;

    strncpy(novo->dado.acao,        acao,       sizeof(novo->dado.acao)       - 1);
    strncpy(novo->dado.ingrediente, ingrediente,sizeof(novo->dado.ingrediente)- 1);
    strncpy(novo->dado.teclas,      teclas,     sizeof(novo->dado.teclas)     - 1);
    strncpy(novo->dado.img_passo,   img_passo,  sizeof(novo->dado.img_passo)  - 1);
    novo->dado.acao[sizeof(novo->dado.acao)-1]               = '\0';
    novo->dado.ingrediente[sizeof(novo->dado.ingrediente)-1] = '\0';
    novo->dado.teclas[sizeof(novo->dado.teclas)-1]           = '\0';
    novo->dado.img_passo[sizeof(novo->dado.img_passo)-1]     = '\0';

    novo->prox = NULL;

    return novo;
}

// verifica se a pilha esta vazia
int pilha_vazia(No* topo) {
    return topo == NULL;
}

// empilha um novo passo
No* push_passo(No* topo, const char *acao, const char *ingrediente,
               const char *teclas, const char *img_passo) {
    No* novo = _criar_no_passo(acao, ingrediente, teclas, img_passo);

    if (novo == NULL) {
        return topo;
    }

    novo->prox = topo;
    topo = novo;

    return topo;
}

// desempilha o topo
No* pop_passo(No* topo) {
    if (topo == NULL) {
        return NULL;
    }

    No* removido = topo;
    topo = topo->prox;
    free(removido);

    return topo;
}

// retorna o passo do topo sem remover
Passo ver_topo(No* topo) {
    if (topo == NULL) {
        Passo vazio = {0};
        return vazio;
    }

    return topo->dado;
}