#ifndef RECEITAS_H
#define RECEITAS_H

#define MAX_INGREDIENTES 12

struct No;

typedef struct receita {
    char nome[60];
    int  dificuldade;
    int  tempo;
    int  pontuacao;
    int  desbloqueada;      // 1 se pode jogar, 0 se bloqueada

    char ingredientes[MAX_INGREDIENTES][60];  // nomes em ordem de uso
    int  n_ingredientes;

    char img_receita_pronta[120];
    char img_inicio[120];

    struct No  *passos;      // pilha de passos jogaveis (push na init, pop ao executar)
    struct receita *prox;
} Receita;

// Lista de receitas
Receita *inserir_receita(Receita *lista, const char *nome, int dificuldade, int tempo, int pontuacao);
void     listar_receitas(Receita *lista);
Receita *buscar_receita(Receita *lista, const char *nome);
Receita *buscar_receita_idx(Receita *lista, int indice);

// Ingredientes
Receita *inserir_ingrediente(Receita *receita, const char *nome);
void     listar_ingredientes(const Receita *receita);

Receita *_criar_no_receita(const char *nome, int dificuldade, int tempo, int pontuacao);

void liberar_receitas(Receita *lista);

#endif