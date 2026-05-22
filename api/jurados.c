#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "jurados.h"

/* -------------------------------------------------------------------------
 * Constantes
 * ---------------------------------------------------------------------- */
#define GROQ_ENDPOINT   "https://api.groq.com/openai/v1/chat/completions"
#define GROQ_MODEL      "llama-3.3-70b-versatile"
#define GROQ_MAX_TOKENS 300
#define BUFFER_SIZE     8192
#define BODY_SIZE       16384

/* -------------------------------------------------------------------------
 * Lê um arquivo .env e injeta as variáveis no ambiente do processo.
 * Formato suportado: CHAVE=valor (linhas com # são ignoradas).
 * Não sobrescreve variáveis já definidas no ambiente.
 * ---------------------------------------------------------------------- */
static void carregar_dotenv(const char *caminho)
{
    FILE *f = fopen(caminho, "r");
    if (!f) return;

    char linha[512];
    while (fgets(linha, sizeof(linha), f)) {
        char *p = linha;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0') continue;

        char *eq = strchr(p, '=');
        if (!eq) continue;

        *eq = '\0';
        char *chave = p;
        char *valor = eq + 1;

        /* Remove \n e \r do final do valor */
        size_t len = strlen(valor);
        while (len > 0 && (valor[len-1] == '\n' || valor[len-1] == '\r'))
            valor[--len] = '\0';

        /* Remove aspas simples ou duplas ao redor do valor */
        if (len >= 2 &&
            ((valor[0] == '"'  && valor[len-1] == '"') ||
             (valor[0] == '\'' && valor[len-1] == '\''))) {
            valor[len-1] = '\0';
            valor++;
        }

#ifdef _WIN32
        _putenv_s(chave, valor);
#else
        setenv(chave, valor, 0); /* 0 = nao sobrescreve se ja existir */
#endif
    }
    fclose(f);
}

/* -------------------------------------------------------------------------
 * Escapa caracteres especiais de JSON em `src` e grava em `dst`.
 * Necessario antes de embutir qualquer string como valor dentro de JSON.
 * ---------------------------------------------------------------------- */
static void json_escape(const char *src, char *dst, size_t dst_size)
{
    size_t j = 0;
    for (size_t i = 0; src[i] && j + 2 < dst_size; i++) {
        if      (src[i] == '"')  { dst[j++] = '\\'; dst[j++] = '"';  }
        else if (src[i] == '\\') { dst[j++] = '\\'; dst[j++] = '\\'; }
        else if (src[i] == '\n') { dst[j++] = '\\'; dst[j++] = 'n';  }
        else if (src[i] == '\r') { dst[j++] = '\\'; dst[j++] = 'r';  }
        else if (src[i] == '\t') { dst[j++] = '\\'; dst[j++] = 't';  }
        else                     { dst[j++] = src[i]; }
    }
    dst[j] = '\0';
}

/* -------------------------------------------------------------------------
 * Buffer dinamico acumulado pelo callback do cURL
 * ---------------------------------------------------------------------- */
typedef struct {
    char  *dados;
    size_t tamanho;
} BufferResposta;

/* Callback chamado pelo cURL a cada chunk recebido; acumula em BufferResposta */
static size_t callback_escrita(void *conteudo, size_t tamanho,
                               size_t nmemb, void *userdata)
{
    size_t bytes_novos = tamanho * nmemb;
    BufferResposta *buf = (BufferResposta *)userdata;

    char *novo = realloc(buf->dados, buf->tamanho + bytes_novos + 1);
    if (!novo) return 0; /* sinaliza falha ao cURL */

    buf->dados = novo;
    memcpy(buf->dados + buf->tamanho, conteudo, bytes_novos);
    buf->tamanho += bytes_novos;
    buf->dados[buf->tamanho] = '\0';
    return bytes_novos;
}

/* -------------------------------------------------------------------------
 * Prompt unico que instrui o modelo a responder como os 3 jurados ao mesmo
 * tempo, garantindo personalidades contrastantes numa so chamada de API.
 * ---------------------------------------------------------------------- */
static const char *PROMPT_JURADOS =
    "Voce e o sistema de avaliacao de um jogo de culinaria pernambucana chamado Receitas de Mainha. "
    "Tres jurados lendarios vao avaliar o desempenho do jogador, cada um com personalidade e criterios MUITO diferentes. "
    "Use os dados de desempenho enviados pelo usuario para calcular notas distintas para cada jurado. "
    "\n\n"
    "ARIANO SUASSUNA: Poeta simpatico do Nordeste. Valoriza esforca, dedicacao e o espírito cultural. "
    "Da notas generosas: mesmo com erros, elogia a tentativa e o coracao. Faixa tipica de notas: 5 a 9. "
    "\n\n"
    "CLARICE LISPECTOR: Exigente e introspectiva. So aceita precisao e execucao impecavel. "
    "Implacavel com erros e passos incompletos. Faixa tipica: 0 a 7; so acima de 8 para desempenho quase perfeito. "
    "\n\n"
    "CHICO SCIENCE: Criativo e energetico do mangue beat. Recompensa quem concluiu com ousadia e zero erros. "
    "Sem meio-termo: receita concluida + sem erros = 9 a 10; mediana = 4 a 6; nenhum passo correto = 0 a 3. "
    "\n\n"
    "REGRA IMPORTANTE: as tres notas DEVEM ser diferentes entre si, refletindo as personalidades opostas dos jurados. "
    "Escreva comentarios curtos em portugues (maximo 2 frases), no estilo de cada jurado. "
    "Responda SOMENTE neste JSON exato, sem texto antes ou depois: "
    "{\"ariano\":{\"nota\":7.5,\"comentario\":\"texto\"},\"clarice\":{\"nota\":5.0,\"comentario\":\"texto\"},\"chico\":{\"nota\":8.0,\"comentario\":\"texto\"}}";

/* -------------------------------------------------------------------------
 * chamar_groq()
 *
 * Faz POST para a API do Groq com o prompt do jurado e a mensagem do usuario.
 * Extrai o campo "content" da resposta e grava em `saida`.
 * Retorna 0 em sucesso, -1 em falha.
 * ---------------------------------------------------------------------- */
static int chamar_groq(const char *chave_api,
                       const char *prompt_sistema,
                       const char *mensagem_usuario,
                       char *saida, size_t tamanho_saida)
{
    CURL *curl;
    CURLcode res;
    int sucesso = -1;

    BufferResposta buf;
    buf.dados   = malloc(1);
    buf.tamanho = 0;
    if (!buf.dados) return -1;
    buf.dados[0] = '\0';

    curl = curl_easy_init();
    if (!curl) {
        free(buf.dados);
        return -1;
    }

    /* Escapa aspas e barras antes de embutir as strings no JSON */
    char prompt_esc[BODY_SIZE];
    char mensagem_esc[1024];
    json_escape(prompt_sistema,   prompt_esc,   sizeof(prompt_esc));
    json_escape(mensagem_usuario, mensagem_esc, sizeof(mensagem_esc));

    /* Monta o corpo JSON da requisicao */
    char corpo[BODY_SIZE];
    snprintf(corpo, sizeof(corpo),
        "{"
          "\"model\":\"%s\","
          "\"max_tokens\":%d,"
          "\"messages\":["
            "{\"role\":\"system\",\"content\":\"%s\"},"
            "{\"role\":\"user\",\"content\":\"%s\"}"
          "]"
        "}",
        GROQ_MODEL, GROQ_MAX_TOKENS,
        prompt_esc,
        mensagem_esc);

    /* Headers obrigatorios para a API do Groq (formato OpenAI) */
    struct curl_slist *headers = NULL;
    char auth_header[512];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", chave_api);
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL,          GROQ_ENDPOINT);
    curl_easy_setopt(curl, CURLOPT_POST,          1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,    corpo);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,    headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback_escrita);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &buf);
    /* Timeout para nao travar o jogo em caso de rede lenta */
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,       30L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "[groq] curl falhou: %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    {
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 200) {
            fprintf(stderr, "[groq] HTTP %ld — resposta: %.300s\n", http_code, buf.dados);
            goto cleanup;
        }
    }

    /* A API retorna JSON no padrao OpenAI:
     *   {"choices":[{"message":{"content":"...JSON do jurado..."}}]}
     * Localizamos "content" e extraimos o valor entre aspas. */
    {
        const char *marca = "\"content\":";
        char *ptr = strstr(buf.dados, marca);
        if (!ptr) {
            fprintf(stderr, "[groq] 'content' nao encontrado na resposta\n");
            goto cleanup;
        }
        ptr += strlen(marca);

        /* Pula espacos e a aspas de abertura */
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n') ptr++;
        if (*ptr != '"') goto cleanup;
        ptr++;

        /* Copia o valor respeitando aspas escapadas (\") */
        size_t i = 0;
        while (*ptr && i < tamanho_saida - 1) {
            if (*ptr == '\\' && *(ptr + 1) == '"') {
                saida[i++] = '"';
                ptr += 2;
            } else if (*ptr == '"') {
                break;
            } else {
                saida[i++] = *ptr++;
            }
        }
        saida[i] = '\0';
        sucesso = 0;
    }

cleanup:
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(buf.dados);
    return sucesso;
}

/* -------------------------------------------------------------------------
 * extrair_nota_comentario()
 *
 * Parseia {"nota": X.X, "comentario": "texto"} sem biblioteca JSON externa.
 * Retorna 0 em sucesso, -1 se o formato for invalido.
 * ---------------------------------------------------------------------- */
static int extrair_nota_comentario(const char *json,
                                   float *nota,
                                   char  *comentario,
                                   size_t tam)
{
    if (!json || !*json) return -1;

    /* Extrai nota: localiza a chave e le o float apos o ':' */
    const char *ptr = strstr(json, "\"nota\"");
    if (!ptr) return -1;
    ptr += strlen("\"nota\"");
    while (*ptr && *ptr != ':') ptr++;
    if (!*ptr) return -1;
    ptr++;
    if (sscanf(ptr, " %f", nota) != 1) return -1;

    /* Extrai comentario: localiza a chave, avanca ate o valor entre aspas */
    ptr = strstr(json, "\"comentario\"");
    if (!ptr) { comentario[0] = '\0'; return 0; }
    ptr += strlen("\"comentario\"");
    while (*ptr && *ptr != ':') ptr++;
    if (!*ptr) { comentario[0] = '\0'; return 0; }
    ptr++;

    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr != '"') { comentario[0] = '\0'; return 0; }
    ptr++;

    size_t i = 0;
    while (*ptr && i < tam - 1) {
        if (*ptr == '\\' && *(ptr + 1) == '"') {
            comentario[i++] = '"';
            ptr += 2;
        } else if (*ptr == '"') {
            break;
        } else {
            comentario[i++] = *ptr++;
        }
    }
    comentario[i] = '\0';
    return 0;
}

/* -------------------------------------------------------------------------
 * extrair_jurado()
 *
 * Extrai {"nota":X,"comentario":"Y"} de um jurado especifico dentro do JSON
 * raiz que contem {"ariano":{...},"clarice":{...},"chico":{...}}.
 * Localiza a chave do jurado, avanca ate o '{' do sub-objeto e delega
 * ao parser generico extrair_nota_comentario.
 * Retorna 0 em sucesso, -1 em falha.
 * ---------------------------------------------------------------------- */
static int extrair_jurado(const char *json_raiz, const char *nome_jurado,
                          float *nota, char *comentario, size_t tam)
{
    /* monta a chave de busca, ex: "ariano" */
    char chave[32];
    snprintf(chave, sizeof(chave), "\"%s\"", nome_jurado);

    const char *ptr = strstr(json_raiz, chave);
    if (!ptr) return -1;
    ptr += strlen(chave);

    /* avanca ate o '{' do sub-objeto */
    while (*ptr && *ptr != '{') ptr++;
    if (!*ptr) return -1;

    return extrair_nota_comentario(ptr, nota, comentario, tam);
}

/* -------------------------------------------------------------------------
 * avaliar_com_jurados()
 *
 * Funcao publica. Faz UMA chamada a API com os 3 jurados no mesmo prompt,
 * parseia o JSON aninhado e retorna a struct ResultadoJurados preenchida.
 * ---------------------------------------------------------------------- */
ResultadoJurados avaliar_com_jurados(EstadoJogo estado)
{
    ResultadoJurados resultado;
    memset(&resultado, 0, sizeof(resultado));

    carregar_dotenv(".env");

    const char *chave = getenv("GROQ_API_KEY");
    if (!chave || chave[0] == '\0') {
        fprintf(stderr, "[groq] GROQ_API_KEY nao definida — jurados indisponiveis\n");
        return resultado;
    }

    /* Monta mensagem com metricas claras e sem ambiguidade */
    char mensagem[512];
    snprintf(mensagem, sizeof(mensagem),
        "Passos corretos: %d de %d. "
        "Erros cometidos: %d. "
        "Tempo extra alem do limite: %d segundo(s). "
        "Receita concluida: %s.",
        estado.passos_acertados, estado.passos_total,
        estado.erro_passo,
        estado.tempo_extra,
        (estado.passos_total > 0 &&
         estado.passos_acertados == estado.passos_total) ? "sim" : "nao");

    fprintf(stderr, "[groq] estado bruto: pontuacao=%d passos=%d/%d tempo_extra=%d erro_passo=%d\n",
            estado.pontuacao, estado.passos_acertados, estado.passos_total,
            estado.tempo_extra, estado.erro_passo);
    fprintf(stderr, "[groq] mensagem enviada: %s\n", mensagem);

    /* Uma unica chamada retorna os 3 jurados */
    char resposta[BUFFER_SIZE];
    memset(resposta, 0, sizeof(resposta));

    if (chamar_groq(chave, PROMPT_JURADOS, mensagem, resposta, sizeof(resposta)) != 0) {
        fprintf(stderr, "[groq] chamada falhou\n");
        return resultado;
    }

    fprintf(stderr, "[groq] raw: %.600s\n", resposta);

    int jurados_ok = 0;

    if (extrair_jurado(resposta, "ariano",
                       &resultado.nota_ariano, resultado.comentario_ariano, 256) == 0) {
        fprintf(stderr, "[groq] Ariano %.1f | %s\n",
                resultado.nota_ariano, resultado.comentario_ariano);
        jurados_ok++;
    } else {
        fprintf(stderr, "[groq] Ariano parse falhou\n");
    }

    if (extrair_jurado(resposta, "clarice",
                       &resultado.nota_clarice, resultado.comentario_clarice, 256) == 0) {
        fprintf(stderr, "[groq] Clarice %.1f | %s\n",
                resultado.nota_clarice, resultado.comentario_clarice);
        jurados_ok++;
    } else {
        fprintf(stderr, "[groq] Clarice parse falhou\n");
    }

    if (extrair_jurado(resposta, "chico",
                       &resultado.nota_chico, resultado.comentario_chico, 256) == 0) {
        fprintf(stderr, "[groq] Chico %.1f | %s\n",
                resultado.nota_chico, resultado.comentario_chico);
        jurados_ok++;
    } else {
        fprintf(stderr, "[groq] Chico parse falhou\n");
    }

    if (jurados_ok > 0)
        resultado.media_final = (resultado.nota_ariano +
                                 resultado.nota_clarice +
                                 resultado.nota_chico) / (float)jurados_ok;

    fprintf(stderr, "[groq] jurados_ok=%d media=%.2f\n", jurados_ok, resultado.media_final);
    return resultado;
}