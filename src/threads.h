#ifndef THREADS
#define THREADS

struct Estado;

// Threads de busca do UCI (Lazy SMP). Cada thread é persistente e tem o próprio
// Estado; elas só se comunicam pela TT. A thread 0 é a principal: controla o
// relógio, imprime info e bestmove e para as auxiliares quando termina. A thread
// do UCI continua lendo comandos (stop, isready, quit).
// bench, testes e ferramentas chamam Search::pensar direto e não usam isto.
namespace Threads{
    // Cria a thread principal. Chamar uma vez, antes das demais funções.
    void iniciar();

    // Muda o número de threads de busca (1..MAX_THREADS). Não pode ser chamado
    // durante uma busca: quem chama espera antes (Threads::esperar).
    void redimensionar(int n);

    // Espera a busca anterior, copia a posição e começa a busca. Volta na hora.
    // Com infinita = true (UCI "go infinite"), o bestmove só sai depois de parar().
    void iniciar_busca(const Estado &posicao, bool infinita);

    // Pede a parada da busca em andamento e espera o bestmove sair.
    void parar();

    // Espera a busca em andamento terminar.
    void esperar();

    // Para a busca em andamento e encerra a thread.
    void encerrar();
};

#endif
