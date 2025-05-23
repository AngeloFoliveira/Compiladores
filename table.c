// symbol_table.c
#include "table.h"
#include "errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


SymbolTable* scopeStack = NULL;
char *func_atual = NULL;
char *func_call = NULL;
int parameter_count = 0;

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

void push_scope() {
    SymbolTable* table = (SymbolTable*) malloc(sizeof(SymbolTable));
    table->symbols = NULL;
    table->prox = scopeStack;
    scopeStack = table;
}

void pop_scope() {
    if (scopeStack) {
        Symbol* sym = scopeStack->symbols;
        while (sym) {
            Symbol* prox = sym->prox;
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
    //trocar para find_in_scope caso possa ter nomes iguais em escopos diferentes
    if (find_symbol(key)) {
        fprintf(stderr, "Erro: símbolo '%s' já declarado. Linha: %d\n", key, linha);
        exit(ERR_DECLARED);
    }
    Symbol* sym = create_symbol(key, nature, tipo);
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
        fprintf(stderr, "Erro: tipos incompatíveis em expressão binária. Linha: %d\n", linha);
        exit(ERR_WRONG_TYPE);
    }
}

void set_func_atual(const char* func) {
    func_atual = strdup(func);
}

void free_func_atual() {
    if (func_atual) {
        free(func_atual);
        func_atual = NULL;
    }
}

//Insere no fim da lista de argumentos
void insert_arg(DataType tipo) {
    // if (!func_atual) {
    //     fprintf(stderr, "Erro: função não definida.\n");
    //     exit(1);
    // }
    Symbol* func = find_symbol(func_atual);
    // if (!func) {
    //     fprintf(stderr, "Erro: função '%s' não declarada.\n", func_atual);
    //     exit(1);
    // }
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

void reset_parameter_count() {
    parameter_count = 0;
}

void set_func_call(const char* func) {
    func_call = strdup(func);
}

void free_func_call() {
    if (func_call) {
        free(func_call);
        func_call = NULL;
    }
}

void check_func_call(DataType tipo, int linha) {
    if (!func_call) {
        fprintf(stderr, "Erro: chamada de função sem função atual.\n");
        exit(1);
    }
    Symbol* func = find_symbol(func_call);
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