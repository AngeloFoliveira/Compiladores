#include <stdio.h>
#include "asd.h"
#include "asm.h"
#include "table.h"

extern int yyparse(void);
extern int yylex_destroy(void);
extern SymbolTable* get_global_scope(void); // Precisa ser criada em table.c

asd_tree_t *arvore = NULL;

int main (int argc, char **argv)
{
    int ret = yyparse();
    if (ret == 0 && arvore != NULL) {
        // 1. Gerar segmento de dados a partir da tabela de símbolos globais
        asm_generate_data_section(stdout, get_global_scope());

        // 2. Gerar segmento de código
        asm_generate_code(stdout, arvore->code);
    }
    
    if (arvore) {
        // Liberar a memória da AST e do código associado
        asm_free_list(arvore->code);
        asd_free(arvore);
    }
    
    yylex_destroy();
    return ret;
}

