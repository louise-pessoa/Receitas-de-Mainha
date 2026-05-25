#ifndef FONTE_H
#define FONTE_H

#include "raylib.h"

// Ajuste global de tamanho: 1.0 = original, 1.2 = 20% maior
#define ESCALA_FONTE 1.2f

void fonte_carregar(const char *caminho, int tamanho_base);
void fonte_descarregar(void);
void txt(const char *texto, int x, int y, int tamanho, Color cor);
int  medir_txt(const char *texto, int tamanho);

#endif
