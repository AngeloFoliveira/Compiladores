MAIN_SRC = main.c
LEX_SRC = scanner.l
LEX_OBJ = lex.yy.c
PARSER = parser.y
PARSER_SRC = lex.yy.c parser.tab.c
PARSER_OBJ = lex.yy.o parser.tab.o
BISON_OBJ = parser.tab.c
OUTPUT = etapa2
CFLAGS= -fsanitize=address

$(OUTPUT): $(MAIN_SRC) $(PARSER_OBJ)
	gcc $(CFLAGS) $(MAIN_SRC) $(PARSER_OBJ) -o $(OUTPUT) -lfl

$(LEX_OBJ): $(LEX_SRC)
	flex $(LEX_SRC)
	
$(BISON_OBJ): $(PARSER)
	bison -d $(PARSER)
	
$(PARSER_OBJ): $(PARSER_SRC)
	gcc -c $(PARSER_SRC)
	
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(LEX_OBJ) $(OUTPUT) $(PARSER_OBJ) $(PARSER_SRC) parser.tab.h

run: $(OUTPUT)
	./$(OUTPUT)
