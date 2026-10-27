#ifndef THREADS
#define THREADS

struct Estado;

// Thread de busca do UCI. A busca roda numa thread persistente com o próprio
// Estado; a thread do UCI continua lendo comandos (stop, isready, quit).
// bench, testes e ferramentas chamam Search::pensar direto e não usam isto.
namespace Threads{
    // Cria a thread de busca. Chamar uma vez, antes das demais funções.
    void iniciar();

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
