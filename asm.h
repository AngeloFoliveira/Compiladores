#ifndef ASM_H
#define ASM_H

#include <stdio.h>
#include "table.h" // Para acesso à tabela de símbolos

// Representa uma única instrução ou rótulo Assembly
typedef struct asm_node {
    char *instruction;
    struct asm_node *next;
} asm_list_t;

// Funções para manipulação da lista de código
asm_list_t* asm_new_node(const char* instruction);
asm_list_t* asm_concat(asm_list_t* l1, asm_list_t* l2);
void asm_print_list(FILE *foutput, asm_list_t* list);
void asm_free_list(asm_list_t* list);

// Funções de geração de alto nível
void asm_generate_data_section(FILE *foutput, SymbolTable* global_scope);
void asm_generate_code(FILE *foutput, asm_list_t* code);

// Funções auxiliares para gerar trechos de código
asm_list_t* asm_generate_prologue(int frame_size);
asm_list_t* asm_generate_epilogue();
asm_list_t* asm_generate_return(asm_list_t* expression_code);
asm_list_t* asm_generate_binary_op(asm_list_t* code1, asm_list_t* code2, const char* op_command);
asm_list_t* asm_generate_load_literal(const char* literal);
asm_list_t* asm_generate_load_variable(const char* var_name);
asm_list_t* asm_generate_store_variable(const char* var_name, asm_list_t* expression_code);

#endif