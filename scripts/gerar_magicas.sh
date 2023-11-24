echo "Gerando mágicas...";
cd ..
make magicsgenerator > /dev/null
./magicsgenerator
make clean > /dev/null
rm -rf magicsgenerator
echo "Mágicas geradas com sucesso!"
