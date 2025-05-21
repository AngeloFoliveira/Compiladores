#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define MAX_SYMBOL_NAME 64

typedef enum {
    LITERAL,
    IDENTIFICADOR,
    FUNCAO
} Natureza;

// Tipos de dados
typedef enum {
    INT,
    FLOAT
} DataType;

// Argumento de função
typedef struct Arg {
    DataType tipo;
    struct Arg* prox;
} Arg;

// Entrada na tabela de símbolos
typedef struct Symbol {
    char key[MAX_SYMBOL_NAME];
    Natureza nature;
    DataType dataType;
    Arg* args;
    union {
        int i_val;
        float f_val;
        char* s_val;
    } value;
    struct Symbol* prox;
} Symbol;

// Tabela de símbolos (lista encadeada)
typedef struct SymbolTable {
    Symbol* symbols;
    struct SymbolTable* prox; // para pilha de escopos
} SymbolTable;

// Interface da Tabela de Símbolos
void push_scope();
void pop_scope();
void declare_symbol(const char* key, Natureza nature, DataType tipo);
Symbol* use_symbol(const char* key, Natureza nature);
void checkTipoExpressaoBinaria(DataType tipo1, DataType tipo2); 


#endif // SYMBOL_TABLE_H