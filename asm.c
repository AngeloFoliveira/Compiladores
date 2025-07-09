#include "asm.h"
#include "table.h"
#include <stdlib.h>
#include <string.h>

asm_list_t* asm_new_node(const char* instruction) {
    asm_list_t *node = (asm_list_t*) malloc(sizeof(asm_list_t));
    if (!node) {
        fprintf(stderr, "Erro de alocação para nó Assembly.\n");
        exit(EXIT_FAILURE);
    }
    node->instruction = instruction ? strdup(instruction) : NULL;
    node->next = NULL;
    return node;
}

asm_list_t* asm_concat(asm_list_t* l1, asm_list_t* l2) {
    if (!l1) return l2;
    if (!l2) return l1;
    asm_list_t *p = l1;
    while (p->next) {
        p = p->next;
    }
    p->next = l2;
    return l1;
}

void asm_print_list(FILE *foutput, asm_list_t* list) {
    if (!foutput || !list) return;
    for (asm_list_t *p = list; p; p = p->next) {
        if (p->instruction) {
            // Rótulos não são indentados
            if (strchr(p->instruction, ':') == NULL) {
                fprintf(foutput, "\t%s\n", p->instruction);
            } else {
                fprintf(foutput, "%s\n", p->instruction);
            }
        }
    }
}

void asm_free_list(asm_list_t* list) {
    while(list) {
        asm_list_t *next = list->next;
        if(list->instruction) free(list->instruction);
        free(list);
        list = next;
    }
}

void asm_generate_data_section(FILE *foutput, SymbolTable* global_scope) {
    if (!foutput || !global_scope) return;
    fprintf(foutput, ".data\n");
    for (Symbol* sym = global_scope->symbols; sym; sym = sym->prox) {
        if (sym->nature == IDENTIFICADOR) {
            fprintf(foutput, "\t.globl %s\n", sym->key);
            fprintf(foutput, "\t.align 4\n");
            fprintf(foutput, "\t.type %s, @object\n", sym->key);
            fprintf(foutput, "\t.size %s, 4\n", sym->key);
            fprintf(foutput, "%s:\n", sym->key);
            fprintf(foutput, "\t.long 0 # Valor inicial para %s\n", sym->key);
        }
    }
}

void asm_generate_code(FILE *foutput, asm_list_t* code) {
    if (!foutput) return;
    fprintf(foutput, ".text\n");
    asm_print_list(foutput, code);
}

asm_list_t* asm_generate_prologue(int frame_size) {
    char buffer[64];
    asm_list_t *list = asm_new_node("pushq %rbp");
    list = asm_concat(list, asm_new_node("movq %rsp, %rbp"));
    if (frame_size > 0) {
        snprintf(buffer, sizeof(buffer), "subq $%d, %%rsp", frame_size);
        list = asm_concat(list, asm_new_node(buffer));
    }
    return list;
}

asm_list_t* asm_generate_epilogue() {
    asm_list_t *list = asm_new_node("leave");
    list = asm_concat(list, asm_new_node("ret"));
    return list;
}

asm_list_t* asm_generate_return(asm_list_t* expression_code) {
    // A convenção é que o resultado da expressão já está em %eax.
    // O código do epílogo se encarrega do resto.
    asm_list_t* code = expression_code;
    code = asm_concat(code, asm_generate_epilogue());
    return code;
}

asm_list_t* asm_generate_binary_op(asm_list_t* code1, asm_list_t* code2, const char* op_command) {
    // Estratégia:
    // 1. Executa code1 (resultado em %eax)
    // 2. Salva resultado na pilha
    // 3. Executa code2 (resultado em %eax)
    // 4. Move o resultado de code2 para %ebx
    // 5. Recupera resultado de code1 da pilha para %eax
    // 6. Executa a operação
    asm_list_t* code = code1;
    code = asm_concat(code, asm_new_node("pushq %rax"));
    code = asm_concat(code, code2);
    code = asm_concat(code, asm_new_node("movl %eax, %ebx"));
    code = asm_concat(code, asm_new_node("popq %rax"));
    code = asm_concat(code, asm_new_node(op_command)); // ex: "addl %ebx, %eax"
    return code;
}

asm_list_t* asm_generate_load_literal(const char* literal) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "movl $%s, %%eax", literal);
    return asm_new_node(buffer);
}

asm_list_t* asm_generate_load_variable(const char* var_name) {
    char buffer[64];
    const char* base_reg = get_symbol_base_reg(var_name);
    int offset = get_symbol_offset(var_name);

    if (strcmp(base_reg, "rbss") == 0) { // Variável global
        snprintf(buffer, sizeof(buffer), "movl %s(%%rip), %%eax", var_name);
    } else { // Variável local
        snprintf(buffer, sizeof(buffer), "movl %d(%%rbp), %%eax", -offset); // Offsets locais são negativos
    }
    return asm_new_node(buffer);
}

asm_list_t* asm_generate_store_variable(const char* var_name, asm_list_t* expression_code) {
    char buffer[64];
    const char* base_reg = get_symbol_base_reg(var_name);
    int offset = get_symbol_offset(var_name);
    
    // O código da expressão já deixa o resultado em %eax
    asm_list_t* code = expression_code;

    if (strcmp(base_reg, "rbss") == 0) { // Variável global
        snprintf(buffer, sizeof(buffer), "movl %%eax, %s(%%rip)", var_name);
    } else { // Variável local
        snprintf(buffer, sizeof(buffer), "movl %%eax, %d(%%rbp)", -offset); // Offsets locais são negativos
    }
    code = asm_concat(code, asm_new_node(buffer));
    return code;
}