#ifndef TESTS
#define TESTS

#include "consts.h"

#define TESTE_SUCESSO cout<<"\033[1;32m TESTE BEM SUCEDIDO \033[0m"<<endl
#define TESTE_FALHO cout<<"\033[1;31m TESTE FALHO \033[0m"<<endl

#define TEST1_VALOR_ESPERADO 4865609

#define TEST2_VALOR_ESPERADO 3000
#define TEST2_TOLERANCIA 100

#define TEST3_VALOR_ESPERADO 4865609

#define TEST4_VALOR_ESPERADO 4865609

#define TEST5_VALOR_ESPERADO 64827465

#define TEST6_VALOR_ESPERADO 1267835

namespace Tests{
    void test1();
    void test2();
    void test3();
    void test4();
    void test5();
    void test6();
}


#endif