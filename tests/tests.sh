#!/bin/sh
CAPIZERO_RELEASE=$(curl http://capizero.xyz/downloads/latest_version/ -s)
CAPIZERO_RATING=$(curl http://capizero.xyz/rating/ -s)

echo "Baixando ultima versão do capizero ($CAPIZERO_RELEASE) ... "

curl http://capizero.xyz/downloads/ --output capizero_latest_version -s

echo "Capizero release $CAPIZERO_RELEASE baixado com sucesso!"

cd ..

echo "Compilando capizero local ..."

make build NAME=capizero_local > /dev/null
make clean > /dev/null
mv capizero_local tests/

echo "Capizero local compilado com sucesso!"

python3 game.py $CAPIZERO_RATING 4