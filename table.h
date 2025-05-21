#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define MAX_SYMBOL_NAME 64

// Naturezas
typedef enum {
    LITERAL,
    IDENTIFIER,
    FUNCTION
} Nature;

// Tipos de dados
typedef enum {
    INT,
    FLOAT
} DataType;

// Argumento de função
typedef struct Arg {
    DataType type;
    struct Arg* next;
} Arg;

// Entrada na tabela de símbolos
typedef struct Symbol {
    char key[MAX_SYMBOL_NAME];
    Nature nature;
    DataType dataType;
    Arg* args;
    union {
        int i_val;
        float f_val;
        char* s_val;
    } value;
    struct Symbol* next;
} Symbol;

// Tabela de símbolos (lista encadeada)
typedef struct SymbolTable {
    Symbol* symbols;
    struct SymbolTable* next; // para pilha de escopos
} SymbolTable;

// Interface da Tabela de Símbolos
void push_scope();
void pop_scope();
void declare_symbol(const char* key, Nature nature, DataType type);
Symbol* use_symbol(const char* key);


#endif // SYMBOL_TABLE_H