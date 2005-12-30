/* Generated by re2c */
#line 1 "repeater.re"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define RET(n) return n

int scan(const char *s, int l){
const char *p = s;
const char *q;
#define YYCTYPE         char
#define YYCURSOR        p
#define YYLIMIT         (s+l)
#define YYMARKER        q
#define YYFILL(n)

#line 19 "<stdout>"
{
	YYCTYPE yych;
	unsigned int yyaccept = 0;
	goto yy0;
	++YYCURSOR;
yy0:
	if((YYLIMIT - YYCURSOR) < 7) YYFILL(7);
	yych = *YYCURSOR;
	switch(yych){
	case 0x0A:
	case 'q':	goto yy7;
	case 'A':	goto yy2;
	case 'a':	goto yy4;
	default:	goto yy6;
	}
yy2:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	switch(yych){
	case 0x0A:	goto yy10;
	case 'A':
	case 'a':	goto yy8;
	default:	goto yy3;
	}
yy3:
#line 22 "repeater.re"
{RET(5);}
#line 44 "<stdout>"
yy4:	yyaccept = 1;
	yych = *(YYMARKER = ++YYCURSOR);
	switch(yych){
	case 0x0A:	goto yy10;
	case 'A':
	case 'a':	goto yy8;
	default:	goto yy5;
	}
yy5:
#line 23 "repeater.re"
{RET(0);}
#line 55 "<stdout>"
yy6:	yych = *++YYCURSOR;
	goto yy3;
yy7:	yych = *++YYCURSOR;
	goto yy5;
yy8:	yych = *++YYCURSOR;
	switch(yych){
	case 0x0A:	goto yy13;
	case 'A':
	case 'a':	goto yy12;
	default:	goto yy9;
	}
yy9:	YYCURSOR = YYMARKER;
	switch(yyaccept){
	case 0:	goto yy3;
	case 1:	goto yy5;
	}
yy10:	++YYCURSOR;
	goto yy11;
yy11:
#line 18 "repeater.re"
{RET(1);}
#line 76 "<stdout>"
yy12:	yych = *++YYCURSOR;
	switch(yych){
	case 0x0A:	goto yy13;
	case 'A':
	case 'a':	goto yy15;
	default:	goto yy9;
	}
yy13:	++YYCURSOR;
	goto yy14;
yy14:
#line 19 "repeater.re"
{RET(2);}
#line 88 "<stdout>"
yy15:	yych = *++YYCURSOR;
	switch(yych){
	case 0x0A:	goto yy17;
	case 'A':
	case 'a':	goto yy16;
	default:	goto yy9;
	}
yy16:	yych = *++YYCURSOR;
	switch(yych){
	case 0x0A:	goto yy17;
	case 'A':
	case 'a':	goto yy19;
	default:	goto yy9;
	}
yy17:	++YYCURSOR;
	goto yy18;
yy18:
#line 21 "repeater.re"
{RET(4);}
#line 106 "<stdout>"
yy19:	yych = *++YYCURSOR;
	switch(yych){
	case 0x0A:	goto yy20;
	default:	goto yy23;
	}
yy20:	++YYCURSOR;
	goto yy21;
yy21:
#line 20 "repeater.re"
{RET(3);}
#line 117 "<stdout>"
yy22:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy23;
yy23:	switch(yych){
	case 0x0A:	goto yy17;
	case 'A':
	case 'a':	goto yy22;
	default:	goto yy9;
	}
}
#line 24 "repeater.re"

}

void _do_scan(int exp, const char * str, int len)
{
	int ret = scan(str, len);
	
	printf("%d %s %d\n", exp, exp == ret ? "==" : "!=", ret);
}

#define do_scan(exp, str) _do_scan(exp, str, sizeof(str) - 1)

main()
{
	do_scan(1, "a\n");
	do_scan(2, "aa\n");
	do_scan(2, "aaa\n");
	do_scan(4, "aaaa\n");
	do_scan(0, "q");
	do_scan(0, "a");
	do_scan(1, "A\n");
	do_scan(2, "AA\n");
	do_scan(2, "aAa\n");
	do_scan(4, "AaaA\n");
	do_scan(5, "Q");
	do_scan(4, "AaaAa\n");
	do_scan(3, "AaaAaA\n");
	do_scan(5, "A");
	do_scan(0, "\n");
	do_scan(5, "0");
	do_scan(0, "a");
	do_scan(0, "q");
	do_scan(5, "x");
}
