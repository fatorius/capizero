# capizero

uma engine de xadrez em c++ 

capizero é uma engine de xadrez que estou desenvolvendo para aprender sobre xadrez computacional.

no momento ela se encontra incompleta pois existem diversos recursos a serem implementados a fim de melhorar sua performance, velocidade e entendimento das posições, porém, já é possível jogar contra ela através do terminal de comandos, basta você baixar e compilar o código fonte, ou baixar uma das builds disponíveis nos releases.

caso queira contribuir com o projeto, fique a vontade, toda ajuda é bem vinda!

## Recursos presentes na engine

### Representação do tabuleiro

* Bitboards

### Pesquisa de lances

* Pesquisa Minimax

* Alpha-beta prunning

* Quiescence search

* Extensões de xeque

* SEE

### Ordenação de lances

* MVV/LVA

* Lances hash

* Heuristica de histórico

### Interface

* Compatibilidade com Xboard

### Avaliação de posições

* Estrutura de peões

* Posicionamento das peças

* Torres na coluna aberta ou semi aberta

* Segurança da ala no qual o rei está rocado

## Recursos a serem adicionados

* Bitboards mágicos

* Counter move heuristic

* Killer move heuristic


## Para compilar

use o g++ para compilar

a versão do c++ é a 11

no terminal, digite:

```
make build
```
