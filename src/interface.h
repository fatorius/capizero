#ifndef INTERFACE
#define INTERFACE

namespace Interface{
    extern bool tempo_fixo;
    extern bool profundidade_fixa;

    extern int tempo_maximo;
    extern int profundidade_maxima;

    extern int tempo_gasto;

    extern int lance_inicio, lance_destino, lance_promove;

    char *lance_para_string(int inicio, int destino, int promove);
    void lance_computador();
    int obter_tempo();
    int converter_lance(char *lnc);
    int computar_tempo_para_lance(int remaining_ms, int inc_ms);
    void obter_pv_uci(char *buf, int max_len, int profundidade);
};

#endif