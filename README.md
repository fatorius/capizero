# capizero

uma engine de xadrez em c++ 

capizero é uma engine de xadrez que estou desenvolvendo para aprender sobre xadrez computacional.

no momento ela se encontra incompleta pois existem diversos recursos a serem implementados a fim de melhorar sua performance, velocidade e entendimento das posições, porém, já é possível jogar contra ela através do terminal de comandos, basta você baixar e compilar o código fonte, ou baixar uma das builds disponíveis nos releases.

caso queira contribuir com o projeto, fique a vontade, toda ajuda é bem vinda!

## Recursos presentes na engine

### 1. Representação do tabuleiro

* [Bitboards](https://www.chessprogramming.org/Bitboards)

### 2. Pesquisa de lances

* [Pesquisa Alpha-beta](https://www.chessprogramming.org/Alpha-Beta)

* [Quiescence search](https://www.chessprogramming.org/Quiescence_Search)

**2.1 Poda de lances**

* [Beta cutoffs](https://www.chessprogramming.org/Beta-Cutoff)

* [SEE](https://www.chessprogramming.org/Static_Exchange_Evaluation)

**2.2 Extensões de pesquisa**

* [Extensões de xeque](https://www.chessprogramming.org/Check_Extensions)

**2.3 Reduções de pesquisa**

### 3. Ordenação de lances

* [MVV/LVA](https://www.chessprogramming.org/MVV-LVA)

* [Lances hash](https://www.chessprogramming.org/Hash_Move)

* [Heuristica de histórico](https://www.chessprogramming.org/History_Heuristic)

### 4. Avaliação de posições

* Estrutura de peões

* Posicionamento das peças

* Torres na coluna aberta ou semi aberta

* Segurança da ala no qual o rei está rocado

### 5. Interface

* Compatibilidade com [Xboard](https://www.chessprogramming.org/Chess_Engine_Communication_Protocol)

## Recursos a serem implementados

* [Bitboards mágicos](https://www.chessprogramming.org/Magic_Bitboards)

* [Killer move heuristic](https://www.chessprogramming.org/Killer_Heuristic)

* [Counter move heuristic](https://www.chessprogramming.org/Countermove_Heuristic)

* [Pesquisa paralela](https://www.chessprogramming.org/Parallel_Search)

* [Late move reductions](https://www.chessprogramming.org/Late_Move_Reductions)

* [Pesquisa da variante principal](https://www.chessprogramming.org/Principal_Variation_Search)

* [Internal Interative Deeping](https://www.chessprogramming.org/Internal_Iterative_Deepening)

* [Null move prunning](https://www.chessprogramming.org/Null_Move_Pruning)

## Para compilar

use o g++ para compilar

a versão do c++ é a 11

no terminal, digite:

```
make build
```
