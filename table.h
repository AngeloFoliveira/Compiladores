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
    int offset; // usado para variáveis locais
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
void declare_symbol(const char* key, Natureza nature, DataType tipo, int linha);
Symbol* use_symbol(const char* key, Natureza nature, int linha);
void checkTipoExpressaoBinaria(DataType tipo1, DataType tipo2, int linha); 
void set_func_atual(const char* func);
void free_func_atual();
void insert_arg(DataType tipo);
void set_func_call(const char* func);
void free_func_call();
void check_func_call(DataType tipo, int linha);
void printArgList(Arg* arg);
void check_parameter_count(int linha);
void check_return_type(DataType tipo, int linha);
int get_symbol_offset(const char* key);
const char* get_symbol_base_reg(const char* key);
int get_current_function_frame_size(void);
SymbolTable* get_global_scope();


#endif // SYMBOL_TABLE_H