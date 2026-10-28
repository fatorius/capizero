#include "threads.h"

#include <stdio.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

#include "game.h"
#include "gen.h"
#include "search.h"
#include "interface.h"
#include "hash.h"
#include "params.h"
#include "estado.h"

static std::mutex mtx;
static std::condition_variable cv;

// Uma thread e um Estado (~3 MB, no heap) por índice; o índice 0 é a principal.
// Persistentes entre buscas: hash de peões e killers continuam de uma para outra.
static std::vector<std::thread> threads_de_busca;
static std::vector<Estado *> estados;

// Protegidos por mtx.
static unsigned busca_atual = 0;    // incrementado a cada go; acorda todas as threads
static int auxiliares_ativas = 0;   // auxiliares que ainda não terminaram a busca atual
static bool buscando = false;       // busca pedida e bestmove ainda não impresso
static bool infinita = false;
static bool parar_pedido = false;
static bool sair = false;

static void imprimir_bestmove(const Gen::lance &m){
    // Sem lance (mate ou afogamento na raiz): UCI usa o lance nulo "0000".
    if (m.inicio == m.destino){
        printf("bestmove 0000\n");
    }
    else{
        printf("bestmove %s\n", Interface::lance_para_string(m.inicio, m.destino, m.promove));
    }
    fflush(stdout);
}

// est e busca_inicial vêm como argumentos: a thread nova não pode ler estados
// nem busca_atual antes de pegar o mutex.
static void laco_de_busca(int id, Estado *estado, unsigned busca_inicial){
    Estado &est = *estado;
    unsigned minha_busca = busca_inicial;
    std::unique_lock<std::mutex> lock(mtx);

    while (true){
        cv.wait(lock, [&]{ return busca_atual != minha_busca || sair; });
        if (sair){
            return;
        }
        minha_busca = busca_atual;

        lock.unlock();
        if (id == 0){
            Search::pensar(est, true, false);
        }
        else{
            Search::pensar_auxiliar(est, id);
        }
        lock.lock();

        if (id != 0){
            auxiliares_ativas--;
            cv.notify_all();
            continue;
        }

        // A principal terminou (tempo, profundidade, mate ou stop): para as
        // auxiliares. O bestmove é o da principal.
        Search::parar_pesquisa.store(true, std::memory_order_relaxed);
        cv.wait(lock, []{ return auxiliares_ativas == 0; });

        // "go infinite": a busca pode acabar sozinha (mate, MAX_PLY), mas o
        // bestmove só pode sair depois do stop.
        cv.wait(lock, []{ return !infinita || parar_pedido; });

        imprimir_bestmove(est.lance_escolhido);
        buscando = false;
        cv.notify_all();
    }
}

// Com mtx e sem busca em andamento.
static void criar_thread(){
    const int id = (int)estados.size();
    Estado *est = new Estado();
    est->auxiliar = (id != 0);
    estados.push_back(est);
    threads_de_busca.push_back(std::thread(laco_de_busca, id, est, busca_atual));
}

// Sem mtx e sem busca em andamento: encerra e libera todas as threads.
static void destruir_threads(){
    {
        std::lock_guard<std::mutex> lock(mtx);
        sair = true;
    }
    cv.notify_all();

    for (size_t i = 0; i < threads_de_busca.size(); i++){
        threads_de_busca[i].join();
    }
    for (size_t i = 0; i < estados.size(); i++){
        delete estados[i];
    }
    threads_de_busca.clear();
    estados.clear();

    std::lock_guard<std::mutex> lock(mtx);
    sair = false;
}

void Threads::iniciar(){
    std::lock_guard<std::mutex> lock(mtx);
    criar_thread();
}

void Threads::redimensionar(int n){
    if (n < 1) n = 1;
    if (n > MAX_THREADS) n = MAX_THREADS;

    esperar();
    if ((int)estados.size() == n){
        return;
    }

    destruir_threads();

    std::lock_guard<std::mutex> lock(mtx);
    for (int i = 0; i < n; i++){
        criar_thread();
    }
}

void Threads::iniciar_busca(const Estado &posicao, bool busca_infinita){
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, []{ return !buscando; });

    // A alocação preguiçosa da TT acontece aqui, e não dentro da busca.
    Hash::garantir_tt();
    for (size_t i = 0; i < estados.size(); i++){
        Game::copiar_posicao(*estados[i], posicao);
    }
    infinita = busca_infinita;
    parar_pedido = false;
    // Zerado aqui, e não em pensar(): um stop que chegue antes de a thread de
    // busca acordar continua valendo.
    Search::parar_pesquisa.store(false, std::memory_order_relaxed);
    Search::nos_auxiliares.store(0, std::memory_order_relaxed);

    auxiliares_ativas = (int)estados.size() - 1;
    buscando = true;
    busca_atual++;
    cv.notify_all();
}

void Threads::parar(){
    std::unique_lock<std::mutex> lock(mtx);
    if (!buscando){
        return;
    }

    parar_pedido = true;
    Search::parar_pesquisa.store(true, std::memory_order_relaxed);
    cv.notify_all();

    cv.wait(lock, []{ return !buscando; });
}

void Threads::esperar(){
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, []{ return !buscando; });
}

void Threads::encerrar(){
    if (threads_de_busca.empty()){
        return;
    }

    parar();
    destruir_threads();
}
