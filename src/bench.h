#ifndef BENCH
#define BENCH

namespace Bench{
    void movegen_performance_test();
    void search_performance_test();
    void eval_performance_test();

    // Runs search at fixed depth over a standard set of FEN positions and
    // prints the aggregate node count and NPS on the final two lines
    // ("Nodes: N" / "NPS: M") so external tools can scrape the output.
    void run_standard_bench(int depth);
}


#endif