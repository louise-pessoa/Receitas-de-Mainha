#ifndef RECEITAS_H
#define RECEITAS_H

#define MAX_PASSOS_JOGAVEIS 8

struct No;

typedef struct ingrediente {
    char nome[60];
    char quantidade[30];
    struct ingrediente *prox;
} Ingrediente;

// passo jogavel: o que o jogador faz na fase de cozinhar.
// teclas eh uma string com a sequencia que ele precisa apertar
// na ordem (ex: "WASDW", "SPACE", "QWE").
typedef struct {
    char acao[80];
    char ingrediente[60];   // nome do ingrediente a clicar
    char teclas[16];        // sequencia de teclas (sem espacos)
    int  tempo_limite;      // segundos para concluir o passo
    char img_passo[120];    // path da imagem durante o passo (ex: "sprites/frigideira_tapioca.png")
} PassoJogavel;


typedef struct receita {
    char nome[60];
    int  dificuldade;
    int  tempo;
    int  pontuacao;
    Ingrediente *ingredientes;

    char passo_acao[50];
    char ingrediente[50];

    PassoJogavel passos_jog[MAX_PASSOS_JOGAVEIS];
    int          n_passos_jog;

    char img_receita_pronta[120];  // path da imagem final da receita pronta (ex: "sprites/tapioca_pronta.png")
    char img_inicio[120];          // path da imagem de inicio da receita (ex: "sprites/frigideira_vazia.png")

    struct No  *passos;
    struct receita *prox;
} Receita;


// Lista de receitas 
Receita *inserir_receita(Receita *lista, const char *nome,int dificuldade, int tempo, int pontuacao);
void listar_receitas(Receita *lista);
Receita *buscar_receita(Receita *lista, const char *nome);     
Receita *buscar_receita_idx(Receita *lista, int indice);

// Lista de ingredientes 
Receita *inserir_ingrediente(Receita *receita, const char *nome, const char *quantidade);
void listar_ingredientes(const Receita *receita);


void definir_passo(Receita *no, const char *acao, const char *ing);

// adiciona um passo jogavel a receita
void adicionar_passo_jogavel(Receita *receita, const char *acao,
                             const char *ingrediente, const char *teclas,
                             int tempo_limite);

// adiciona um passo jogavel com imagem
void adicionar_passo_jogavel_com_img(Receita *receita, const char *acao,
                                     const char *ingrediente, const char *teclas,
                                     int tempo_limite, const char *img_passo);

Receita *_criar_no_receita(const char *nome, int dificuldade, int tempo, int pontuacao);

Ingrediente *_criar_no_ingrediente(const char *nome, const char *quantidade);

void liberar_receitas(Receita *lista);

#endif 