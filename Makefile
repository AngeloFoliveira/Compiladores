CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lfl
TARGET = etapa6

SOURCES = main.c asd.c table.c asm.c parser.tab.c lex.yy.c

OBJECTS = $(SOURCES:.c=.o)

HEADERS = asd.h table.h asm.h errors.h parser.tab.h

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

parser.tab.c parser.tab.h: parser.y
	bison -d parser.y

lex.yy.c: scanner.l parser.tab.h
	flex scanner.l

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS) parser.tab.c parser.tab.h lex.yy.c *.s ex*

run: $(TARGET)
	./run_test.sh