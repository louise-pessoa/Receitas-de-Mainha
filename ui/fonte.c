#include "fonte.h"

static Font fonte_jogo = {0};

void fonte_carregar(const char *caminho, int tamanho_base) {
    int cps[256];
    for (int i = 0; i < 256; i++) cps[i] = i;
    fonte_jogo = LoadFontEx(caminho, tamanho_base, cps, 256);
    if (fonte_jogo.texture.id == 0) {
        fonte_jogo = GetFontDefault();
    } else {
        SetTextureFilter(fonte_jogo.texture, TEXTURE_FILTER_BILINEAR);
    }
}

void fonte_descarregar(void) {
    if (fonte_jogo.texture.id != 0)
        UnloadFont(fonte_jogo);
}

void txt(const char *texto, int x, int y, int tamanho, Color cor) {
    if (fonte_jogo.texture.id == 0) {
        DrawText(texto, x, y, tamanho, cor);
        return;
    }
    float tam = (float)tamanho * ESCALA_FONTE;
    DrawTextEx(fonte_jogo, texto, (Vector2){(float)x, (float)y}, tam, tam * 0.04f, cor);
}

int medir_txt(const char *texto, int tamanho) {
    if (fonte_jogo.texture.id == 0)
        return MeasureText(texto, tamanho);
    float tam = (float)tamanho * ESCALA_FONTE;
    return (int)MeasureTextEx(fonte_jogo, texto, tam, tam * 0.04f).x;
}
