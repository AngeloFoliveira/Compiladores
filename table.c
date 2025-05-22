// symbol_table.c
#include "table.h"
#include "errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


SymbolTable* scopeStack = NULL;

// Funções auxiliares

Symbol* create_symbol(const char* key, Natureza nature, DataType tipo, Arg* args, int linha) {
    Symbol* sym = (Symbol*) malloc(sizeof(Symbol));
    strncpy(sym->key, key, MAX_SYMBOL_NAME);
    sym->nature = nature;
    sym->dataType = tipo;
    sym->args = args;
    sym->line = linha;
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

void declare_symbol(const char* key, Natureza nature, DataType tipo, Arg* args, int linha) {
    //trocar para find_in_scope caso possa ter nomes iguais em escopos diferentes
    if (find_symbol(key)) {
        fprintf(stderr, "Erro: símbolo '%s' já declarado na linha %d.\n", key, find_symbol(key)->line);
        exit(ERR_DECLARED);
    }
    
    Symbol* sym = create_symbol(key, nature, tipo, args, linha);
    sym->prox = scopeStack->symbols;
    scopeStack->symbols = sym;
}

Symbol* use_symbol(const char* key, Natureza nature, int linha) {
    Symbol* sym = find_symbol(key);
    if (!sym) {
        fprintf(stderr, "Erro: símbolo '%s' não declarado, checar linha %d.\n", key, linha);
        exit(ERR_UNDECLARED);
    }
    if(nature == IDENTIFICADOR && sym->nature == FUNCAO) {
        fprintf(stderr, "Erro: símbolo '%s' é uma função, não uma variável, checar linha %d.\n", key, sym->line);
        exit(ERR_FUNCTION);
    }
    if(nature == FUNCAO && sym->nature == IDENTIFICADOR) {
        fprintf(stderr, "Erro: símbolo '%s' é uma variável, não uma função, checar linha %d.\n", key, sym->line);
        exit(ERR_VARIABLE);
    }
    return sym;
}


Arg* create_arg(DataType tipo) {
    Arg* novo = (Arg*) malloc(sizeof(Arg));
    novo->tipo = tipo;
    novo->prox = NULL;
    return novo;
}


Arg* append_arg(Arg* lista, Arg* novo) {
    if (!lista) return novo;

    Arg* atual = lista;
    while (atual->prox) {
        atual = atual->prox;
    }
    atual->prox = novo;
    return lista;
}


void checkChamadaFuncao(const char* key, Arg* args) {
    Symbol* func = find_symbol(key);
    if (!func || func->nature != FUNCAO) {
        fprintf(stderr, "Erro: função '%s' não declarada.\n", key);
        exit(ERR_UNDECLARED);
    }

    Arg* param = func->args;

    while (param && args) {
        if (param->tipo != args->tipo) {
            fprintf(stderr, "Erro: tipos incompatíveis em chamada de função '%s' na linha %d.\n", key, func->line);
            exit(ERR_WRONG_TYPE_ARGS);
        }
        param = param->prox;
        args = args->prox;
    }

    if (param && !args) {
        fprintf(stderr, "Erro: número insuficiente de argumentos para função '%s' na linha %d.\n", key, func->line);
        exit(ERR_MISSING_ARGS);
    }
    if (!param && args) {
        fprintf(stderr, "Erro: número excessivo de argumentos para função '%s' na linha %d.\n", key, func->line);
        exit(ERR_EXCESS_ARGS);
    }
}



void checkTipoExpressaoBinaria(DataType tipo1, DataType tipo2, int linha) {
    if (tipo1 != tipo2) {
        fprintf(stderr, "Erro: tipos incompatíveis em expressão binária na linha %d.\n", linha);
        exit(ERR_WRONG_TYPE);
    }
}
