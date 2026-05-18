# capizero

uma engine de xadrez em c++ 

capizero é uma engine de xadrez que estou desenvolvendo para aprender sobre xadrez computacional.

no momento ela se encontra incompleta pois existem diversos recursos a serem implementados a fim de melhorar sua performance, velocidade e entendimento das posições, porém, já é possível jogar contra ela através do terminal de comandos, basta você baixar e compilar o código fonte, ou baixar uma das builds disponíveis nos releases.

caso queira contribuir com o projeto, fique a vontade, toda ajuda é bem vinda!

## Recursos presentes na engine

### 1. Representação do tabuleiro

* [Bitboards](https://www.chessprogramming.org/Bitboards)

**1.2 Geração de lances**

* [Bitboards mágicos](https://www.chessprogramming.org/Magic_Bitboards)
* [Bitboards PEXT](https://www.josherv.in/2022/08/28/chess-3/)

### 2. Pesquisa de lances

* [Pesquisa Alpha-beta](https://www.chessprogramming.org/Alpha-Beta)

* [Quiescence search](https://www.chessprogramming.org/Quiescence_Search)

* [Pesquisa da variante principal](https://www.chessprogramming.org/Principal_Variation_Search)

* [Internal Interative Deeping](https://www.chessprogramming.org/Internal_Iterative_Deepening)
 
**2.1 Poda de lances**

* [Beta cutoffs](https://www.chessprogramming.org/Beta-Cutoff)

* [SEE](https://www.chessprogramming.org/Static_Exchange_Evaluation)

* [Cortes por tabela de transposição](https://www.chessprogramming.org/Transposition_Table)

**2.2 Extensões de pesquisa**

* [Extensões de xeque](https://www.chessprogramming.org/Check_Extensions)

**2.3 Reduções de pesquisa**

* [Late move reductions](https://www.chessprogramming.org/Late_Move_Reductions)

### 3. Ordenação de lances

* [MVV/LVA](https://www.chessprogramming.org/MVV-LVA)

* [Killer move heuristic](https://www.chessprogramming.org/Killer_Heuristic)

* [Tabela de Transposição](https://www.chessprogramming.org/Transposition_Table)

* [Heuristica de histórico](https://www.chessprogramming.org/History_Heuristic)

* [Counter move heuristic](https://www.chessprogramming.org/Countermove_Heuristic)

### 4. Hash

* [Zobrist Hashing](https://www.chessprogramming.org/Zobrist_Hashing)

### 5. Avaliação de posições

* Estrutura de peões

* Posicionamento das peças

* Torres na coluna aberta ou semi aberta

* Segurança da ala no qual o rei está rocado

### 6. Interface

* Modo interativo via terminal (comandos em português)
* Compatibilidade com [Xboard](https://www.chessprogramming.org/Chess_Engine_Communication_Protocol)
* Compatibilidade com [UCI](https://www.chessprogramming.org/UCI) — permite usar cutechess-cli, Fastchess, Arena, etc.

## Para compilar

use o g++ para compilar

a versão do c++ é a 11

no terminal, digite:

```
make build
```

## Para executar

depois de compilar o binário `capizero_<versao>` fica na raiz do repositório. modos disponíveis:

```
./capizero_<versao>                # modo interativo (português)
./capizero_<versao> uci            # protocolo UCI
./capizero_<versao> xboard         # protocolo XBoard
./capizero_<versao> bench [prof]   # executa o bench padrão (default prof=13)
                                   # imprime Nodes: N / NPS: M no final
```

## Para tunar os pesos da avaliação

o tuner é um binário separado que ajusta os pesos das tabelas de pesos das peças (PSTs) e das tabelas de mobilidade através de [Texel tuning](https://www.chessprogramming.org/Texel%27s_Tuning_Method) — descida coordenada com perda MSE entre o sigmoid da avaliação estática e o resultado real das partidas.

### Compilar o tuner

```
make tuner
```

gera o binário `capi_tuner` na raiz do repositório.

### Dataset

o tuner espera um arquivo texto com uma posição por linha no formato:

```
<FEN> <resultado>
```

onde `<FEN>` são os 6 campos padrão FEN e `<resultado>` é `1.0` / `0.5` / `0.0` (ou `1-0` / `1/2-1/2` / `0-1`). variações com colchetes (`[0.5]`) ou pipe (`| 0.5`) também são aceitas. exemplos compatíveis:

```
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 [0.5]
r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4 1.0
```

datasets recomendados:
* [Andrew Grant's Ethereal tuning data](https://github.com/AndyGrant/EtherealDev) — ~10M posições D12-resolved (formato compatível direto)
* posições geradas por self-play da própria capizero (mais representativas, mas mais lentas de gerar)

### Executar o tuner

```
./capi_tuner <dataset.txt> [--max N] [--passes P]
```

* `--max N` — limita o dataset a N posições amostradas aleatoriamente (padrão: 200000; use `0` para o conjunto completo).
* `--passes P` — interrompe após P passes mesmo sem convergência (padrão: 30; use `0` para ilimitado).

logs vão para `stderr`; o output formatado dos valores tunados vai para `stdout`. exemplo de uso típico (run rápido para iteração):

```
./capi_tuner ./E12.33-1M-D12-Resolved.book --max 200000 --passes 30 > tuned_values.h.txt 2> tuner.log
```

run completo sobre o dataset inteiro

```
./capi_tuner ./E12.33-1M-D12-Resolved.book --max 0 --passes 1 > tuned_values.h.txt 2> tuner.log
```

### Aplicar os valores tunados

1. abra `tuned_values.h.txt` — contém 12 arrays de PST (6 peças × mg/eg) e 8 arrays de mobilidade.
2. substitua os arrays correspondentes em [src/values.h](src/values.h).
3. as PSTs tunadas já têm material embutido (PeSTO `VALOR_*_MG` / `VALOR_*_EG` baked in). para evitar somar material duas vezes, zere os defines `VALOR_*_MG` / `VALOR_*_EG` em [src/values.h](src/values.h) (ou remova a soma em `Eval::init_eval_tables`).
4. recompile (`make build`) e rode `./capizero_<versao> bench` para sanity check.
5. valide a mudança via SPRT — veja a seção "Testes de força" abaixo.


## Testes de força

duas ferramentas estão disponíveis em [tests/](tests/) para medir a força da engine:

* **SPRT** (teste relativo patch-vs-baseline) — veja [tests/sprt/README.md](tests/sprt/README.md).
  útil para validar se uma mudança ganhou ou perdeu Elo.

* **Gauntlet** (teste absoluto vs. Stockfish com força reduzida) — veja [tests/gauntlet/README.md](tests/gauntlet/README.md).
  útil para estimar o rating absoluto da engine.

ambas as ferramentas usam o [Fastchess](https://github.com/Disservin/fastchess) como driver.

## Para contribuir

leia o nosso [Guia de contribuição](https://github.com/fatorius/capizero/blob/main/CONTRIBUTING.md)
