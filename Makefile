MAIN_SRC = main.c
LEX_SRC = scanner.l
LEX_OBJ = lex.yy.c
OUTPUT = etapa1

$(OUTPUT): $(MAIN_SRC) $(LEX_OBJ)
	gcc $(MAIN_SRC) $(LEX_OBJ) -o $(OUTPUT) -lfl

lex.yy.c: $(LEX_SRC)
	flex $(LEX_SRC)

clean:
	rm -f $(LEX_OBJ) $(OUTPUT)

run: $(OUTPUT)
	./$(OUTPUT)
