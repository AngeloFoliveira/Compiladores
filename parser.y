%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asd.h"
#include "table.h"
#include "asm.h"

int yylex(void);
void yyerror (char const *mensagem);
int get_line_number(void);
extern asd_tree_t *arvore;

// Helper para criar rótulos únicos
static int label_count = 0;
char* new_label() {
    char* buffer = (char*) malloc(16);
    snprintf(buffer, 16, "L%d", label_count++);
    return buffer;
}
%}

%code requires {
    #include "table.h"
    typedef struct asd_tree asd_tree_t;
    typedef struct valor valor_t;
}

%union {
    asd_tree_t *no;
    valor_t *valor_lexico;
    DataType data_type;
}

%destructor {
} <no>;

%token TK_PR_AS
%token TK_PR_DECLARE
%token TK_PR_ELSE
%token TK_PR_FLOAT
%token TK_PR_IF
%token TK_PR_INT
%token TK_PR_IS
%token TK_PR_RETURN
%token TK_PR_RETURNS
%token TK_PR_WHILE
%token TK_PR_WITH
%token TK_OC_LE
%token TK_OC_GE
%token TK_OC_EQ
%token TK_OC_NE
%token<valor_lexico> TK_ID
%token<valor_lexico> TK_LI_INT
%token<valor_lexico> TK_LI_FLOAT
%token TK_ER

%type<no> programa
%type<no> lista
%type<no> elemento
%type<no> decl_var_global
%type<no> def_func
%type<no> corpo2
%type<no> comando_simples
%type<no> lista_comandos
%type<no> decl_var
%type<data_type> tipo
%type<no> atribuicao
%type<no> comando_retorno
%type<no> bloco_codigo_funcao
%type<no> construcao_fluxo
%type<no> literal
%type<no> e0
%type<no> e1
%type<no> e2
%type<no> e3
%type<no> e4
%type<no> e5
%type<no> e6
%type<no> e7
%type<no> expressao

%define parse.error verbose

%%

programa: criaEscopo lista destroiEscopo ';'{ 
        $$ = $2; 
        arvore = $$;
    };
programa: { arvore = NULL; $$ = NULL; };

criaEscopo: {push_scope();};
destroiEscopo: {pop_scope();};

lista: elemento { $$ = $1; };
lista : elemento ',' lista {
        if ($1 == NULL) { $$ = $3; }
        else if ($3 == NULL) { $$ = $1; }
        else {
            $$ = $1;
            asd_add_child($1, $3);
            $$->code = asm_concat($1->code, $3->code);
        }
    };

elemento: def_func { $$ = $1; };
elemento: decl_var_global { $$ = NULL; };

decl_var_global: TK_PR_DECLARE TK_ID TK_PR_AS tipo {
    declare_symbol($2->lexema, IDENTIFICADOR, $4, get_line_number());
    $$ = NULL;
    free($2->lexema); free($2);
};


def_func: TK_ID TK_PR_RETURNS tipo TK_PR_IS {
    declare_symbol($1->lexema, FUNCAO, $3, get_line_number()); 
    set_func_atual($1->lexema);
    free($1->lexema); 
    free($1);
} criaEscopo bloco_codigo_funcao destroiEscopo {
    $$ = $7; // O resultado da regra agora é o nó criado por 'bloco_codigo_funcao'
    free_func_atual(); // Libera a string 'func_atual'
};



bloco_codigo_funcao: corpo2 {
    // Usa a variável global 'func_atual' que já é setada na regra 'def_func'
    extern char* func_atual;

    $$ = asd_new(func_atual); 
    if ($1 != NULL) asd_add_child($$, $1);

    int frame_size = get_current_function_frame_size();
    frame_size = (frame_size + 15) & -16;

    char buffer[64];
    snprintf(buffer, sizeof(buffer), ".globl %s", func_atual);
    $$->code = asm_new_node(buffer);
    snprintf(buffer, sizeof(buffer), "%s:", func_atual);
    $$->code = asm_concat($$->code, asm_new_node(buffer));
    $$->code = asm_concat($$->code, asm_generate_prologue(frame_size));

    if ($1 != NULL) {
        $$->code = asm_concat($$->code, $1->code);
    }
};


corpo2: '[' ']' { $$ = NULL; };
corpo2: '[' lista_comandos ']' { $$ = $2; };

comando_simples: decl_var { $$ = $1; };
comando_simples: atribuicao { $$ = $1; };
// comando_simples: chamada_funcao { $$ = $1; }; // REMOVIDO
comando_simples: comando_retorno { $$ = $1; };
comando_simples: construcao_fluxo { $$ = $1; };
comando_simples: criaEscopo corpo2 destroiEscopo { $$ = $2; };

lista_comandos: comando_simples { $$ = $1; };
lista_comandos: comando_simples lista_comandos { 
    if ($1 == NULL) { $$ = $2; } 
    else if ($2 == NULL) { $$ = $1; } 
    else {
        $$ = $1;
        asd_add_child($1, $2);
        $$->code = asm_concat($1->code, $2->code); 
    }
};


decl_var: TK_PR_DECLARE TK_ID TK_PR_AS tipo {
    declare_symbol($2->lexema, IDENTIFICADOR, $4, get_line_number());
    $$ = NULL;
    free($2->lexema); free($2);
};

decl_var: TK_PR_DECLARE TK_ID TK_PR_AS tipo TK_PR_WITH literal {
    declare_symbol($2->lexema, IDENTIFICADOR, $4, get_line_number());
    checkTipoExpressaoBinaria($4, $6->tipo, get_line_number());

    $$ = asd_new("with");
    asd_add_child($$, asd_new($2->lexema));
    asd_add_child($$, $6);
    $$->tipo = $4;
    
    $$->code = asm_generate_store_variable($2->lexema, $6->code);
    
    free($2->lexema); free($2);
};

tipo: TK_PR_FLOAT {$$ = FLOAT;};
tipo: TK_PR_INT {$$ = INT;};

literal: TK_LI_INT {
    $$ = asd_new($1->lexema);
    $$->tipo = INT;
    $$->code = asm_generate_load_literal($1->lexema);
    free($1->lexema); free($1);
};

literal: TK_LI_FLOAT { 
    $$ = asd_new($1->lexema); 
    $$->tipo=FLOAT; 
    $$->code = asm_new_node("# ERRO: Geração de float não implementada");
    free($1->lexema); free($1); 
};

atribuicao: TK_ID TK_PR_IS expressao {
    $$ = asd_new("is");
    Symbol *sym = use_symbol($1->lexema, IDENTIFICADOR, get_line_number());
    $$->tipo = sym->dataType;
    checkTipoExpressaoBinaria($$->tipo, $3->tipo, get_line_number());

    asd_add_child($$, asd_new($1->lexema));
    asd_add_child($$, $3);

    $$->code = asm_generate_store_variable($1->lexema, $3->code);

    free($1->lexema); free($1);
};

comando_retorno: TK_PR_RETURN expressao TK_PR_AS tipo { 
    $$ = asd_new("return"); 
    checkTipoExpressaoBinaria($2->tipo, $4, get_line_number()); 
    check_return_type($4, get_line_number());  
    $$->tipo=$4; 
    if ($2 != NULL) asd_add_child($$,$2);
    
    $$->code = asm_generate_return($2->code);
};

construcao_fluxo: TK_PR_IF '(' expressao ')' criaEscopo corpo2 destroiEscopo {
    $$ = asd_new("if");
    asd_add_child($$, $3);
    if($6) asd_add_child($$, $6);

    char* label_end = new_label();
    char buffer[64];

    $$->code = $3->code;
    $$->code = asm_concat($$->code, asm_new_node("cmpl $0, %eax"));
    snprintf(buffer, sizeof(buffer), "je %s", label_end);
    $$->code = asm_concat($$->code, asm_new_node(buffer));
    
    if($6) $$->code = asm_concat($$->code, $6->code);
    
    snprintf(buffer, sizeof(buffer), "%s:", label_end);
    $$->code = asm_concat($$->code, asm_new_node(buffer));

    free(label_end);
};

construcao_fluxo: TK_PR_IF '(' expressao ')' criaEscopo corpo2 destroiEscopo TK_PR_ELSE criaEscopo corpo2 destroiEscopo {
    $$ = asd_new("if-else"); 
    asd_add_child($$, $3);
    if($6) asd_add_child($$, $6);
    if($10) asd_add_child($$, $10);

    char* label_false = new_label();
    char* label_end = new_label();
    char buffer[64];

    $$->code = $3->code;
    $$->code = asm_concat($$->code, asm_new_node("cmpl $0, %eax"));
    snprintf(buffer, sizeof(buffer), "je %s", label_false);
    $$->code = asm_concat($$->code, asm_new_node(buffer));

    if($6) $$->code = asm_concat($$->code, $6->code);
    snprintf(buffer, sizeof(buffer), "jmp %s", label_end);
    $$->code = asm_concat($$->code, asm_new_node(buffer));

    snprintf(buffer, sizeof(buffer), "%s:", label_false);
    $$->code = asm_concat($$->code, asm_new_node(buffer));
    if($10) $$->code = asm_concat($$->code, $10->code);

    snprintf(buffer, sizeof(buffer), "%s:", label_end);
    $$->code = asm_concat($$->code, asm_new_node(buffer));

    free(label_false);
    free(label_end);
};

construcao_fluxo: TK_PR_WHILE '(' expressao ')' criaEscopo corpo2 destroiEscopo {
    $$ = asd_new("while");
    asd_add_child($$,$3); 
    if($6) asd_add_child($$,$6); 
 
    char* label_start = new_label();
    char* label_end = new_label();
    char buffer[64];

    snprintf(buffer, sizeof(buffer), "%s:", label_start);
    $$->code = asm_new_node(buffer);
    
    $$->code = asm_concat($$->code, $3->code);
    $$->code = asm_concat($$->code, asm_new_node("cmpl $0, %eax"));
    snprintf(buffer, sizeof(buffer), "je %s", label_end);
    $$->code = asm_concat($$->code, asm_new_node(buffer));
    
    if($6) $$->code = asm_concat($$->code, $6->code);
    
    snprintf(buffer, sizeof(buffer), "jmp %s", label_start);
    $$->code = asm_concat($$->code, asm_new_node(buffer));

    snprintf(buffer, sizeof(buffer), "%s:", label_end);
    $$->code = asm_concat($$->code, asm_new_node(buffer));
    
    free(label_start);
    free(label_end);
};

expressao: e7 { $$ = $1; };

e7: e6 { $$ = $1; };
e6: e5 { $$ = $1; };

e5: e5 TK_OC_EQ e4 { 
    $$ = asd_new("=="); 
    checkTipoExpressaoBinaria($1->tipo, $3->tipo, get_line_number());
    $$->tipo = INT; 
    asd_add_child($$, $1); asd_add_child($$, $3);
    $$->code = asm_generate_binary_op($1->code, $3->code, "cmpl %ebx, %eax");
    $$->code = asm_concat($$->code, asm_new_node("sete %al"));
    $$->code = asm_concat($$->code, asm_new_node("movzbl %al, %eax"));
};
e5: e5 TK_OC_NE e4 { 
    $$ = asd_new("!="); 
    checkTipoExpressaoBinaria($1->tipo, $3->tipo, get_line_number());
    $$->tipo = INT; 
    asd_add_child($$, $1); asd_add_child($$, $3);
    $$->code = asm_generate_binary_op($1->code, $3->code, "cmpl %ebx, %eax");
    $$->code = asm_concat($$->code, asm_new_node("setne %al"));
    $$->code = asm_concat($$->code, asm_new_node("movzbl %al, %eax"));
};
e5: e4 { $$ = $1; };
 
e4: e4 '<' e3 { 
    $$ = asd_new("<"); 
    checkTipoExpressaoBinaria($1->tipo, $3->tipo, get_line_number());
    $$->tipo = INT; 
    asd_add_child($$, $1); asd_add_child($$, $3); 
    $$->code = asm_generate_binary_op($1->code, $3->code, "cmpl %ebx, %eax");
    $$->code = asm_concat($$->code, asm_new_node("setl %al"));
    $$->code = asm_concat($$->code, asm_new_node("movzbl %al, %eax"));
};
e4: e4 '>' e3 {
    $$ = asd_new(">");
    checkTipoExpressaoBinaria($1->tipo, $3->tipo, get_line_number());
    $$->tipo = INT;
    asd_add_child($$, $1); asd_add_child($$, $3);
    $$->code = asm_generate_binary_op($1->code, $3->code, "cmpl %ebx, %eax");
    $$->code = asm_concat($$->code, asm_new_node("setg %al"));
    $$->code = asm_concat($$->code, asm_new_node("movzbl %al, %eax"));
};
e4: e4 TK_OC_LE e3 {
    $$ = asd_new("<=");
    checkTipoExpressaoBinaria($1->tipo, $3->tipo, get_line_number());
    $$->tipo = INT;
    asd_add_child($$, $1); asd_add_child($$, $3);
    $$->code = asm_generate_binary_op($1->code, $3->code, "cmpl %ebx, %eax");
    $$->code = asm_concat($$->code, asm_new_node("setle %al"));
    $$->code = asm_concat($$->code, asm_new_node("movzbl %al, %eax"));
};
e4: e4 TK_OC_GE e3 {
    $$ = asd_new(">=");
    checkTipoExpressaoBinaria($1->tipo, $3->tipo, get_line_number());
    $$->tipo = INT;
    asd_add_child($$, $1); asd_add_child($$, $3);
    $$->code = asm_generate_binary_op($1->code, $3->code, "cmpl %ebx, %eax");
    $$->code = asm_concat($$->code, asm_new_node("setge %al"));
    $$->code = asm_concat($$->code, asm_new_node("movzbl %al, %eax"));
};
e4: e3 { $$ = $1; };

e3: e3 '+' e2 {
    $$ = asd_new("+");
    checkTipoExpressaoBinaria($1->tipo, $3->tipo, get_line_number());
    $$->tipo = $1->tipo;
    asd_add_child($$, $1); asd_add_child($$, $3);
    $$->code = asm_generate_binary_op($1->code, $3->code, "addl %ebx, %eax");
};
e3: e3 '-' e2 {
    $$ = asd_new("-");
    checkTipoExpressaoBinaria($1->tipo, $3->tipo, get_line_number());
    $$->tipo = $1->tipo;
    asd_add_child($$, $1); asd_add_child($$, $3);
    $$->code = asm_generate_binary_op($1->code, $3->code, "subl %ebx, %eax");
};
e3: e2 { $$ = $1; };
 
e2: e2 '*' e1 {
    $$ = asd_new("*");
    checkTipoExpressaoBinaria($1->tipo, $3->tipo, get_line_number());
    $$->tipo = $1->tipo;
    asd_add_child($$, $1); asd_add_child($$, $3);
    $$->code = asm_generate_binary_op($1->code, $3->code, "imull %ebx, %eax");
};
e2: e2 '/' e1 {
    $$ = asd_new("/");
    checkTipoExpressaoBinaria($1->tipo, $3->tipo, get_line_number());
    $$->tipo = $1->tipo;
    asd_add_child($$, $1); asd_add_child($$, $3);
    $$->code = $1->code;
    $$->code = asm_concat($$->code, asm_new_node("pushq %rax"));
    $$->code = asm_concat($$->code, $3->code);
    $$->code = asm_concat($$->code, asm_new_node("movl %eax, %ebx"));
    $$->code = asm_concat($$->code, asm_new_node("popq %rax"));
    $$->code = asm_concat($$->code, asm_new_node("cdq"));
    $$->code = asm_concat($$->code, asm_new_node("idivl %ebx"));
};
e2: e1 { $$ = $1; };

e1: '-' e1 {
    $$ = asd_new("u-");
    $$->tipo = $2->tipo;
    asd_add_child($$, $2);
    $$->code = $2->code;
    $$->code = asm_concat($$->code, asm_new_node("negl %eax"));
};
e1: '!' e1 {
    $$ = asd_new("!");
    $$->tipo = INT;
    asd_add_child($$, $2);
    $$->code = $2->code;
    $$->code = asm_concat($$->code, asm_new_node("cmpl $0, %eax"));
    $$->code = asm_concat($$->code, asm_new_node("sete %al"));
    $$->code = asm_concat($$->code, asm_new_node("movzbl %al, %eax"));
};
e1: e0 { $$ = $1; };

// e0: chamada_funcao {$$ = $1;}; // REMOVIDO
e0: TK_ID {
    $$ = asd_new($1->lexema);
    Symbol *sym = use_symbol($1->lexema, IDENTIFICADOR, get_line_number());
    $$->tipo = sym->dataType;
    $$->code = asm_generate_load_variable($1->lexema);
    free($1->lexema); free($1);
};
e0: literal {$$ = $1;};
e0: '(' expressao ')' { $$ = $2; };
%%

void yyerror(char const * mensagem)
{
    fprintf(stderr, "Erro sintático com a mensagem '%s' na linha %d\n", mensagem, get_line_number());
    exit(1);
}