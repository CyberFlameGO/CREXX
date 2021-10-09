/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i
#include <assert.h>
typedef enum { OK, FAIL } Result;




Result lex(const char *YYCURSOR)
{
    const char *YYMARKER;
    
{
	char yych;
	unsigned int yyaccept = 0;
	yych = *YYCURSOR;
	switch (yych) {
	case '.':	goto yy4;
	case '0':	goto yy5;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy6;
	default:	goto yy2;
	}
yy2:
	++YYCURSOR;
yy3:
	{ return FAIL; }
yy4:
	yych = *++YYCURSOR;
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy9;
	default:	goto yy3;
	}
yy5:
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	switch (yych) {
	case '.':	goto yy9;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy12;
	case 'E':
	case 'e':	goto yy15;
	default:	goto yy3;
	}
yy6:
	yyaccept = 1;
	yych = *(YYMARKER = ++YYCURSOR);
	switch (yych) {
	case '.':	goto yy9;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy6;
	case 'E':
	case 'e':	goto yy15;
	default:	goto yy8;
	}
yy8:
	{ return OK; }
yy9:
	yyaccept = 2;
	yych = *(YYMARKER = ++YYCURSOR);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy9;
	case 'E':
	case 'e':	goto yy15;
	default:	goto yy11;
	}
yy11:
	{ return OK; }
yy12:
	yych = *++YYCURSOR;
	switch (yych) {
	case '.':	goto yy9;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy12;
	case 'E':
	case 'e':	goto yy15;
	default:	goto yy14;
	}
yy14:
	YYCURSOR = YYMARKER;
	switch (yyaccept) {
	case 0:
		goto yy3;
	case 1:
		goto yy8;
	default:
		goto yy11;
	}
yy15:
	yych = *++YYCURSOR;
	switch (yych) {
	case '+':
	case '-':	goto yy16;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy17;
	default:	goto yy14;
	}
yy16:
	yych = *++YYCURSOR;
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy17;
	default:	goto yy14;
	}
yy17:
	yych = *++YYCURSOR;
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy17;
	default:	goto yy11;
	}
}

}

int main()
{
    assert(lex("123") == OK);
    assert(lex("123.4567") == OK);
    return 0;
}