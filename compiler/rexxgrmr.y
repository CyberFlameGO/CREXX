/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */
/* Grammar                      */

%token_type { Token* }

%default_type { ASTNode* }

%include {
/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */
/* Grammar                      */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "compiler.h"
}

%code {
    const char* token_type_name(int type) {
        return yyTokenName[type];
    }
}

%extra_argument { Assembler_Context *context }

%parse_accept { printf("The parser has completed successfully.\n"); }

%syntax_error {
        int i, n;
        fprintf(stderr, "Error line %d - expecting", context->line+1);
        n = sizeof(yyTokenName) / sizeof(yyTokenName[0]);
        for (i = 0; i < n; ++i) {
                int a = yy_find_shift_action((YYCODETYPE)yymajor, (YYCODETYPE)i);
                if (a < YYNSTATE + YYNRULE) {
                        fprintf(stderr," %s", yyTokenName[i]);
                }
        }
        fprintf(stderr, "\n");
}

%parse_failure { fprintf(stderr, "Parse failure\n"); }

%start_symbol program

%wildcard ANYTHING.

/*
// Make TK_EQUAL (for assignments) have higher priority
%nonassoc TK_SYMBOL TK_SYMBOL_STEM TK_SYMBOL_COMPOUND.
%nonassoc TK_EQUAL.
*/

/* Strings */
string(A)        ::= SY_STRING(S). { A = ast_f(STRING,S); }

/* Number */
number(A)        ::= SY_NUMBER(S). { A = ast_f(NUMBER,S); }

/* Symbols */
const_symbol(A)  ::= SY_CONST_SYMBOL(S). { A = ast_f(CONST_SYMBOL, S); }

var_symbol(A)    ::= SY_VAR_SYMBOL(S). { A = ast_f(VAR_SYMBOL, S); }

label(A)         ::= SY_LABEL(S). { A = ast_f(LABEL, S); }
value(A)         ::= var_symbol(B). { A = B; }
value(A)         ::= const_symbol(B). { A = B; }
value(A)         ::= number(B). { A = B; }
value(A)         ::= string(B). { A = B; }

/* Program & Structure */

program(P)       ::= rexx_options(R) instruction_list(I) SY_EOS.
                     {
                        P = ast_ft(PROGRAM_FILE); context->ast = P;
                        add_ast(P,R);
                        add_ast(P,I);
                     }
program(P)       ::= rexx_options(R) SY_EOS.
                     {
                        P = ast_ft(PROGRAM_FILE); context->ast = P;
                        add_ast(P,R);
                     }

ncl(N)              ::= SY_EOC. { N = 0; }
ncl(N)              ::= ANYTHING(T) resync. { N = ast_error("21.1", T); }

resync           ::= SY_EOC.
resync           ::= junk SY_EOC.

junk             ::= junk ANYTHING.
junk             ::= ANYTHING.

instruction_list(I)  ::= labeled_instruction(L).
                         { I = ast_ft(INSTRUCTIONS); add_ast(I,L); }
instruction_list(I)  ::= instruction_list(I1) labeled_instruction(L).
                         { I = I1; add_ast(I,L); }

labeled_instruction(I) ::= group(B). { I = B; }
labeled_instruction(I) ::= single_instruction(B). { I = B; }
labeled_instruction(I) ::= label(B). { I = B; }
labeled_instruction(I) ::= ncl(B). { I = B; }

instruction(I) ::= group(B). { I = B; }
instruction(I) ::= single_instruction(B). { I = B; }

single_instruction(I) ::= assignment(B). { I = B; }
single_instruction(I) ::= keyword_instruction(B). { I = B; }
single_instruction(I) ::= command(B). { I = B; }

assignment(I) ::=  var_symbol(V) OP_EQUAL expression(E) ncl(ER).
    { I = ast_f(ASSIGN, T); add_ast(I,V); add_ast(I,E); add_ast(I,ER);}

assignment(I) ::=  SY_NUMBER(T) OP_EQUAL expression(E) ncl(ER).
    { I = ast_f(ASSIGN, T); add_ast(I,ast_error("31.1", T));
      add_ast(I,E); add_ast(I,ER);}

assignment(I) ::=  SY_CONST_SYMBOL(T) OP_EQUAL expression(E) ncl(ER).
    { I = ast_f(ASSIGN, T); add_ast(I,ast_error("31.2", T));
      add_ast(I,E); add_ast(I,ER);}

keyword_instruction(I) ::= address(K). { I = K; }
keyword_instruction(I) ::= arg(K). { I = K; }
keyword_instruction(I) ::= call(K). { I = K; }
keyword_instruction(I) ::= iterate(K). { I = K; }
keyword_instruction(I) ::= leave(K). { I = K; }
keyword_instruction(I) ::= nop(K). { I = K; }
keyword_instruction(I) ::= parse(K). { I = K; }
keyword_instruction(I) ::= procedure(K). { I = K; }
keyword_instruction(I) ::= pull(K). { I = K; }
keyword_instruction(I) ::= return(K). { I = K; }
keyword_instruction(I) ::= say(K). { I = K; }
keyword_instruction(I) ::= KW_THEN(T) resync. { I = ast_error("8.1", T); }
keyword_instruction(I) ::= KW_ELSE(T) resync. { I = ast_error("8.2", T); }
keyword_instruction(I) ::= KW_WHEN(T) resync. { I = ast_error("9.1", T); }
keyword_instruction(I) ::= KW_OTHERWISE(T) resync. { I = ast_error("9.2", T); }
keyword_instruction(I) ::= KW_END(T) resync. { I = ast_error("10.1", T); }

command(I)             ::= expression(E). { I = ast_ft(ADDRESS); add_ast(I,E); }

group(I) ::= simple_do(K). { I = K; }
group(I) ::= do(K). { I = K; }
group(I) ::= if(K). { I = K; }

/* Language Options (Level B) */
rexx_options(I) ::= KW_REXXLEVEL const_symbol(L) ncl(ER1)
                    KW_REXXOPTION const_symbol(O) ncl(ER2). TODO multiple options
                    {    -> (REXX L options);


/*

## Language Options (Level B)
    rexx_options ::= 'REXXLEVEL' const_symbol(L) end_of_clause
                       'REXXOPTION' options:CONST_SYMBOL* end_of_clause
                        -> (REXX level options);

## Groups

### Simple DO Group
    do ::= 'DO' (ncl / t:. resync -> (ERROR[27.1] t)) i:instruction_list? simple_do_ending
       -> i;

    simple_do_ending ::= 'END' ncl
		 / EOS -> ERROR[14.1]
		 / t:. resync -> (ERROR[35.1] t);

### DO Group
    do ::= 'DO' r:dorep (ncl / t:. resync -> (ERROR[27.1] t)) i:instruction_list? do_ending
       -> (DO r i);

    do_ending ::= 'END' VAR_SYMBOL? ncl
		 / EOS -> ERROR[14.1]
		 / t:. resync -> (ERROR[35.1] t);

    dorep ::= ( a:assignment {? t:dot b:dob f:dof} )
          -> (REPEAT a t? b? f?);
    dot ::= 'TO' e:expression -> (TO e);
    dob ::= 'BY' e:expression -> (BY e);
    dof ::= 'FOR' e:expression -> (FOR e);

### IF Group
    if ::= 'IF' e:expression ncl* (t:then / ((. -> ERROR[18.1]) resync )) f:else?
    -> (IF e t f?);

    then ::= 'THEN' ncl* (i:instruction -> (INSTRUCTIONS i) / eos -> ERROR{14.3] / 'END' -> ERROR[10.5] ) ;

    else ::= 'ELSE' ncl* (i:instruction -> (INSTRUCTIONS i) / eos -> ERROR[14.4] / 'END' -> ERROR[10.6] ) ;

## Instructions

### ADDRESS
    address ::= 'ADDRESS' e:taken_constant c:expression? -> (ADDRESS ENVIRONMENT[e] c);

### Arg
    arg ::= 'ARG' t:template_list?
        -> (PARSE (OPTIONS UPPER?) ARG t?)

### Call
    call ::= 'CALL' (f:taken_constant / ( (. -> ERROR[19.2]) resync) ) e:expression_list?
         -> (CALL CONST_SYMBOL[f] e);
    expression_list ::= expr (',' expr)*;

### Iterate
    iterate ::= 'ITERATE' ( v:VAR_SYMBOL / (. -> ERROR[20.2]) resync) )?
            -> (ITERATE v?)

### Leave
    leave ::= 'LEAVE' ( v:VAR_SYMBOL / (. -> ERROR[20.2]) resync) )?
          -> (LEAVE v?);

### Nop
    nop ::= 'NOP';

### Parse
    parse ::= ('PARSE' (in:parse_type / (. -> ERROR[25.12]) resync)) out:template_list?)
             -> (PARSE OPTIONS in out)

           / ('PARSE' 'op:UPPER' (in:parse_type / (. -> ERROR[25.13]) resync)) out:template_list?)
             -> (PARSE (OPTIONS op) in out);

    parse_type ::= parse_key;
    parse_key ::= 'ARG'->ARG / 'PULL'->PULL;

### Procedure
    procedure ::= LABEL 'PROCEDURE' ncl
                 ( !(EOS / procedure) i:labeled_instruction )*
              -> (PROCEDURE LABEL (INSTRUCTIONS i));


### Pull
    pull ::= 'PULL' t:template_list?
         -> (PARSE (OPTIONS UPPER?) PULL t?);

### Return
    return ::= 'RETURN' e:expression?
           -> (RETURN e?);

### Say
    say ::= 'SAY' e:expression?
        -> (SAY e?);

### Parse Templates
    template_list ::= t:template (',' t:template)*
                  -> (TEMPLATES t+);
    template ::= (trigger / target / ((. -> ERROR[38.1]) resync)+;
    target ::= (VAR_SYMBOL / '.')
           -> TARGET;
    trigger ::= pattern / positional;
    pattern ::= STRING / vrefp
            -> PATTERN;
    vrefp ::= '('
                ( VAR_SYMBOL / ((. -> ERROR[19.7]) resync) )
                ( ')' / ((. -> ERROR[46.1]) resync) );
    positional ::= absolute_positional / relative_positional;
    absolute_positional ::= (NUMBER / '=' position)
                        -> ABS_POS;
    position ::= NUMBER / vrefp / ((. -> ERROR[38.2]) resync);
    relative_positional ::= s:('+' / '-') position
                        -> (REL_POS SIGN[s] position);

## Expressions
    expression ::= expr
               ( (',' -> ERROR[37.1] resync) / (')' -> ERROR[37.2] resync) )? ;

    expr ::= and_expression /
            ( (a:expr op:or_operator b:and_expression)->(op a b) );
    or_operator ::= ('|' / '&&') -> OP_OR ;

    and_expression ::= comparison /
            ( (a:and_expression op:and_operator b:comparison)->(op a b) );
    and_operator ::= ('&') -> OP_AND ;

    comparison ::= concatenation /
            ( (a: comparison op:comparison_operator b:concatenation)->(op a b) );
    comparison_operator ::= (normal_compare / strict_compare) -> OP_COMPARE;
    normal_compare::= '=' / '\\=' / '<>' / '><' / '>' / '<' / '>=' / '<=' / '\\>' / '\\<';
    strict_compare::= '==' / '\\==' / '>>' / '<<' / '>>=' / '<<=' / '\\>>' / '\\<<';

    concatenation ::= addition
                    / ( (a:concatenation &| b:addition)->(OP_CONCAT a b) )
                    / ( (a:concatenation b:addition)->(OP_SCONCAT a b) )
                    / ( (a:concatenation op:concat_operator b:addition)->(op a b) );
    concat_operator ::= '||' -> OP_CONCAT ;

    addition ::= multiplication
                    / ( (a:addition op:additive_operator b:multiplication)->(op a b) );
    additive_operator ::= ('+' / '\-')->OP_ADD;

    multiplication ::= power_expression
                    / (a:multiplication op:multiplicative_operator b:power_expression)->(op a b) );
    multiplicative_operator ::= ('*' / '/' / '//' / '%')->OP_MULT;

    power_expression ::= prefix_expression
                    / ( (a:power_expression op:power_operator b:prefix_expression)->(op a b) );
    power_operator ::= '**' -> OP_POWER ;

    prefix_expression ::= ( (op:prefix_operator a:prefix_expression)->(op a) )
               / term
               / ( ( e:.->(ERROR["35.1"] e) ) resync);
    prefix_operator ::= ('+' / '-' / '\') -> OP_PREFIX ;

    term ::= value
          / function
          / '(' expr ( (e:',' -> (ERROR["37.1"] e) ) resync) / ')'
          / ( ( e:. -> (ERROR["36"] e) ) resync) );

    function ::= (f:taken_constant '(' p:expression_list? (')') -> (FUNCTION[f] p)
              / ((e:. -> (ERROR["36"] e)) resync);

*/

/*
// EXPRESSIONS
term(T)                ::= var_symbol(V). [TK_SYMBOL] { T = V; }
term(T)                ::= TK_CONST(C). { T = ast_f(C); }
term(T)                ::= TK_STRING(S). { T = ast_f(S); }
term(T)                ::= function(F). [TK_SYMBOL] { T = F; }

bracket(B)             ::= term(T). { B = T; }
bracket(B)             ::= TK_BOPEN expression(E) TK_BCLOSE. { B = E; }

prefix_expression(P)   ::= bracket(B). { P = B; }
prefix_expression(P)   ::= TK_PLUS(T) prefix_expression(E). { P = ast_f(T); add_ast(P,E); }
prefix_expression(P)   ::= TK_MINUS(T) prefix_expression(E). { P = ast_f(T); add_ast(P,E); }
prefix_expression(P)   ::= TK_NOT(T) prefix_expression(E). { P = ast_f(T); add_ast(P,E); }

power_expression(P)    ::= prefix_expression(E). { P = E; }
power_expression(P)    ::= power_expression(E1) TK_POWER(T) prefix_expression(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}

multiplication(P)      ::= power_expression(E). { P = E; }
multiplication(P)      ::= multiplication(E1) TK_MULT(T) power_expression(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
multiplication(P)      ::= multiplication(E1) TK_DIV(T) power_expression(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
multiplication(P)      ::= multiplication(E1) TK_IDIV(T) power_expression(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
multiplication(P)      ::= multiplication(E1) TK_REMAIN(T) power_expression(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}

addition(P)            ::= multiplication(E). { P = E; }
addition(P)            ::= addition(E1) TK_PLUS(T) multiplication(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
addition(P)            ::= addition(E1) TK_MINUS(T) multiplication(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}

concatenation(P)       ::= addition(E). { P = E; }
concatenation(P)       ::= concatenation(E1) term(E2).
                           { P = ast_ft(TK_CONCAT); add_ast(P,E1); add_ast(P,E2); }
concatenation(P)       ::= concatenation(E1) TK_CONCAT(T) addition(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}

comparison(P)          ::= concatenation(E). { P = E; }
comparison(P)          ::= comparison(E1) TK_EQUAL(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_NOT_EQUAL(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_GT(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_LT(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_GE(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_LE(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_EQUAL_EQUAL(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_NOT_EQUAL_EQUAL(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_GT_STRICT(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_LT_STRICT(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_GE_STRICT(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_LE_STRICT(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}

or_expression(P)       ::= comparison(E). { P = E; }
or_expression(P)       ::= or_expression(E1) TK_OR(T) comparison(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
or_expression(P)       ::= or_expression(E1) TK_XOR(T) comparison(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}

and_expression(P)      ::= or_expression(E). { P = E; }
and_expression(P)      ::= and_expression(E1) TK_AND(T) or_expression(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}

expression(P)          ::= and_expression(E). { P = E; }

valueexp(P)            ::= TK_VALUE(T) expression(E). { P = ast_f(T); add_ast(P,E); }

expression0(P)         ::= . { P = ast_ft(TK_NULL); }
expression0(P)         ::= expression(E). { P = E; }

expression_list(L)     ::= . { L = ast_ft(TK_LIST); }
expression_list(L)     ::= expression_list1(E). { L = E; }
expression_list1(L)    ::= expression(E). { L = ast_ft(TK_LIST); add_ast(L,E); }
expression_list1(L)    ::= expression_list1(L1) TK_COMMA expression(E). { L = L1; add_ast(L,E); }

// VARIABLE & SYMBOLS
var_symbol(T)          ::= TK_SYMBOL(S). { T = ast_f(S); }
var_symbol(T)          ::= TK_SYMBOL_STEM(S). { T = ast_f(S); }
var_symbol(T)          ::= TK_SYMBOL_COMPOUND(S). { T = ast_f(S); }
var_symbol_list1(L)    ::= var_symbol(V). { L = ast_ft(TK_LIST); add_ast(L,V); }
var_symbol_list1(L)    ::= var_symbol_list1(L1) var_symbol(V). { L = L1; add_ast(L,V); }

variable(V)            ::= vref(R). { V = R; }
variable(V)            ::= var_symbol(S). { V = S; }
vref(V)                ::= TK_BOPEN var_symbol(R) TK_BCLOSE. { V = ast_ft(TK_VREF); add_ast(V,R); }
variable_list(L)       ::= variable(V). { L = ast_ft(TK_LIST); add_ast(L,V); }
variable_list(L)       ::= variable_list(L1) variable(V). { L = L1; add_ast(L,V); }

// FUNCTIONS
function(F)            ::= taken_constant(T) function_parameters(P).
                           { F = ast_ft(TK_FUNCTION); add_ast(F,T); add_ast(F,P); }
taken_constant(T)      ::= TK_SYMBOL(S). { T = ast_f(S); }
taken_constant(T)      ::= TK_SYMBOL_COMPOUND(S). { T = ast_f(S); }
taken_constant(T)      ::= TK_STRING(S). { T = ast_f(S); }
function_parameters(P) ::= TK_BOPEN expression_list(E) TK_BCLOSE. { P = E; }

// TEMPLATES
template_list(L)       ::= . { L = ast_ft(TK_LIST); }
template_list(L)       ::= templates(T). { L = ast_ft(TK_LIST); add_ast(L,T); }
template_list(L)       ::= template_list(L1) TK_COMMA templates(T). { L = L1; add_ast(L,T); }
templates(L)           ::= template(T). { L = ast_ft(TK_LIST); add_ast(L,T); }
templates(L)           ::= templates(L1) template(T). { L = L1; add_ast(L,T); }
template(T)            ::= var_symbol(V). { T = V; }
template(T)            ::= TK_STOP(TK). { T = ast_f(TK); }
template(T)            ::= TK_STRING(TK). { T = ast_f(TK); }
template(T)            ::= vref(V). { T = V; }
template(T)            ::= TK_CONST(TK). { T = ast_f(TK); }
template(T)            ::= TK_EQUAL(TK) position(P). { T = ast_f(TK); add_ast(T,P); }
template(T)            ::= TK_PLUS(TK) position(P). { T = ast_f(TK); add_ast(T,P); }
template(T)            ::= TK_MINUS(TK) position(P). { T = ast_f(TK); add_ast(T,P); }
position(P)            ::= TK_CONST(TK). { P = ast_f(TK); }
position(P)            ::= vref(V). { P = V; }

// INSTRUCTIONS
address(I)             ::= TK_ADDRESS(T). { I = ast_f(T); }
address(I)             ::= TK_ADDRESS(T) taken_constant(A1). { I = ast_f(T); add_ast(I,A1); }
address(I)             ::= TK_ADDRESS(T) taken_constant(A1) expression(A2).
                           { I = ast_f(T); add_ast(I,A1); add_ast(I,A2); }
address(I)             ::= TK_ADDRESS(T) valueexp(P). { I = ast_f(T); add_ast(I,P); }
arg(I)                 ::= TK_ARG(T) template_list(P). { I = ast_f(T); add_ast(I,P); }

call(I)                ::= TK_CALL(T) callon_spec(P). { I = ast_f(T); add_ast(I,P); }
call(I)                ::= TK_CALL(T) taken_constant(P) expression_list(E).
                           { I = ast_f(T); add_ast(I,P); add_ast(I,E); }
callon_spec(I)         ::= TK_ON(T) callable_condition(P). { I = ast_f(T); add_ast(I,P); }
callon_spec(I)         ::= TK_ON(T) callable_condition(P1) TK_NAME taken_constant(P2).
                          { I = ast_f(T); add_ast(I,P1); add_ast(I,P2); }
callon_spec(I)         ::= TK_OFF(T) callable_condition(P). { I = ast_f(T); add_ast(I,P); }
callable_condition(I)  ::= TK_ERROR(T). { I = ast_f(T); }
callable_condition(I)  ::= TK_FAILURE(T). { I = ast_f(T); }
callable_condition(I)  ::= TK_HALT(T). { I = ast_f(T); }

drop(I)                ::= TK_DROP(T) variable_list(P). { I = ast_f(T); add_ast(I,P); }

exit(I)                ::= TK_EXIT(T) expression0(P). { I = ast_f(T); add_ast(I,P); }
interpret(I)           ::= TK_INTERPRET(T) expression(P). { I = ast_f(T); add_ast(I,P); }
iterate(I)             ::= TK_ITERATE(T). { I = ast_f(T); }
iterate(I)             ::= TK_ITERATE(T) var_symbol(P). { I = ast_f(T); add_ast(I,P); }
leave(I)               ::= TK_LEAVE(T). { I = ast_f(T); }
leave(I)               ::= TK_LEAVE(T) var_symbol(P). { I = ast_f(T); add_ast(I,P); }
nop(I)                 ::= TK_NOP(T). { I = ast_f(T); }

numeric(I)             ::= TK_NUMERIC(T) TK_DIGITS(T2) expression0(P).
                           { I = ast_f(T); add_ast(add_ast(I, ast_f(T2)), P); }
numeric(I)             ::= TK_NUMERIC(T) TK_FORM(P). { I = ast_f(T); add_ast(I,ast_f(P)); }
numeric(I)             ::= TK_NUMERIC(T) TK_FORM TK_ENGINEERING(P). { I = ast_f(T); add_ast(I,ast_f(P)); }
numeric(I)             ::= TK_NUMERIC(T) TK_FORM TK_SCIENTIFIC(P). { I = ast_f(T); add_ast(I,ast_f(P)); }
numeric(I)             ::= TK_NUMERIC(T) TK_FORM(T2) valueexp(P).
                           { I = ast_f(T); add_ast(add_ast(I, ast_f(T2)), P); }
numeric(I)             ::= TK_NUMERIC(T) TK_FORM(T2) expression(P).
                           { I = ast_f(T); add_ast(add_ast(I, ast_f(T2)), P); }
numeric(I)             ::= TK_NUMERIC(T) TK_FUZZ(T2) expression0(P).
                           { I = ast_f(T); add_ast(add_ast(I, ast_f(T2)), P); }

options(I)             ::= TK_OPTIONS(T) expression(P). { I = ast_f(T); add_ast(I,P); }

parse(I)               ::= TK_PARSE(T) parse_type(PT) template_list(L).
                           { I = ast_f(T); add_ast(I,PT); add_ast(I,L);}
parse(I)               ::= TK_PARSE(T) TK_UPPER(U) parse_type(PT) template_list(L).
                           { I = ast_f(T); add_ast(I,PT); add_ast(I,L); add_ast(I,ast_f(U)); }
parse_type(I)          ::= TK_ARG(T). { I = ast_f(T); }
parse_type(I)          ::= TK_EXTERNAL(T). { I = ast_f(T); }
parse_type(I)          ::= TK_NUMERIC(T). { I = ast_f(T); }
parse_type(I)          ::= TK_PULL(T). { I = ast_f(T); }
parse_type(I)          ::= TK_SOURCE(T). { I = ast_f(T); }
parse_type(I)          ::= TK_VERSION(T). { I = ast_f(T); }
parse_type(I)          ::= TK_LINEIN(T). { I = ast_f(T); }
parse_type(I)          ::= TK_VALUE(T) expression0(E) TK_WITH.
                           { I = ast_f(T); add_ast(I,E); }
parse_type(I)          ::= TK_VAR(T) var_symbol(V).
                           { I = ast_f(T); add_ast(I,V); }

procedure(I)           ::= TK_PROCEDURE(T). { I = ast_f(T); }
procedure(I)           ::= TK_PROCEDURE(T) TK_EXPOSE(T2) variable_list(P).
                           { I = ast_f(T); add_ast(add_ast(I, ast_f(T2)), P); }
pull(I)                ::= TK_PULL(T) template_list(V). { I = ast_f(T); add_ast(I,V); }
push(I)                ::= TK_PUSH(T) expression0(V). { I = ast_f(T); add_ast(I,V); }
queue(I)               ::= TK_QUEUE(T) expression0(V). { I = ast_f(T); add_ast(I,V); }
return(I)              ::= TK_RETURN(T) expression0(V). { I = ast_f(T); add_ast(I,V); }
say(I)                 ::= TK_SAY(T) expression0(V). { I = ast_f(T); add_ast(I,V); }
signal(I)              ::= TK_SIGNAL(T) valueexp(V). { I = ast_f(T); add_ast(I,V); }
signal(I)              ::= TK_SIGNAL(T) taken_constant(V). { I = ast_f(T); add_ast(I,V); }
signal(I)              ::= TK_SIGNAL(T) TK_ON(T2) condition(P) TK_NAME taken_constant(N).
                           { I = ast_f(T); add_ast(add_ast(I, ast_f(T2)), P); add_ast(I, N); }
signal(I)              ::= TK_SIGNAL(T) TK_ON(T2) condition(P).
                           { I = ast_f(T); add_ast(add_ast(I, ast_f(T2)), P); }
signal(I)              ::= TK_SIGNAL(T) TK_OFF(T2) condition(P).
                           { I = ast_f(T); add_ast(add_ast(I, ast_f(T2)), P); }
condition(I)           ::= callable_condition(C). { I = C; }
condition(I)           ::= TK_NOVALUE(T). { I = ast_f(T); }
condition(I)           ::= TK_SYNTAX(T). { I = ast_f(T); }
trace(I)               ::= TK_TRACE(T) taken_constant(P). { I = ast_f(T); add_ast(I,P); }
trace(I)               ::= TK_TRACE(T) valueexp(P). { I = ast_f(T); add_ast(I,P); }
trace(I)               ::= TK_TRACE(T) TK_ERROR(T2). { I = ast_f(T); add_ast(I,ast_f(T2)); }
trace(I)               ::= TK_TRACE(T) TK_FAILURE(T2). { I = ast_f(T); add_ast(I,ast_f(T2)); }
trace(I)               ::= TK_TRACE(T) TK_OFF(T2). { I = ast_f(T); add_ast(I,ast_f(T2)); }
upper(I)               ::= TK_UPPER(T) var_symbol_list1(P). { I = ast_f(T); add_ast(I,P); }

do(I)                  ::= TK_DO(T) do_rep(A) do_cond(B) delim program_instructions(C) TK_END.
                           { I = ast_f(T); add_ast(I,A); add_ast(I,B); add_ast(I,C); add_ast(I,ast_ft(TK_NULL)); }
do(I)                  ::= TK_DO(T) do_rep(A) do_cond(B) delim program_instructions(C) TK_END var_symbol(D).
                           { I = ast_f(T); add_ast(I,A); add_ast(I,B); add_ast(I,C); add_ast(I,D); }
do_rep(I)              ::= . { I = ast_ft(TK_NULL); }
do_rep(I)              ::= assignment(A) do_cnt(B). { I = ast_ft(TK_REP); add_ast(I,A); add_ast(I,B); }
do_rep(I)              ::= assignment(A). { I = ast_ft(TK_REP); add_ast(I,A); }
do_rep(I)              ::= TK_FOREVER(A). { I = ast_ft(TK_REP); add_ast(I,ast_f(A)); }
do_rep(I)              ::= expression(A). { I = ast_ft(TK_REP); add_ast(I,A); }
cnt(I)                 ::= dob(A). { I = A; }
cnt(I)                 ::= dof(A). { I = A; }
cnt(I)                 ::= dot(A). { I = A; }
do_cnt(I)              ::= cnt(A). { I = ast_ft(TK_LIST); add_ast(I,A); }
do_cnt(I)              ::= do_cnt(I1) cnt(A). { I = I1; add_ast(I,A); }
dot(I)                 ::= TK_TO(T) expression(A). { I = ast_f(T); add_ast(I,A); }
dob(I)                 ::= TK_BY(T) expression(A). { I = ast_f(T); add_ast(I,A); }
dof(I)                 ::= TK_FOR(T) expression(A). { I = ast_f(T); add_ast(I,A); }
do_cond(I)             ::= . { I = ast_ft(TK_NULL); }
do_cond(I)             ::= TK_WHILE(T) expression(A). { I = ast_f(T); add_ast(I,A); }
do_cond(I)             ::= TK_UNTIL(T) expression(A). { I = ast_f(T); add_ast(I,A); }
if(I)                  ::= TK_IF(T) expression(E) delim0
                           TK_THEN ncl0 instruction(S1).
                           { I = ast_f(T); add_ast(I,E); add_ast(I,S1);}
else(I)                ::= TK_ELSE(T) ncl0 instruction(S1).
                           { I = ast_f(T); add_ast(I,S1); }
select(I)              ::= TK_SELECT(T) delim0 when_list(A) TK_END.
                           { I = ast_f(T); add_ast(I,A); add_ast(I,ast_ft(TK_NULL)); }
select(I)              ::= TK_SELECT(T) delim0 when_list(A) otherwise(B) TK_END.
                           { I = ast_f(T); add_ast(I,A); add_ast(I,B); }
when_list(I)           ::= when(A). { I = ast_ft(TK_LIST); add_ast(I,A); }
when_list(I)           ::= when_list(I1) when(A). { I = I1; add_ast(I,A); }
when(I)                ::= TK_WHEN(T) expression(E) delim0
                           TK_THEN ncl0 instruction(S) ncl0.
                           { I = ast_f(T); add_ast(I,E); add_ast(I,S); }
otherwise(I)           ::= TK_OTHERWISE(T) program_instructions(P). { I = ast_f(T); add_ast(I,P); }
command(I)             ::= expression(E). { I = ast_ft(TK_ADDRESS); add_ast(I,E); }

keyword_instruction(I) ::= address(K). { I = K; }
keyword_instruction(I) ::= arg(K). { I = K; }
keyword_instruction(I) ::= call(K). { I = K; }
keyword_instruction(I) ::= drop(K). { I = K; }
keyword_instruction(I) ::= exit(K). { I = K; }
keyword_instruction(I) ::= interpret(K). { I = K; }
keyword_instruction(I) ::= iterate(K). { I = K; }
keyword_instruction(I) ::= leave(K). { I = K; }
keyword_instruction(I) ::= nop(K). { I = K; }
keyword_instruction(I) ::= numeric(K). { I = K; }
keyword_instruction(I) ::= options(K). { I = K; }
keyword_instruction(I) ::= parse(K). { I = K; }
keyword_instruction(I) ::= procedure(K). { I = K; }
keyword_instruction(I) ::= pull(K). { I = K; }
keyword_instruction(I) ::= push(K). { I = K; }
keyword_instruction(I) ::= queue(K). { I = K; }
keyword_instruction(I) ::= return(K). { I = K; }
keyword_instruction(I) ::= say(K). { I = K; }
keyword_instruction(I) ::= signal(K). { I = K; }
keyword_instruction(I) ::= trace(K). { I = K; }
keyword_instruction(I) ::= upper(K). { I = K; }

assignment(I)          ::= var_symbol(V) TK_EQUAL(T) expression(E). [TK_EQUAL]
                           { I = ast_f(T); add_ast(I,V); add_ast(I,E); }
assignment(I)          ::= TK_SYMBOL_COMPOUND(V) TK_EQUAL(T) expression(E). [TK_EQUAL]
                           { I = ast_f(T); add_ast(I,ast_f(V)); add_ast(I,E); }

instruction(I)         ::= do(K). { I = K; }
instruction(I)         ::= if(K). { I = K; }
instruction(I)         ::= else(K). { I = K; }
instruction(I)         ::= select(K). { I = K; }
instruction(I)         ::= assignment(K). { I = K; }
instruction(I)         ::= keyword_instruction(K). { I = K; }
instruction(I)         ::= command(K). { I = K; }
instruction(I)         ::= error. { I = ast_ft(TK_PARSEERROR); }

// PROGRAM STRUCTURE
ncl0                   ::= .
ncl0                   ::= ncl.
ncl                    ::= ncl null_clause.
ncl                    ::= null_clause.
null_clause            ::= label.
null_clause            ::= delim.

label(I)               ::= TK_LABEL(T). { I = ast_f(T); }

delim                  ::= TK_EOC.
delim                  ::= TK_EOL.
delim0                 ::= .
delim0                 ::= delim.


program_instruction(I) ::= label(K). { I = K; }
program_instruction(I) ::= instruction(K) delim. { I = K; }

program_instructions(I)::= program_instruction(A). { I = ast_ft(TK_LIST); add_ast(I,A); }
program_instructions(I)::= delim. { I = ast_ft(TK_LIST); }
program_instructions(I)::= program_instructions(I1) program_instruction(A). { I = I1; add_ast(I,A); }
program_instructions(I)::= program_instructions(I1) delim. { I = I1; }

program(P)             ::= TK_EOF. { P = ast_ft(TK_NULL); context->ast = P; }
program(P)             ::= program_instructions(I) TK_EOF. { P = I;  context->ast = P; }
*/