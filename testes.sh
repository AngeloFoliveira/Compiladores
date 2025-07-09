#!/bin/bash

# Executa a primeira etapa, redirecionando entrada e saída
./etapa6 < testes.txt > saida.s

# Compila o arquivo saida.s
gcc saida.s -o saida

# Executa o programa compilado
./saida

# Exibe o código de saída do último comando (./saida)
echo "Código de saída: $?"
