#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jurados.h"

/* Avalia o desempenho localmente, sem chamada de rede.
 * As notas refletem as personalidades dos 3 jurados. */
ResultadoJurados avaliar_com_jurados(EstadoJogo estado)
{
    ResultadoJurados resultado;
    memset(&resultado, 0, sizeof(resultado));

    float acerto = (estado.passos_total > 0)
        ? (float)estado.passos_acertados / (float)estado.passos_total
        : 0.0f;

    int perfeito = (estado.passos_acertados == estado.passos_total &&
                    estado.passos_total > 0 &&
                    estado.erro_passo == 0 &&
                    estado.tempo_extra == 0);

    /* Ariano: generoso, valoriza esforca */
    resultado.nota_ariano = 5.0f + acerto * 4.0f;
    if (estado.erro_passo == 0) resultado.nota_ariano += 0.5f;
    if (resultado.nota_ariano > 10.0f) resultado.nota_ariano = 10.0f;
    if (perfeito)
        strncpy(resultado.comentario_ariano,
                "Que riqueza de talento! O Nordeste se orgulha de voce!",
                sizeof(resultado.comentario_ariano) - 1);
    else if (acerto >= 0.7f)
        strncpy(resultado.comentario_ariano,
                "Bela dedicacao, meu povo! A cultura nordestina vive em cada prato.",
                sizeof(resultado.comentario_ariano) - 1);
    else
        strncpy(resultado.comentario_ariano,
                "O esforca e louvavel, continue tentando com o coracao!",
                sizeof(resultado.comentario_ariano) - 1);

    /* Clarice: exigente, so aceita precisao */
    resultado.nota_clarice = acerto * 7.0f;
    resultado.nota_clarice -= estado.erro_passo * 0.8f;
    resultado.nota_clarice -= estado.tempo_extra * 0.3f;
    if (resultado.nota_clarice < 0.0f) resultado.nota_clarice = 0.0f;
    if (perfeito) resultado.nota_clarice = 9.5f;
    if (resultado.nota_clarice > 10.0f) resultado.nota_clarice = 10.0f;
    if (perfeito)
        strncpy(resultado.comentario_clarice,
                "Execucao impecavel. Raros sao os que atingem tal precisao.",
                sizeof(resultado.comentario_clarice) - 1);
    else if (acerto >= 0.8f)
        strncpy(resultado.comentario_clarice,
                "Quase. A perfeicao exige ainda mais silencio e foco.",
                sizeof(resultado.comentario_clarice) - 1);
    else
        strncpy(resultado.comentario_clarice,
                "Erros nao sao romantismo. Sao descuido.",
                sizeof(resultado.comentario_clarice) - 1);

    /* Chico Science: sem meio-termo, energia do mangue beat */
    if (perfeito) {
        resultado.nota_chico = 9.5f;
        strncpy(resultado.comentario_chico,
                "Zum zum zum! Perfeito como o mangue beat — sem erro, sem frescura!",
                sizeof(resultado.comentario_chico) - 1);
    } else if (acerto >= 0.8f && estado.erro_passo == 0) {
        resultado.nota_chico = 7.0f + acerto * 2.0f;
        if (resultado.nota_chico > 10.0f) resultado.nota_chico = 10.0f;
        strncpy(resultado.comentario_chico,
                "Caranguejos com cerebro! Quase la, bora repetir com mais forca!",
                sizeof(resultado.comentario_chico) - 1);
    } else if (acerto >= 0.5f) {
        resultado.nota_chico = 4.0f + acerto * 2.0f;
        strncpy(resultado.comentario_chico,
                "No meio do caminho tinha um erro... mas a vibe tava boa!",
                sizeof(resultado.comentario_chico) - 1);
    } else {
        resultado.nota_chico = acerto * 3.0f;
        strncpy(resultado.comentario_chico,
                "Da lama ao caos — voltou mais ao caos do que ao prato!",
                sizeof(resultado.comentario_chico) - 1);
    }

    resultado.media_final = (resultado.nota_ariano +
                             resultado.nota_clarice +
                             resultado.nota_chico) / 3.0f;

    fprintf(stderr, "[jurados] Ariano=%.1f Clarice=%.1f Chico=%.1f media=%.2f\n",
            resultado.nota_ariano, resultado.nota_clarice,
            resultado.nota_chico, resultado.media_final);

    return resultado;
}
