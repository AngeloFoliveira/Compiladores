#ifndef ILOC_H
#define ILOC_H

#include "asd.h"

// Enumeração para todos os opcodes ILOC
typedef enum {
    ILOC_NOP, ILOC_HALT,
    ILOC_ADD, ILOC_SUB, ILOC_MULT, ILOC_DIV,
    ILOC_ADDI, ILOC_SUBI, ILOC_RSUBI, ILOC_MULTI, ILOC_DIVI,
    ILOC_LSHIFT, ILOC_LSHIFTI, ILOC_RSHIFT, ILOC_RSHIFTI,
    ILOC_AND, ILOC_ANDI, ILOC_OR, ILOC_ORI, ILOC_XOR, ILOC_XORI,
    ILOC_LOADI, ILOC_LOAD, ILOC_LOADAI, ILOC_LOADAO,
    ILOC_CLOAD, ILOC_CLOADAI, ILOC_CLOADAO,
    ILOC_STORE, ILOC_STOREAI, ILOC_STOREAO,
    ILOC_CSTORE, ILOC_CSTOREAI, ILOC_CSTOREAO,
    ILOC_I2I, ILOC_C2C, ILOC_C2I, ILOC_I2C,
    ILOC_JUMPI, ILOC_JUMP,
    ILOC_CBR,
    ILOC_CMP_LT, ILOC_CMP_LE, ILOC_CMP_EQ, ILOC_CMP_NE, ILOC_CMP_GT, ILOC_CMP_GE,
    ILOC_CALL, ILOC_LABEL // Opcodes "virtuais" para a geração
} iloc_opcode_t;

// Estrutura para uma instrução ILOC
typedef struct iloc_instruction {
    iloc_opcode_t opcode;
    char *arg1, *arg2, *arg3, *label;
} iloc_t;

// Lista encadeada de instruções ILOC
typedef struct iloc_list {
    iloc_t *instruction;
    struct iloc_list *next;
} iloc_list_t;

// Protótipos das funções
iloc_list_t* iloc_new_list(iloc_t *instruction);
iloc_t* iloc_new_instruction(iloc_opcode_t opcode, char *arg1, char *arg2, char *arg3, char *label);
iloc_list_t* iloc_concat(iloc_list_t *l1, iloc_list_t *l2);
void iloc_print_list(iloc_list_t *list);
void iloc_build_binary_operation(asd_tree_t *result_node, asd_tree_t *op1, asd_tree_t *op2, iloc_opcode_t opcode);


char* iloc_new_temp();
char* iloc_new_label();

#endif