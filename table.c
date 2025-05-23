// symbol_table.c
#include "table.h"
#include "errors.h"
#include "asd.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

SymbolTable *scopeStack = NULL;

// Funções auxiliares

Symbol *create_symbol(const char *key, Natureza nature, DataType tipo, Arg *args, int linha)
{
    Symbol *sym = (Symbol *)malloc(sizeof(Symbol));
    strncpy(sym->key, key, MAX_SYMBOL_NAME);
    sym->nature = nature;
    sym->dataType = tipo;
    sym->args = args;
    sym->line = linha;
    sym->prox = NULL;
    return sym;
}

void push_scope()
{
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    table->symbols = NULL;
    table->prox = scopeStack;
    scopeStack = table;
}

void pop_scope()
{
    if (scopeStack)
    {
        Symbol *sym = scopeStack->symbols;
        while (sym)
        {
            Symbol *prox = sym->prox;
            free_args(sym->args);
            free(sym);
            sym = prox;
        }
        SymbolTable *old = scopeStack;
        scopeStack = scopeStack->prox;
        free(old);
    }
}

Symbol *find_in_scope(SymbolTable *table, const char *key)
{
    Symbol *sym = table->symbols;
    while (sym)
    {
        if (strcmp(sym->key, key) == 0)
            return sym;
        sym = sym->prox;
    }
    return NULL;
}

Symbol *find_symbol(const char *key)
{
    SymbolTable *current = scopeStack;
    while (current)
    {
        Symbol *sym = find_in_scope(current, key);
        if (sym)
            return sym;
        current = current->prox;
    }
    return NULL;
}

// Interface pública

void declare_symbol(const char *key, Natureza nature, DataType tipo, Arg *args, int linha)
{
    // trocar para find_in_scope caso possa ter nomes iguais em escopos diferentes
    if (find_symbol(key))
    {
        fprintf(stderr, "Erro: símbolo '%s' já declarado na linha %d.\n", key, find_symbol(key)->line);
        exit(ERR_DECLARED);
    }
    // printf("Declarando símbolo '%s' na linha %d.\n", key, linha);
    // printf("Tipo: %d\n", tipo);
    // printf("Natureza: %d\n", nature);
    // while (args)
    //{
    //     printf("Argumento: %d\n", args->tipo);
    //     args = args->prox;
    // }
    Symbol *sym = create_symbol(key, nature, tipo, args, linha);
    sym->prox = scopeStack->symbols;
    scopeStack->symbols = sym;
}

Symbol *use_symbol(const char *key, Natureza nature, int linha)
{
    Symbol *sym = find_symbol(key);
    if (!sym && nature == IDENTIFICADOR)
    {
        fprintf(stderr, "Erro: variável '%s' não declarado, checar linha %d.\n", key, linha);
        exit(ERR_UNDECLARED);
    }
    if (!sym && nature == FUNCAO)
    {
        fprintf(stderr, "Erro: função '%s' não declarada, checar linha %d.\n", key, linha);
        exit(ERR_UNDECLARED);
    }
    if (nature == IDENTIFICADOR && sym->nature == FUNCAO)
    {
        fprintf(stderr, "Erro: símbolo '%s' é uma função, não uma variável, checar linha %d.\n", key, sym->line);
        exit(ERR_FUNCTION);
    }
    if (nature == FUNCAO && sym->nature == IDENTIFICADOR)
    {
        fprintf(stderr, "Erro: símbolo '%s' é uma variável, não uma função, checar linha %d.\n", key, sym->line);
        exit(ERR_VARIABLE);
    }
    return sym;
}

Arg *create_arg(DataType tipo)
{
    Arg *novo = (Arg *)malloc(sizeof(Arg));
    novo->tipo = tipo;
    novo->prox = NULL;
    // printf("Criando argumento do tipo %d\n", tipo);
    return novo;
}

Arg *append_arg(Arg *lista, Arg *novo)
{

    Arg *atual = lista;
    // printf("Valor da lista inicial: %d\n", lista->tipo);
    while (atual->prox)
    {
        // printf("Valor da lista sequencia: %d\n", atual->tipo);
        atual = atual->prox;
    }
    atual->prox = novo;
    return lista;
}

void checkChamadaFuncao(const char *key, Arg *args)
{
    Symbol *func = find_symbol(key);
    Arg *param = func->args;

    /* Arg *aux1 = args;
     while (aux1 != NULL)
     {
         printf("Valor da lista de args da chamada: %d\n", aux1->tipo);
         aux1 = aux1->prox;
     }

     Arg *aux2 = func->args;
     while (aux2 != NULL)
     {
         printf("Valor da lista de args da funcao correspondente: %d\n", aux2->tipo);
         aux2 = aux2->prox;
     }
         */

    while (param != NULL && args != NULL)
    {
        if (param->tipo != args->tipo)
        {
            fprintf(stderr, "Erro: tipos incompatíveis na chamada da função '%s' na linha %d.\n", key, func->line);
            exit(ERR_WRONG_TYPE_ARGS);
        }
        param = param->prox;
        args = args->prox;
    }

    // Verifica se há argumentos a mais ou a menos DEPOIS do loop
    if (args != NULL)
    {
        fprintf(stderr, "Erro: chamada da função '%s' com argumentos a mais na linha %d.\n", key, func->line);
        exit(ERR_EXCESS_ARGS);
    }

    if (param != NULL)
    {
        fprintf(stderr, "Erro: chamada da função '%s' com argumentos a menos na linha %d.\n", key, func->line);
        exit(ERR_MISSING_ARGS);
    }
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

Arg *transformar_asd_em_lista(asd_tree_t *tree, Arg *lista)
{

    for (int i = 0; i < tree->number_of_children; i++)
    {
        if (lista == NULL)
        {
            // se for numero, ver se é inteiro ou float
            // printf("Valor que será convertido para criar a lista!: %s\n", tree->label);
            float valor = strtod(tree->label, NULL);
            // printf("Valor convertido para numero: %f\n", valor);
            if (floor(valor) == valor)
            {
                lista = create_arg(INT);
                // printf("VALOR INTEIRO ADICIONADO NA ARVORE RECEM CRIADA!\n");
                if (tree->children[i] != NULL)
                {
                    float valorFilho = strtod(tree->children[i]->label, NULL);
                    // printf("Valor convertido para numero: %f\n", valorFilho);
                    if (floor(valorFilho) == valorFilho)
                    {
                        lista = append_arg(lista, create_arg(INT));
                        // printf("Novo valor: %s\n", tree->children[i]->label);
                    }
                    else
                    {
                        lista = append_arg(lista, create_arg(FLOAT));
                        // printf("Novo valor: %s\n", tree->children[i]->label);
                    }
                }
            }
            else
            {
                lista = create_arg(FLOAT);
                // printf("VALOR FLOAT ADICIONADO NA ARVORE RECEM CRIADA!\n");
                if (tree->children[i] != NULL)
                {
                    float valorFilho = strtod(tree->children[i]->label, NULL);
                    // printf("Valor convertido para numero: %f\n", valorFilho);
                    if (floor(valorFilho) == valorFilho)
                    {
                        lista = append_arg(lista, create_arg(INT));
                        // printf("Novo valor: %s\n", tree->children[i]->label);
                    }
                    else
                    {
                        lista = append_arg(lista, create_arg(FLOAT));
                        // printf("Novo valor: %s\n", tree->children[i]->label);
                    }
                }
            }
        }
        else
        {
            if (tree->children[i]->tipo == INT)
            {
                lista = append_arg(lista, create_arg(INT));
                // printf("Novo valor: %s\n", tree->children[i]->label);
            }
            else if (tree->children[i]->tipo == FLOAT)
            {
                lista = append_arg(lista, create_arg(FLOAT));
                // printf("Novo valor: %s\n", tree->children[i]->label);
            }
        }
        lista = transformar_asd_em_lista(tree->children[i], lista);
    }

    //printa a lista
    Arg *aux = lista;
    while (aux != NULL)
    {
        printf("Valor da lista: %d\n", aux->tipo);
        aux = aux->prox;
    }

    return lista;
}

void checkTipoExpressaoBinaria(DataType tipo1, DataType tipo2, int linha)
{
    if (tipo1 != tipo2)
    {
        fprintf(stderr, "Erro: tipos incompatíveis em expressão binária na linha %d.\n", linha);
        exit(ERR_WRONG_TYPE);
    }
}
