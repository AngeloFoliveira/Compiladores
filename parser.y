%{
#include <stdio.h>
#include "asd.h"
int yylex(void);
void yyerror (char const *mensagem);
int get_line_number(void);
extern asd_tree_t *arvore;

%}

%code requires {
    typedef struct asd_tree asd_tree_t;
    typedef struct valor valor_t;
}

%union {
	asd_tree_t *no;
	valor_t *valor_lexico;
}


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
%type<no> cabecalho
%type<no> corpo2
%type<no> comando_simples
%type<no> lista_comandos
%type<no> decl_var
%type<no> atribuicao
%type<no> chamada_funcao
%type<no> argumentos
%type<no> comando_retorno
%type<no> construcao_fluxo
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

programa: lista ';'{ arvore = $$; $$ = $1; };
programa: { arvore = NULL; };

lista: elemento { $$ = $1; };
lista : elemento ',' lista { $$ = $1; asd_add_child($$, $3); };

elemento: def_func { $$ = $1; };
elemento: decl_var_global { $$ = $1; };

decl_var_global: TK_PR_DECLARE TK_ID TK_PR_AS tipo { $$ = NULL; free($2->lexema); free($2);};

def_func: TK_ID TK_PR_RETURNS TK_PR_FLOAT TK_PR_IS corpo2 { $$ = asd_new($1->lexema); asd_add_child($$, $5); free($1->lexema); free($1);}; 
def_func: TK_ID TK_PR_RETURNS TK_PR_FLOAT TK_PR_WITH lista_parametros TK_PR_IS corpo2 { $$ = asd_new($1->lexema); asd_add_child($$, $7); free($1->lexema); free($1);}; 
def_func: TK_ID TK_PR_RETURNS TK_PR_INT TK_PR_IS corpo2 { $$ = asd_new($1->lexema); asd_add_child($$, $5); free($1->lexema); free($1);}; 
def_func: TK_ID TK_PR_RETURNS TK_PR_INT TK_PR_WITH lista_parametros TK_PR_IS corpo2 { $$ = asd_new($1->lexema); asd_add_child($$, $7); free($1->lexema); free($1);}; 

corpo2: '[' ']' { $$ = NULL; };
corpo2: '[' lista_comandos ']' { $$ = $2; };


lista_parametros: parametro;
lista_parametros: parametro ',' lista_parametros;

parametro: TK_ID TK_PR_AS TK_PR_INT { free($1->lexema); free($1); };
parametro: TK_ID TK_PR_AS TK_PR_FLOAT { free($1->lexema); free($1); };


comando_simples: decl_var { $$ = $1; };
comando_simples: atribuicao { $$ = $1; };
comando_simples: chamada_funcao { $$ = $1; };
comando_simples: comando_retorno { $$ = $1; };
comando_simples: construcao_fluxo { $$ = $1; };

lista_comandos: comando_simples { $$ = $1; };
lista_comandos: comando_simples lista_comandos { $$ = $1; asd_add_child($$, $2); };

decl_var: TK_PR_DECLARE TK_ID TK_PR_AS tipo { $$ = NULL; free($2->lexema); free($2);};
decl_var: TK_PR_DECLARE TK_ID TK_PR_AS tipo TK_PR_WITH literal { $$ = asd_new("with"); free($2->lexema); free($2); };

tipo: TK_PR_FLOAT;
tipo: TK_PR_INT;

literal: TK_LI_FLOAT { free($1->lexema); free($1); };
literal: TK_LI_INT { free($1->lexema); free($1); };

atribuicao: TK_ID TK_PR_IS expressao { $$ = asd_new("is"); asd_add_child($$,asd_new($1->lexema)); asd_add_child($$,$3); free($1->lexema); free($1); };

chamada_funcao: TK_ID '(' ')' { char buffer[256];
    				snprintf(buffer, sizeof(buffer), "call %s", $1->lexema);
    				$$ = asd_new(buffer);
    				free($1->lexema);
    				free($1); };
chamada_funcao: TK_ID '(' argumentos ')' { char buffer[256];
    				snprintf(buffer, sizeof(buffer), "call %s", $1->lexema);
    				$$ = asd_new(buffer);
    				asd_add_child($$,$3);
    				free($1->lexema);
    				free($1); };

argumentos: expressao { $$ = $1; };
argumentos: expressao ',' argumentos { $$ = $1; asd_add_child($$,$3); };

comando_retorno: TK_PR_RETURN expressao TK_PR_AS TK_PR_FLOAT { $$ = asd_new("return"); asd_add_child($$,$2); };
comando_retorno: TK_PR_RETURN expressao TK_PR_AS TK_PR_INT { $$ = asd_new("return"); asd_add_child($$,$2); };

construcao_fluxo: TK_PR_IF '(' expressao ')' corpo2 { $$ = asd_new("if"); asd_add_child($$,$3); asd_add_child($$,$5); };
construcao_fluxo: TK_PR_IF '(' expressao ')' corpo2 TK_PR_ELSE corpo2 { $$ = asd_new("if"); asd_add_child($$,$3); asd_add_child($$,$5); asd_add_child($$,$7); };
construcao_fluxo: TK_PR_WHILE '(' expressao ')' corpo2 { $$ = asd_new("while"); asd_add_child($$,$3); asd_add_child($$,$5); };

expressao: e7 { $$ = $1; };

e7: e7 '|' e6 { $$ = asd_new("|"); asd_add_child($$, $1); asd_add_child($$, $3); };
e7: e6 { $$ = $1; };

e6: e6 '&' e5 { $$ = asd_new("&"); asd_add_child($$, $1); asd_add_child($$, $3); };
e6: e5 { $$ = $1; };

e5: e5 TK_OC_EQ e4 { $$ = asd_new("=="); asd_add_child($$, $1); asd_add_child($$, $3); };
e5: e5 TK_OC_NE e4 { $$ = asd_new("!="); asd_add_child($$, $1); asd_add_child($$, $3); };
e5: e4 { $$ = $1; };
 
e4: e4 '<' e3 { $$ = asd_new("<"); asd_add_child($$, $1); asd_add_child($$, $3); };
e4: e4 '>' e3 { $$ = asd_new(">"); asd_add_child($$, $1); asd_add_child($$, $3); };
e4: e4 TK_OC_LE e3 { $$ = asd_new("<="); asd_add_child($$, $1); asd_add_child($$, $3); };
e4: e4 TK_OC_GE e3 { $$ = asd_new(">="); asd_add_child($$, $1); asd_add_child($$, $3); };
e4: e3 { $$ = $1; };

e3: e3 '+' e2 { $$ = asd_new("+"); asd_add_child($$, $1); asd_add_child($$, $3); };
e3: e3 '-' e2 { $$ = asd_new("-"); asd_add_child($$, $1); asd_add_child($$, $3); };
e3: e2 { $$ = $1; };
 
e2: e2 '*' e1 { $$ = asd_new("*"); asd_add_child($$, $1); asd_add_child($$, $3); };
e2: e2 '/' e1 { $$ = asd_new("/"); asd_add_child($$, $1); asd_add_child($$, $3); };
e2: e2 '%' e1 { $$ = asd_new("%"); asd_add_child($$, $1); asd_add_child($$, $3); };
e2: e1 { $$ = $1; };
 
e1: '+' e1 { $$ = asd_new("+"); asd_add_child($$, $2); };
e1: '-' e1 { $$ = asd_new("-"); asd_add_child($$, $2); };
e1: '!' e1 { $$ = asd_new("!"); asd_add_child($$, $2); };
e1: e0 { $$ = $1; };

e0: chamada_funcao;
e0: TK_ID { $$ = asd_new($1->lexema); free($1->lexema); free($1); };
e0: TK_LI_INT { $$ = asd_new($1->lexema); free($1->lexema); free($1); }; 
e0: TK_LI_FLOAT { $$ = asd_new($1->lexema); free($1->lexema); free($1); };
e0: '(' expressao ')' { $$ = $2; };
%%

#include <stdio.h>

void yyerror(char const * mensagem)
{
	printf("erro com a mensagem %s na linha %d\n", mensagem, get_line_number());
}

