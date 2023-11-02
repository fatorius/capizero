#!/bin/sh
CAPIZERO_RATING=$(curl http://capizero.xyz/rating/ -s)

if ! test -f "capizero_latest_version"; then
    CAPIZERO_RELEASE=$(curl http://capizero.xyz/downloads/latest_version/ -s)

    echo "Baixando o código fonte da ultima versão do capizero ($CAPIZERO_RELEASE) ... "

    curl http://capizero.xyz/downloads/ --output capizero-latest-version-source.zip -s

    echo "Código fonte da release $CAPIZERO_RELEASE baixado com sucesso!"

    echo "Extraindo arquivos..."

    unzip -qq capizero-latest-version-source.zip

    echo "Compilando capizero ultima versão"

    cd capizero-$CAPIZERO_RELEASE
    make build NAME=capizero_latest_version > /dev/null
    make clean > /dev/null
    mv capizero_latest_version ..

    echo "Última versão do capizero compilada com sucesso!"

    echo "Deletando arquivos de compilação..."

    cd ..

    rm -rf capizero-$CAPIZERO_RELEASE
    rm -rf capizero-latest-version-source.zip
fi

if ! test -f "capizero_local"; then
    cd ..

    echo "Compilando capizero local ..."

    make build NAME=capizero_local > /dev/null
    make clean > /dev/null
    mv capizero_local tests/

    echo "Capizero local compilado com sucesso!"

    cd tests
fi

echo "Iniciando script de testes..."

python3 game.py $CAPIZERO_RATING 4 1