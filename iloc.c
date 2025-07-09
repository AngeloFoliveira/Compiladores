#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iloc.h"

// Contadores estáticos para gerar novos temporários e rótulos
static int temp_count = 0;
static int label_count = 0;

char* iloc_new_temp() {
    char buffer[16];
    sprintf(buffer, "r%d", temp_count++);
    return strdup(buffer);
}

char* iloc_new_label() {
    char buffer[16];
    sprintf(buffer, "L%d", label_count++);
    return strdup(buffer);
}

iloc_t* iloc_new_instruction(iloc_opcode_t opcode, char *arg1, char *arg2, char *arg3, char *label) {
    iloc_t *inst = (iloc_t*) malloc(sizeof(iloc_t));
    if (!inst) {
        fprintf(stderr, "Erro de alocação para instrução ILOC.\n");
        exit(EXIT_FAILURE);
    }
    inst->opcode = opcode;
    inst->arg1 = arg1 ? strdup(arg1) : NULL;
    inst->arg2 = arg2 ? strdup(arg2) : NULL;
    inst->arg3 = arg3 ? strdup(arg3) : NULL;
    inst->label = label ? strdup(label) : NULL;
    return inst;
}

iloc_list_t* iloc_new_list(iloc_t *instruction) {
    iloc_list_t *list = (iloc_list_t*) malloc(sizeof(iloc_list_t));
     if (!list) {
        fprintf(stderr, "Erro de alocação para lista ILOC.\n");
        exit(EXIT_FAILURE);
    }
    list->instruction = instruction;
    list->next = NULL;
    return list;
}

iloc_list_t* iloc_concat(iloc_list_t *l1, iloc_list_t *l2) {
    if (!l1) return l2;
    if (!l2) return l1;

    iloc_list_t *p = l1;
    while (p->next) {
        p = p->next;
    }
    p->next = l2;
    return l1;
}

// Mapeia o opcode para sua representação em string
const char* opcode_to_string(iloc_opcode_t opcode) {
    static const char * const strings[] = {
        "nop", "halt", "add", "sub", "mult", "div", "addI", "subI", "rsubI",
        "multI", "divI", "lshift", "lshiftI", "rshift", "rshiftI",
        "and", "andI", "or", "orI", "xor", "xorI", "loadI", "load",
        "loadAI", "loadAO", "cload", "cloadAI", "cloadAO", "store",
        "storeAI", "storeAO", "cstore", "cstoreAI", "cstoreAO", "i2i",
        "c2c", "c2i", "i2c", "jumpI", "jump", "cbr", "cmp_LT", "cmp_LE",
        "cmp_EQ", "cmp_NE", "cmp_GT", "cmp_GE", "call"
    };
    if (opcode > ILOC_CALL) return "invalid_op";
    return strings[opcode];
}

void iloc_print_list(iloc_list_t *list) {
    if (!list) return;

    iloc_list_t *p = list;
    while (p) {
        iloc_t *inst = p->instruction;
        if (inst) {
            const char *op_str = opcode_to_string(inst->opcode);

            // Imprime o rótulo se ele existir
            if(inst->label) {
                printf("%s:", inst->label);
            }

            // Usa um switch para formatar cada tipo de instrução corretamente
            switch(inst->opcode) {
                // Instruções sem argumentos
                case ILOC_HALT:
                case ILOC_NOP:
                    printf("\t%s\n", op_str);
                    break;

                // Instruções com 1 fonte, 1 destino (formato: op fonte => destino)
                case ILOC_LOAD:
                case ILOC_I2I:
                    printf("\t%s %s => %s\n", op_str, inst->arg1, inst->arg3);
                    break;

                // Instruções com 1 fonte C, 1 destino R (formato: op fonte => destino)
                case ILOC_LOADI:
                    printf("\t%s %s => %s\n", op_str, inst->arg1, inst->arg3);
                    break;

                // Instruções com 2 fontes, 1 destino (formato: op f1, f2 => destino)
                case ILOC_ADD:
                case ILOC_SUB:
                case ILOC_MULT:
                case ILOC_DIV:
                case ILOC_ADDI:
                case ILOC_AND:
                case ILOC_OR:
                    printf("\t%s %s, %s => %s\n", op_str, inst->arg1, inst->arg2, inst->arg3);
                    break;

                // Instruções de armazenamento (formato: op fonte => base, offset)
                case ILOC_STOREAI:
                    printf("\t%s %s => %s, %s\n", op_str, inst->arg1, inst->arg2, inst->arg3);
                    break;
                
                // Instruções de carga (formato: op base, offset => destino)
                case ILOC_LOADAI:
                     printf("\t%s %s, %s => %s\n", op_str, inst->arg1, inst->arg2, inst->arg3);
                     break;

                // Comparações (formato: op f1, f2 -> destino)
                case ILOC_CMP_LT:
                case ILOC_CMP_GT:
                case ILOC_CMP_LE:
                case ILOC_CMP_GE:
                case ILOC_CMP_EQ:
                case ILOC_CMP_NE:
                    printf("\t%s %s, %s -> %s\n", op_str, inst->arg1, inst->arg2, inst->arg3);
                    break;

                // Desvios
                case ILOC_CBR:
                    printf("\t%s %s -> %s, %s\n", op_str, inst->arg1, inst->arg2, inst->arg3);
                    break;
                case ILOC_JUMPI:
                    printf("\t%s -> %s\n", op_str, inst->arg1);
                    break;

                case ILOC_LABEL:
                    printf("\n");
                    break;

                default:
                    fprintf(stderr, "ERRO: Opcode desconhecido na impressao: %s\n", op_str);
                    break;
            }
        }
        p = p->next;
    }
}

void iloc_build_binary_operation(asd_tree_t *result_node, asd_tree_t *op1, asd_tree_t *op2, iloc_opcode_t opcode) {
    // Cria um novo registrador temporário para armazenar o resultado da operação.
    result_node->temp = iloc_new_temp();

    // Cria a instrução ILOC específica para a operação.
    //    Usa os temporários dos operandos como fonte
    //    e o novo temporário como destino.
    iloc_t *op_inst = iloc_new_instruction(opcode, op1->temp, op2->temp, result_node->temp, NULL);
    iloc_list_t *op_list = iloc_new_list(op_inst);

    // Concatena o código. 
    result_node->code = iloc_concat(op1->code, op2->code);
    result_node->code = iloc_concat(result_node->code, op_list);
}