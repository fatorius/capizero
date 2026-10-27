#!/usr/bin/env bash
# Regenera src/magics.h. Pode ser executado de qualquer diretório.
set -e
cd "$(git rev-parse --show-toplevel)"
echo "Gerando mágicas...";
make magics > /dev/null
./generate_magics
rm -f generate_magics
make clean > /dev/null
echo "Mágicas geradas com sucesso!"
