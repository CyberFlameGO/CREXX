/* Generated by re2c */
#line 1 "control_flow_yymarker_yyaccept_ok.re"
// re2c $INPUT -o $OUTPUT 

#line 6 "control_flow_yymarker_yyaccept_ok.c"
{
	YYCTYPE yych;
	unsigned int yyaccept = 0;
	if ((YYLIMIT - YYCURSOR) < 6) YYFILL(6);
	yych = *YYCURSOR++;
	yych = *YYCURSOR;
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	goto yy5;
yy4:
#line 6 "control_flow_yymarker_yyaccept_ok.re"
	{ action3 }
#line 19 "control_flow_yymarker_yyaccept_ok.c"
yy5:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':	goto yy7;
	default:	goto yy6;
	}
yy6:
	yych = *++YYCURSOR;
	goto yy9;
yy7:
	yyaccept = 1;
	yych = *(YYMARKER = ++YYCURSOR);
	goto yy9;
yy8:
#line 5 "control_flow_yymarker_yyaccept_ok.re"
	{ action2 }
#line 36 "control_flow_yymarker_yyaccept_ok.c"
yy9:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':	goto yy11;
	default:	goto yy10;
	}
yy10:
	YYCURSOR = YYMARKER;
	if (yyaccept == 0) {
		goto yy4;
	} else {
		goto yy8;
	}
yy11:
	++YYCURSOR;
#line 4 "control_flow_yymarker_yyaccept_ok.re"
	{ action1 }
#line 54 "control_flow_yymarker_yyaccept_ok.c"
}
#line 8 "control_flow_yymarker_yyaccept_ok.re"
