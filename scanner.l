%{
#include "parser.tab.h"
#include <stdio.h>
#include <string.h>
#include "asd.h"

#define IDENTIFICADOR 0
#define LITERAL 1
int linha = 1;
%}
%option yylineno
%option noyywrap


%%

as return TK_PR_AS;
declare return TK_PR_DECLARE;
else return TK_PR_ELSE;
float return TK_PR_FLOAT;
if return TK_PR_IF;
int return TK_PR_INT;
is return TK_PR_IS;
return return TK_PR_RETURN;
returns return TK_PR_RETURNS;
while return TK_PR_WHILE;
with return TK_PR_WITH;

[\!\*\/\%\+\-\<\>\[\]\(\)\=\,\;\&\|] return yytext[0];


\<= return TK_OC_LE;
>= return TK_OC_GE;
== return TK_OC_EQ;
!= return TK_OC_NE;

[a-z_][a-z0-9_]* {
	valor_t *aux = NULL;
	aux = (valor_t*) malloc (sizeof(valor_t));
	aux->lexema = strdup(yytext);
	aux->linha = get_line_number();
	aux->tipo_token = IDENTIFICADOR;
	yylval.valor_lexico = aux;
	return TK_ID;}
[0-9]+ {
	valor_t *aux = NULL;
	aux = (valor_t*) malloc (sizeof(valor_t));
	aux->lexema = strdup(yytext);
	aux->linha = get_line_number();
	aux->tipo_token = LITERAL;
	yylval.valor_lexico = aux;
	return TK_LI_INT;}
[0-9]*\.[0-9]+ {
	valor_t *aux = NULL;
	aux = (valor_t*) malloc (sizeof(valor_t));
	aux->lexema = strdup(yytext);
	aux->linha = get_line_number();
	aux->tipo_token = LITERAL;
	yylval.valor_lexico = aux;	
	return TK_LI_FLOAT;}


\n linha++;

"//".*

[ \s\t]

. return TK_ER;

%%
//PREENCHE AQUI EM CIMA

int get_line_number(void){
 return linha;
}
