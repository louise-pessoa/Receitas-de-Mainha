# 🎮 Jogo AED: Receitas de Mainha

> Jogo desenvolvido em **Linguagem C** sobre as comidas típicas do Recife. O projeto surgiu de uma atividade prática e avaliativa da disciplina de Algoritmos e Estruturas de Dados.

O jogador entra na pele de um aprendiz de cozinheiro que, com a ajuda de sua mainha, se prepara para uma competição gastronômica regional cuja fase final acontece no **Marco Zero**.

Inspirado em *Cooking Mama*, o jogo desafia o jogador a preparar pratos como **Bolo de Rolo**, **Tapioca** e **Sururu** seguindo a ordem correta dos passos e respeitando o tempo de cada etapa.

---

## 📍 Vibe Recifense
A ambientação celebra a culinária pernambucana e a cultura popular de Recife. A última fase se passa no **Marco Zero**, ponto turístico mais simbólico da cidade, onde os jurados avaliam o desempenho do jogador na grande final.

---

## 📖 Sobre o Jogo

* **Público-alvo:** Jogadores iniciantes e casuais.
* **Narrativa:** O personagem principal decide entrar em uma competição de comidas típicas e treina para ela com a ajuda de sua mainha. A cada fase, uma nova receita regional precisa ser dominada; ao final, figuras icônicas avaliam os pratos numa etapa decisiva no Marco Zero.
* **Objetivo:** Aprender todas as receitas, acumular boa pontuação e vencer a competição final (desempenho mínimo de 70%).

---

## ⚙️ Mecânica Principal

O coração da jogabilidade é uma **pilha (LIFO)**. Cada receita é representada como uma sequência de passos empilhados; o jogador deve sempre executar o passo que está **no topo** da pilha.

* **Acertou?** `pop` — o próximo passo aparece.
* **Errou ou estourou o tempo?** Penalidade na pontuação.

Quando a pilha esvazia, o prato está pronto e a fase termina.

---

## 🏗️ Estruturas de Dados

### 1. Pilha (Estrutura Central)
A pilha é a estrutura **principal** do jogo, responsável por organizar os passos das receitas. O jogador "navega pela pilha" enquanto cozinha: cada execução correta de um passo dispara um `pop`, e o jogo só avança quando toda a pilha é consumida na ordem certa. Essa escolha se justifica porque o preparo de uma receita tem ordem rígida (LIFO casa naturalmente com o conceito de "última etapa empilhada é a próxima a ser feita" quando as receitas são carregadas de trás para frente).

### 2. Lista Encadeada
Duas listas encadeadas dão suporte ao jogo:
* **Lista de receitas disponíveis:** O jogador percorre essa lista para escolher qual prato vai preparar.
* **Lista de ingredientes:** Cada receita possui sua própria lista de ingredientes coletados antes do preparo.

---

## 🔢 Algoritmos de Ordenação

* **Insertion Sort (Principal):** Organiza os ingredientes coletados pelo jogador, que entram desordenados e precisam ser arrumados antes do preparo.

---

## 🤖 Inteligência Artificial na Dinâmica

O jogo utiliza a **API da OpenAI** para simular os jurados da competição final. A IA avalia o desempenho do jogador considerando a ordem dos passos executados, o tempo gasto em cada etapa e a quantidade de erros, retornando uma **nota** e um **feedback textual** personalizado para cada partida.

---

## 📁 Estrutura de Pastas

```
JogoAED/
├── main.c              # Entry point: loop principal, janela, transições de tela
├── jogo.h / jogo.c     # Estado global: pontuação, timer, penalidades
│
├── dados/              # Estruturas de dados (núcleo da disciplina AED)
│   ├── pilha.h/c       # Pilha LIFO — controla os passos da receita
│   └── receitas.h/c    # Listas encadeadas de receitas e ingredientes
│
├── fases/              # Os três minijogos em sequência
│   ├── catcher.h/c     # Fase 1 — coleta de ingredientes (25s)
│   ├── ordenacao.h/c   # Fase 2 — Insertion Sort animado
│   └── cozinhar.h/c    # Fase 3 — execução da pilha de passos
│
├── ui/                 # Renderização Raylib (telas, botões, cores)
│   └── interface.h/c
│
├── api/                # Integração com serviço externo
│   └── groq.h/c        # Jurados IA via Groq API (LLaMA 3.1)
│
└── docs/               # Documentação do projeto
    ├── README.md
    ├── CONTEXT.md
    └── INSTALACAO.md
```

---

## 🚀 Como compilar e executar

**Linux/WSL:**
```bash
gcc main.c jogo.c dados/receitas.c dados/pilha.c \
    ui/interface.c fases/catcher.c fases/ordenacao.c fases/cozinhar.c \
    api/groq.c -o jogo -I. \
    -lraylib -lm -lpthread -ldl -lrt -lX11 -lcurl
```

**Windows (MinGW):**
```bash
gcc main.c jogo.c dados/receitas.c dados/pilha.c \
    ui/interface.c fases/catcher.c fases/ordenacao.c fases/cozinhar.c \
    api/groq.c -o jogo.exe -I. \
    -lraylib -lopengl32 -lgdi32 -lwinmm -lcurl
```

> A variável de ambiente `GROQ_API_KEY` precisa estar definida para a fase dos jurados funcionar.

---

## 👥 Equipe

| Nome | Função | GitHub |
| :--- | :--- | :--- |
| Louise Pessoa | Desenvolvedora | [@Louise](https://github.com/louise-pessoa) |
| Marília Liz Alves | Desenvolvedora | [@Marilia](https://github.com/marilializ) |
| Mateus Lins | Desenvolvedor | [@Mateus](https://github.com/mateuslinsf) |
| Pedro David Oliveira | Desenvolvedor | [@Pedro](https://github.com/Pedrodavidob) |
| Victor Martins | Desenvolvedor | [@Victor](https://github.com/usuario) |

---