/* A Bison parser, made by GNU Bison 2.4.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2009, 2010 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */



#include "src/util/c99_stdint.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "src/conf/msg.h"
#include "src/codegen/output.h"
#include "src/ir/compile.h"
#include "src/ir/adfa/adfa.h"
#include "src/ir/regexp/encoding/enc.h"
#include "src/ir/regexp/encoding/range_suffix.h"
#include "src/ir/regexp/regexp.h"
#include "src/ir/skeleton/skeleton.h"
#include "src/parse/extop.h"
#include "src/parse/parser.h"
#include "src/parse/scanner.h"
#include "src/util/free_list.h"
#include "src/util/range.h"

#define YYMALLOC malloc
#define YYFREE free

using namespace re2c;

extern "C" {

int yylex(Scanner &in, context_t&);
void yyerror(Scanner &in, context_t&, const char*);

} // extern "C"

/* Bison version 1.875 emits a definition that is not working
 * with several g++ version. Hence we disable it here.
 */
#if defined(__GNUC__)
#define __attribute__(x)
#endif

static void parse_cleanup(re2c::context_t &context)
{
	RegExp::flist.clear();
	Code::flist.clear();
	Range::vFreeList.clear();
	RangeSuffix::freeList.clear();
	context.clear();
}

static void check_default(const Spec &spec, const std::string &cond)
{
	Spec::const_iterator
		e = spec.end(),
		i = std::find_if(spec.begin(), e, RegExpRule::is_def),
		j = std::find_if(i + 1, e, RegExpRule::is_def);
	if (j != e) {
		error("line %u: code to default rule %sis already defined at line %u",
			(*j)->code->fline, incond(cond).c_str(), (*i)->code->fline);
		exit(1);
	}
}

static void check(const context_t &context, bool cflag)
{
	const SpecMap &specs = context.specMap;
	const SetupMap &setups = context.ruleSetupMap;
	const size_t nspec = specs.size();

	for (SpecMap::const_iterator i = specs.begin(); i != specs.end(); ++i) {
		check_default(i->second, i->first);
	}
	check_default(context.spec_all, "*");

	if (!cflag) {
		static const uint32_t NOL = ~0u;
		uint32_t l = NOL;
		for (SpecMap::const_iterator i = specs.begin(); i != specs.end(); ++i) {
			if (i->first != "") {
				l = std::min(l, i->second[0]->code->fline);
			}
		}
		if (!context.spec_all.empty()) {
			l = std::min(l, context.spec_all[0]->code->fline);
		}
		if (!setups.empty()) {
			l = std::min(l, setups.begin()->second->fline);
		}
		if (context.startup) {
			l = std::min(l, context.startup->fline);
		}
		if (l != NOL) {
			error("line %u: conditions are only allowed with '-c', '--conditions' option", l);
			exit(1);
		}
	} else {
		SpecMap::const_iterator i = specs.find("");
		if (i != specs.end()) {
			error("line %u: non-conditional rules are not allowed with '-c', '--conditions' option",
				i->second[0]->code->fline);
			exit(1);
		}
		for (SetupMap::const_iterator i = setups.begin(); i != setups.end(); ++i) {
			const std::string c = i->first;
			if (c != "*" && specs.find(c) == specs.end()) {
				error("line %u: setup for non existing condition '%s' found",
					i->second->fline, c.c_str());
				exit(1);
			}
		}
		if (setups.size() > nspec) {
			SetupMap::const_iterator i = setups.find("*");
			if (i != setups.end()) {
				error("line %u: setup for all conditions '<!*>' is illegal "
					"if setup for each condition is defined explicitly",
					i->second->fline);
				exit(1);
			}
		}
	}
}

static void delay_default(Spec &spec)
{
	// default rule(s) should go last
	std::stable_partition(spec.begin(), spec.end(), RegExpRule::isnt_def);
}

static void make_rule(context_t &context, RegExpRule *rule, const Code *code)
{
	rule->code = code;
	context.specMap[""].push_back(rule);
}

static void make_cond(context_t &context, CondList *clist,
	RegExpRule *rule, const Code *code)
{
	rule->code = code;
	for(CondList::const_iterator i = clist->begin(); i != clist->end(); ++i) {
		const std::string &cond = *i;
		if (context.specMap.find(cond) == context.specMap.end()) {
			context.condnames.push_back(cond);
		}
		context.specMap[cond].push_back(rule);
	}
	delete clist;
}

static void make_star(context_t &context, RegExpRule *rule, const Code *code)
{
	rule->code = code;
	context.spec_all.push_back(rule);
}

static void make_zero(context_t &context, const Code *code)
{
	if (context.startup) {
		error("line %u: startup code is already defined at line %u",
			code->fline, context.startup->fline);
		exit(1);
	}
	context.startup = code;
}

static void make_setup(Scanner &in, SetupMap &ruleSetupMap,
	CondList *clist, const Code *code)
{
	if (!clist) {
		clist = new CondList;
		clist->insert("*");
	}
	assert(code);
	for (CondList::const_iterator i = clist->begin(); i != clist->end(); ++i) {
		if (ruleSetupMap.find(*i) != ruleSetupMap.end()) {
			in.fatalf_at(code->fline, "code to setup rule '%s' is already defined", i->c_str());
		}
		ruleSetupMap[*i] = code;
	}
	delete clist;
}

static std::string find_setup_rule(const SetupMap &map, const std::string &key)
{
	SetupMap::const_iterator e = map.end(), i;

	i = map.find(key);
	if (i != e) return i->second->text;

	i = map.find("*");
	if (i != e) return i->second->text;

	return "";
}




/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOKEN_CLOSESIZE = 258,
     TOKEN_CODE = 259,
     TOKEN_CONF = 260,
     TOKEN_ID = 261,
     TOKEN_FID = 262,
     TOKEN_FID_END = 263,
     TOKEN_REGEXP = 264
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{


	const re2c::RegExp * regexp;
	re2c::Code * code;
	char op;
	re2c::ExtOp extop;
	std::string * str;
	re2c::CondList * clist;
	re2c::RegExpRule *rule;



} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */



#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   71

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  25
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  16
/* YYNRULES -- Number of rules.  */
#define YYNRULES  42
/* YYNRULES -- Number of states.  */
#define YYNSTATES  72

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   264

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    16,     2,     2,     2,     2,     2,     2,
      23,    24,    15,    21,    18,     2,     2,    10,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    17,    12,
      13,    11,    14,    22,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,    20,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    19,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    13,    17,    21,    24,
      26,    28,    30,    33,    39,    45,    49,    55,    61,    63,
      68,    73,    75,    79,    81,    85,    87,    89,    93,    95,
      99,   101,   104,   106,   109,   112,   114,   117,   119,   121,
     123,   125,   127
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      26,     0,    -1,    -1,    26,     5,    -1,    26,    27,    -1,
      26,    30,    -1,    28,    34,    29,    -1,    28,    34,    10,
      -1,     6,    11,    -1,     7,    -1,    12,    -1,     8,    -1,
      33,     4,    -1,    13,    32,    14,    33,    31,    -1,    13,
      15,    14,    33,    31,    -1,    13,    14,    31,    -1,    13,
      16,    32,    14,     4,    -1,    13,    16,    15,    14,     4,
      -1,     4,    -1,    11,    14,     6,     4,    -1,    17,    11,
      14,     6,    -1,     6,    -1,    32,    18,     6,    -1,    34,
      -1,    34,    10,    34,    -1,    15,    -1,    35,    -1,    34,
      19,    35,    -1,    36,    -1,    35,    20,    36,    -1,    37,
      -1,    36,    37,    -1,    40,    -1,    40,    38,    -1,    40,
       3,    -1,    39,    -1,    38,    39,    -1,    15,    -1,    21,
      -1,    22,    -1,     6,    -1,     9,    -1,    23,    34,    24,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   238,   238,   240,   241,   242,   246,   253,   258,   261,
     265,   265,   268,   272,   276,   280,   284,   288,   293,   295,
     301,   308,   314,   321,   325,   330,   335,   339,   346,   350,
     357,   361,   368,   372,   389,   408,   409,   413,   414,   415,
     419,   429,   433
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOKEN_CLOSESIZE", "TOKEN_CODE",
  "TOKEN_CONF", "TOKEN_ID", "TOKEN_FID", "TOKEN_FID_END", "TOKEN_REGEXP",
  "'/'", "'='", "';'", "'<'", "'>'", "'*'", "'!'", "':'", "','", "'|'",
  "'\\\\'", "'+'", "'?'", "'('", "')'", "$accept", "spec", "def", "name",
  "enddef", "rule", "ccode", "clist", "trailexpr", "expr", "diff", "term",
  "factor", "closes", "close", "primary", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
      47,    61,    59,    60,    62,    42,    33,    58,    44,   124,
      92,    43,    63,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    25,    26,    26,    26,    26,    27,    27,    28,    28,
      29,    29,    30,    30,    30,    30,    30,    30,    31,    31,
      31,    32,    32,    33,    33,    33,    34,    34,    35,    35,
      36,    36,    37,    37,    37,    38,    38,    39,    39,    39,
      40,    40,    40
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     2,     3,     3,     2,     1,
       1,     1,     2,     5,     5,     3,     5,     5,     1,     4,
       4,     1,     3,     1,     3,     1,     1,     3,     1,     3,
       1,     2,     1,     2,     2,     1,     2,     1,     1,     1,
       1,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     3,    40,     9,    41,     0,    25,     0,
       4,     0,     5,     0,    23,    26,    28,    30,    32,     8,
      21,     0,     0,     0,     0,    40,     0,     0,    12,     0,
       0,     0,    31,    34,    37,    38,    39,    33,    35,    18,
       0,     0,    15,     0,     0,     0,     0,     0,    42,    11,
       7,    10,     6,    24,    27,    29,    36,     0,     0,     0,
       0,     0,     0,    22,     0,     0,    14,    17,    16,    13,
      19,    20
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,    10,    11,    52,    12,    42,    24,    13,    14,
      15,    16,    17,    37,    38,    18
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -17
static const yytype_int8 yypact[] =
{
     -17,     1,   -17,   -17,    -8,   -17,   -17,    29,   -17,     9,
     -17,     9,   -17,     7,    27,   -15,     9,   -17,     6,   -17,
     -17,     8,    -1,    32,    35,   -17,    17,    21,   -17,     9,
       9,     9,   -17,   -17,   -17,   -17,   -17,    33,   -17,   -17,
      16,    12,   -17,    11,    28,    38,    11,    44,   -17,   -17,
     -17,   -17,   -17,    40,   -15,     9,   -17,    45,    48,     8,
      59,    60,     8,   -17,    61,    62,   -17,   -17,   -17,   -17,
     -17,   -17
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -17,   -17,   -17,   -17,   -17,   -17,    -2,    43,    15,    -7,
      37,    39,   -16,   -17,    34,   -17
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      32,     2,    26,    19,    27,    31,     3,     4,     5,    33,
       6,    28,    39,    43,     7,    25,     8,    25,     6,    40,
       6,    34,    53,    58,     9,    41,     8,    35,    36,    49,
      57,    50,     9,    51,     9,    20,    30,    29,    20,    32,
      30,    48,    60,    21,    22,    23,    30,    44,    34,    46,
      63,    64,    61,    47,    35,    36,    47,    66,    59,    30,
      69,    62,    65,    67,    68,    70,    45,    54,    71,     0,
      55,    56
};

static const yytype_int8 yycheck[] =
{
      16,     0,     9,    11,    11,    20,     5,     6,     7,     3,
       9,     4,     4,    14,    13,     6,    15,     6,     9,    11,
       9,    15,    29,    11,    23,    17,    15,    21,    22,     8,
      14,    10,    23,    12,    23,     6,    19,    10,     6,    55,
      19,    24,    14,    14,    15,    16,    19,    15,    15,    14,
       6,     6,    14,    18,    21,    22,    18,    59,    43,    19,
      62,    46,    14,     4,     4,     4,    23,    30,     6,    -1,
      31,    37
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    26,     0,     5,     6,     7,     9,    13,    15,    23,
      27,    28,    30,    33,    34,    35,    36,    37,    40,    11,
       6,    14,    15,    16,    32,     6,    34,    34,     4,    10,
      19,    20,    37,     3,    15,    21,    22,    38,    39,     4,
      11,    17,    31,    14,    15,    32,    14,    18,    24,     8,
      10,    12,    29,    34,    35,    36,    39,    14,    11,    33,
      14,    14,    33,     6,     6,    14,    31,     4,     4,    31,
       4,     6
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (in, context, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex (in, context)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, in, context); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, Scanner &in, context_t &context)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, in, context)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    Scanner &in;
    context_t &context;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (in);
  YYUSE (context);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, Scanner &in, context_t &context)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, in, context)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    Scanner &in;
    context_t &context;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, in, context);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, Scanner &in, context_t &context)
#else
static void
yy_reduce_print (yyvsp, yyrule, in, context)
    YYSTYPE *yyvsp;
    int yyrule;
    Scanner &in;
    context_t &context;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       , in, context);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, in, context); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, Scanner &in, context_t &context)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, in, context)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    Scanner &in;
    context_t &context;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (in);
  YYUSE (context);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (Scanner &in, context_t &context);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (Scanner &in, context_t &context)
#else
int
yyparse (in, context)
    Scanner &in;
    context_t &context;
#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 6:

    {
		if (!context.symbol_table.insert(std::make_pair(*(yyvsp[(1) - (3)].str), (yyvsp[(2) - (3)].regexp))).second) {
			in.fatal("sym already defined");
		}
		delete (yyvsp[(1) - (3)].str);
	;}
    break;

  case 7:

    {
		in.fatal("trailing contexts are not allowed in named definitions");
	;}
    break;

  case 8:

    {
		(yyval.str) = (yyvsp[(1) - (2)].str);
	;}
    break;

  case 9:

    {
		(yyval.str) = (yyvsp[(1) - (1)].str);
	;}
    break;

  case 12:

    {
		make_rule(context, (yyvsp[(1) - (2)].rule), (yyvsp[(2) - (2)].code));
	;}
    break;

  case 13:

    {
		make_cond(context, (yyvsp[(2) - (5)].clist), (yyvsp[(4) - (5)].rule), (yyvsp[(5) - (5)].code));
	;}
    break;

  case 14:

    {
		make_star(context, (yyvsp[(4) - (5)].rule), (yyvsp[(5) - (5)].code));
	;}
    break;

  case 15:

    {
		make_zero(context, (yyvsp[(3) - (3)].code));
	;}
    break;

  case 16:

    {
		make_setup(in, context.ruleSetupMap, (yyvsp[(3) - (5)].clist), (yyvsp[(5) - (5)].code));
	;}
    break;

  case 17:

    {
		make_setup(in, context.ruleSetupMap, NULL, (yyvsp[(5) - (5)].code));
	;}
    break;

  case 19:

    {
		(yyval.code) = (yyvsp[(4) - (4)].code);
		(yyval.code)->cond = *(yyvsp[(3) - (4)].str);
		delete (yyvsp[(3) - (4)].str);
	;}
    break;

  case 20:

    {
		(yyval.code) = new Code(in.get_fname(), in.get_cline());
		(yyval.code)->cond = *(yyvsp[(4) - (4)].str);
		delete (yyvsp[(4) - (4)].str);
	;}
    break;

  case 21:

    {
		(yyval.clist) = new CondList;
		(yyval.clist)->insert(*(yyvsp[(1) - (1)].str));
		delete (yyvsp[(1) - (1)].str);
	;}
    break;

  case 22:

    {
		(yyvsp[(1) - (3)].clist)->insert(*(yyvsp[(3) - (3)].str));
		delete (yyvsp[(3) - (3)].str);
		(yyval.clist) = (yyvsp[(1) - (3)].clist);
	;}
    break;

  case 23:

    {
		(yyval.rule) = new RegExpRule((yyvsp[(1) - (1)].regexp), false);
	;}
    break;

  case 24:

    {
		(yyval.rule) = new RegExpRule(RegExp::make_cat((yyvsp[(1) - (3)].regexp),
			RegExp::make_cat(RegExp::make_tag(NULL), (yyvsp[(3) - (3)].regexp))), false);
	;}
    break;

  case 25:

    { /* default rule */
		(yyval.rule) = new RegExpRule(in.mkDefault(), true);
	;}
    break;

  case 26:

    {
			(yyval.regexp) = (yyvsp[(1) - (1)].regexp);
		;}
    break;

  case 27:

    {
			(yyval.regexp) = mkAlt((yyvsp[(1) - (3)].regexp), (yyvsp[(3) - (3)].regexp));
		;}
    break;

  case 28:

    {
			(yyval.regexp) = (yyvsp[(1) - (1)].regexp);
		;}
    break;

  case 29:

    {
			(yyval.regexp) = in.mkDiff((yyvsp[(1) - (3)].regexp), (yyvsp[(3) - (3)].regexp));
		;}
    break;

  case 30:

    {
			(yyval.regexp) = (yyvsp[(1) - (1)].regexp);
		;}
    break;

  case 31:

    {
			(yyval.regexp) = RegExp::make_cat((yyvsp[(1) - (2)].regexp), (yyvsp[(2) - (2)].regexp));
		;}
    break;

  case 32:

    {
			(yyval.regexp) = (yyvsp[(1) - (1)].regexp);
		;}
    break;

  case 33:

    {
			// see note [Kleene star is expressed in terms of plus]
			switch((yyvsp[(2) - (2)].op))
			{
			case '*':
				(yyval.regexp) = RegExp::make_alt(RegExp::make_nil(),
					RegExp::make_iter((yyvsp[(1) - (2)].regexp)));
				break;
			case '+':
				(yyval.regexp) = RegExp::make_iter((yyvsp[(1) - (2)].regexp));
				break;
			case '?':
				(yyval.regexp) = mkAlt((yyvsp[(1) - (2)].regexp), RegExp::make_nil());
				break;
			}
		;}
    break;

  case 34:

    {
			if ((yyvsp[(2) - (2)].extop).max == std::numeric_limits<uint32_t>::max())
			{
				(yyval.regexp) = repeat_from ((yyvsp[(1) - (2)].regexp), (yyvsp[(2) - (2)].extop).min);
			}
			else if ((yyvsp[(2) - (2)].extop).min == (yyvsp[(2) - (2)].extop).max)
			{
				(yyval.regexp) = repeat ((yyvsp[(1) - (2)].regexp), (yyvsp[(2) - (2)].extop).min);
			}
			else
			{
				(yyval.regexp) = repeat_from_to ((yyvsp[(1) - (2)].regexp), (yyvsp[(2) - (2)].extop).min, (yyvsp[(2) - (2)].extop).max);
			}
			(yyval.regexp) = (yyval.regexp) ? (yyval.regexp) : RegExp::make_nil();
		;}
    break;

  case 36:

    { (yyval.op) = ((yyvsp[(1) - (2)].op) == (yyvsp[(2) - (2)].op)) ? (yyvsp[(1) - (2)].op) : '*'; ;}
    break;

  case 37:

    { (yyval.op) = '*'; ;}
    break;

  case 38:

    { (yyval.op) = '+'; ;}
    break;

  case 39:

    { (yyval.op) = '?'; ;}
    break;

  case 40:

    {
			symbol_table_t::iterator i = context.symbol_table.find (* (yyvsp[(1) - (1)].str));
			delete (yyvsp[(1) - (1)].str);
			if (i == context.symbol_table.end ())
			{
				in.fatal("can't find symbol");
			}
			(yyval.regexp) = i->second;
		;}
    break;

  case 41:

    {
			(yyval.regexp) = (yyvsp[(1) - (1)].regexp);
		;}
    break;

  case 42:

    {
			(yyval.regexp) = (yyvsp[(2) - (3)].regexp);
		;}
    break;



      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (in, context, YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (in, context, yymsg);
	  }
	else
	  {
	    yyerror (in, context, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, in, context);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, in, context);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (in, context, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, in, context);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, in, context);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}





extern "C" {

void yyerror(Scanner &in, context_t&, const char* s)
{
	in.fatal(s);
}

int yylex(Scanner &in, context_t&)
{
	return in.scan();
}

} // extern "C"

namespace re2c
{

void parse(Scanner &in, Output & o)
{
	dfa_map_t dfa_map;
	context_t context;
	ScannerState rules_state, curr_state;
	Opt &opts = in.opts;

	o.source.wversion_time ()
		.wline_info (in.get_cline (), in.get_fname ().c_str ());
	if (opts->target == opt_t::SKELETON)
	{
		emit_prolog (o.source);
	}

	Enc encodingOld = opts->encoding;
	for (Scanner::ParseMode mode; (mode = in.echo()) != Scanner::Stop;) {
		o.source.new_block ();
		bool bPrologBrace = false;

		in.save_state(curr_state);
		if (opts->rFlag && mode == Scanner::Rules && dfa_map.size())
		{
			in.fatal("cannot have a second 'rules:re2c' block");
		}
		if (mode == Scanner::Reuse)
		{
			if (dfa_map.empty())
			{
				in.fatal("got 'use:re2c' without 'rules:re2c'");
			}
		}
		else if (mode == Scanner::Rules)
		{
			in.save_state(rules_state);
		}
		else
		{
			dfa_map.clear();
		}
		context.specMap.clear();
		yyparse(in, context);
		if (opts->rFlag && mode == Scanner::Reuse) {
			if (!context.specMap.empty() || opts->encoding != encodingOld) {
				// Re-parse rules
				mode = Scanner::Parse;
				in.restore_state(rules_state);
				in.reuse();
				dfa_map.clear();
				parse_cleanup(context);
				context.specMap.clear();
				yyparse(in, context);

				// Now append potential new rules
				in.restore_state(curr_state);
				mode = Scanner::Parse;
				yyparse(in, context);
			}
			encodingOld = opts->encoding;
		}

		o.source.block().line = in.get_cline();

		// compile regular expressions to automata
		if (mode != Scanner::Reuse) {
			check(context, opts->cFlag);

			// merge <*> rules to all conditions except "0" with lowest priority
			for (SpecMap::iterator it = context.specMap.begin(); it != context.specMap.end(); ++it) {
				it->second.insert(it->second.end(), context.spec_all.begin(), context.spec_all.end());
			}

			// insert "0" condition
			if (context.startup) {
				RegExpRule *zero = new RegExpRule(RegExp::make_nil(), false);
				zero->code = context.startup;
				context.condnames.insert(context.condnames.begin(), "0"); // first
				context.specMap["0"].push_back(zero);
			}

			for (SpecMap::iterator it = context.specMap.begin(); it != context.specMap.end(); ++it) {
				delay_default(it->second);
				const std::string &setup = find_setup_rule(context.ruleSetupMap, it->first);
				dfa_map[it->first] = compile(it->second, o, it->first, setup);
			}
		}

		o.source.block().types = context.condnames;

		// generate code
		if (mode != Scanner::Rules) {
			uint32_t ind = opts->topIndent;
			size_t nCount = dfa_map.size();
			if (dfa_map.find("") != dfa_map.end()) {
				dfa_map[""]->emit(o, ind, !--nCount, bPrologBrace);
			} else {
				std::vector<std::string>::const_iterator
					i = context.condnames.begin(),
					e = context.condnames.end();
				for (; i != e; ++i) {
					dfa_map[*i]->emit(o, ind, !--nCount, bPrologBrace);
				}
			}
		}

		o.source.wline_info (in.get_cline (), in.get_fname ().c_str ());
		/* restore original char handling mode*/
		opts.reset_encoding (encodingOld);
	}

	if (opts->target == opt_t::SKELETON)
	{
		emit_epilog (o.source, o.skeletons);
	}

	parse_cleanup(context);
}

} // end namespace re2c

