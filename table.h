#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define MAX_SYMBOL_NAME 64
#include "asd.h"

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
    int line; // linha de declaração
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
void free_args(Arg *args);
Symbol* find_symbol(const char* key);
Symbol* find_in_scope(SymbolTable* table, const char* key);
void declare_symbol(const char* key, Natureza nature, DataType tipo, Arg* args, int linha);
Symbol* use_symbol(const char* key, Natureza nature, int linha);
Symbol* create_symbol(const char* key, Natureza nature, DataType tipo, Arg* args, int linha);
void checkTipoExpressaoBinaria(DataType tipo1, DataType tipo2, int linha); 
void checkChamadaFuncao(const char* key, Arg* args);
Arg* transformar_asd_em_lista(asd_tree_t* tree, Arg* lista);
Arg* create_arg(DataType tipo);
Arg* append_arg(Arg* lista, Arg* novo);



#endif // SYMBOL_TABLE_H
