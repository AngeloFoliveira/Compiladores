%{
int yylex(void);
void yyerror (char const *mensagem);
int get_line_number(void);
%}

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
%token TK_ID
%token TK_LI_INT
%token TK_LI_FLOAT
%token TK_ER

%define parse.error verbose

%%

programa: lista ';';

lista: elemento;
lista : lista ',' elemento;

elemento: def_func;
elemento: decl_var;

def_func: cabecalho corpo;


cabecalho: TK_ID TK_PR_RETURNS TK_PR_FLOAT;
cabecalho: TK_ID TK_PR_RETURNS TK_PR_FLOAT TK_PR_WITH lista_parametros TK_PR_IS;
cabecalho: TK_ID TK_PR_RETURNS TK_PR_INT;
cabecalho: TK_ID TK_PR_RETURNS TK_PR_INT TK_PR_WITH lista_parametros TK_PR_IS;

lista_parametros: parametro;
lista_parametros: parametro ',' lista_parametros;

parametro: TK_ID TK_PR_AS TK_PR_INT;
parametro: TK_ID TK_PR_AS TK_PR_FLOAT;

corpo: bloco_comandos;

comando_simples: '[' bloco_comandos ']';
comando_simples: '[' ']';
comando_simples: decl_var;
comando_simples: atribuicao;
comando_simples: chamada_funcao;
comando_simples: comando_retorno;
comando_simples: construcao_fluxo;


bloco_comandos: comando_simples;
bloco_comandos: comando_simples bloco_comandos;


decl_var: TK_PR_DECLARE TK_ID TK_PR_AS tipo;
decl_var: TK_PR_DECLARE TK_ID TK_PR_AS tipo TK_PR_WITH literal;

tipo: TK_PR_FLOAT;
tipo: TK_PR_INT;

literal: TK_LI_FLOAT;
literal: TK_LI_INT;

atribuicao: TK_ID TK_PR_IS expressao;


chamada_funcao: TK_ID '(' ')';
chamada_funcao: TK_ID '(' argumentos ')';

argumentos: expressao;
argumentos: expressao ',' argumentos;


comando_retorno: TK_PR_RETURN expressao TK_PR_AS TK_PR_FLOAT;
comando_retorno: TK_PR_RETURN expressao TK_PR_AS TK_PR_INT;


construcao_fluxo: TK_PR_IF '(' expressao ')' bloco_comandos;
construcao_fluxo: TK_PR_IF '(' expressao ')' bloco_comandos TK_PR_ELSE bloco_comandos;
construcao_fluxo: TK_PR_WHILE '(' expressao ')' bloco_comandos;


expressao: e7;

e7: e7 '|' e6;
e7: e6;

e6: e6 '&' e5;
e6: e5;

e5: e5 TK_OC_EQ e4;
e5: e5 TK_OC_NE e4;
e5: e4;

e4: e4 '<' e3;
e4: e4 '>' e3;
e4: e4 TK_OC_LE e3;
e4: e4 TK_OC_GE e3;

e3: e3 '+' e2;
e3: e3 '-' e2;
e3: e2;

e2: e2 '*' e1;
e2: e2 '/' e1;
e2: e2 '%' e1;
e2: e1;

e1: '+' e1;
e1: '-' e1;
e1: '!' e1;
e1: e0;

e0: chamada_funcao;
e0: TK_ID;
e0: TK_LI_INT;
e0: TK_LI_FLOAT;
e0: '(' expressao ')';


%%

#include <stdio.h>

void yyerror(char const * mensagem)
{
	printf("erro com a mensagem %s na linha %d\n", mensagem, get_line_number());
}

