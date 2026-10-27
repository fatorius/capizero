#include "threads.h"

#include <stdio.h>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "game.h"
#include "gen.h"
#include "search.h"
#include "interface.h"
#include "hash.h"
#include "estado.h"

static std::thread thread_de_busca;
static std::mutex mtx;
static std::condition_variable cv;

// Estado da thread de busca (~3 MB, no heap). Persistente entre buscas, como o
// estado_principal era antes: hash de peões e killers continuam de uma para outra.
static Estado *estado_busca = NULL;

// Protegidos por mtx.
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

static void laco_de_busca(){
    std::unique_lock<std::mutex> lock(mtx);

    while (true){
        cv.wait(lock, []{ return buscando || sair; });
        if (sair){
            return;
        }

        lock.unlock();
        Search::pensar(*estado_busca, true, false);
        lock.lock();

        // "go infinite": a busca pode acabar sozinha (mate, MAX_PLY), mas o
        // bestmove só pode sair depois do stop.
        cv.wait(lock, []{ return !infinita || parar_pedido; });

        imprimir_bestmove(estado_busca->lance_escolhido);
        buscando = false;
        cv.notify_all();
    }
}

void Threads::iniciar(){
    estado_busca = new Estado();
    thread_de_busca = std::thread(laco_de_busca);
}

void Threads::iniciar_busca(const Estado &posicao, bool busca_infinita){
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, []{ return !buscando; });

    // A alocação preguiçosa da TT acontece aqui, e não dentro da busca.
    Hash::garantir_tt();
    Game::copiar_posicao(*estado_busca, posicao);
    infinita = busca_infinita;
    parar_pedido = false;
    // Zerado aqui, e não em pensar(): um stop que chegue antes de a thread de
    // busca acordar continua valendo.
    Search::parar_pesquisa.store(false, std::memory_order_relaxed);

    buscando = true;
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
    if (!thread_de_busca.joinable()){
        return;
    }

    parar();

    {
        std::lock_guard<std::mutex> lock(mtx);
        sair = true;
    }
    cv.notify_all();

    thread_de_busca.join();

    delete estado_busca;
    estado_busca = NULL;
}
