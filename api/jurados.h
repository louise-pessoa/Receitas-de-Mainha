#ifndef JURADOS_H
#define JURADOS_H

#include "../jogo.h"

/* Resultado retornado pelos 3 jurados apos avaliacao via API do Groq */
typedef struct {
    float nota_ariano;
    float nota_clarice;
    float nota_chico;
    char  comentario_ariano[256];
    char  comentario_clarice[256];
    char  comentario_chico[256];
    float media_final;
} ResultadoJurados;

/*
 * Chama os 3 jurados em sequencia via API do Groq e retorna as notas e
 * comentarios. A chave de API e lida da variavel de ambiente GROQ_API_KEY.
 * Se a variavel nao existir ou a chamada falhar, retorna struct zerada.
 */
ResultadoJurados avaliar_com_jurados(EstadoJogo estado);

#endif