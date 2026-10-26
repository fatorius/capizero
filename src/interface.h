#ifndef INTERFACE
#define INTERFACE

struct Estado;

namespace Interface{
    extern bool tempo_fixo;
    extern bool profundidade_fixa;

    extern int tempo_maximo;
    extern int profundidade_maxima;

    extern int tempo_gasto;

    char *lance_para_string(int inicio, int destino, int promove);
    void lance_computador(Estado &est);
    int obter_tempo();
    int converter_lance(const Estado &est, char *lnc);
    int computar_tempo_para_lance(int remaining_ms, int inc_ms);
    void obter_pv_uci(Estado &est, char *buf, int max_len, int profundidade);
};

#endif