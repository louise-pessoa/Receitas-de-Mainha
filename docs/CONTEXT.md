# Contexto do Projeto — Receitas de Mainha

## Visão Geral
Jogo em linguagem C chamado **"Receitas de Mainha"** que simula um jogador
preparando receitas típicas pernambucanas para uma competição culinária.
A fase final acontece no Marco Zero, no Recife.
Estilo: arcade leve (cozy), com interação por menu, teclado e interface
gráfica em Raylib.

---

## Estruturas de Dados e Algoritmos (obrigatórios)

### Lista encadeada — receitas (`dados/receitas.h`)
- As receitas disponíveis no jogo formam uma lista encadeada (`struct receita *prox`)
- Cada receita é um nó alocado dinamicamente via `inserir_receita`

### Array — ingredientes de cada receita (`dados/receitas.h`)
- Os ingredientes de cada receita são armazenados como array estático de strings: `char ingredientes[MAX_INGREDIENTES][60]`
- Adicionados em ordem de uso com `inserir_ingrediente(receita, nome)`
- Acesso por índice: `receita->ingredientes[i]`

### Pilha — passos de cozinhar (`dados/pilha.h/c`)
- Os passos jogáveis de cada receita são armazenados em uma pilha (`struct No *passos`)
- Criados com `push_passo` na inicialização do jogo (em ordem de execução)
- Na fase de cozinhar, uma cópia de trabalho é feita; `pop_passo` remove o passo do topo ao ser executado corretamente
- A pilha original da receita não é consumida, permitindo replay

### Insertion sort — fase de ordenação (`fases/ordenacao.h/c`)
- Após o catcher, os ingredientes coletados ficam em um array na **ordem em que foram coletados pelo jogador**
- O insertion sort os ordena para a **ordem correta de uso da receita** (definida por `ordem_correta = índice na receita`)
- O algoritmo é animado frame a frame (um passo a cada 550ms) para fins didáticos
- Não há embaralhamento artificial — a desordem vem naturalmente da coleta

---

## Estrutura de Pastas

```
JogoAED/
├── main.c / jogo.h/c          # Loop principal e estado global do jogo
├── dados/
│   ├── pilha.h/c              # Pilha LIFO (passos da receita)
│   └── receitas.h/c           # Listas encadeadas de receitas e ingredientes
├── fases/
│   ├── catcher.h/c            # Fase 1: coleta de ingredientes
│   ├── ordenacao.h/c          # Fase 2: Insertion Sort animado
│   └── cozinhar.h/c           # Fase 3: execução da pilha
├── ui/interface.h/c           # Renderização Raylib (todas as telas)
├── api/groq.h/c               # Jurados IA (Groq API)
└── docs/                      # Documentação
```

## Módulos do Projeto

| Arquivo | Responsável | Escopo |
|---|---|---|
| `dados/receitas.c/.h` | Mateus | Lista encadeada de receitas e ingredientes |
| `dados/pilha.c/.h` | Marília | Pilha de passos da receita |
| `jogo.c/.h` | Victor | Lógica, fluxo e estado do jogo |
| `jogo.c/.h` | Louise | Timer, penalidades e integração |
| `ui/interface.c/.h` | Pedro | Raylib — menus, renderização, input |
| `api/groq.c/.h` | — | Integração com API do Groq (jurados IA) |
| `fases/catcher.c/.h` | — | Minijogo de coleta de ingredientes |
| `fases/ordenacao.c/.h` | — | Visualização do Insertion Sort |
| `fases/cozinhar.c/.h` | — | Execução dos passos via pilha |

---

## Fluxo do Jogo

```
MENU → RECEITAS → INGREDIENTES (preview)
     → CATCHER (minigame: pegar ingredientes em 25s)
     → ORDENAÇÃO (insertion sort animado dos ingredientes coletados)
     → COZINHAR / PILHA (executar passos da receita — pilha pop a pop)
     → RESULTADO (avaliação dos jurados IA)
```

---

## Sistema de Penalidades

```c
// Dentro de EstadoJogo
int tempo_extra;   // incrementa a cada segundo excedido ao pegar ingredientes
int erro_passo;    // incrementa a cada clique/tecla errada nos passos
```

- `tempo_extra++` — jogador não pegou os ingredientes certos dentro do tempo
- `erro_passo++` — jogador clicou ou pressionou tecla errada durante um passo

---

## Sistema de Jurados com IA (fase final)

API utilizada: **Groq** (`llama3-70b-8192`)
Chave via variável de ambiente: `GROQ_API_KEY`
Dependência: `libcurl` para requisições HTTP

### Os 3 Jurados

| Jurado | Personalidade | Foco na avaliação |
|---|---|---|
| **Ariano Suassuna** | Simpático | Lado emocional e cultural |
| **Clarice Lispector** | Exigente | Técnica e precisão |
| **Chico Science** | Criativo | Inovação e originalidade |

### Dados enviados para cada jurado

```c
typedef struct {
    int pontuacao;        // começa em 100; vitória >= 70
    int passos_acertados; // passos executados corretamente
    int passos_total;     // total de passos da receita
    int tempo_extra;      // segundos excedidos ao pegar ingredientes
    int erro_passo;       // erros de clique/tecla nos passos
} EstadoJogo;
```

### Formato de resposta esperado da IA (JSON)

```json
{"nota": 7.5, "comentario": "texto em português, máximo 2 frases"}
```

### Cálculo da nota final

```
media_final = (nota_ariano + nota_clarice + nota_chico) / 3
```

---

## Convenções do Projeto
- Linguagem: **C99**, compilador `gcc`
- Nunca acessar campos internos de outro módulo diretamente —
  sempre usar funções públicas do `.h`
- Ponteiros sempre validados antes do uso
- Funções internas marcadas com `static` e prefixo `_`
- Chave de API nunca versionada — usar `.env` no `.gitignore`
- Plataforma: Linux/Windows
