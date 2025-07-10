// symbol_table.c
#include "table.h"
#include "errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


SymbolTable* scopeStack = NULL;
int variable_count = 0; // Contador de variáveis locais
char *func_atual = NULL;
char *func_call = NULL;
int parameter_count = 0;
static int current_offset = 0; // var locais/parametros
static int global_offset = 0; // var globais

// Funções auxiliares

Symbol* create_symbol(const char* key, Natureza nature, DataType tipo) {
    Symbol* sym = (Symbol*) malloc(sizeof(Symbol));
    strncpy(sym->key, key, MAX_SYMBOL_NAME);
    sym->nature = nature;
    sym->dataType = tipo;
    sym->args = NULL;
    sym->prox = NULL;
    return sym;
}

SymbolTable* get_global_scope() {
    SymbolTable* global_scope = scopeStack;
    if (!global_scope) return NULL;
    while (global_scope->prox != NULL) {
        global_scope = global_scope->prox;
    }
    return global_scope;
}

void push_scope() {
    SymbolTable* table = (SymbolTable*) malloc(sizeof(SymbolTable));
    table->symbols = NULL;
    table->prox = scopeStack;
    scopeStack = table;
}

void free_args(Arg *args)
{
    Arg *atual = args;
    while (atual != NULL)
    {
        Arg *prox = atual->prox;
        free(atual);
        atual = prox;
    }
}

void pop_scope() {
    if (scopeStack) {
        Symbol* sym = scopeStack->symbols;
        while (sym) {
            Symbol* prox = sym->prox;
            free_args(sym->args);
            free(sym);
            sym = prox;
        }
        SymbolTable* old = scopeStack;
        scopeStack = scopeStack->prox;
        free(old);
    }
}

Symbol* find_in_scope(SymbolTable* table, const char* key) {
    Symbol* sym = table->symbols;
    while (sym) {
        if (strcmp(sym->key, key) == 0)
            return sym;
        sym = sym->prox;
    }
    return NULL;
}

Symbol* find_symbol(const char* key) {
    SymbolTable* current = scopeStack;
    while (current) {
        Symbol* sym = find_in_scope(current, key);
        if (sym) return sym;
        current = current->prox;
    }
    return NULL;
}

// Interface pública

void declare_symbol(const char* key, Natureza nature, DataType tipo, int linha) {
    if (find_in_scope(scopeStack, key)) {
        fprintf(stderr, "Erro: símbolo '%s' já declarado. Linha: %d\n", key, linha);
        exit(ERR_DECLARED);
    }
    Symbol* sym = create_symbol(key, nature, tipo);

    if (nature == IDENTIFICADOR) {
        if (scopeStack->prox == NULL) { // Escopo Global
            global_offset += 4; // Assumindo variáveis de 4 bytes
            sym->offset = global_offset;
        } else { // Escopo Local (função)
            variable_count++;
            current_offset += 4; // Assumindo variáveis de 4 bytes
            sym->offset = current_offset;
        }
    } 

    sym->prox = scopeStack->symbols;
    scopeStack->symbols = sym;
}

Symbol* use_symbol(const char* key, Natureza nature, int linha) {
    Symbol* sym = find_symbol(key);
    if (!sym) {
        fprintf(stderr, "Erro: símbolo '%s' não declarado. Linha: %d\n", key, linha);
        exit(ERR_UNDECLARED);
    }
    if(nature == IDENTIFICADOR && sym->nature == FUNCAO) {
        fprintf(stderr, "Erro: símbolo '%s' é uma função, mas sendo chamado como uma variável. Linha: %d\n", key, linha);
        exit(ERR_FUNCTION);
    }
    if(nature == FUNCAO && sym->nature == IDENTIFICADOR) {
        fprintf(stderr, "Erro: símbolo '%s' é uma variável, mas sendo chamado como função. Linha: %d\n", key, linha);
        exit(ERR_VARIABLE);
    }
    return sym;
}

void checkTipoExpressaoBinaria(DataType tipo1, DataType tipo2, int linha) {
    if (tipo1 != tipo2) {
        fprintf(stderr, "Erro: tipos incompatíveis. Linha: %d\n", linha);
        exit(ERR_WRONG_TYPE);
    }
}

void set_func_atual(const char* func) {
    func_atual = strdup(func);
    current_offset = 0; // Reseta o offset ao iniciar uma nova função
}

void free_func_atual() {
    if (func_atual) {
        free(func_atual);
        func_atual = NULL;
    }
}

//Insere no fim da lista de argumentos
void insert_arg(DataType tipo) {
    if (!func_atual) {
        //EM TEORIA NUNCA ENTRA AQUI, MAS É POR PRECAUÇÃO
        fprintf(stderr, "Erro: função não definida.\n");
        exit(ERR_UNDECLARED);
    }
    Symbol* func = find_symbol(func_atual);
    if (!func) {
        //EM TEORIA NUNCA ENTRA AQUI, MAS É POR PRECAUÇÃO
        fprintf(stderr, "Erro: função '%s' não declarada.\n", func_atual);
        exit(ERR_UNDECLARED);
    }
    Arg* arg = (Arg*) malloc(sizeof(Arg));
    arg->tipo = tipo;
    arg->prox = NULL;

    Arg* temp = func->args;
    if(temp != NULL){
        while (temp->prox) {
            temp = temp->prox;
        }
        temp->prox = arg;
    }
    else{
        func->args = arg;
    }
    
}

void printArgList(Arg* arg) {
    printf("funcao: %s ", func_atual);
    while (arg) {
        if (arg->tipo == INT) {
            printf("INT ");
        } else if (arg->tipo == FLOAT) {
            printf("FLOAT ");
        }
        arg = arg->prox;
    }
    printf("\n");
}


void set_func_call(const char* func) {
    func_call = strdup(func);
    parameter_count = 0; // Reseta o contador de parâmetros ao iniciar uma nova chamada de função
}

void free_func_call() {
    if (func_call) {
        free(func_call);
        func_call = NULL;
    }
}

void check_func_call(DataType tipo, int linha) {
    if (!func_call) {
        //EM TEORIA NUNCA ENTRA AQUI, MAS É POR PRECAUÇÃO
        fprintf(stderr, "Erro: chamada de função sem função atual. Linha: %d\n", linha);
        exit(ERR_UNDECLARED);
    }
    Symbol* func = find_symbol(func_call);

    if (!func) {
        fprintf(stderr, "Erro: função '%s' não declarada. Linha: %d\n", func_call, linha);
        exit(ERR_UNDECLARED);
    }

    Arg* arg = func->args;
    int n = 0;
    if(!arg) {
        fprintf(stderr, "Erro: número de argumentos acima do requerido na chamada da função '%s'. Linha: %d\n", func_call, linha);
        exit(ERR_EXCESS_ARGS);
    }
    while (n < parameter_count) {
        n++;
        arg = arg->prox;
        if(!arg) {
            fprintf(stderr, "Erro: número de argumentos acima do requerido na chamada da função '%s'. Linha: %d\n", func_call, linha);
            exit(ERR_EXCESS_ARGS);
        }
    }
    if (arg->tipo != tipo) {
        fprintf(stderr, "Erro: tipos incompatíveis na chamada da função '%s'. Linha: %d\n", func_call, linha);
        exit(ERR_WRONG_TYPE_ARGS);
    }
    parameter_count++;
}

void check_parameter_count(int linha) {
    int n = 0;
    Symbol* func = find_symbol(func_call);
    Arg* arg = func->args;
    while (arg) {
        n++;
        arg = arg->prox;
    }
    if (parameter_count < n) {
        fprintf(stderr, "Erro: número de argumentos abaixo do requerido na chamada da função '%s'. Linha: %d\n", func_call, linha);
        exit(ERR_MISSING_ARGS);
    }
    if (parameter_count > n) {
        fprintf(stderr, "Erro: número de argumentos acima do requerido na chamada da função '%s'. Linha: %d\n", func_call, linha);
        exit(ERR_EXCESS_ARGS);
    }
}

void check_return_type(DataType tipo, int linha) {
    Symbol* func = find_symbol(func_atual);
    if (func->dataType != tipo) {
        fprintf(stderr, "Erro: tipo de retorno incompatível na função '%s'. Linha: %d\n", func_atual, linha);
        exit(ERR_WRONG_TYPE);
    }
}

int get_symbol_offset(const char* key) {
    Symbol* sym = find_symbol(key);
    if (sym) {
        return sym->offset;
    }
    return -1; // Erro ou não encontrado
}

const char* get_symbol_base_reg(const char* key) {
    // Para simplificar: se está no escopo global (último da pilha), usa rbss. Senão, rfp.
    SymbolTable* global_scope = scopeStack;
    while (global_scope != NULL && global_scope->prox != NULL) {
        global_scope = global_scope->prox;
    }

    SymbolTable* current = scopeStack;
    while (current) {
        Symbol* sym = find_in_scope(current, key);
        if (sym) {
            if (current == global_scope) {
                return "rbss"; // Registrador base para globais
            } else {
                return "rfp";  // Registrador base para locais
            }
        }
        current = current->prox;
    }
    return "r_error"; // Símbolo não encontrado
} 

int get_current_function_frame_size(void) {

    return variable_count * 4; 
}