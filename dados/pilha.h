#ifndef PILHA_H
#define PILHA_H

// passo de receita armazenado na pilha
typedef struct Passo {
    char acao[80];
    char ingrediente[60];
    char teclas[16];
    char img_passo[120];
} Passo;

// no da pilha
typedef struct No {
    Passo dado;
    struct No* prox;
} No;

// funcoes da pilha
int   pilha_vazia(No *topo);
No   *push_passo(No *topo, const char *acao, const char *ingrediente,
                 const char *teclas, const char *img_passo);
No   *pop_passo(No *topo);
Passo ver_topo(No *topo);

#endif
