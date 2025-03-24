# Defina o nome do seu arquivo fonte
MAIN_SRC = main.c
LEX_SRC = scanner.l
LEX_OBJ = lex.yy.c
OUTPUT = programa
ANALISADOR = analisador

# Definir as dependências
OBJ = $(MAIN_SRC:.c=.o)

# Compilação do programa principal
$(OUTPUT): $(OBJ) $(LEX_OBJ)
	gcc $(OBJ) $(LEX_OBJ) -o $(OUTPUT) -lfl

# Regra para gerar o arquivo lex.yy.c a partir do scanner.l
$(LEX_OBJ): $(LEX_SRC)
	flex $(LEX_SRC)
	gcc lex.yy.c -o $(ANALISADOR) -lfl

# Compilação do arquivo main.c
$(OBJ): $(MAIN_SRC)
	gcc -c $(MAIN_SRC)

# Limpeza dos arquivos gerados
clean:
	rm -f $(OBJ) $(LEX_OBJ) $(OUTPUT) $(ANALISADOR)

# Executar o programa gerado
run: $(OUTPUT)
	./$(OUTPUT)
