// symbol_table.c
#include "table.h"
#include "errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


SymbolTable* scopeStack = NULL;

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

void declare_symbol(const char* key, Natureza nature, DataType tipo) {
    //trocar para find_in_scope caso possa ter nomes iguais em escopos diferentes
    if (find_symbol(key)) {
        fprintf(stderr, "Erro: símbolo '%s' já declarado.\n", key);
        exit(ERR_DECLARED);
    }
    Symbol* sym = create_symbol(key, nature, tipo);
    sym->prox = scopeStack->symbols;
    scopeStack->symbols = sym;
}

Symbol* use_symbol(const char* key) {
    Symbol* sym = find_symbol(key);
    if (!sym) {
        fprintf(stderr, "Erro: símbolo '%s' não declarado.\n", key);
        exit(ERR_UNDECLARED);
    }
    return sym;
}
