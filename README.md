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

### 2. Pesquisa de lances

* [Pesquisa Alpha-beta](https://www.chessprogramming.org/Alpha-Beta)

* [Quiescence search](https://www.chessprogramming.org/Quiescence_Search)

* [Pesquisa da variante principal](https://www.chessprogramming.org/Principal_Variation_Search)

* [Internal Interative Deeping](https://www.chessprogramming.org/Internal_Iterative_Deepening)
 
**2.1 Poda de lances**

* [Beta cutoffs](https://www.chessprogramming.org/Beta-Cutoff)

* [SEE](https://www.chessprogramming.org/Static_Exchange_Evaluation)

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

* Compatibilidade com [Xboard](https://www.chessprogramming.org/Chess_Engine_Communication_Protocol)[https://www.chessprogramming.org/Null_Move_Pruning)

## Para compilar

use o g++ para compilar

a versão do c++ é a 11

no terminal, digite:

```
make build
```

## Para contribuir

leia o nosso [Guia de contribuição](https://github.com/fatorius/capizero/blob/main/CONTRIBUTING.md)
