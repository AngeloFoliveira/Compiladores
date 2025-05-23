MAIN_SRC = main.c
LEX_SRC = scanner.l
LEX_OBJ = lex.yy.c
PARSER = parser.y
ASD_SRC = asd.c
ASD_OBJ = asd.o
TABLE_SRC = table.c
TABLE_OBJ = table.o
PARSER_SRC = lex.yy.c parser.tab.c
PARSER_OBJ = lex.yy.o parser.tab.o
BISON_OBJ = parser.tab.c
OUTPUT = etapa3
CFLAGS= 

$(OUTPUT): $(MAIN_SRC) $(PARSER_OBJ) $(ASD_OBJ) $(TABLE_OBJ)
	gcc $(MAIN_SRC) $(PARSER_OBJ) $(ASD_OBJ) $(TABLE_OBJ) -o $(OUTPUT) -lfl $(CFLAGS)

$(LEX_OBJ): $(LEX_SRC)
	flex $(LEX_SRC)
	
$(BISON_OBJ): $(PARSER)
	bison -d $(PARSER)
	
$(PARSER_OBJ): $(PARSER_SRC) asd.c
	gcc -c $(PARSER_SRC) asd.c -lfl
	
$(ASD_OBJ): $(ASD_SRC) asd.h
	gcc -c $(ASD_SRC) $(CFLAGS)

$(TABLE_OBJ): $(TABLE_SRC) table.h
	gcc -c $(TABLE_SRC) $(CFLAGS)
	
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(LEX_OBJ) $(OUTPUT) $(PARSER_OBJ) $(PARSER_SRC) $(ASD_OBJ) $(TABLE_OBJ) parser.tab.h

run: $(OUTPUT)
	./$(OUTPUT)
