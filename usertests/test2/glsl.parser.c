/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         GLSL_STYPE
#define YYLTYPE         GLSL_LTYPE
/* Substitute the variable and function names.  */
#define yyparse         glsl_parse
#define yylex           glsl_lex
#define yyerror         glsl_error
#define yydebug         glsl_debug
#define yynerrs         glsl_nerrs

/* First part of user prologue.  */
#line 1 "glsl.y"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include "glsl_parser.h" //For context struct
#include "glsl.parser.h" //For GLSL_STYPE and GLSL_LTYPE
#include "glsl.lexer.h" //For glsl_lex()

static void glsl_error(GLSL_LTYPE *loc, struct glsl_parse_context *c, const char *s);

#define GLSL_STACK_BUFFER_SIZE (1024*1024)
#define GLSL_STACK_BUFFER_PAYLOAD_SIZE (GLSL_STACK_BUFFER_SIZE - sizeof(intptr_t))

uint8_t *glsl_parse_alloc(struct glsl_parse_context *context, size_t size, int align)
{
	uint8_t *ret;

	if (size + align > (context->cur_buffer_end - context->cur_buffer)) {
		uint8_t *next_buffer = (uint8_t *)malloc(GLSL_STACK_BUFFER_SIZE);
		if (context->cur_buffer) {
			uint8_t **pnext = (uint8_t **)context->cur_buffer_end;
			*pnext = next_buffer;
		}
		context->cur_buffer_start = next_buffer;
		context->cur_buffer = next_buffer;
		context->cur_buffer_end = next_buffer + GLSL_STACK_BUFFER_PAYLOAD_SIZE;
		if (!context->first_buffer) {
			context->first_buffer = context->cur_buffer;
		}
		*((uint8_t **)context->cur_buffer_end) = NULL;
	}

	ret = context->cur_buffer;

	uint8_t *trunc = (uint8_t *)((~((intptr_t)align - 1)) & ((intptr_t)ret));
	if (trunc != ret) {
		ret = trunc + align;
	}
	context->cur_buffer = ret + size;
	return ret;
}

void glsl_parse_dealloc(struct glsl_parse_context *context)
{
	uint8_t *buffer = context->first_buffer;
	while (buffer) {
		uint8_t *next = *((uint8_t **)(buffer + GLSL_STACK_BUFFER_PAYLOAD_SIZE));
		free(buffer);
		buffer = next;
	}
}

static char *glsl_parse_strdup(struct glsl_parse_context *context, const char *c)
{
	int len = strlen(c);
	char *ret = (char *)glsl_parse_alloc(context, len + 1, 1);
	strcpy(ret, c);
	return ret;
}

struct glsl_node *new_glsl_node(struct glsl_parse_context *context, int code, ...)
{
	struct glsl_node *temp;
	int i;
	int n = 0;
	va_list vl;
	va_start(vl, code);
	while (1) {
		temp = va_arg(vl, struct glsl_node *);
		if (temp)
			n++;
		else
			break;
	}
	va_end(vl);
	struct glsl_node *g = (struct glsl_node *)glsl_parse_alloc(context, offsetof(struct glsl_node, children[n]), 8);
	g->code = code;
	g->child_count = n;
	va_start(vl, code);
	for (i = 0; i < n; i++) {
		temp = va_arg(vl, struct glsl_node *);
		g->children[i] = temp;
	}
	va_end(vl);
	return g;
}

static struct glsl_node *new_glsl_identifier(struct glsl_parse_context *context, const char *str)
{
	struct glsl_node *n = new_glsl_node(context, IDENTIFIER, NULL);
	if (!str)
		n->data.str = NULL;
	else
		n->data.str = glsl_parse_strdup(context, str);
	return n;
}

static struct glsl_node *new_glsl_string(struct glsl_parse_context *context, int code, const char *str)
{
	struct glsl_node *n = new_glsl_node(context, code, NULL);
	n->data.str = glsl_parse_strdup(context, str);
	return n;
}

#define scanner context->scanner //To allow the scanner to find it's context


#line 190 "glsl.parser.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_GLSL_GLSL_PARSER_H_INCLUDED
# define YY_GLSL_GLSL_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef GLSL_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define GLSL_DEBUG 1
#  else
#   define GLSL_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define GLSL_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined GLSL_DEBUG */
#if GLSL_DEBUG
extern int glsl_debug;
#endif

/* Token type.  */
#ifndef GLSL_TOKENTYPE
# define GLSL_TOKENTYPE
  enum glsl_tokentype
  {
    CONST = 258,
    BOOL = 259,
    FLOAT = 260,
    DOUBLE = 261,
    INT = 262,
    UINT = 263,
    BREAK = 264,
    CONTINUE = 265,
    DO = 266,
    ELSE = 267,
    FOR = 268,
    IF = 269,
    DISCARD = 270,
    RETURN = 271,
    RETURN_VALUE = 272,
    SWITCH = 273,
    CASE = 274,
    DEFAULT = 275,
    SUBROUTINE = 276,
    BVEC2 = 277,
    BVEC3 = 278,
    BVEC4 = 279,
    IVEC2 = 280,
    IVEC3 = 281,
    IVEC4 = 282,
    UVEC2 = 283,
    UVEC3 = 284,
    UVEC4 = 285,
    VEC2 = 286,
    VEC3 = 287,
    VEC4 = 288,
    MAT2 = 289,
    MAT3 = 290,
    MAT4 = 291,
    CENTROID = 292,
    IN = 293,
    OUT = 294,
    INOUT = 295,
    UNIFORM = 296,
    PATCH = 297,
    SAMPLE = 298,
    BUFFER = 299,
    SHARED = 300,
    COHERENT = 301,
    VOLATILE = 302,
    RESTRICT = 303,
    READONLY = 304,
    WRITEONLY = 305,
    DVEC2 = 306,
    DVEC3 = 307,
    DVEC4 = 308,
    DMAT2 = 309,
    DMAT3 = 310,
    DMAT4 = 311,
    NOPERSPECTIVE = 312,
    FLAT = 313,
    SMOOTH = 314,
    LAYOUT = 315,
    MAT2X2 = 316,
    MAT2X3 = 317,
    MAT2X4 = 318,
    MAT3X2 = 319,
    MAT3X3 = 320,
    MAT3X4 = 321,
    MAT4X2 = 322,
    MAT4X3 = 323,
    MAT4X4 = 324,
    DMAT2X2 = 325,
    DMAT2X3 = 326,
    DMAT2X4 = 327,
    DMAT3X2 = 328,
    DMAT3X3 = 329,
    DMAT3X4 = 330,
    DMAT4X2 = 331,
    DMAT4X3 = 332,
    DMAT4X4 = 333,
    ATOMIC_UINT = 334,
    SAMPLER1D = 335,
    SAMPLER2D = 336,
    SAMPLER3D = 337,
    SAMPLERCUBE = 338,
    SAMPLER1DSHADOW = 339,
    SAMPLER2DSHADOW = 340,
    SAMPLERCUBESHADOW = 341,
    SAMPLER1DARRAY = 342,
    SAMPLER2DARRAY = 343,
    SAMPLER1DARRAYSHADOW = 344,
    SAMPLER2DARRAYSHADOW = 345,
    ISAMPLER1D = 346,
    ISAMPLER2D = 347,
    ISAMPLER3D = 348,
    ISAMPLERCUBE = 349,
    ISAMPLER1DARRAY = 350,
    ISAMPLER2DARRAY = 351,
    USAMPLER1D = 352,
    USAMPLER2D = 353,
    USAMPLER3D = 354,
    USAMPLERCUBE = 355,
    USAMPLER1DARRAY = 356,
    USAMPLER2DARRAY = 357,
    SAMPLER2DRECT = 358,
    SAMPLER2DRECTSHADOW = 359,
    ISAMPLER2DRECT = 360,
    USAMPLER2DRECT = 361,
    SAMPLERBUFFER = 362,
    ISAMPLERBUFFER = 363,
    USAMPLERBUFFER = 364,
    SAMPLERCUBEARRAY = 365,
    SAMPLERCUBEARRAYSHADOW = 366,
    ISAMPLERCUBEARRAY = 367,
    USAMPLERCUBEARRAY = 368,
    SAMPLER2DMS = 369,
    ISAMPLER2DMS = 370,
    USAMPLER2DMS = 371,
    SAMPLER2DMSARRAY = 372,
    ISAMPLER2DMSARRAY = 373,
    USAMPLER2DMSARRAY = 374,
    IMAGE1D = 375,
    IIMAGE1D = 376,
    UIMAGE1D = 377,
    IMAGE2D = 378,
    IIMAGE2D = 379,
    UIMAGE2D = 380,
    IMAGE3D = 381,
    IIMAGE3D = 382,
    UIMAGE3D = 383,
    IMAGE2DRECT = 384,
    IIMAGE2DRECT = 385,
    UIMAGE2DRECT = 386,
    IMAGECUBE = 387,
    IIMAGECUBE = 388,
    UIMAGECUBE = 389,
    IMAGEBUFFER = 390,
    IIMAGEBUFFER = 391,
    UIMAGEBUFFER = 392,
    IMAGE1DARRAY = 393,
    IIMAGE1DARRAY = 394,
    UIMAGE1DARRAY = 395,
    IMAGE2DARRAY = 396,
    IIMAGE2DARRAY = 397,
    UIMAGE2DARRAY = 398,
    IMAGECUBEARRAY = 399,
    IIMAGECUBEARRAY = 400,
    UIMAGECUBEARRAY = 401,
    IMAGE2DMS = 402,
    IIMAGE2DMS = 403,
    UIMAGE2DMS = 404,
    IMAGE2DMSARRAY = 405,
    IIMAGE2DMSARRAY = 406,
    UIMAGE2DMSARRAY = 407,
    STRUCT = 408,
    VOID = 409,
    WHILE = 410,
    IDENTIFIER = 411,
    FLOATCONSTANT = 412,
    DOUBLECONSTANT = 413,
    INTCONSTANT = 414,
    UINTCONSTANT = 415,
    TRUE_VALUE = 416,
    FALSE_VALUE = 417,
    LEFT_OP = 418,
    RIGHT_OP = 419,
    INC_OP = 420,
    DEC_OP = 421,
    LE_OP = 422,
    GE_OP = 423,
    EQ_OP = 424,
    NE_OP = 425,
    AND_OP = 426,
    OR_OP = 427,
    XOR_OP = 428,
    MUL_ASSIGN = 429,
    DIV_ASSIGN = 430,
    ADD_ASSIGN = 431,
    MOD_ASSIGN = 432,
    LEFT_ASSIGN = 433,
    RIGHT_ASSIGN = 434,
    AND_ASSIGN = 435,
    XOR_ASSIGN = 436,
    OR_ASSIGN = 437,
    SUB_ASSIGN = 438,
    LEFT_PAREN = 439,
    RIGHT_PAREN = 440,
    LEFT_BRACKET = 441,
    RIGHT_BRACKET = 442,
    LEFT_BRACE = 443,
    RIGHT_BRACE = 444,
    DOT = 445,
    COMMA = 446,
    COLON = 447,
    EQUAL = 448,
    SEMICOLON = 449,
    BANG = 450,
    DASH = 451,
    TILDE = 452,
    PLUS = 453,
    STAR = 454,
    SLASH = 455,
    PERCENT = 456,
    LEFT_ANGLE = 457,
    RIGHT_ANGLE = 458,
    VERTICAL_BAR = 459,
    CARET = 460,
    AMPERSAND = 461,
    QUESTION = 462,
    INVARIANT = 463,
    PRECISE = 464,
    HIGHP = 465,
    MEDIUMP = 466,
    LOWP = 467,
    PRECISION = 468,
    AT = 469,
    UNARY_PLUS = 470,
    UNARY_DASH = 471,
    PRE_INC_OP = 472,
    PRE_DEC_OP = 473,
    POST_DEC_OP = 474,
    POST_INC_OP = 475,
    ARRAY_REF_OP = 476,
    FUNCTION_CALL = 477,
    TYPE_NAME_LIST = 478,
    TYPE_SPECIFIER = 479,
    POSTFIX_EXPRESSION = 480,
    TYPE_QUALIFIER_LIST = 481,
    STRUCT_DECLARATION = 482,
    STRUCT_DECLARATOR = 483,
    STRUCT_SPECIFIER = 484,
    FUNCTION_DEFINITION = 485,
    DECLARATION = 486,
    STATEMENT_LIST = 487,
    TRANSLATION_UNIT = 488,
    PRECISION_DECLARATION = 489,
    BLOCK_DECLARATION = 490,
    TYPE_QUALIFIER_DECLARATION = 491,
    IDENTIFIER_LIST = 492,
    INIT_DECLARATOR_LIST = 493,
    FULLY_SPECIFIED_TYPE = 494,
    SINGLE_DECLARATION = 495,
    SINGLE_INIT_DECLARATION = 496,
    INITIALIZER_LIST = 497,
    EXPRESSION_STATEMENT = 498,
    SELECTION_STATEMENT = 499,
    SELECTION_STATEMENT_ELSE = 500,
    SWITCH_STATEMENT = 501,
    FOR_REST_STATEMENT = 502,
    WHILE_STATEMENT = 503,
    DO_STATEMENT = 504,
    FOR_STATEMENT = 505,
    CASE_LABEL = 506,
    CONDITION_OPT = 507,
    ASSIGNMENT_CONDITION = 508,
    EXPRESSION_CONDITION = 509,
    FUNCTION_HEADER = 510,
    FUNCTION_DECLARATION = 511,
    FUNCTION_PARAMETER_LIST = 512,
    PARAMETER_DECLARATION = 513,
    PARAMETER_DECLARATOR = 514,
    UNINITIALIZED_DECLARATION = 515,
    ARRAY_SPECIFIER = 516,
    ARRAY_SPECIFIER_LIST = 517,
    STRUCT_DECLARATOR_LIST = 518,
    FUNCTION_CALL_PARAMETER_LIST = 519,
    STRUCT_DECLARATION_LIST = 520,
    LAYOUT_QUALIFIER_ID = 521,
    LAYOUT_QUALIFIER_ID_LIST = 522,
    SUBROUTINE_TYPE = 523,
    PAREN_EXPRESSION = 524,
    INIT_DECLARATOR = 525,
    INITIALIZER = 526,
    TERNARY_EXPRESSION = 527,
    FIELD_IDENTIFIER = 528,
    NUM_GLSL_TOKEN = 529
  };
#endif

/* Value type.  */
#if ! defined GLSL_STYPE && ! defined GLSL_STYPE_IS_DECLARED
union GLSL_STYPE
{

  /* IDENTIFIER  */
  char * IDENTIFIER;
  /* DOUBLECONSTANT  */
  double DOUBLECONSTANT;
  /* FLOATCONSTANT  */
  float FLOATCONSTANT;
  /* INTCONSTANT  */
  int INTCONSTANT;
  /* assignment_operator  */
  int assignment_operator;
  /* unary_operator  */
  int unary_operator;
  /* translation_unit  */
  struct glsl_node * translation_unit;
  /* block_identifier  */
  struct glsl_node * block_identifier;
  /* decl_identifier  */
  struct glsl_node * decl_identifier;
  /* struct_name  */
  struct glsl_node * struct_name;
  /* type_name  */
  struct glsl_node * type_name;
  /* param_name  */
  struct glsl_node * param_name;
  /* function_name  */
  struct glsl_node * function_name;
  /* field_identifier  */
  struct glsl_node * field_identifier;
  /* variable_identifier  */
  struct glsl_node * variable_identifier;
  /* layout_identifier  */
  struct glsl_node * layout_identifier;
  /* type_specifier_identifier  */
  struct glsl_node * type_specifier_identifier;
  /* external_declaration  */
  struct glsl_node * external_declaration;
  /* function_definition  */
  struct glsl_node * function_definition;
  /* compound_statement_no_new_scope  */
  struct glsl_node * compound_statement_no_new_scope;
  /* statement  */
  struct glsl_node * statement;
  /* statement_list  */
  struct glsl_node * statement_list;
  /* compound_statement  */
  struct glsl_node * compound_statement;
  /* simple_statement  */
  struct glsl_node * simple_statement;
  /* declaration  */
  struct glsl_node * declaration;
  /* identifier_list  */
  struct glsl_node * identifier_list;
  /* init_declarator_list  */
  struct glsl_node * init_declarator_list;
  /* single_declaration  */
  struct glsl_node * single_declaration;
  /* initializer  */
  struct glsl_node * initializer;
  /* initializer_list  */
  struct glsl_node * initializer_list;
  /* expression_statement  */
  struct glsl_node * expression_statement;
  /* selection_statement  */
  struct glsl_node * selection_statement;
  /* switch_statement  */
  struct glsl_node * switch_statement;
  /* switch_statement_list  */
  struct glsl_node * switch_statement_list;
  /* case_label  */
  struct glsl_node * case_label;
  /* iteration_statement  */
  struct glsl_node * iteration_statement;
  /* statement_no_new_scope  */
  struct glsl_node * statement_no_new_scope;
  /* for_init_statement  */
  struct glsl_node * for_init_statement;
  /* conditionopt  */
  struct glsl_node * conditionopt;
  /* condition  */
  struct glsl_node * condition;
  /* for_rest_statement  */
  struct glsl_node * for_rest_statement;
  /* jump_statement  */
  struct glsl_node * jump_statement;
  /* function_prototype  */
  struct glsl_node * function_prototype;
  /* function_declarator  */
  struct glsl_node * function_declarator;
  /* function_parameter_list  */
  struct glsl_node * function_parameter_list;
  /* parameter_declaration  */
  struct glsl_node * parameter_declaration;
  /* parameter_declarator  */
  struct glsl_node * parameter_declarator;
  /* function_header  */
  struct glsl_node * function_header;
  /* fully_specified_type  */
  struct glsl_node * fully_specified_type;
  /* parameter_type_specifier  */
  struct glsl_node * parameter_type_specifier;
  /* type_specifier  */
  struct glsl_node * type_specifier;
  /* array_specifier_list  */
  struct glsl_node * array_specifier_list;
  /* array_specifier  */
  struct glsl_node * array_specifier;
  /* type_specifier_nonarray  */
  struct glsl_node * type_specifier_nonarray;
  /* struct_specifier  */
  struct glsl_node * struct_specifier;
  /* struct_declaration_list  */
  struct glsl_node * struct_declaration_list;
  /* struct_declaration  */
  struct glsl_node * struct_declaration;
  /* struct_declarator_list  */
  struct glsl_node * struct_declarator_list;
  /* struct_declarator  */
  struct glsl_node * struct_declarator;
  /* type_qualifier  */
  struct glsl_node * type_qualifier;
  /* single_type_qualifier  */
  struct glsl_node * single_type_qualifier;
  /* layout_qualifier  */
  struct glsl_node * layout_qualifier;
  /* layout_qualifier_id_list  */
  struct glsl_node * layout_qualifier_id_list;
  /* layout_qualifier_id  */
  struct glsl_node * layout_qualifier_id;
  /* precision_qualifier  */
  struct glsl_node * precision_qualifier;
  /* interpolation_qualifier  */
  struct glsl_node * interpolation_qualifier;
  /* invariant_qualifier  */
  struct glsl_node * invariant_qualifier;
  /* precise_qualifier  */
  struct glsl_node * precise_qualifier;
  /* storage_qualifier  */
  struct glsl_node * storage_qualifier;
  /* type_name_list  */
  struct glsl_node * type_name_list;
  /* expression  */
  struct glsl_node * expression;
  /* assignment_expression  */
  struct glsl_node * assignment_expression;
  /* constant_expression  */
  struct glsl_node * constant_expression;
  /* conditional_expression  */
  struct glsl_node * conditional_expression;
  /* logical_or_expression  */
  struct glsl_node * logical_or_expression;
  /* logical_xor_expression  */
  struct glsl_node * logical_xor_expression;
  /* logical_and_expression  */
  struct glsl_node * logical_and_expression;
  /* inclusive_or_expression  */
  struct glsl_node * inclusive_or_expression;
  /* exclusive_or_expression  */
  struct glsl_node * exclusive_or_expression;
  /* and_expression  */
  struct glsl_node * and_expression;
  /* equality_expression  */
  struct glsl_node * equality_expression;
  /* relational_expression  */
  struct glsl_node * relational_expression;
  /* shift_expression  */
  struct glsl_node * shift_expression;
  /* additive_expression  */
  struct glsl_node * additive_expression;
  /* multiplicative_expression  */
  struct glsl_node * multiplicative_expression;
  /* unary_expression  */
  struct glsl_node * unary_expression;
  /* postfix_expression  */
  struct glsl_node * postfix_expression;
  /* integer_expression  */
  struct glsl_node * integer_expression;
  /* function_call  */
  struct glsl_node * function_call;
  /* function_call_or_method  */
  struct glsl_node * function_call_or_method;
  /* function_call_generic  */
  struct glsl_node * function_call_generic;
  /* function_call_parameter_list  */
  struct glsl_node * function_call_parameter_list;
  /* function_identifier  */
  struct glsl_node * function_identifier;
  /* primary_expression  */
  struct glsl_node * primary_expression;
  /* UINTCONSTANT  */
  unsigned int UINTCONSTANT;
#line 716 "glsl.parser.c"

};
typedef union GLSL_STYPE GLSL_STYPE;
# define GLSL_STYPE_IS_TRIVIAL 1
# define GLSL_STYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined GLSL_LTYPE && ! defined GLSL_LTYPE_IS_DECLARED
typedef struct GLSL_LTYPE GLSL_LTYPE;
struct GLSL_LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define GLSL_LTYPE_IS_DECLARED 1
# define GLSL_LTYPE_IS_TRIVIAL 1
#endif



int glsl_parse (struct glsl_parse_context * context);

#endif /* !YY_GLSL_GLSL_PARSER_H_INCLUDED  */



#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))

/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined GLSL_LTYPE_IS_TRIVIAL && GLSL_LTYPE_IS_TRIVIAL \
             && defined GLSL_STYPE_IS_TRIVIAL && GLSL_STYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  177
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4503

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  275
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  92
/* YYNRULES -- Number of rules.  */
#define YYNRULES  360
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  492

#define YYUNDEFTOK  2
#define YYMAXUTOK   529


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274
};

#if GLSL_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   503,   503,   504,   507,   510,   514,   517,   520,   523,
     526,   529,   532,   535,   538,   541,   544,   545,   548,   553,
     560,   561,   564,   565,   568,   569,   572,   573,   576,   577,
     578,   579,   580,   581,   582,   585,   586,   587,   594,   604,
     614,   624,   631,   639,   649,   650,   654,   655,   663,   671,
     680,   691,   698,   705,   708,   711,   720,   721,   722,   725,
     727,   731,   732,   735,   738,   742,   746,   747,   750,   751,
     754,   757,   760,   764,   765,   768,   769,   772,   773,   776,
     779,   783,   786,   790,   793,   796,   799,   802,   806,   809,
     815,   822,   825,   829,   832,   838,   841,   848,   851,   855,
     859,   865,   869,   873,   879,   883,   886,   890,   893,   897,
     898,   899,   900,   901,   902,   903,   904,   905,   906,   907,
     908,   909,   910,   911,   912,   913,   914,   915,   916,   917,
     918,   919,   920,   921,   922,   923,   924,   925,   926,   927,
     928,   929,   930,   931,   932,   933,   934,   935,   936,   937,
     938,   939,   940,   941,   942,   943,   944,   945,   946,   947,
     948,   949,   950,   951,   952,   953,   954,   955,   956,   957,
     958,   959,   960,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,   982,   983,   984,   985,   986,   987,
     988,   989,   990,   991,   992,   993,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1020,  1023,  1030,  1032,  1036,  1043,  1047,  1050,  1054,  1057,
    1061,  1063,  1067,  1068,  1069,  1070,  1071,  1072,  1075,  1078,
    1080,  1084,  1087,  1090,  1094,  1095,  1096,  1099,  1100,  1101,
    1104,  1107,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1132,
    1133,  1137,  1138,  1142,  1143,  1147,  1148,  1149,  1150,  1151,
    1152,  1153,  1154,  1155,  1156,  1157,  1160,  1163,  1164,  1168,
    1169,  1173,  1174,  1178,  1179,  1183,  1184,  1188,  1189,  1193,
    1194,  1198,  1200,  1203,  1207,  1209,  1212,  1215,  1218,  1222,
    1224,  1227,  1231,  1233,  1236,  1240,  1242,  1245,  1248,  1252,
    1254,  1257,  1260,  1264,  1265,  1266,  1267,  1270,  1272,  1275,
    1277,  1280,  1283,  1287,  1290,  1293,  1296,  1299,  1305,  1312,
    1315,  1319,  1321,  1325,  1327,  1330,  1333,  1336,  1339,  1342,
    1345
};
#endif

#if GLSL_DEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "CONST", "BOOL", "FLOAT", "DOUBLE",
  "INT", "UINT", "BREAK", "CONTINUE", "DO", "ELSE", "FOR", "IF", "DISCARD",
  "RETURN", "RETURN_VALUE", "SWITCH", "CASE", "DEFAULT", "SUBROUTINE",
  "BVEC2", "BVEC3", "BVEC4", "IVEC2", "IVEC3", "IVEC4", "UVEC2", "UVEC3",
  "UVEC4", "VEC2", "VEC3", "VEC4", "MAT2", "MAT3", "MAT4", "CENTROID",
  "IN", "OUT", "INOUT", "UNIFORM", "PATCH", "SAMPLE", "BUFFER", "SHARED",
  "COHERENT", "VOLATILE", "RESTRICT", "READONLY", "WRITEONLY", "DVEC2",
  "DVEC3", "DVEC4", "DMAT2", "DMAT3", "DMAT4", "NOPERSPECTIVE", "FLAT",
  "SMOOTH", "LAYOUT", "MAT2X2", "MAT2X3", "MAT2X4", "MAT3X2", "MAT3X3",
  "MAT3X4", "MAT4X2", "MAT4X3", "MAT4X4", "DMAT2X2", "DMAT2X3", "DMAT2X4",
  "DMAT3X2", "DMAT3X3", "DMAT3X4", "DMAT4X2", "DMAT4X3", "DMAT4X4",
  "ATOMIC_UINT", "SAMPLER1D", "SAMPLER2D", "SAMPLER3D", "SAMPLERCUBE",
  "SAMPLER1DSHADOW", "SAMPLER2DSHADOW", "SAMPLERCUBESHADOW",
  "SAMPLER1DARRAY", "SAMPLER2DARRAY", "SAMPLER1DARRAYSHADOW",
  "SAMPLER2DARRAYSHADOW", "ISAMPLER1D", "ISAMPLER2D", "ISAMPLER3D",
  "ISAMPLERCUBE", "ISAMPLER1DARRAY", "ISAMPLER2DARRAY", "USAMPLER1D",
  "USAMPLER2D", "USAMPLER3D", "USAMPLERCUBE", "USAMPLER1DARRAY",
  "USAMPLER2DARRAY", "SAMPLER2DRECT", "SAMPLER2DRECTSHADOW",
  "ISAMPLER2DRECT", "USAMPLER2DRECT", "SAMPLERBUFFER", "ISAMPLERBUFFER",
  "USAMPLERBUFFER", "SAMPLERCUBEARRAY", "SAMPLERCUBEARRAYSHADOW",
  "ISAMPLERCUBEARRAY", "USAMPLERCUBEARRAY", "SAMPLER2DMS", "ISAMPLER2DMS",
  "USAMPLER2DMS", "SAMPLER2DMSARRAY", "ISAMPLER2DMSARRAY",
  "USAMPLER2DMSARRAY", "IMAGE1D", "IIMAGE1D", "UIMAGE1D", "IMAGE2D",
  "IIMAGE2D", "UIMAGE2D", "IMAGE3D", "IIMAGE3D", "UIMAGE3D", "IMAGE2DRECT",
  "IIMAGE2DRECT", "UIMAGE2DRECT", "IMAGECUBE", "IIMAGECUBE", "UIMAGECUBE",
  "IMAGEBUFFER", "IIMAGEBUFFER", "UIMAGEBUFFER", "IMAGE1DARRAY",
  "IIMAGE1DARRAY", "UIMAGE1DARRAY", "IMAGE2DARRAY", "IIMAGE2DARRAY",
  "UIMAGE2DARRAY", "IMAGECUBEARRAY", "IIMAGECUBEARRAY", "UIMAGECUBEARRAY",
  "IMAGE2DMS", "IIMAGE2DMS", "UIMAGE2DMS", "IMAGE2DMSARRAY",
  "IIMAGE2DMSARRAY", "UIMAGE2DMSARRAY", "STRUCT", "VOID", "WHILE",
  "IDENTIFIER", "FLOATCONSTANT", "DOUBLECONSTANT", "INTCONSTANT",
  "UINTCONSTANT", "TRUE_VALUE", "FALSE_VALUE", "LEFT_OP", "RIGHT_OP",
  "INC_OP", "DEC_OP", "LE_OP", "GE_OP", "EQ_OP", "NE_OP", "AND_OP",
  "OR_OP", "XOR_OP", "MUL_ASSIGN", "DIV_ASSIGN", "ADD_ASSIGN",
  "MOD_ASSIGN", "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN", "XOR_ASSIGN",
  "OR_ASSIGN", "SUB_ASSIGN", "LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACKET",
  "RIGHT_BRACKET", "LEFT_BRACE", "RIGHT_BRACE", "DOT", "COMMA", "COLON",
  "EQUAL", "SEMICOLON", "BANG", "DASH", "TILDE", "PLUS", "STAR", "SLASH",
  "PERCENT", "LEFT_ANGLE", "RIGHT_ANGLE", "VERTICAL_BAR", "CARET",
  "AMPERSAND", "QUESTION", "INVARIANT", "PRECISE", "HIGHP", "MEDIUMP",
  "LOWP", "PRECISION", "AT", "UNARY_PLUS", "UNARY_DASH", "PRE_INC_OP",
  "PRE_DEC_OP", "POST_DEC_OP", "POST_INC_OP", "ARRAY_REF_OP",
  "FUNCTION_CALL", "TYPE_NAME_LIST", "TYPE_SPECIFIER",
  "POSTFIX_EXPRESSION", "TYPE_QUALIFIER_LIST", "STRUCT_DECLARATION",
  "STRUCT_DECLARATOR", "STRUCT_SPECIFIER", "FUNCTION_DEFINITION",
  "DECLARATION", "STATEMENT_LIST", "TRANSLATION_UNIT",
  "PRECISION_DECLARATION", "BLOCK_DECLARATION",
  "TYPE_QUALIFIER_DECLARATION", "IDENTIFIER_LIST", "INIT_DECLARATOR_LIST",
  "FULLY_SPECIFIED_TYPE", "SINGLE_DECLARATION", "SINGLE_INIT_DECLARATION",
  "INITIALIZER_LIST", "EXPRESSION_STATEMENT", "SELECTION_STATEMENT",
  "SELECTION_STATEMENT_ELSE", "SWITCH_STATEMENT", "FOR_REST_STATEMENT",
  "WHILE_STATEMENT", "DO_STATEMENT", "FOR_STATEMENT", "CASE_LABEL",
  "CONDITION_OPT", "ASSIGNMENT_CONDITION", "EXPRESSION_CONDITION",
  "FUNCTION_HEADER", "FUNCTION_DECLARATION", "FUNCTION_PARAMETER_LIST",
  "PARAMETER_DECLARATION", "PARAMETER_DECLARATOR",
  "UNINITIALIZED_DECLARATION", "ARRAY_SPECIFIER", "ARRAY_SPECIFIER_LIST",
  "STRUCT_DECLARATOR_LIST", "FUNCTION_CALL_PARAMETER_LIST",
  "STRUCT_DECLARATION_LIST", "LAYOUT_QUALIFIER_ID",
  "LAYOUT_QUALIFIER_ID_LIST", "SUBROUTINE_TYPE", "PAREN_EXPRESSION",
  "INIT_DECLARATOR", "INITIALIZER", "TERNARY_EXPRESSION",
  "FIELD_IDENTIFIER", "NUM_GLSL_TOKEN", "$accept", "root",
  "translation_unit", "block_identifier", "decl_identifier", "struct_name",
  "type_name", "param_name", "function_name", "field_identifier",
  "variable_identifier", "layout_identifier", "type_specifier_identifier",
  "external_declaration", "function_definition",
  "compound_statement_no_new_scope", "statement", "statement_list",
  "compound_statement", "simple_statement", "declaration",
  "identifier_list", "init_declarator_list", "single_declaration",
  "initializer", "initializer_list", "expression_statement",
  "selection_statement", "switch_statement", "switch_statement_list",
  "case_label", "iteration_statement", "statement_no_new_scope",
  "for_init_statement", "conditionopt", "condition", "for_rest_statement",
  "jump_statement", "function_prototype", "function_declarator",
  "function_parameter_list", "parameter_declaration",
  "parameter_declarator", "function_header", "fully_specified_type",
  "parameter_type_specifier", "type_specifier", "array_specifier_list",
  "array_specifier", "type_specifier_nonarray", "struct_specifier",
  "struct_declaration_list", "struct_declaration",
  "struct_declarator_list", "struct_declarator", "type_qualifier",
  "single_type_qualifier", "layout_qualifier", "layout_qualifier_id_list",
  "layout_qualifier_id", "precision_qualifier", "interpolation_qualifier",
  "invariant_qualifier", "precise_qualifier", "storage_qualifier",
  "type_name_list", "expression", "assignment_expression",
  "assignment_operator", "constant_expression", "conditional_expression",
  "logical_or_expression", "logical_xor_expression",
  "logical_and_expression", "inclusive_or_expression",
  "exclusive_or_expression", "and_expression", "equality_expression",
  "relational_expression", "shift_expression", "additive_expression",
  "multiplicative_expression", "unary_expression", "unary_operator",
  "postfix_expression", "integer_expression", "function_call",
  "function_call_or_method", "function_call_generic",
  "function_call_parameter_list", "function_identifier",
  "primary_expression", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   509,   510,   511,   512,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529
};
# endif

#define YYPACT_NINF (-429)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-353)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    1930,  -429,  -429,  -429,  -429,  -429,  -429,  -119,  -429,  -429,
    -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,
    -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,
    -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,
    -429,  -429,  -429,  -429,  -429,  -429,  -106,  -429,  -429,  -429,
    -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,
    -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,
    -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,
    -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,
    -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,
    -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,
    -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,
    -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,
    -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -133,
    -429,  -429,  -429,  -429,  -429,  -429,  -429,   -67,    34,  1930,
    -429,  -429,  -429,  -429,  -118,  -429,  -159,  -147,  3087,  -105,
    -429,   -76,  -429,  2319,  -429,  -429,  -429,  -429,  -429,  -429,
    -429,   -62,   -28,  -429,  3087,   -48,  4347,  -429,  -429,   -39,
    -429,   483,  -429,  -429,  -429,   -44,  -429,  -429,  -429,     8,
    3087,   -13,  -154,    -9,  3435,   -76,  -429,  -134,  -429,   -11,
    -102,  -429,  -429,  -429,  -429,  -117,  -429,  -429,    -2,   -92,
    -429,    36,  2511,  -429,  3087,  3087,    -1,  -429,  -140,     5,
       7,  1327,    11,    12,     9,  3619,    18,  3985,    13,    20,
      38,  -429,  -429,  -429,  -429,  -429,  -429,  3985,  3985,  3985,
     694,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,   905,
    -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,    14,
      22,   -73,  -429,  -429,  -151,    37,    42,     3,    15,    16,
     -14,  -143,     2,   -50,   -47,   -95,  3985,  -123,  -429,  -429,
    -429,    25,  -429,  3087,  -429,   -76,  -429,  -429,  3800,  -122,
    -429,  -429,  -429,    29,  -429,  -429,  -429,  3087,   -39,  -429,
     -61,  -429,   -62,  3985,  -429,   -28,  -429,   -76,   -60,  -429,
    -429,  -429,    36,  2703,  -429,  3800,  -116,  -429,  -429,    62,
    1734,  3985,  -429,  -429,   -55,  3985,   -89,  -429,  2123,  -429,
    -429,   -80,  -429,  1116,  -429,  -429,  3985,  -429,  3985,  3985,
    3985,  3985,  3985,  3985,  3985,  3985,  3985,  3985,  3985,  3985,
    3985,  3985,  3985,  3985,  3985,  3985,  3985,  3985,  -429,  -429,
    -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  -429,  3985,
    -429,  -429,  -429,  3985,    36,  4166,  -429,   -76,  3800,  -429,
    -429,  3800,  -429,  2895,  -429,   -39,  -429,  -429,  -429,  -429,
     -76,    36,  -429,   -53,  -429,  -429,  3800,    35,  -429,  -429,
    2123,   -79,  -429,   -71,  -429,    33,    67,  3087,    39,  -429,
    -429,  -429,    37,   -22,    42,     3,    15,    16,   -14,  -143,
    -143,     2,     2,     2,     2,   -50,   -50,   -47,   -47,  -429,
    -429,  -429,  -429,    39,    40,  -429,    41,  3985,  -429,   -59,
    -429,   -29,  -429,  -136,  -429,  -429,  -429,  -429,  3985,    30,
    -429,    43,  1327,    44,  1538,  -429,    46,  3985,  -429,  -429,
    -429,  3985,  -429,  3249,  -429,  -150,   -56,  3985,  1538,   217,
    1327,  -429,  -429,  -429,  3800,  -429,  -429,  -429,  -429,  -429,
    -149,    49,    39,  -429,  1327,  1327,    55,  -429,  -429,  -429,
    -429,  -429
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       2,   262,   114,   110,   111,   112,   113,   277,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   115,   116,   117,
     130,   131,   132,   266,   264,   265,   263,   269,   267,   268,
     270,   271,   272,   273,   274,   275,   276,   118,   119,   120,
     142,   143,   144,   259,   258,   257,     0,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   168,   169,   170,
     171,   172,   173,   175,   176,   177,   178,   179,   180,   182,
     183,   184,   185,   186,   187,   188,   166,   167,   174,   181,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,     0,
     109,    15,   260,   261,   254,   255,   256,     0,     0,     3,
     229,     4,    16,    17,     0,    46,    19,     0,    89,    51,
     100,   103,   228,     0,   240,   243,   244,   245,   246,   247,
     242,     0,     0,     8,     0,     0,     0,     1,     5,     0,
      36,     0,    35,    18,    88,    90,    91,    94,    96,   102,
       0,     7,    52,     0,     0,   104,   105,     9,    41,     0,
       0,   101,   241,     9,   279,     0,   253,    14,   251,     0,
     249,     0,     0,   232,     0,     0,     0,     7,    47,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,   356,   359,   354,   355,   357,   358,     0,     0,     0,
       0,    20,    61,   335,   334,   336,   333,   353,    24,     0,
      22,    23,    28,    29,    30,    31,    32,    33,    34,     0,
     100,     0,   281,   283,   297,   299,   301,   303,   305,   307,
     309,   311,   314,   319,   322,   325,     0,   329,   339,   344,
     345,     0,   337,     0,    10,    97,    93,    95,     0,    53,
      99,   107,   351,     0,   296,   325,   106,     0,     0,    42,
       0,   278,     0,     0,   248,     0,    12,   238,     0,   236,
     231,   233,     0,     0,    37,     0,    48,    84,    83,     0,
       0,     0,    87,    85,     0,     0,     0,    69,     0,   330,
     331,     0,    26,     0,    21,    25,     0,    62,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   286,   287,
     289,   288,   291,   292,   293,   294,   295,   290,   285,     0,
     332,   341,   342,     0,     0,     0,    92,    98,     0,    55,
      56,     0,   108,     0,    44,     0,    43,   280,   252,   250,
     239,     0,   234,     0,   230,    50,     0,     0,    76,    75,
      78,     0,    86,     0,    68,     0,     0,     0,    79,   360,
      27,   282,   300,     0,   302,   304,   306,   308,   310,   312,
     313,   317,   318,   315,   316,   320,   321,   324,   323,   326,
     327,   328,   284,   343,     0,   340,   109,   347,   349,     0,
      59,     0,    54,     0,    45,   237,   235,    49,     0,     0,
      77,     0,     0,     0,     0,    13,     0,     0,   338,   348,
     346,     0,    57,     0,    38,     0,     0,    81,     0,    63,
      66,    73,    74,    70,     0,   298,   350,    58,    60,    39,
       0,     0,    82,    72,     0,    67,     0,    80,    40,    71,
      64,    65
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -429,  -429,  -429,  -429,  -174,  -429,  -160,  -429,  -429,  -141,
    -175,  -429,  -429,    86,  -429,    89,  -218,  -232,  -429,  -428,
       1,  -429,  -429,  -429,  -306,  -429,   -74,  -429,  -429,  -429,
    -429,  -429,  -221,  -429,  -429,  -152,  -429,  -429,    10,  -429,
    -429,   -32,    63,  -429,  -309,    66,     0,  -188,  -193,  -429,
    -429,  -197,  -205,   -54,  -139,  -146,  -157,  -429,  -429,   -46,
     110,  -429,  -429,  -429,  -429,  -429,  -212,  -274,  -429,   -43,
    -178,  -429,   -77,   -78,   -70,   -72,   -75,   -81,  -173,  -300,
    -172,  -168,     4,  -429,  -429,  -429,  -429,  -429,  -429,  -429,
    -429,  -429
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,   148,   149,   199,   192,   175,   200,   285,   193,   307,
     247,   208,   150,   151,   152,   471,   248,   249,   250,   251,
     252,   300,   154,   155,   379,   441,   253,   254,   255,   486,
     256,   257,   473,   400,   449,   405,   451,   258,   259,   157,
     185,   186,   187,   158,   159,   188,   292,   195,   196,   161,
     162,   212,   213,   308,   309,   163,   164,   165,   209,   210,
     166,   167,   168,   169,   170,   205,   261,   262,   369,   293,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   434,   278,   279,   280,   439,
     281,   282
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     160,   153,   296,   319,   289,   218,   202,   311,   333,   395,
     156,   204,   190,   324,   380,   326,   294,   206,   313,   406,
     217,   338,   -15,   173,   347,   348,   472,   331,   214,   181,
     316,   335,   194,   202,   177,   182,   194,   194,   184,   288,
     472,   380,   371,   372,   479,   488,   194,   421,   422,   423,
     424,   191,   -15,   315,    -6,   174,   339,   202,   464,   349,
     350,  -352,   411,   373,   194,   171,   214,   374,   301,   214,
     194,   381,   440,   179,   302,   442,   180,   396,   172,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   298,
     447,   406,   299,   304,   203,   432,   296,   377,   368,   305,
     383,   438,   336,   404,   380,   409,   452,   380,   311,   401,
     194,   336,   336,   403,   453,   335,   408,   217,   336,   390,
     336,   337,   380,   296,   384,   294,   460,   413,   207,   481,
     385,   391,   461,   386,   392,   336,   336,   190,   391,   402,
     215,   446,   387,   144,   145,   146,   353,   283,   354,   160,
     153,   214,   355,   356,   357,   345,   346,   478,   189,   156,
     462,   433,   463,   201,   284,   351,   352,   214,   487,   336,
     457,   -11,   419,   420,   211,   290,   216,   297,   311,   425,
     426,   260,   407,   475,   296,   427,   428,   476,   408,   380,
     189,   303,   306,   314,   -15,   320,   321,   296,   295,   317,
     380,   318,   325,   322,   328,   327,  -351,   342,   182,   375,
     340,   444,   211,   341,   312,   211,   382,   397,   454,   448,
     343,   260,   344,   455,   467,   331,   459,   458,   468,   484,
     336,   456,   470,   435,   469,   178,   466,   214,   485,   474,
     260,   329,   330,   489,   491,   183,   399,   483,   450,   260,
     202,   376,   445,   286,   407,   482,   287,   176,   393,   389,
     388,   412,   414,   418,     0,     0,   490,   335,   417,   465,
     416,   415,     0,     0,     0,     0,     0,   480,     0,     0,
     370,     0,     0,   189,     0,     0,     0,   296,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   211,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   295,     0,     0,
       0,     0,     0,   211,     0,     0,     0,     0,     0,     0,
     260,   398,     0,     0,     0,     0,     0,     0,   260,     0,
       0,     0,     0,   260,     0,     0,     0,     0,     0,     0,
       0,     0,   295,     0,   295,   295,   295,   295,   295,   295,
     295,   295,   295,   295,   295,   295,   295,   295,   295,   429,
     430,   431,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   211,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     260,     0,     0,     0,     0,     0,     0,   201,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   260,     0,   260,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   260,     0,
     260,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   260,   260,     1,     2,     3,     4,
       5,     6,   219,   220,   221,     0,   222,   223,   224,   225,
       0,   226,   227,   228,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   229,   230,
     231,   232,   233,   234,   235,   236,     0,     0,   237,   238,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   239,     0,     0,
       0,   240,   241,     0,     0,     0,     0,   242,   243,   244,
     245,   246,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   142,   143,   144,   145,   146,   147,     1,     2,     3,
       4,     5,     6,   219,   220,   221,     0,   222,   223,   224,
     225,     0,   226,   227,   228,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   229,
     230,   231,   232,   233,   234,   235,   236,     0,     0,   237,
     238,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   239,     0,
       0,     0,   240,   332,     0,     0,     0,     0,   242,   243,
     244,   245,   246,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   142,   143,   144,   145,   146,   147,     1,     2,
       3,     4,     5,     6,   219,   220,   221,     0,   222,   223,
     224,   225,     0,   226,   227,   228,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     229,   230,   231,   232,   233,   234,   235,   236,     0,     0,
     237,   238,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   239,
       0,     0,     0,   240,   334,     0,     0,     0,     0,   242,
     243,   244,   245,   246,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   142,   143,   144,   145,   146,   147,     1,
       2,     3,     4,     5,     6,   219,   220,   221,     0,   222,
     223,   224,   225,     0,   226,   227,   228,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   229,   230,   231,   232,   233,   234,   235,   236,     0,
       0,   237,   238,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     239,     0,     0,     0,   240,   410,     0,     0,     0,     0,
     242,   243,   244,   245,   246,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   142,   143,   144,   145,   146,   147,
       1,     2,     3,     4,     5,     6,   219,   220,   221,     0,
     222,   223,   224,   225,     0,   226,   227,   228,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   229,   230,   231,   232,   233,   234,   235,   236,
       0,     0,   237,   238,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   239,     0,     0,     0,   240,     0,     0,     0,     0,
       0,   242,   243,   244,   245,   246,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   142,   143,   144,   145,   146,
     147,     1,     2,     3,     4,     5,     6,   219,   220,   221,
       0,   222,   223,   224,   225,     0,   226,   227,   228,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   229,   230,   231,   232,   233,   234,   235,
     236,     0,     0,   237,   238,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   239,     0,     0,     0,   181,     0,     0,     0,
       0,     0,   242,   243,   244,   245,   246,     1,     2,     3,
       4,     5,     6,     0,     0,     0,   142,   143,   144,   145,
     146,   147,     0,     0,     0,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,     0,
     230,   231,   232,   233,   234,   235,   236,     0,     0,   237,
     238,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   239,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   242,   243,
     244,   245,   246,     1,     2,     3,     4,     5,     6,     0,
       0,     0,   142,   143,   144,   145,   146,   147,     0,     0,
       0,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,     0,   141,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     1,     2,     3,     4,
       5,     6,     0,     0,     0,     0,     0,     0,   142,   143,
     144,   145,   146,   147,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,     0,   230,
     231,   232,   233,   234,   235,   236,     0,     0,   237,   238,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   239,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   243,   244,
     245,   246,     1,     2,     3,     4,     5,     6,     0,     0,
       0,   142,   143,   144,   145,   146,     0,     0,     0,     0,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,     0,   197,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   198,     1,     2,     3,     4,     5,     6,
       0,     0,     0,     0,     0,     0,     0,   142,   143,   144,
     145,   146,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,     0,   141,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     310,     0,     0,     0,     0,     0,     1,     2,     3,     4,
       5,     6,     0,     0,     0,     0,     0,     0,     0,   142,
     143,   144,   145,   146,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,     0,   141,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   394,     0,     0,     0,     0,     0,     1,     2,
       3,     4,     5,     6,     0,     0,     0,     0,     0,     0,
       0,   142,   143,   144,   145,   146,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
       0,   141,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   443,     0,     0,     0,     0,     0,
       1,     2,     3,     4,     5,     6,     0,     0,     0,     0,
       0,     0,     0,   142,   143,   144,   145,   146,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,     0,   141,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     2,     3,     4,     5,     6,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   142,   143,   144,   145,   146,
      37,    38,    39,    40,    41,    42,     0,     0,     0,     0,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,     0,   230,   231,   232,   233,   234,
     235,   236,     0,     0,   237,   238,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   239,     0,     0,     0,   378,   477,     2,
       3,     4,     5,     6,   243,   244,   245,   246,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    37,    38,    39,    40,
      41,    42,     0,     0,     0,     0,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
       0,   230,   231,   232,   233,   234,   235,   236,     0,     0,
     237,   238,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   239,
       0,     0,   291,     2,     3,     4,     5,     6,     0,     0,
     243,   244,   245,   246,     0,     0,     0,     0,     0,     0,
       0,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      37,    38,    39,    40,    41,    42,     0,     0,     0,     0,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,     0,   230,   231,   232,   233,   234,
     235,   236,     0,     0,   237,   238,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   239,     2,     3,     4,     5,     6,     0,
       0,     0,     0,   323,   243,   244,   245,   246,     0,     0,
       0,     0,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    37,    38,    39,    40,    41,    42,     0,     0,     0,
       0,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,     0,   230,   231,   232,   233,
     234,   235,   236,     0,     0,   237,   238,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   239,     0,     0,     0,   378,     2,
       3,     4,     5,     6,     0,   243,   244,   245,   246,     0,
       0,     0,     0,     0,     0,     0,     0,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    37,    38,    39,    40,
      41,    42,     0,     0,     0,     0,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
       0,   230,   231,   232,   233,   234,   235,   236,     0,     0,
     237,   238,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   239,
       2,     3,     4,     5,     6,     0,     0,     0,     0,     0,
     243,   244,   245,   246,     0,     0,     0,     0,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    37,    38,    39,
      40,    41,    42,     0,     0,     0,     0,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     436,     0,   230,   231,   232,   233,   234,   235,   236,     0,
       0,   237,   238,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     437,     2,     3,     4,     5,     6,     0,     0,     0,     0,
       0,   243,   244,   245,   246,     0,     0,     0,     0,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    37,    38,
      39,    40,    41,    42,     0,     0,     0,     0,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,     0,   141
};

static const yytype_int16 yycheck[] =
{
       0,     0,   195,   221,   192,   179,   163,   212,   240,   315,
       0,   171,   158,   225,   288,   227,   194,    45,   215,   328,
     156,   172,   156,   156,   167,   168,   454,   239,   174,   188,
     218,   249,   186,   190,     0,   194,   186,   186,   185,   193,
     468,   315,   165,   166,   194,   194,   186,   347,   348,   349,
     350,   156,   186,   193,   188,   188,   207,   214,   194,   202,
     203,   184,   336,   186,   186,   184,   212,   190,   185,   215,
     186,   193,   378,   191,   191,   381,   194,   193,   184,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   191,
     396,   400,   194,   185,   156,   369,   289,   285,   193,   191,
     297,   375,   191,   192,   378,   185,   185,   381,   313,   321,
     186,   191,   191,   325,   185,   333,   328,   156,   191,   307,
     191,   194,   396,   316,   298,   303,   185,   339,   156,   185,
     191,   191,   191,   194,   194,   191,   191,   283,   191,   194,
     188,   194,   302,   210,   211,   212,   196,   191,   198,   149,
     149,   297,   199,   200,   201,   169,   170,   463,   158,   149,
     189,   373,   191,   163,   156,   163,   164,   313,   474,   191,
     192,   184,   345,   346,   174,   184,   176,   188,   383,   351,
     352,   181,   328,   457,   377,   353,   354,   461,   400,   463,
     190,   193,   156,   194,   156,   184,   184,   390,   194,   194,
     474,   194,   184,   194,   184,   192,   184,   204,   194,   184,
     173,   385,   212,   171,   214,   215,   187,   155,   185,   184,
     205,   221,   206,   156,   194,   437,   185,   187,   185,    12,
     191,   406,   188,   374,   452,   149,   448,   383,   470,   193,
     240,   237,   238,   194,   189,   156,   320,   468,   400,   249,
     407,   283,   391,   190,   400,   467,   190,   147,   312,   305,
     303,   338,   340,   344,    -1,    -1,   484,   485,   343,   443,
     342,   341,    -1,    -1,    -1,    -1,    -1,   465,    -1,    -1,
     276,    -1,    -1,   283,    -1,    -1,    -1,   480,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   297,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   303,    -1,    -1,
      -1,    -1,    -1,   313,    -1,    -1,    -1,    -1,    -1,    -1,
     320,   320,    -1,    -1,    -1,    -1,    -1,    -1,   328,    -1,
      -1,    -1,    -1,   333,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   338,    -1,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   383,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     400,    -1,    -1,    -1,    -1,    -1,    -1,   407,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   452,    -1,   454,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   468,    -1,
     470,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   484,   485,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    -1,    13,    14,    15,    16,
      -1,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,    -1,    -1,   165,   166,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,    -1,    -1,
      -1,   188,   189,    -1,    -1,    -1,    -1,   194,   195,   196,
     197,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   208,   209,   210,   211,   212,   213,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    -1,    13,    14,    15,
      16,    -1,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,    -1,    -1,   165,
     166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,    -1,
      -1,    -1,   188,   189,    -1,    -1,    -1,    -1,   194,   195,
     196,   197,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   208,   209,   210,   211,   212,   213,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    -1,    13,    14,
      15,    16,    -1,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,    -1,    -1,
     165,   166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
      -1,    -1,    -1,   188,   189,    -1,    -1,    -1,    -1,   194,
     195,   196,   197,   198,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   208,   209,   210,   211,   212,   213,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    -1,    13,
      14,    15,    16,    -1,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,    -1,
      -1,   165,   166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     184,    -1,    -1,    -1,   188,   189,    -1,    -1,    -1,    -1,
     194,   195,   196,   197,   198,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   208,   209,   210,   211,   212,   213,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    -1,
      13,    14,    15,    16,    -1,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
      -1,    -1,   165,   166,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   184,    -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,
      -1,   194,   195,   196,   197,   198,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   208,   209,   210,   211,   212,
     213,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      -1,    13,    14,    15,    16,    -1,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,    -1,    -1,   165,   166,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,    -1,    -1,    -1,   188,    -1,    -1,    -1,
      -1,    -1,   194,   195,   196,   197,   198,     3,     4,     5,
       6,     7,     8,    -1,    -1,    -1,   208,   209,   210,   211,
     212,   213,    -1,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,    -1,
     156,   157,   158,   159,   160,   161,   162,    -1,    -1,   165,
     166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   194,   195,
     196,   197,   198,     3,     4,     5,     6,     7,     8,    -1,
      -1,    -1,   208,   209,   210,   211,   212,   213,    -1,    -1,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,    -1,   208,   209,
     210,   211,   212,   213,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,    -1,   156,
     157,   158,   159,   160,   161,   162,    -1,    -1,   165,   166,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,   196,
     197,   198,     3,     4,     5,     6,     7,     8,    -1,    -1,
      -1,   208,   209,   210,   211,   212,    -1,    -1,    -1,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   194,     3,     4,     5,     6,     7,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   208,   209,   210,
     211,   212,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     189,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   208,
     209,   210,   211,   212,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   189,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   208,   209,   210,   211,   212,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
      -1,   156,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   189,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   208,   209,   210,   211,   212,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     4,     5,     6,     7,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   208,   209,   210,   211,   212,
      51,    52,    53,    54,    55,    56,    -1,    -1,    -1,    -1,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,    -1,   156,   157,   158,   159,   160,
     161,   162,    -1,    -1,   165,   166,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   184,    -1,    -1,    -1,   188,   189,     4,
       5,     6,     7,     8,   195,   196,   197,   198,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    53,    54,
      55,    56,    -1,    -1,    -1,    -1,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
      -1,   156,   157,   158,   159,   160,   161,   162,    -1,    -1,
     165,   166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
      -1,    -1,   187,     4,     5,     6,     7,     8,    -1,    -1,
     195,   196,   197,   198,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    52,    53,    54,    55,    56,    -1,    -1,    -1,    -1,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,    -1,   156,   157,   158,   159,   160,
     161,   162,    -1,    -1,   165,   166,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   184,     4,     5,     6,     7,     8,    -1,
      -1,    -1,    -1,   194,   195,   196,   197,   198,    -1,    -1,
      -1,    -1,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    52,    53,    54,    55,    56,    -1,    -1,    -1,
      -1,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,    -1,   156,   157,   158,   159,
     160,   161,   162,    -1,    -1,   165,   166,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   184,    -1,    -1,    -1,   188,     4,
       5,     6,     7,     8,    -1,   195,   196,   197,   198,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    53,    54,
      55,    56,    -1,    -1,    -1,    -1,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
      -1,   156,   157,   158,   159,   160,   161,   162,    -1,    -1,
     165,   166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
       4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,
     195,   196,   197,   198,    -1,    -1,    -1,    -1,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    53,
      54,    55,    56,    -1,    -1,    -1,    -1,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,    -1,   156,   157,   158,   159,   160,   161,   162,    -1,
      -1,   165,   166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     184,     4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,
      -1,   195,   196,   197,   198,    -1,    -1,    -1,    -1,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,
      53,    54,    55,    56,    -1,    -1,    -1,    -1,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,    -1,   156
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   156,   208,   209,   210,   211,   212,   213,   276,   277,
     287,   288,   289,   295,   297,   298,   313,   314,   318,   319,
     321,   324,   325,   330,   331,   332,   335,   336,   337,   338,
     339,   184,   184,   156,   188,   280,   335,     0,   288,   191,
     194,   188,   194,   290,   185,   315,   316,   317,   320,   321,
     330,   156,   279,   283,   186,   322,   323,   156,   194,   278,
     281,   321,   331,   156,   281,   340,    45,   156,   286,   333,
     334,   321,   326,   327,   330,   188,   321,   156,   279,     9,
      10,    11,    13,    14,    15,    16,    18,    19,    20,   155,
     156,   157,   158,   159,   160,   161,   162,   165,   166,   184,
     188,   189,   194,   195,   196,   197,   198,   285,   291,   292,
     293,   294,   295,   301,   302,   303,   305,   306,   312,   313,
     321,   341,   342,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   361,   362,
     363,   365,   366,   191,   156,   282,   317,   320,   193,   322,
     184,   187,   321,   344,   345,   357,   323,   188,   191,   194,
     296,   185,   191,   193,   185,   191,   156,   284,   328,   329,
     189,   327,   321,   326,   194,   193,   322,   194,   194,   291,
     184,   184,   194,   194,   341,   184,   341,   192,   184,   357,
     357,   341,   189,   292,   189,   291,   191,   194,   172,   207,
     173,   171,   204,   205,   206,   169,   170,   167,   168,   202,
     203,   163,   164,   196,   198,   199,   200,   201,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   193,   343,
     357,   165,   166,   186,   190,   184,   316,   322,   188,   299,
     342,   193,   187,   326,   279,   191,   194,   281,   344,   334,
     322,   191,   194,   328,   189,   299,   193,   155,   295,   301,
     308,   341,   194,   341,   192,   310,   319,   330,   341,   185,
     189,   342,   347,   341,   348,   349,   350,   351,   352,   353,
     353,   354,   354,   354,   354,   355,   355,   356,   356,   357,
     357,   357,   342,   341,   360,   284,   154,   184,   342,   364,
     299,   300,   299,   189,   279,   329,   194,   299,   184,   309,
     310,   311,   185,   185,   185,   156,   285,   192,   187,   185,
     185,   191,   189,   191,   194,   279,   341,   194,   185,   291,
     188,   290,   294,   307,   193,   342,   342,   189,   299,   194,
     322,   185,   341,   307,    12,   292,   304,   299,   194,   194,
     291,   189
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int16 yyr1[] =
{
       0,   275,   276,   276,   277,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   288,   289,   289,
     290,   290,   291,   291,   292,   292,   293,   293,   294,   294,
     294,   294,   294,   294,   294,   295,   295,   295,   295,   295,
     295,   295,   295,   295,   296,   296,   297,   297,   297,   297,
     297,   298,   298,   298,   298,   298,   299,   299,   299,   300,
     300,   301,   301,   302,   302,   303,   304,   304,   305,   305,
     306,   306,   306,   307,   307,   308,   308,   309,   309,   310,
     310,   311,   311,   312,   312,   312,   312,   312,   313,   314,
     314,   315,   315,   316,   316,   316,   316,   317,   317,   318,
     319,   319,   320,   321,   321,   322,   322,   323,   323,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     325,   325,   326,   326,   327,   327,   328,   328,   329,   329,
     330,   330,   331,   331,   331,   331,   331,   331,   332,   333,
     333,   334,   334,   334,   335,   335,   335,   336,   336,   336,
     337,   338,   339,   339,   339,   339,   339,   339,   339,   339,
     339,   339,   339,   339,   339,   339,   339,   339,   339,   340,
     340,   341,   341,   342,   342,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   344,   345,   345,   346,
     346,   347,   347,   348,   348,   349,   349,   350,   350,   351,
     351,   352,   352,   352,   353,   353,   353,   353,   353,   354,
     354,   354,   355,   355,   355,   356,   356,   356,   356,   357,
     357,   357,   357,   358,   358,   358,   358,   359,   359,   359,
     359,   359,   359,   360,   361,   362,   363,   363,   363,   364,
     364,   365,   365,   366,   366,   366,   366,   366,   366,   366,
     366
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     1,
       2,     3,     1,     1,     1,     2,     2,     3,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     4,     6,     7,
       8,     2,     3,     4,     2,     3,     1,     3,     4,     6,
       5,     1,     2,     3,     5,     4,     1,     3,     4,     1,
       3,     1,     2,     5,     7,     7,     0,     1,     3,     2,
       5,     7,     6,     1,     1,     1,     1,     1,     0,     1,
       4,     2,     3,     2,     2,     2,     3,     2,     2,     1,
       2,     1,     3,     2,     1,     2,     1,     2,     3,     3,
       1,     2,     1,     1,     2,     1,     2,     2,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       5,     4,     1,     2,     3,     4,     1,     3,     1,     2,
       1,     2,     1,     1,     1,     1,     1,     1,     4,     1,
       3,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     4,     1,
       3,     1,     3,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     5,     1,
       3,     1,     3,     1,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     3,     1,     3,     3,     3,     3,     1,
       3,     3,     1,     3,     3,     1,     3,     3,     3,     1,
       2,     2,     2,     1,     1,     1,     1,     1,     4,     1,
       3,     2,     2,     1,     1,     1,     4,     3,     4,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (&yylloc, context, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if GLSL_DEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined GLSL_LTYPE_IS_TRIVIAL && GLSL_LTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, context); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, struct glsl_parse_context * context)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  YYUSE (yylocationp);
  YYUSE (context);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, struct glsl_parse_context * context)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyo, *yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yytype, yyvaluep, yylocationp, context);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, struct glsl_parse_context * context)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[+yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , context);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, context); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !GLSL_DEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !GLSL_DEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
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
            else
              goto append;

          append:
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

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[+*yyssp];
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, struct glsl_parse_context * context)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (context);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (struct glsl_parse_context * context)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined GLSL_LTYPE_IS_TRIVIAL && GLSL_LTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYPTRDIFF_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

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
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, &yylloc, scanner);
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
      if (yytable_value_is_error (yyn))
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2:
#line 503 "glsl.y"
                          { context->root = new_glsl_node(context, TRANSLATION_UNIT, NULL); }
#line 3315 "glsl.parser.c"
    break;

  case 3:
#line 504 "glsl.y"
                                           { context->root = (yyvsp[0].translation_unit); }
#line 3321 "glsl.parser.c"
    break;

  case 4:
#line 508 "glsl.y"
                                { (yyval.translation_unit) = new_glsl_node(context, TRANSLATION_UNIT, (yyvsp[0].external_declaration), NULL); }
#line 3327 "glsl.parser.c"
    break;

  case 5:
#line 511 "glsl.y"
                                { (yyval.translation_unit) = new_glsl_node(context, TRANSLATION_UNIT, (yyvsp[-1].translation_unit), (yyvsp[0].external_declaration), NULL); }
#line 3333 "glsl.parser.c"
    break;

  case 6:
#line 514 "glsl.y"
                                     { (yyval.block_identifier) = new_glsl_identifier(context, (yyvsp[0].IDENTIFIER)); }
#line 3339 "glsl.parser.c"
    break;

  case 7:
#line 517 "glsl.y"
                                     { (yyval.decl_identifier) = new_glsl_identifier(context, (yyvsp[0].IDENTIFIER)); }
#line 3345 "glsl.parser.c"
    break;

  case 8:
#line 520 "glsl.y"
                                     { (yyval.struct_name) = new_glsl_identifier(context, (yyvsp[0].IDENTIFIER)); }
#line 3351 "glsl.parser.c"
    break;

  case 9:
#line 523 "glsl.y"
                                     { (yyval.type_name) = new_glsl_identifier(context, (yyvsp[0].IDENTIFIER)); }
#line 3357 "glsl.parser.c"
    break;

  case 10:
#line 526 "glsl.y"
                                     { (yyval.param_name) = new_glsl_identifier(context, (yyvsp[0].IDENTIFIER)); }
#line 3363 "glsl.parser.c"
    break;

  case 11:
#line 529 "glsl.y"
                                     { (yyval.function_name) = new_glsl_identifier(context, (yyvsp[0].IDENTIFIER)); }
#line 3369 "glsl.parser.c"
    break;

  case 12:
#line 532 "glsl.y"
                                     { (yyval.field_identifier) = new_glsl_string(context, FIELD_IDENTIFIER, (yyvsp[0].IDENTIFIER)); }
#line 3375 "glsl.parser.c"
    break;

  case 13:
#line 535 "glsl.y"
                                     { (yyval.variable_identifier) = new_glsl_identifier(context, (yyvsp[0].IDENTIFIER)); }
#line 3381 "glsl.parser.c"
    break;

  case 14:
#line 538 "glsl.y"
                                     { (yyval.layout_identifier) = new_glsl_identifier(context, (yyvsp[0].IDENTIFIER)); }
#line 3387 "glsl.parser.c"
    break;

  case 15:
#line 541 "glsl.y"
                                       { (yyval.type_specifier_identifier) = new_glsl_identifier(context, (yyvsp[0].IDENTIFIER)); }
#line 3393 "glsl.parser.c"
    break;

  case 16:
#line 544 "glsl.y"
                                              { (yyval.external_declaration) = (yyvsp[0].function_definition); }
#line 3399 "glsl.parser.c"
    break;

  case 17:
#line 545 "glsl.y"
                                      { (yyval.external_declaration) = (yyvsp[0].declaration); }
#line 3405 "glsl.parser.c"
    break;

  case 18:
#line 549 "glsl.y"
                                { (yyval.function_definition) = new_glsl_node(context, FUNCTION_DEFINITION,
					(yyvsp[-1].function_prototype),
					(yyvsp[0].compound_statement_no_new_scope),
					NULL); }
#line 3414 "glsl.parser.c"
    break;

  case 19:
#line 554 "glsl.y"
                                { (yyval.function_definition) = new_glsl_node(context, FUNCTION_DEFINITION,
					(yyvsp[0].function_prototype),
					new_glsl_node(context, STATEMENT_LIST, NULL),
					NULL); }
#line 3423 "glsl.parser.c"
    break;

  case 20:
#line 560 "glsl.y"
                                                         { (yyval.compound_statement_no_new_scope) = new_glsl_node(context, STATEMENT_LIST, NULL); }
#line 3429 "glsl.parser.c"
    break;

  case 21:
#line 561 "glsl.y"
                                                                { (yyval.compound_statement_no_new_scope) = (yyvsp[-1].statement_list); }
#line 3435 "glsl.parser.c"
    break;

  case 22:
#line 564 "glsl.y"
                                             { (yyval.statement) = (yyvsp[0].compound_statement); }
#line 3441 "glsl.parser.c"
    break;

  case 23:
#line 565 "glsl.y"
                                           { (yyval.statement) = (yyvsp[0].simple_statement); }
#line 3447 "glsl.parser.c"
    break;

  case 24:
#line 568 "glsl.y"
                                    { (yyval.statement_list) = new_glsl_node(context, STATEMENT_LIST, (yyvsp[0].statement), NULL); }
#line 3453 "glsl.parser.c"
    break;

  case 25:
#line 569 "glsl.y"
                                                   { (yyval.statement_list) = new_glsl_node(context, STATEMENT_LIST, (yyvsp[-1].statement_list), (yyvsp[0].statement), NULL); }
#line 3459 "glsl.parser.c"
    break;

  case 26:
#line 572 "glsl.y"
                                                 { (yyval.compound_statement) = new_glsl_node(context, STATEMENT_LIST, NULL); }
#line 3465 "glsl.parser.c"
    break;

  case 27:
#line 573 "glsl.y"
                                                                { (yyval.compound_statement) = (yyvsp[-1].statement_list); }
#line 3471 "glsl.parser.c"
    break;

  case 28:
#line 576 "glsl.y"
                                      { (yyval.simple_statement) = (yyvsp[0].declaration); }
#line 3477 "glsl.parser.c"
    break;

  case 29:
#line 577 "glsl.y"
                                               { (yyval.simple_statement) = (yyvsp[0].expression_statement); }
#line 3483 "glsl.parser.c"
    break;

  case 30:
#line 578 "glsl.y"
                                              { (yyval.simple_statement) = (yyvsp[0].selection_statement); }
#line 3489 "glsl.parser.c"
    break;

  case 31:
#line 579 "glsl.y"
                                           { (yyval.simple_statement) = (yyvsp[0].switch_statement); }
#line 3495 "glsl.parser.c"
    break;

  case 32:
#line 580 "glsl.y"
                                     { (yyval.simple_statement)= (yyvsp[0].case_label); }
#line 3501 "glsl.parser.c"
    break;

  case 33:
#line 581 "glsl.y"
                                              { (yyval.simple_statement) = (yyvsp[0].iteration_statement); }
#line 3507 "glsl.parser.c"
    break;

  case 34:
#line 582 "glsl.y"
                                         { (yyval.simple_statement) = (yyvsp[0].jump_statement); }
#line 3513 "glsl.parser.c"
    break;

  case 35:
#line 585 "glsl.y"
                                                       { (yyval.declaration) = new_glsl_node(context, DECLARATION, (yyvsp[-1].function_prototype), NULL); }
#line 3519 "glsl.parser.c"
    break;

  case 36:
#line 586 "glsl.y"
                                                         { (yyval.declaration) = new_glsl_node(context, DECLARATION, (yyvsp[-1].init_declarator_list), NULL); }
#line 3525 "glsl.parser.c"
    break;

  case 37:
#line 588 "glsl.y"
                                { (yyval.declaration) = new_glsl_node(context, DECLARATION,
						new_glsl_node(context, PRECISION_DECLARATION,
							(yyvsp[-2].precision_qualifier),
							(yyvsp[-1].type_specifier),
							NULL),
						NULL); }
#line 3536 "glsl.parser.c"
    break;

  case 38:
#line 595 "glsl.y"
                                { (yyval.declaration) = new_glsl_node(context, DECLARATION,
						new_glsl_node(context, BLOCK_DECLARATION,
							(yyvsp[-5].type_qualifier),
							(yyvsp[-4].block_identifier),
							(yyvsp[-2].struct_declaration_list),
							new_glsl_identifier(context, NULL),
							new_glsl_node(context, ARRAY_SPECIFIER_LIST, NULL),
							NULL),
						NULL); }
#line 3550 "glsl.parser.c"
    break;

  case 39:
#line 605 "glsl.y"
                                { (yyval.declaration) = new_glsl_node(context, DECLARATION,
						new_glsl_node(context, BLOCK_DECLARATION,
							(yyvsp[-6].type_qualifier),
							(yyvsp[-5].block_identifier),
							(yyvsp[-3].struct_declaration_list),
							(yyvsp[-1].decl_identifier),
							new_glsl_node(context, ARRAY_SPECIFIER_LIST, NULL),
							NULL),
						NULL); }
#line 3564 "glsl.parser.c"
    break;

  case 40:
#line 615 "glsl.y"
                                { (yyval.declaration) = new_glsl_node(context, DECLARATION,
						new_glsl_node(context, BLOCK_DECLARATION,
							(yyvsp[-7].type_qualifier),
							(yyvsp[-6].block_identifier),
							(yyvsp[-4].struct_declaration_list),
							(yyvsp[-2].decl_identifier),
							(yyvsp[-1].array_specifier_list),
							NULL),
						NULL); }
#line 3578 "glsl.parser.c"
    break;

  case 41:
#line 625 "glsl.y"
                                { (yyval.declaration) = new_glsl_node(context, DECLARATION,
						new_glsl_node(context, UNINITIALIZED_DECLARATION,
							(yyvsp[-1].type_qualifier),
							new_glsl_identifier(context, NULL),
							NULL),
						NULL); }
#line 3589 "glsl.parser.c"
    break;

  case 42:
#line 632 "glsl.y"
                                { (yyval.declaration) = new_glsl_node(context, DECLARATION,
						new_glsl_node(context, UNINITIALIZED_DECLARATION,
							(yyvsp[-2].type_qualifier),
							(yyvsp[-1].type_name),
							new_glsl_node(context, IDENTIFIER_LIST, NULL),
							NULL),
						NULL); }
#line 3601 "glsl.parser.c"
    break;

  case 43:
#line 640 "glsl.y"
                                { (yyval.declaration) = new_glsl_node(context, DECLARATION,
						new_glsl_node(context, UNINITIALIZED_DECLARATION,
							(yyvsp[-3].type_qualifier),
							(yyvsp[-2].type_name),
							(yyvsp[-1].identifier_list),
							NULL),
						NULL); }
#line 3613 "glsl.parser.c"
    break;

  case 44:
#line 649 "glsl.y"
                                                { (yyval.identifier_list) = new_glsl_node(context, IDENTIFIER_LIST, (yyvsp[0].decl_identifier), NULL); }
#line 3619 "glsl.parser.c"
    break;

  case 45:
#line 651 "glsl.y"
                                { (yyval.identifier_list) = new_glsl_node(context, IDENTIFIER_LIST, (yyvsp[-2].identifier_list), (yyvsp[0].decl_identifier), NULL); }
#line 3625 "glsl.parser.c"
    break;

  case 46:
#line 654 "glsl.y"
                                             { (yyval.init_declarator_list) = new_glsl_node(context, INIT_DECLARATOR_LIST, (yyvsp[0].single_declaration), NULL); }
#line 3631 "glsl.parser.c"
    break;

  case 47:
#line 656 "glsl.y"
                                { (yyval.init_declarator_list) = new_glsl_node(context, INIT_DECLARATOR_LIST,
						(yyvsp[-2].init_declarator_list),
						new_glsl_node(context, INIT_DECLARATOR,
							(yyvsp[0].decl_identifier),
							new_glsl_node(context, ARRAY_SPECIFIER_LIST, NULL),
							NULL),
						NULL); }
#line 3643 "glsl.parser.c"
    break;

  case 48:
#line 664 "glsl.y"
                                { (yyval.init_declarator_list) = new_glsl_node(context, INIT_DECLARATOR_LIST,
						(yyvsp[-3].init_declarator_list),
						new_glsl_node(context, INIT_DECLARATOR,
							(yyvsp[-1].decl_identifier),
							(yyvsp[0].array_specifier_list),
							NULL),
						NULL); }
#line 3655 "glsl.parser.c"
    break;

  case 49:
#line 672 "glsl.y"
                                { (yyval.init_declarator_list) = new_glsl_node(context, INIT_DECLARATOR_LIST,
						(yyvsp[-5].init_declarator_list),
						new_glsl_node(context, INIT_DECLARATOR,
							(yyvsp[-3].decl_identifier),
							(yyvsp[-2].array_specifier_list),
							(yyvsp[0].initializer),
							NULL),
						NULL); }
#line 3668 "glsl.parser.c"
    break;

  case 50:
#line 681 "glsl.y"
                                { (yyval.init_declarator_list) = new_glsl_node(context, INIT_DECLARATOR_LIST,
						(yyvsp[-4].init_declarator_list),
						new_glsl_node(context, INIT_DECLARATOR,
							(yyvsp[-2].decl_identifier),
							new_glsl_node(context, ARRAY_SPECIFIER_LIST, NULL),
							(yyvsp[0].initializer),
							NULL),
						NULL); }
#line 3681 "glsl.parser.c"
    break;

  case 51:
#line 692 "glsl.y"
                                { (yyval.single_declaration) = new_glsl_node(context, SINGLE_DECLARATION,
					(yyvsp[0].fully_specified_type),
					new_glsl_identifier(context, NULL),
					new_glsl_node(context, ARRAY_SPECIFIER_LIST, NULL),
					NULL); }
#line 3691 "glsl.parser.c"
    break;

  case 52:
#line 699 "glsl.y"
                                { (yyval.single_declaration) = new_glsl_node(context, SINGLE_DECLARATION,
					(yyvsp[-1].fully_specified_type),
					(yyvsp[0].decl_identifier),
					new_glsl_node(context, ARRAY_SPECIFIER_LIST, NULL),
					NULL); }
#line 3701 "glsl.parser.c"
    break;

  case 53:
#line 706 "glsl.y"
                                { (yyval.single_declaration) = new_glsl_node(context, SINGLE_DECLARATION, (yyvsp[-2].fully_specified_type), (yyvsp[-1].decl_identifier), (yyvsp[0].array_specifier_list), NULL); }
#line 3707 "glsl.parser.c"
    break;

  case 54:
#line 709 "glsl.y"
                                { (yyval.single_declaration) = new_glsl_node(context, SINGLE_INIT_DECLARATION, (yyvsp[-4].fully_specified_type), (yyvsp[-3].decl_identifier), (yyvsp[-2].array_specifier_list), (yyvsp[0].initializer), NULL); }
#line 3713 "glsl.parser.c"
    break;

  case 55:
#line 712 "glsl.y"
                                { (yyval.single_declaration) = new_glsl_node(context, SINGLE_INIT_DECLARATION,
					(yyvsp[-3].fully_specified_type),
					(yyvsp[-2].decl_identifier),
					new_glsl_node(context, ARRAY_SPECIFIER_LIST, NULL),
					(yyvsp[0].initializer),
					NULL); }
#line 3724 "glsl.parser.c"
    break;

  case 56:
#line 720 "glsl.y"
                                                { (yyval.initializer) = new_glsl_node(context, INITIALIZER, (yyvsp[0].assignment_expression), NULL); }
#line 3730 "glsl.parser.c"
    break;

  case 57:
#line 721 "glsl.y"
                                                                  { (yyval.initializer) = new_glsl_node(context, INITIALIZER, (yyvsp[-1].initializer_list), NULL); }
#line 3736 "glsl.parser.c"
    break;

  case 58:
#line 722 "glsl.y"
                                                                        { (yyval.initializer) = new_glsl_node(context, INITIALIZER, (yyvsp[-2].initializer_list), NULL); }
#line 3742 "glsl.parser.c"
    break;

  case 59:
#line 726 "glsl.y"
                                { (yyval.initializer_list) = new_glsl_node(context, INITIALIZER_LIST, (yyvsp[0].initializer), NULL); }
#line 3748 "glsl.parser.c"
    break;

  case 60:
#line 728 "glsl.y"
                                { (yyval.initializer_list) = new_glsl_node(context, INITIALIZER_LIST, (yyvsp[-2].initializer_list), (yyvsp[0].initializer), NULL); }
#line 3754 "glsl.parser.c"
    break;

  case 61:
#line 731 "glsl.y"
                                    { (yyval.expression_statement) = new_glsl_node(context, EXPRESSION_STATEMENT, NULL); }
#line 3760 "glsl.parser.c"
    break;

  case 62:
#line 732 "glsl.y"
                                               { (yyval.expression_statement) = new_glsl_node(context, EXPRESSION_STATEMENT, (yyvsp[-1].expression), NULL); }
#line 3766 "glsl.parser.c"
    break;

  case 63:
#line 736 "glsl.y"
                                { (yyval.selection_statement) = new_glsl_node(context, SELECTION_STATEMENT, (yyvsp[-2].expression), (yyvsp[0].statement), NULL); }
#line 3772 "glsl.parser.c"
    break;

  case 64:
#line 739 "glsl.y"
                                { (yyval.selection_statement) = new_glsl_node(context, SELECTION_STATEMENT_ELSE, (yyvsp[-4].expression), (yyvsp[-2].statement), (yyvsp[0].statement), NULL); }
#line 3778 "glsl.parser.c"
    break;

  case 65:
#line 743 "glsl.y"
                                { (yyval.switch_statement) = new_glsl_node(context, SWITCH_STATEMENT, (yyvsp[-4].expression), (yyvsp[-1].switch_statement_list), NULL); }
#line 3784 "glsl.parser.c"
    break;

  case 66:
#line 746 "glsl.y"
                          { (yyval.switch_statement_list) = new_glsl_node(context, STATEMENT_LIST, NULL); }
#line 3790 "glsl.parser.c"
    break;

  case 67:
#line 747 "glsl.y"
                                         { (yyval.switch_statement_list) = (yyvsp[0].statement_list); }
#line 3796 "glsl.parser.c"
    break;

  case 68:
#line 750 "glsl.y"
                                                { (yyval.case_label) = new_glsl_node(context, CASE_LABEL, (yyvsp[-1].expression), NULL); }
#line 3802 "glsl.parser.c"
    break;

  case 69:
#line 751 "glsl.y"
                                        { (yyval.case_label) = new_glsl_node(context, CASE_LABEL, NULL); }
#line 3808 "glsl.parser.c"
    break;

  case 70:
#line 755 "glsl.y"
                                { (yyval.iteration_statement) = new_glsl_node(context, WHILE_STATEMENT, (yyvsp[-2].condition), (yyvsp[0].statement_no_new_scope), NULL); }
#line 3814 "glsl.parser.c"
    break;

  case 71:
#line 758 "glsl.y"
                                { (yyval.iteration_statement) = new_glsl_node(context, DO_STATEMENT, (yyvsp[-5].statement), (yyvsp[-2].expression), NULL); }
#line 3820 "glsl.parser.c"
    break;

  case 72:
#line 761 "glsl.y"
                                { (yyval.iteration_statement) = new_glsl_node(context, FOR_STATEMENT, (yyvsp[-3].for_init_statement), (yyvsp[-2].for_rest_statement), (yyvsp[0].statement_no_new_scope), NULL); }
#line 3826 "glsl.parser.c"
    break;

  case 73:
#line 764 "glsl.y"
                                                          { (yyval.statement_no_new_scope) = (yyvsp[0].compound_statement_no_new_scope); }
#line 3832 "glsl.parser.c"
    break;

  case 74:
#line 765 "glsl.y"
                                           { (yyval.statement_no_new_scope) = (yyvsp[0].simple_statement); }
#line 3838 "glsl.parser.c"
    break;

  case 75:
#line 768 "glsl.y"
                                               { (yyval.for_init_statement) = (yyvsp[0].expression_statement); }
#line 3844 "glsl.parser.c"
    break;

  case 76:
#line 769 "glsl.y"
                                      { (yyval.for_init_statement) = (yyvsp[0].declaration); }
#line 3850 "glsl.parser.c"
    break;

  case 77:
#line 772 "glsl.y"
                                    { (yyval.conditionopt) = new_glsl_node(context, CONDITION_OPT, (yyvsp[0].condition), NULL); }
#line 3856 "glsl.parser.c"
    break;

  case 78:
#line 773 "glsl.y"
                          { (yyval.conditionopt) = new_glsl_node(context, CONDITION_OPT, NULL); }
#line 3862 "glsl.parser.c"
    break;

  case 79:
#line 777 "glsl.y"
                                { (yyval.condition) = new_glsl_node(context, EXPRESSION_CONDITION, (yyvsp[0].expression), NULL); }
#line 3868 "glsl.parser.c"
    break;

  case 80:
#line 780 "glsl.y"
                                { (yyval.condition) = new_glsl_node(context, ASSIGNMENT_CONDITION, (yyvsp[-3].fully_specified_type), (yyvsp[-2].variable_identifier), (yyvsp[0].initializer), NULL); }
#line 3874 "glsl.parser.c"
    break;

  case 81:
#line 784 "glsl.y"
                                { (yyval.for_rest_statement) = new_glsl_node(context, FOR_REST_STATEMENT, (yyvsp[-1].conditionopt), NULL); }
#line 3880 "glsl.parser.c"
    break;

  case 82:
#line 787 "glsl.y"
                                { (yyval.for_rest_statement) = new_glsl_node(context, FOR_REST_STATEMENT, (yyvsp[-2].conditionopt), (yyvsp[0].expression), NULL); }
#line 3886 "glsl.parser.c"
    break;

  case 83:
#line 791 "glsl.y"
                                { (yyval.jump_statement) = new_glsl_node(context, CONTINUE, NULL); }
#line 3892 "glsl.parser.c"
    break;

  case 84:
#line 794 "glsl.y"
                                { (yyval.jump_statement) = new_glsl_node(context, BREAK, NULL); }
#line 3898 "glsl.parser.c"
    break;

  case 85:
#line 797 "glsl.y"
                                { (yyval.jump_statement) = new_glsl_node(context, RETURN, NULL); }
#line 3904 "glsl.parser.c"
    break;

  case 86:
#line 800 "glsl.y"
                                { (yyval.jump_statement) = new_glsl_node(context, RETURN_VALUE, (yyvsp[-1].expression), NULL); }
#line 3910 "glsl.parser.c"
    break;

  case 87:
#line 803 "glsl.y"
                                { (yyval.jump_statement) = new_glsl_node(context, DISCARD, NULL); }
#line 3916 "glsl.parser.c"
    break;

  case 88:
#line 806 "glsl.y"
                                                          { (yyval.function_prototype) = (yyvsp[-1].function_declarator); }
#line 3922 "glsl.parser.c"
    break;

  case 89:
#line 810 "glsl.y"
                                { (yyval.function_declarator) = new_glsl_node(context, FUNCTION_DECLARATION,
					(yyvsp[0].function_header),
					new_glsl_node(context, FUNCTION_PARAMETER_LIST, NULL),
					NULL); }
#line 3931 "glsl.parser.c"
    break;

  case 90:
#line 816 "glsl.y"
                                { (yyval.function_declarator) = new_glsl_node(context, FUNCTION_DECLARATION,
					(yyvsp[-1].function_header),
					(yyvsp[0].function_parameter_list),
					NULL); }
#line 3940 "glsl.parser.c"
    break;

  case 91:
#line 823 "glsl.y"
                                { (yyval.function_parameter_list) = new_glsl_node(context, FUNCTION_PARAMETER_LIST, (yyvsp[0].parameter_declaration), NULL); }
#line 3946 "glsl.parser.c"
    break;

  case 92:
#line 826 "glsl.y"
                                { (yyval.function_parameter_list) = new_glsl_node(context, FUNCTION_PARAMETER_LIST, (yyvsp[-2].function_parameter_list), (yyvsp[0].parameter_declaration), NULL); }
#line 3952 "glsl.parser.c"
    break;

  case 93:
#line 830 "glsl.y"
                                { (yyval.parameter_declaration) = new_glsl_node(context, PARAMETER_DECLARATION, (yyvsp[-1].type_qualifier), (yyvsp[0].parameter_declarator), NULL); }
#line 3958 "glsl.parser.c"
    break;

  case 94:
#line 833 "glsl.y"
                                { (yyval.parameter_declaration) = new_glsl_node(context, PARAMETER_DECLARATION,
					new_glsl_node(context, TYPE_QUALIFIER_LIST, NULL),
					(yyvsp[0].parameter_declarator),
					NULL); }
#line 3967 "glsl.parser.c"
    break;

  case 95:
#line 839 "glsl.y"
                                { (yyval.parameter_declaration) = new_glsl_node(context, PARAMETER_DECLARATION, (yyvsp[-1].type_qualifier), (yyvsp[0].parameter_type_specifier), NULL); }
#line 3973 "glsl.parser.c"
    break;

  case 96:
#line 842 "glsl.y"
                                { (yyval.parameter_declaration) = new_glsl_node(context, PARAMETER_DECLARATION,
					new_glsl_node(context, TYPE_QUALIFIER_LIST, NULL),
					(yyvsp[0].parameter_type_specifier),
					NULL); }
#line 3982 "glsl.parser.c"
    break;

  case 97:
#line 849 "glsl.y"
                                { (yyval.parameter_declarator) = new_glsl_node(context, PARAMETER_DECLARATOR, (yyvsp[-1].type_specifier), (yyvsp[0].param_name), NULL); }
#line 3988 "glsl.parser.c"
    break;

  case 98:
#line 852 "glsl.y"
                                { (yyval.parameter_declarator) = new_glsl_node(context, PARAMETER_DECLARATOR, (yyvsp[-2].type_specifier), (yyvsp[-1].param_name), (yyvsp[0].array_specifier_list), NULL);}
#line 3994 "glsl.parser.c"
    break;

  case 99:
#line 856 "glsl.y"
                                { (yyval.function_header) = new_glsl_node(context, FUNCTION_HEADER, (yyvsp[-2].fully_specified_type), (yyvsp[-1].function_name), NULL); }
#line 4000 "glsl.parser.c"
    break;

  case 100:
#line 860 "glsl.y"
                                { (yyval.fully_specified_type) = new_glsl_node(context, FULLY_SPECIFIED_TYPE,
					new_glsl_node(context, TYPE_QUALIFIER_LIST, NULL),
					(yyvsp[0].type_specifier),
					NULL); }
#line 4009 "glsl.parser.c"
    break;

  case 101:
#line 866 "glsl.y"
                                { (yyval.fully_specified_type) = new_glsl_node(context, FULLY_SPECIFIED_TYPE, (yyvsp[-1].type_qualifier), (yyvsp[0].type_specifier), NULL); }
#line 4015 "glsl.parser.c"
    break;

  case 102:
#line 870 "glsl.y"
                                { (yyval.parameter_type_specifier) = new_glsl_node(context, PARAMETER_DECLARATOR, (yyvsp[0].type_specifier), NULL); }
#line 4021 "glsl.parser.c"
    break;

  case 103:
#line 874 "glsl.y"
                                { (yyval.type_specifier) = new_glsl_node(context, TYPE_SPECIFIER,
					(yyvsp[0].type_specifier_nonarray),
					new_glsl_node(context, ARRAY_SPECIFIER_LIST, NULL),
					NULL); }
#line 4030 "glsl.parser.c"
    break;

  case 104:
#line 880 "glsl.y"
                                { (yyval.type_specifier) = new_glsl_node(context, TYPE_SPECIFIER, (yyvsp[-1].type_specifier_nonarray), (yyvsp[0].array_specifier_list), NULL); }
#line 4036 "glsl.parser.c"
    break;

  case 105:
#line 884 "glsl.y"
                                { (yyval.array_specifier_list) = new_glsl_node(context, ARRAY_SPECIFIER_LIST, (yyvsp[0].array_specifier), NULL); }
#line 4042 "glsl.parser.c"
    break;

  case 106:
#line 887 "glsl.y"
                                { (yyval.array_specifier_list) = new_glsl_node(context, ARRAY_SPECIFIER_LIST, (yyvsp[-1].array_specifier_list), (yyvsp[0].array_specifier), NULL); }
#line 4048 "glsl.parser.c"
    break;

  case 107:
#line 891 "glsl.y"
                                { (yyval.array_specifier) = new_glsl_node(context, ARRAY_SPECIFIER, NULL); }
#line 4054 "glsl.parser.c"
    break;

  case 108:
#line 894 "glsl.y"
                                { (yyval.array_specifier) = new_glsl_node(context, ARRAY_SPECIFIER, (yyvsp[-1].constant_expression), NULL); }
#line 4060 "glsl.parser.c"
    break;

  case 109:
#line 897 "glsl.y"
                               { (yyval.type_specifier_nonarray) = new_glsl_node(context, VOID, NULL); }
#line 4066 "glsl.parser.c"
    break;

  case 110:
#line 898 "glsl.y"
                                { (yyval.type_specifier_nonarray) = new_glsl_node(context, FLOAT, NULL); }
#line 4072 "glsl.parser.c"
    break;

  case 111:
#line 899 "glsl.y"
                                 { (yyval.type_specifier_nonarray) = new_glsl_node(context, DOUBLE, NULL); }
#line 4078 "glsl.parser.c"
    break;

  case 112:
#line 900 "glsl.y"
                              { (yyval.type_specifier_nonarray) = new_glsl_node(context, INT, NULL); }
#line 4084 "glsl.parser.c"
    break;

  case 113:
#line 901 "glsl.y"
                               { (yyval.type_specifier_nonarray) = new_glsl_node(context, UINT, NULL); }
#line 4090 "glsl.parser.c"
    break;

  case 114:
#line 902 "glsl.y"
                               { (yyval.type_specifier_nonarray) = new_glsl_node(context, BOOL, NULL); }
#line 4096 "glsl.parser.c"
    break;

  case 115:
#line 903 "glsl.y"
                               { (yyval.type_specifier_nonarray) = new_glsl_node(context, VEC2, NULL); }
#line 4102 "glsl.parser.c"
    break;

  case 116:
#line 904 "glsl.y"
                               { (yyval.type_specifier_nonarray) = new_glsl_node(context, VEC3, NULL); }
#line 4108 "glsl.parser.c"
    break;

  case 117:
#line 905 "glsl.y"
                               { (yyval.type_specifier_nonarray) = new_glsl_node(context, VEC4, NULL); }
#line 4114 "glsl.parser.c"
    break;

  case 118:
#line 906 "glsl.y"
                                { (yyval.type_specifier_nonarray) = new_glsl_node(context, DVEC2, NULL); }
#line 4120 "glsl.parser.c"
    break;

  case 119:
#line 907 "glsl.y"
                                { (yyval.type_specifier_nonarray) = new_glsl_node(context, DVEC3, NULL); }
#line 4126 "glsl.parser.c"
    break;

  case 120:
#line 908 "glsl.y"
                                { (yyval.type_specifier_nonarray) = new_glsl_node(context, DVEC4, NULL); }
#line 4132 "glsl.parser.c"
    break;

  case 121:
#line 909 "glsl.y"
                                { (yyval.type_specifier_nonarray) = new_glsl_node(context, BVEC2, NULL); }
#line 4138 "glsl.parser.c"
    break;

  case 122:
#line 910 "glsl.y"
                                { (yyval.type_specifier_nonarray) = new_glsl_node(context, BVEC3, NULL); }
#line 4144 "glsl.parser.c"
    break;

  case 123:
#line 911 "glsl.y"
                                { (yyval.type_specifier_nonarray) = new_glsl_node(context, BVEC4, NULL); }
#line 4150 "glsl.parser.c"
    break;

  case 124:
#line 912 "glsl.y"
                                { (yyval.type_specifier_nonarray) = new_glsl_node(context, IVEC2, NULL); }
#line 4156 "glsl.parser.c"
    break;

  case 125:
#line 913 "glsl.y"
                                { (yyval.type_specifier_nonarray) = new_glsl_node(context, IVEC3, NULL); }
#line 4162 "glsl.parser.c"
    break;

  case 126:
#line 914 "glsl.y"
                                { (yyval.type_specifier_nonarray) = new_glsl_node(context, IVEC4, NULL); }
#line 4168 "glsl.parser.c"
    break;

  case 127:
#line 915 "glsl.y"
                                { (yyval.type_specifier_nonarray) = new_glsl_node(context, UVEC2, NULL); }
#line 4174 "glsl.parser.c"
    break;

  case 128:
#line 916 "glsl.y"
                                { (yyval.type_specifier_nonarray) = new_glsl_node(context, UVEC3, NULL); }
#line 4180 "glsl.parser.c"
    break;

  case 129:
#line 917 "glsl.y"
                                { (yyval.type_specifier_nonarray) = new_glsl_node(context, UVEC4, NULL); }
#line 4186 "glsl.parser.c"
    break;

  case 130:
#line 918 "glsl.y"
                               { (yyval.type_specifier_nonarray) = new_glsl_node(context, MAT2, NULL); }
#line 4192 "glsl.parser.c"
    break;

  case 131:
#line 919 "glsl.y"
                               { (yyval.type_specifier_nonarray) = new_glsl_node(context, MAT3, NULL); }
#line 4198 "glsl.parser.c"
    break;

  case 132:
#line 920 "glsl.y"
                               { (yyval.type_specifier_nonarray) = new_glsl_node(context, MAT4, NULL); }
#line 4204 "glsl.parser.c"
    break;

  case 133:
#line 921 "glsl.y"
                                 { (yyval.type_specifier_nonarray) = new_glsl_node(context, MAT2X2, NULL); }
#line 4210 "glsl.parser.c"
    break;

  case 134:
#line 922 "glsl.y"
                                 { (yyval.type_specifier_nonarray) = new_glsl_node(context, MAT2X3, NULL); }
#line 4216 "glsl.parser.c"
    break;

  case 135:
#line 923 "glsl.y"
                                 { (yyval.type_specifier_nonarray) = new_glsl_node(context, MAT2X4, NULL); }
#line 4222 "glsl.parser.c"
    break;

  case 136:
#line 924 "glsl.y"
                                 { (yyval.type_specifier_nonarray) = new_glsl_node(context, MAT3X2, NULL); }
#line 4228 "glsl.parser.c"
    break;

  case 137:
#line 925 "glsl.y"
                                 { (yyval.type_specifier_nonarray) = new_glsl_node(context, MAT3X3, NULL); }
#line 4234 "glsl.parser.c"
    break;

  case 138:
#line 926 "glsl.y"
                                 { (yyval.type_specifier_nonarray) = new_glsl_node(context, MAT3X4, NULL); }
#line 4240 "glsl.parser.c"
    break;

  case 139:
#line 927 "glsl.y"
                                 { (yyval.type_specifier_nonarray) = new_glsl_node(context, MAT4X2, NULL); }
#line 4246 "glsl.parser.c"
    break;

  case 140:
#line 928 "glsl.y"
                                 { (yyval.type_specifier_nonarray) = new_glsl_node(context, MAT4X3, NULL); }
#line 4252 "glsl.parser.c"
    break;

  case 141:
#line 929 "glsl.y"
                                 { (yyval.type_specifier_nonarray) = new_glsl_node(context, MAT4X4, NULL); }
#line 4258 "glsl.parser.c"
    break;

  case 142:
#line 930 "glsl.y"
                                { (yyval.type_specifier_nonarray) = new_glsl_node(context, DMAT2, NULL); }
#line 4264 "glsl.parser.c"
    break;

  case 143:
#line 931 "glsl.y"
                                { (yyval.type_specifier_nonarray) = new_glsl_node(context, DMAT3, NULL); }
#line 4270 "glsl.parser.c"
    break;

  case 144:
#line 932 "glsl.y"
                                { (yyval.type_specifier_nonarray) = new_glsl_node(context, DMAT4, NULL); }
#line 4276 "glsl.parser.c"
    break;

  case 145:
#line 933 "glsl.y"
                                  { (yyval.type_specifier_nonarray) = new_glsl_node(context, DMAT2X2, NULL); }
#line 4282 "glsl.parser.c"
    break;

  case 146:
#line 934 "glsl.y"
                                  { (yyval.type_specifier_nonarray) = new_glsl_node(context, DMAT2X3, NULL); }
#line 4288 "glsl.parser.c"
    break;

  case 147:
#line 935 "glsl.y"
                                  { (yyval.type_specifier_nonarray) = new_glsl_node(context, DMAT2X4, NULL); }
#line 4294 "glsl.parser.c"
    break;

  case 148:
#line 936 "glsl.y"
                                  { (yyval.type_specifier_nonarray) = new_glsl_node(context, DMAT3X2, NULL); }
#line 4300 "glsl.parser.c"
    break;

  case 149:
#line 937 "glsl.y"
                                  { (yyval.type_specifier_nonarray) = new_glsl_node(context, DMAT3X3, NULL); }
#line 4306 "glsl.parser.c"
    break;

  case 150:
#line 938 "glsl.y"
                                  { (yyval.type_specifier_nonarray) = new_glsl_node(context, DMAT3X4, NULL); }
#line 4312 "glsl.parser.c"
    break;

  case 151:
#line 939 "glsl.y"
                                  { (yyval.type_specifier_nonarray) = new_glsl_node(context, DMAT4X2, NULL); }
#line 4318 "glsl.parser.c"
    break;

  case 152:
#line 940 "glsl.y"
                                  { (yyval.type_specifier_nonarray) = new_glsl_node(context, DMAT4X3, NULL); }
#line 4324 "glsl.parser.c"
    break;

  case 153:
#line 941 "glsl.y"
                                  { (yyval.type_specifier_nonarray) = new_glsl_node(context, DMAT4X4, NULL); }
#line 4330 "glsl.parser.c"
    break;

  case 154:
#line 942 "glsl.y"
                                      { (yyval.type_specifier_nonarray) = new_glsl_node(context, UINT, NULL); }
#line 4336 "glsl.parser.c"
    break;

  case 155:
#line 943 "glsl.y"
                                    { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLER1D, NULL); }
#line 4342 "glsl.parser.c"
    break;

  case 156:
#line 944 "glsl.y"
                                    { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLER2D, NULL); }
#line 4348 "glsl.parser.c"
    break;

  case 157:
#line 945 "glsl.y"
                                    { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLER3D, NULL); }
#line 4354 "glsl.parser.c"
    break;

  case 158:
#line 946 "glsl.y"
                                      { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLERCUBE, NULL); }
#line 4360 "glsl.parser.c"
    break;

  case 159:
#line 947 "glsl.y"
                                          { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLER1DSHADOW, NULL); }
#line 4366 "glsl.parser.c"
    break;

  case 160:
#line 948 "glsl.y"
                                          { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLER2DSHADOW, NULL); }
#line 4372 "glsl.parser.c"
    break;

  case 161:
#line 949 "glsl.y"
                                            { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLERCUBESHADOW, NULL); }
#line 4378 "glsl.parser.c"
    break;

  case 162:
#line 950 "glsl.y"
                                         { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLER1DARRAY, NULL); }
#line 4384 "glsl.parser.c"
    break;

  case 163:
#line 951 "glsl.y"
                                         { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLER2DARRAY, NULL); }
#line 4390 "glsl.parser.c"
    break;

  case 164:
#line 952 "glsl.y"
                                               { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLER1DARRAYSHADOW, NULL); }
#line 4396 "glsl.parser.c"
    break;

  case 165:
#line 953 "glsl.y"
                                               { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLER2DARRAYSHADOW, NULL); }
#line 4402 "glsl.parser.c"
    break;

  case 166:
#line 954 "glsl.y"
                                           { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLERCUBEARRAY, NULL); }
#line 4408 "glsl.parser.c"
    break;

  case 167:
#line 955 "glsl.y"
                                                 { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLERCUBEARRAYSHADOW, NULL); }
#line 4414 "glsl.parser.c"
    break;

  case 168:
#line 956 "glsl.y"
                                     { (yyval.type_specifier_nonarray) = new_glsl_node(context, ISAMPLER1D, NULL); }
#line 4420 "glsl.parser.c"
    break;

  case 169:
#line 957 "glsl.y"
                                     { (yyval.type_specifier_nonarray) = new_glsl_node(context, ISAMPLER2D, NULL); }
#line 4426 "glsl.parser.c"
    break;

  case 170:
#line 958 "glsl.y"
                                     { (yyval.type_specifier_nonarray) = new_glsl_node(context, ISAMPLER3D, NULL); }
#line 4432 "glsl.parser.c"
    break;

  case 171:
#line 959 "glsl.y"
                                       { (yyval.type_specifier_nonarray) = new_glsl_node(context, ISAMPLERCUBE, NULL); }
#line 4438 "glsl.parser.c"
    break;

  case 172:
#line 960 "glsl.y"
                                          { (yyval.type_specifier_nonarray) = new_glsl_node(context, ISAMPLER1DARRAY, NULL); }
#line 4444 "glsl.parser.c"
    break;

  case 173:
#line 961 "glsl.y"
                                          { (yyval.type_specifier_nonarray) = new_glsl_node(context, ISAMPLER2DARRAY, NULL); }
#line 4450 "glsl.parser.c"
    break;

  case 174:
#line 962 "glsl.y"
                                            { (yyval.type_specifier_nonarray) = new_glsl_node(context, ISAMPLERCUBEARRAY, NULL); }
#line 4456 "glsl.parser.c"
    break;

  case 175:
#line 963 "glsl.y"
                                     { (yyval.type_specifier_nonarray) = new_glsl_node(context, USAMPLER1D, NULL); }
#line 4462 "glsl.parser.c"
    break;

  case 176:
#line 964 "glsl.y"
                                     { (yyval.type_specifier_nonarray) = new_glsl_node(context, USAMPLER2D, NULL); }
#line 4468 "glsl.parser.c"
    break;

  case 177:
#line 965 "glsl.y"
                                     { (yyval.type_specifier_nonarray) = new_glsl_node(context, USAMPLER3D, NULL); }
#line 4474 "glsl.parser.c"
    break;

  case 178:
#line 966 "glsl.y"
                                       { (yyval.type_specifier_nonarray) = new_glsl_node(context, USAMPLERCUBE, NULL); }
#line 4480 "glsl.parser.c"
    break;

  case 179:
#line 967 "glsl.y"
                                          { (yyval.type_specifier_nonarray) = new_glsl_node(context, USAMPLER1DARRAY, NULL); }
#line 4486 "glsl.parser.c"
    break;

  case 180:
#line 968 "glsl.y"
                                          { (yyval.type_specifier_nonarray) = new_glsl_node(context, USAMPLER2DARRAY, NULL); }
#line 4492 "glsl.parser.c"
    break;

  case 181:
#line 969 "glsl.y"
                                            { (yyval.type_specifier_nonarray) = new_glsl_node(context, USAMPLERCUBEARRAY, NULL); }
#line 4498 "glsl.parser.c"
    break;

  case 182:
#line 970 "glsl.y"
                                        { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLER2DRECT, NULL); }
#line 4504 "glsl.parser.c"
    break;

  case 183:
#line 971 "glsl.y"
                                              { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLER2DRECTSHADOW, NULL); }
#line 4510 "glsl.parser.c"
    break;

  case 184:
#line 972 "glsl.y"
                                         { (yyval.type_specifier_nonarray) = new_glsl_node(context, ISAMPLER2DRECT, NULL); }
#line 4516 "glsl.parser.c"
    break;

  case 185:
#line 973 "glsl.y"
                                         { (yyval.type_specifier_nonarray) = new_glsl_node(context, USAMPLER2DRECT, NULL); }
#line 4522 "glsl.parser.c"
    break;

  case 186:
#line 974 "glsl.y"
                                        { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLERBUFFER, NULL); }
#line 4528 "glsl.parser.c"
    break;

  case 187:
#line 975 "glsl.y"
                                         { (yyval.type_specifier_nonarray) = new_glsl_node(context, ISAMPLERBUFFER, NULL); }
#line 4534 "glsl.parser.c"
    break;

  case 188:
#line 976 "glsl.y"
                                         { (yyval.type_specifier_nonarray) = new_glsl_node(context, USAMPLERBUFFER, NULL); }
#line 4540 "glsl.parser.c"
    break;

  case 189:
#line 977 "glsl.y"
                                      { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLER2DMS, NULL); }
#line 4546 "glsl.parser.c"
    break;

  case 190:
#line 978 "glsl.y"
                                       { (yyval.type_specifier_nonarray) = new_glsl_node(context, ISAMPLER2DMS, NULL); }
#line 4552 "glsl.parser.c"
    break;

  case 191:
#line 979 "glsl.y"
                                       { (yyval.type_specifier_nonarray) = new_glsl_node(context, USAMPLER2DMS, NULL); }
#line 4558 "glsl.parser.c"
    break;

  case 192:
#line 980 "glsl.y"
                                           { (yyval.type_specifier_nonarray) = new_glsl_node(context, SAMPLER2DMSARRAY, NULL); }
#line 4564 "glsl.parser.c"
    break;

  case 193:
#line 981 "glsl.y"
                                            { (yyval.type_specifier_nonarray) = new_glsl_node(context, ISAMPLER2DMSARRAY, NULL); }
#line 4570 "glsl.parser.c"
    break;

  case 194:
#line 982 "glsl.y"
                                            { (yyval.type_specifier_nonarray) = new_glsl_node(context, USAMPLER2DMSARRAY, NULL); }
#line 4576 "glsl.parser.c"
    break;

  case 195:
#line 983 "glsl.y"
                                  { (yyval.type_specifier_nonarray) = new_glsl_node(context, IMAGE1D, NULL); }
#line 4582 "glsl.parser.c"
    break;

  case 196:
#line 984 "glsl.y"
                                   { (yyval.type_specifier_nonarray) = new_glsl_node(context, IIMAGE1D, NULL); }
#line 4588 "glsl.parser.c"
    break;

  case 197:
#line 985 "glsl.y"
                                   { (yyval.type_specifier_nonarray) = new_glsl_node(context, UIMAGE1D, NULL); }
#line 4594 "glsl.parser.c"
    break;

  case 198:
#line 986 "glsl.y"
                                  { (yyval.type_specifier_nonarray) = new_glsl_node(context, IMAGE2D, NULL); }
#line 4600 "glsl.parser.c"
    break;

  case 199:
#line 987 "glsl.y"
                                   { (yyval.type_specifier_nonarray) = new_glsl_node(context, IIMAGE2D, NULL); }
#line 4606 "glsl.parser.c"
    break;

  case 200:
#line 988 "glsl.y"
                                   { (yyval.type_specifier_nonarray) = new_glsl_node(context, UIMAGE2D, NULL); }
#line 4612 "glsl.parser.c"
    break;

  case 201:
#line 989 "glsl.y"
                                  { (yyval.type_specifier_nonarray) = new_glsl_node(context, IMAGE3D, NULL); }
#line 4618 "glsl.parser.c"
    break;

  case 202:
#line 990 "glsl.y"
                                   { (yyval.type_specifier_nonarray) = new_glsl_node(context, IIMAGE3D, NULL); }
#line 4624 "glsl.parser.c"
    break;

  case 203:
#line 991 "glsl.y"
                                   { (yyval.type_specifier_nonarray) = new_glsl_node(context, UIMAGE3D, NULL); }
#line 4630 "glsl.parser.c"
    break;

  case 204:
#line 992 "glsl.y"
                                      { (yyval.type_specifier_nonarray) = new_glsl_node(context, IMAGE2DRECT, NULL); }
#line 4636 "glsl.parser.c"
    break;

  case 205:
#line 993 "glsl.y"
                                       { (yyval.type_specifier_nonarray) = new_glsl_node(context, IIMAGE2DRECT, NULL); }
#line 4642 "glsl.parser.c"
    break;

  case 206:
#line 994 "glsl.y"
                                       { (yyval.type_specifier_nonarray) = new_glsl_node(context, UIMAGE2DRECT, NULL); }
#line 4648 "glsl.parser.c"
    break;

  case 207:
#line 995 "glsl.y"
                                    { (yyval.type_specifier_nonarray) = new_glsl_node(context, IMAGECUBE, NULL); }
#line 4654 "glsl.parser.c"
    break;

  case 208:
#line 996 "glsl.y"
                                     { (yyval.type_specifier_nonarray) = new_glsl_node(context, IIMAGECUBE, NULL); }
#line 4660 "glsl.parser.c"
    break;

  case 209:
#line 997 "glsl.y"
                                     { (yyval.type_specifier_nonarray) = new_glsl_node(context, UIMAGECUBE, NULL); }
#line 4666 "glsl.parser.c"
    break;

  case 210:
#line 998 "glsl.y"
                                      { (yyval.type_specifier_nonarray) = new_glsl_node(context, IMAGEBUFFER, NULL); }
#line 4672 "glsl.parser.c"
    break;

  case 211:
#line 999 "glsl.y"
                                       { (yyval.type_specifier_nonarray) = new_glsl_node(context, IIMAGEBUFFER, NULL); }
#line 4678 "glsl.parser.c"
    break;

  case 212:
#line 1000 "glsl.y"
                                       { (yyval.type_specifier_nonarray) = new_glsl_node(context, UIMAGEBUFFER, NULL); }
#line 4684 "glsl.parser.c"
    break;

  case 213:
#line 1001 "glsl.y"
                                       { (yyval.type_specifier_nonarray) = new_glsl_node(context, IMAGE1DARRAY, NULL); }
#line 4690 "glsl.parser.c"
    break;

  case 214:
#line 1002 "glsl.y"
                                        { (yyval.type_specifier_nonarray) = new_glsl_node(context, IIMAGE1DARRAY, NULL); }
#line 4696 "glsl.parser.c"
    break;

  case 215:
#line 1003 "glsl.y"
                                        { (yyval.type_specifier_nonarray) = new_glsl_node(context, UIMAGE1DARRAY, NULL); }
#line 4702 "glsl.parser.c"
    break;

  case 216:
#line 1004 "glsl.y"
                                       { (yyval.type_specifier_nonarray) = new_glsl_node(context, IMAGE2DARRAY, NULL); }
#line 4708 "glsl.parser.c"
    break;

  case 217:
#line 1005 "glsl.y"
                                        { (yyval.type_specifier_nonarray) = new_glsl_node(context, IIMAGE2DARRAY, NULL); }
#line 4714 "glsl.parser.c"
    break;

  case 218:
#line 1006 "glsl.y"
                                        { (yyval.type_specifier_nonarray) = new_glsl_node(context, UIMAGE2DARRAY, NULL); }
#line 4720 "glsl.parser.c"
    break;

  case 219:
#line 1007 "glsl.y"
                                         { (yyval.type_specifier_nonarray) = new_glsl_node(context, IMAGECUBEARRAY, NULL); }
#line 4726 "glsl.parser.c"
    break;

  case 220:
#line 1008 "glsl.y"
                                          { (yyval.type_specifier_nonarray) = new_glsl_node(context, IIMAGECUBEARRAY, NULL); }
#line 4732 "glsl.parser.c"
    break;

  case 221:
#line 1009 "glsl.y"
                                          { (yyval.type_specifier_nonarray) = new_glsl_node(context, UIMAGECUBEARRAY, NULL); }
#line 4738 "glsl.parser.c"
    break;

  case 222:
#line 1010 "glsl.y"
                                    { (yyval.type_specifier_nonarray) = new_glsl_node(context, IMAGE2DMS, NULL); }
#line 4744 "glsl.parser.c"
    break;

  case 223:
#line 1011 "glsl.y"
                                     { (yyval.type_specifier_nonarray) = new_glsl_node(context, IIMAGE2DMS, NULL); }
#line 4750 "glsl.parser.c"
    break;

  case 224:
#line 1012 "glsl.y"
                                     { (yyval.type_specifier_nonarray) = new_glsl_node(context, UIMAGE2DMS, NULL); }
#line 4756 "glsl.parser.c"
    break;

  case 225:
#line 1013 "glsl.y"
                                         { (yyval.type_specifier_nonarray) = new_glsl_node(context, IMAGE2DMSARRAY, NULL); }
#line 4762 "glsl.parser.c"
    break;

  case 226:
#line 1014 "glsl.y"
                                          { (yyval.type_specifier_nonarray) = new_glsl_node(context, IIMAGE2DMSARRAY, NULL); }
#line 4768 "glsl.parser.c"
    break;

  case 227:
#line 1015 "glsl.y"
                                          { (yyval.type_specifier_nonarray) = new_glsl_node(context, UIMAGE2DMSARRAY, NULL); }
#line 4774 "glsl.parser.c"
    break;

  case 228:
#line 1016 "glsl.y"
                                           { (yyval.type_specifier_nonarray) = (yyvsp[0].struct_specifier); }
#line 4780 "glsl.parser.c"
    break;

  case 229:
#line 1017 "glsl.y"
                                                    { (yyval.type_specifier_nonarray) = (yyvsp[0].type_specifier_identifier); }
#line 4786 "glsl.parser.c"
    break;

  case 230:
#line 1021 "glsl.y"
                                { (yyval.struct_specifier) = new_glsl_node(context, STRUCT_SPECIFIER, (yyvsp[-3].struct_name), (yyvsp[-1].struct_declaration_list), NULL);}
#line 4792 "glsl.parser.c"
    break;

  case 231:
#line 1024 "glsl.y"
                                { (yyval.struct_specifier) = new_glsl_node(context, STRUCT_SPECIFIER,
						new_glsl_identifier(context, NULL),
						(yyvsp[-1].struct_declaration_list),
						NULL); }
#line 4801 "glsl.parser.c"
    break;

  case 232:
#line 1031 "glsl.y"
                                { (yyval.struct_declaration_list) = new_glsl_node(context, STRUCT_DECLARATION_LIST, (yyvsp[0].struct_declaration), NULL); }
#line 4807 "glsl.parser.c"
    break;

  case 233:
#line 1033 "glsl.y"
                                { (yyval.struct_declaration_list) = new_glsl_node(context, STRUCT_DECLARATION_LIST, (yyvsp[-1].struct_declaration_list), (yyvsp[0].struct_declaration), NULL); }
#line 4813 "glsl.parser.c"
    break;

  case 234:
#line 1037 "glsl.y"
                                { (yyval.struct_declaration) = new_glsl_node(context, STRUCT_DECLARATION,
					new_glsl_node(context, TYPE_QUALIFIER_LIST, NULL),
					(yyvsp[-2].type_specifier),
					(yyvsp[-1].struct_declarator_list),
					NULL); }
#line 4823 "glsl.parser.c"
    break;

  case 235:
#line 1044 "glsl.y"
                                { (yyval.struct_declaration) = new_glsl_node(context, STRUCT_DECLARATION, (yyvsp[-3].type_qualifier), (yyvsp[-2].type_specifier), (yyvsp[-1].struct_declarator_list), NULL); }
#line 4829 "glsl.parser.c"
    break;

  case 236:
#line 1048 "glsl.y"
                                { (yyval.struct_declarator_list) = new_glsl_node(context, STRUCT_DECLARATOR_LIST, (yyvsp[0].struct_declarator), NULL); }
#line 4835 "glsl.parser.c"
    break;

  case 237:
#line 1051 "glsl.y"
                                { (yyval.struct_declarator_list) = new_glsl_node(context, STRUCT_DECLARATOR_LIST, (yyvsp[-2].struct_declarator_list), (yyvsp[0].struct_declarator), NULL); }
#line 4841 "glsl.parser.c"
    break;

  case 238:
#line 1055 "glsl.y"
                                { (yyval.struct_declarator) = new_glsl_node(context, STRUCT_DECLARATOR, (yyvsp[0].field_identifier), NULL); }
#line 4847 "glsl.parser.c"
    break;

  case 239:
#line 1058 "glsl.y"
                                { (yyval.struct_declarator) = new_glsl_node(context, STRUCT_DECLARATOR, (yyvsp[-1].field_identifier), (yyvsp[0].array_specifier_list), NULL); }
#line 4853 "glsl.parser.c"
    break;

  case 240:
#line 1062 "glsl.y"
                                { (yyval.type_qualifier) = new_glsl_node(context, TYPE_QUALIFIER_LIST, (yyvsp[0].single_type_qualifier), NULL); }
#line 4859 "glsl.parser.c"
    break;

  case 241:
#line 1064 "glsl.y"
                                { (yyval.type_qualifier) = new_glsl_node(context, TYPE_QUALIFIER_LIST, (yyvsp[-1].type_qualifier), (yyvsp[0].single_type_qualifier), NULL); }
#line 4865 "glsl.parser.c"
    break;

  case 242:
#line 1067 "glsl.y"
                                            { (yyval.single_type_qualifier) = (yyvsp[0].storage_qualifier); }
#line 4871 "glsl.parser.c"
    break;

  case 243:
#line 1068 "glsl.y"
                                           { (yyval.single_type_qualifier) = (yyvsp[0].layout_qualifier); }
#line 4877 "glsl.parser.c"
    break;

  case 244:
#line 1069 "glsl.y"
                                              { (yyval.single_type_qualifier) = (yyvsp[0].precision_qualifier); }
#line 4883 "glsl.parser.c"
    break;

  case 245:
#line 1070 "glsl.y"
                                                  { (yyval.single_type_qualifier) = (yyvsp[0].interpolation_qualifier); }
#line 4889 "glsl.parser.c"
    break;

  case 246:
#line 1071 "glsl.y"
                                              { (yyval.single_type_qualifier) = (yyvsp[0].invariant_qualifier); }
#line 4895 "glsl.parser.c"
    break;

  case 247:
#line 1072 "glsl.y"
                                            { (yyval.single_type_qualifier) = (yyvsp[0].precise_qualifier); }
#line 4901 "glsl.parser.c"
    break;

  case 248:
#line 1075 "glsl.y"
                                                                                 { (yyval.layout_qualifier) = (yyvsp[-1].layout_qualifier_id_list); }
#line 4907 "glsl.parser.c"
    break;

  case 249:
#line 1078 "glsl.y"
                                              { (yyval.layout_qualifier_id_list) = (yyvsp[0].layout_qualifier_id); }
#line 4913 "glsl.parser.c"
    break;

  case 250:
#line 1081 "glsl.y"
                                { (yyval.layout_qualifier_id_list) = new_glsl_node(context, LAYOUT_QUALIFIER_ID_LIST, (yyvsp[-2].layout_qualifier_id_list), (yyvsp[0].layout_qualifier_id), NULL); }
#line 4919 "glsl.parser.c"
    break;

  case 251:
#line 1085 "glsl.y"
                                { (yyval.layout_qualifier_id) = new_glsl_node(context, LAYOUT_QUALIFIER_ID, (yyvsp[0].layout_identifier), NULL); }
#line 4925 "glsl.parser.c"
    break;

  case 252:
#line 1088 "glsl.y"
                                { (yyval.layout_qualifier_id) = new_glsl_node(context, LAYOUT_QUALIFIER_ID, (yyvsp[-2].layout_identifier), (yyvsp[0].constant_expression), NULL);}
#line 4931 "glsl.parser.c"
    break;

  case 253:
#line 1091 "glsl.y"
                                { (yyval.layout_qualifier_id) = new_glsl_node(context, SHARED, NULL); }
#line 4937 "glsl.parser.c"
    break;

  case 254:
#line 1094 "glsl.y"
                                { (yyval.precision_qualifier) = new_glsl_node(context, HIGHP, NULL); }
#line 4943 "glsl.parser.c"
    break;

  case 255:
#line 1095 "glsl.y"
                                  { (yyval.precision_qualifier) = new_glsl_node(context, MEDIUMP, NULL); }
#line 4949 "glsl.parser.c"
    break;

  case 256:
#line 1096 "glsl.y"
                               { (yyval.precision_qualifier) = new_glsl_node(context, LOWP, NULL); }
#line 4955 "glsl.parser.c"
    break;

  case 257:
#line 1099 "glsl.y"
                                 { (yyval.interpolation_qualifier) = new_glsl_node(context, SMOOTH, NULL); }
#line 4961 "glsl.parser.c"
    break;

  case 258:
#line 1100 "glsl.y"
                               { (yyval.interpolation_qualifier) = new_glsl_node(context, FLAT, NULL); }
#line 4967 "glsl.parser.c"
    break;

  case 259:
#line 1101 "glsl.y"
                                        { (yyval.interpolation_qualifier) = new_glsl_node(context, NOPERSPECTIVE, NULL); }
#line 4973 "glsl.parser.c"
    break;

  case 260:
#line 1104 "glsl.y"
                                    { (yyval.invariant_qualifier) = new_glsl_node(context, INVARIANT, NULL); }
#line 4979 "glsl.parser.c"
    break;

  case 261:
#line 1107 "glsl.y"
                                  { (yyval.precise_qualifier) = new_glsl_node(context, PRECISE, NULL); }
#line 4985 "glsl.parser.c"
    break;

  case 262:
#line 1110 "glsl.y"
                                { (yyval.storage_qualifier) = new_glsl_node(context, CONST, NULL); }
#line 4991 "glsl.parser.c"
    break;

  case 263:
#line 1111 "glsl.y"
                                { (yyval.storage_qualifier) = new_glsl_node(context, INOUT, NULL); }
#line 4997 "glsl.parser.c"
    break;

  case 264:
#line 1112 "glsl.y"
                             { (yyval.storage_qualifier) = new_glsl_node(context, IN, NULL); }
#line 5003 "glsl.parser.c"
    break;

  case 265:
#line 1113 "glsl.y"
                              { (yyval.storage_qualifier) = new_glsl_node(context, OUT, NULL); }
#line 5009 "glsl.parser.c"
    break;

  case 266:
#line 1114 "glsl.y"
                                   { (yyval.storage_qualifier) = new_glsl_node(context, CENTROID, NULL); }
#line 5015 "glsl.parser.c"
    break;

  case 267:
#line 1115 "glsl.y"
                                { (yyval.storage_qualifier) = new_glsl_node(context, PATCH, NULL); }
#line 5021 "glsl.parser.c"
    break;

  case 268:
#line 1116 "glsl.y"
                                 { (yyval.storage_qualifier) = new_glsl_node(context, SAMPLE, NULL); }
#line 5027 "glsl.parser.c"
    break;

  case 269:
#line 1117 "glsl.y"
                                  { (yyval.storage_qualifier) = new_glsl_node(context, UNIFORM, NULL); }
#line 5033 "glsl.parser.c"
    break;

  case 270:
#line 1118 "glsl.y"
                                 { (yyval.storage_qualifier) = new_glsl_node(context, BUFFER, NULL); }
#line 5039 "glsl.parser.c"
    break;

  case 271:
#line 1119 "glsl.y"
                                 { (yyval.storage_qualifier) = new_glsl_node(context, SHARED, NULL); }
#line 5045 "glsl.parser.c"
    break;

  case 272:
#line 1120 "glsl.y"
                                   { (yyval.storage_qualifier) = new_glsl_node(context, COHERENT, NULL); }
#line 5051 "glsl.parser.c"
    break;

  case 273:
#line 1121 "glsl.y"
                                   { (yyval.storage_qualifier) = new_glsl_node(context, VOLATILE, NULL); }
#line 5057 "glsl.parser.c"
    break;

  case 274:
#line 1122 "glsl.y"
                                   { (yyval.storage_qualifier) = new_glsl_node(context, RESTRICT, NULL); }
#line 5063 "glsl.parser.c"
    break;

  case 275:
#line 1123 "glsl.y"
                                   { (yyval.storage_qualifier) = new_glsl_node(context, READONLY, NULL); }
#line 5069 "glsl.parser.c"
    break;

  case 276:
#line 1124 "glsl.y"
                                    { (yyval.storage_qualifier) = new_glsl_node(context, WRITEONLY, NULL); }
#line 5075 "glsl.parser.c"
    break;

  case 277:
#line 1125 "glsl.y"
                                     { (yyval.storage_qualifier) = new_glsl_node(context, SUBROUTINE, NULL); }
#line 5081 "glsl.parser.c"
    break;

  case 278:
#line 1127 "glsl.y"
                                { (yyval.storage_qualifier) = new_glsl_node(context, SUBROUTINE_TYPE,
					new_glsl_node(context, TYPE_NAME_LIST, (yyvsp[-1].type_name_list), NULL),
					NULL); }
#line 5089 "glsl.parser.c"
    break;

  case 279:
#line 1132 "glsl.y"
                                    { (yyval.type_name_list) = (yyvsp[0].type_name); }
#line 5095 "glsl.parser.c"
    break;

  case 280:
#line 1134 "glsl.y"
                                { (yyval.type_name_list) = new_glsl_node(context, TYPE_NAME_LIST, (yyvsp[-2].type_name_list), (yyvsp[0].type_name), NULL); }
#line 5101 "glsl.parser.c"
    break;

  case 281:
#line 1137 "glsl.y"
                                                { (yyval.expression) = (yyvsp[0].assignment_expression); }
#line 5107 "glsl.parser.c"
    break;

  case 282:
#line 1139 "glsl.y"
                                { (yyval.expression) = new_glsl_node(context, COMMA, (yyvsp[-2].expression), (yyvsp[0].assignment_expression), NULL); }
#line 5113 "glsl.parser.c"
    break;

  case 283:
#line 1142 "glsl.y"
                                                 { (yyval.assignment_expression) = (yyvsp[0].conditional_expression); }
#line 5119 "glsl.parser.c"
    break;

  case 284:
#line 1144 "glsl.y"
                                { (yyval.assignment_expression) = new_glsl_node(context,(yyvsp[-1].assignment_operator), (yyvsp[-2].unary_expression), (yyvsp[0].assignment_expression), NULL); }
#line 5125 "glsl.parser.c"
    break;

  case 285:
#line 1147 "glsl.y"
                                { (yyval.assignment_operator) = EQUAL; }
#line 5131 "glsl.parser.c"
    break;

  case 286:
#line 1148 "glsl.y"
                                     { (yyval.assignment_operator) = MUL_ASSIGN; }
#line 5137 "glsl.parser.c"
    break;

  case 287:
#line 1149 "glsl.y"
                                     { (yyval.assignment_operator) = DIV_ASSIGN; }
#line 5143 "glsl.parser.c"
    break;

  case 288:
#line 1150 "glsl.y"
                                     { (yyval.assignment_operator) = MOD_ASSIGN; }
#line 5149 "glsl.parser.c"
    break;

  case 289:
#line 1151 "glsl.y"
                                     { (yyval.assignment_operator) = ADD_ASSIGN; }
#line 5155 "glsl.parser.c"
    break;

  case 290:
#line 1152 "glsl.y"
                                     { (yyval.assignment_operator) = SUB_ASSIGN; }
#line 5161 "glsl.parser.c"
    break;

  case 291:
#line 1153 "glsl.y"
                                      { (yyval.assignment_operator) = LEFT_ASSIGN; }
#line 5167 "glsl.parser.c"
    break;

  case 292:
#line 1154 "glsl.y"
                                       { (yyval.assignment_operator) = RIGHT_ASSIGN; }
#line 5173 "glsl.parser.c"
    break;

  case 293:
#line 1155 "glsl.y"
                                     { (yyval.assignment_operator) = AND_ASSIGN; }
#line 5179 "glsl.parser.c"
    break;

  case 294:
#line 1156 "glsl.y"
                                     { (yyval.assignment_operator) = XOR_ASSIGN; }
#line 5185 "glsl.parser.c"
    break;

  case 295:
#line 1157 "glsl.y"
                                    { (yyval.assignment_operator) = OR_ASSIGN; }
#line 5191 "glsl.parser.c"
    break;

  case 296:
#line 1160 "glsl.y"
                                                 { (yyval.constant_expression) = (yyvsp[0].conditional_expression); }
#line 5197 "glsl.parser.c"
    break;

  case 297:
#line 1163 "glsl.y"
                                                { (yyval.conditional_expression) = (yyvsp[0].logical_or_expression); }
#line 5203 "glsl.parser.c"
    break;

  case 298:
#line 1165 "glsl.y"
                                { (yyval.conditional_expression) = new_glsl_node(context, TERNARY_EXPRESSION, (yyvsp[-4].logical_or_expression), (yyvsp[-2].expression), (yyvsp[0].assignment_expression), NULL); }
#line 5209 "glsl.parser.c"
    break;

  case 299:
#line 1168 "glsl.y"
                                                 { (yyval.logical_or_expression) = (yyvsp[0].logical_xor_expression); }
#line 5215 "glsl.parser.c"
    break;

  case 300:
#line 1170 "glsl.y"
                                { (yyval.logical_or_expression) = new_glsl_node(context, OR_OP, (yyvsp[-2].logical_or_expression), (yyvsp[0].logical_xor_expression), NULL); }
#line 5221 "glsl.parser.c"
    break;

  case 301:
#line 1173 "glsl.y"
                                                 { (yyval.logical_xor_expression) = (yyvsp[0].logical_and_expression); }
#line 5227 "glsl.parser.c"
    break;

  case 302:
#line 1175 "glsl.y"
                                { (yyval.logical_xor_expression) = new_glsl_node(context, XOR_OP, (yyvsp[-2].logical_xor_expression), (yyvsp[0].logical_and_expression), NULL); }
#line 5233 "glsl.parser.c"
    break;

  case 303:
#line 1178 "glsl.y"
                                                  { (yyval.logical_and_expression) = (yyvsp[0].inclusive_or_expression); }
#line 5239 "glsl.parser.c"
    break;

  case 304:
#line 1180 "glsl.y"
                                { (yyval.logical_and_expression) = new_glsl_node(context, AND_OP, (yyvsp[-2].logical_and_expression), (yyvsp[0].inclusive_or_expression), NULL); }
#line 5245 "glsl.parser.c"
    break;

  case 305:
#line 1183 "glsl.y"
                                                  { (yyval.inclusive_or_expression) = (yyvsp[0].exclusive_or_expression); }
#line 5251 "glsl.parser.c"
    break;

  case 306:
#line 1185 "glsl.y"
                                { (yyval.inclusive_or_expression) = new_glsl_node(context, VERTICAL_BAR, (yyvsp[-2].inclusive_or_expression), (yyvsp[0].exclusive_or_expression), NULL); }
#line 5257 "glsl.parser.c"
    break;

  case 307:
#line 1188 "glsl.y"
                                         { (yyval.exclusive_or_expression) = (yyvsp[0].and_expression); }
#line 5263 "glsl.parser.c"
    break;

  case 308:
#line 1190 "glsl.y"
                                { (yyval.exclusive_or_expression) = new_glsl_node(context, CARET, (yyvsp[-2].exclusive_or_expression), (yyvsp[0].and_expression), NULL); }
#line 5269 "glsl.parser.c"
    break;

  case 309:
#line 1193 "glsl.y"
                                              { (yyval.and_expression) = (yyvsp[0].equality_expression); }
#line 5275 "glsl.parser.c"
    break;

  case 310:
#line 1195 "glsl.y"
                                { (yyval.and_expression) = new_glsl_node(context, AMPERSAND, (yyvsp[-2].and_expression), (yyvsp[0].equality_expression), NULL); }
#line 5281 "glsl.parser.c"
    break;

  case 311:
#line 1198 "glsl.y"
                                                { (yyval.equality_expression) = (yyvsp[0].relational_expression); }
#line 5287 "glsl.parser.c"
    break;

  case 312:
#line 1201 "glsl.y"
                                { (yyval.equality_expression) = new_glsl_node(context, EQ_OP, (yyvsp[-2].equality_expression), (yyvsp[0].relational_expression), NULL); }
#line 5293 "glsl.parser.c"
    break;

  case 313:
#line 1204 "glsl.y"
                                { (yyval.equality_expression) = new_glsl_node(context, NE_OP, (yyvsp[-2].equality_expression), (yyvsp[0].relational_expression), NULL); }
#line 5299 "glsl.parser.c"
    break;

  case 314:
#line 1207 "glsl.y"
                                           { (yyval.relational_expression) = (yyvsp[0].shift_expression); }
#line 5305 "glsl.parser.c"
    break;

  case 315:
#line 1210 "glsl.y"
                                { (yyval.relational_expression) = new_glsl_node(context, LEFT_ANGLE, (yyvsp[-2].relational_expression), (yyvsp[0].shift_expression), NULL); }
#line 5311 "glsl.parser.c"
    break;

  case 316:
#line 1213 "glsl.y"
                                { (yyval.relational_expression) = new_glsl_node(context, RIGHT_ANGLE, (yyvsp[-2].relational_expression), (yyvsp[0].shift_expression), NULL); }
#line 5317 "glsl.parser.c"
    break;

  case 317:
#line 1216 "glsl.y"
                                { (yyval.relational_expression) = new_glsl_node(context, LE_OP, (yyvsp[-2].relational_expression), (yyvsp[0].shift_expression), NULL); }
#line 5323 "glsl.parser.c"
    break;

  case 318:
#line 1219 "glsl.y"
                                { (yyval.relational_expression) = new_glsl_node(context, GE_OP, (yyvsp[-2].relational_expression), (yyvsp[0].shift_expression), NULL); }
#line 5329 "glsl.parser.c"
    break;

  case 319:
#line 1222 "glsl.y"
                                              { (yyval.shift_expression) = (yyvsp[0].additive_expression); }
#line 5335 "glsl.parser.c"
    break;

  case 320:
#line 1225 "glsl.y"
                                { (yyval.shift_expression) = new_glsl_node(context, LEFT_OP, (yyvsp[-2].shift_expression), (yyvsp[0].additive_expression), NULL); }
#line 5341 "glsl.parser.c"
    break;

  case 321:
#line 1228 "glsl.y"
                                { (yyval.shift_expression) = new_glsl_node(context, RIGHT_OP, (yyvsp[-2].shift_expression), (yyvsp[0].additive_expression), NULL); }
#line 5347 "glsl.parser.c"
    break;

  case 322:
#line 1231 "glsl.y"
                                                    { (yyval.additive_expression) = (yyvsp[0].multiplicative_expression); }
#line 5353 "glsl.parser.c"
    break;

  case 323:
#line 1234 "glsl.y"
                                { (yyval.additive_expression) = new_glsl_node(context, PLUS, (yyvsp[-2].additive_expression), (yyvsp[0].multiplicative_expression), NULL); }
#line 5359 "glsl.parser.c"
    break;

  case 324:
#line 1237 "glsl.y"
                                { (yyval.additive_expression) = new_glsl_node(context, DASH, (yyvsp[-2].additive_expression), (yyvsp[0].multiplicative_expression), NULL); }
#line 5365 "glsl.parser.c"
    break;

  case 325:
#line 1240 "glsl.y"
                                             { (yyval.multiplicative_expression) = (yyvsp[0].unary_expression); }
#line 5371 "glsl.parser.c"
    break;

  case 326:
#line 1243 "glsl.y"
                                { (yyval.multiplicative_expression) = new_glsl_node(context, STAR, (yyvsp[-2].multiplicative_expression), (yyvsp[0].unary_expression), NULL); }
#line 5377 "glsl.parser.c"
    break;

  case 327:
#line 1246 "glsl.y"
                                { (yyval.multiplicative_expression) = new_glsl_node(context, SLASH, (yyvsp[-2].multiplicative_expression), (yyvsp[0].unary_expression), NULL); }
#line 5383 "glsl.parser.c"
    break;

  case 328:
#line 1249 "glsl.y"
                                { (yyval.multiplicative_expression) = new_glsl_node(context, PERCENT, (yyvsp[-2].multiplicative_expression), (yyvsp[0].unary_expression), NULL); }
#line 5389 "glsl.parser.c"
    break;

  case 329:
#line 1252 "glsl.y"
                                             { (yyval.unary_expression) = (yyvsp[0].postfix_expression); }
#line 5395 "glsl.parser.c"
    break;

  case 330:
#line 1255 "glsl.y"
                                { (yyval.unary_expression) = new_glsl_node(context, PRE_INC_OP, (yyvsp[0].unary_expression), NULL); }
#line 5401 "glsl.parser.c"
    break;

  case 331:
#line 1258 "glsl.y"
                                { (yyval.unary_expression) = new_glsl_node(context, PRE_DEC_OP, (yyvsp[0].unary_expression), NULL); }
#line 5407 "glsl.parser.c"
    break;

  case 332:
#line 1261 "glsl.y"
                                { (yyval.unary_expression) = new_glsl_node(context,(yyvsp[-1].unary_operator), (yyvsp[0].unary_expression), NULL); }
#line 5413 "glsl.parser.c"
    break;

  case 333:
#line 1264 "glsl.y"
                               { (yyval.unary_operator) = UNARY_PLUS; }
#line 5419 "glsl.parser.c"
    break;

  case 334:
#line 1265 "glsl.y"
                               { (yyval.unary_operator) = UNARY_DASH; }
#line 5425 "glsl.parser.c"
    break;

  case 335:
#line 1266 "glsl.y"
                               { (yyval.unary_operator) = BANG; }
#line 5431 "glsl.parser.c"
    break;

  case 336:
#line 1267 "glsl.y"
                                { (yyval.unary_operator) = TILDE; }
#line 5437 "glsl.parser.c"
    break;

  case 337:
#line 1270 "glsl.y"
                                             { (yyval.postfix_expression) = (yyvsp[0].primary_expression); }
#line 5443 "glsl.parser.c"
    break;

  case 338:
#line 1273 "glsl.y"
                                { (yyval.postfix_expression) = new_glsl_node(context, ARRAY_REF_OP, (yyvsp[-3].postfix_expression), (yyvsp[-1].integer_expression), NULL); }
#line 5449 "glsl.parser.c"
    break;

  case 339:
#line 1275 "glsl.y"
                                        { (yyval.postfix_expression) = (yyvsp[0].function_call); }
#line 5455 "glsl.parser.c"
    break;

  case 340:
#line 1278 "glsl.y"
                                { (yyval.postfix_expression) = new_glsl_node(context, DOT, (yyvsp[-2].postfix_expression), (yyvsp[0].field_identifier), NULL);}
#line 5461 "glsl.parser.c"
    break;

  case 341:
#line 1281 "glsl.y"
                                { (yyval.postfix_expression) = new_glsl_node(context, POST_INC_OP, (yyvsp[-1].postfix_expression), NULL); }
#line 5467 "glsl.parser.c"
    break;

  case 342:
#line 1284 "glsl.y"
                                { (yyval.postfix_expression) = new_glsl_node(context, POST_DEC_OP, (yyvsp[-1].postfix_expression), NULL); }
#line 5473 "glsl.parser.c"
    break;

  case 343:
#line 1287 "glsl.y"
                                     { (yyval.integer_expression) = (yyvsp[0].expression); }
#line 5479 "glsl.parser.c"
    break;

  case 344:
#line 1290 "glsl.y"
                                                  { (yyval.function_call) = (yyvsp[0].function_call_or_method); }
#line 5485 "glsl.parser.c"
    break;

  case 345:
#line 1293 "glsl.y"
                                                { (yyval.function_call_or_method) = (yyvsp[0].function_call_generic); }
#line 5491 "glsl.parser.c"
    break;

  case 346:
#line 1297 "glsl.y"
                                { (yyval.function_call_generic) = new_glsl_node(context, FUNCTION_CALL, (yyvsp[-3].function_identifier), (yyvsp[-1].function_call_parameter_list), NULL); }
#line 5497 "glsl.parser.c"
    break;

  case 347:
#line 1300 "glsl.y"
                                { (yyval.function_call_generic) = new_glsl_node(context, FUNCTION_CALL,
					(yyvsp[-2].function_identifier),
					new_glsl_node(context, FUNCTION_CALL_PARAMETER_LIST, NULL),
					NULL); }
#line 5506 "glsl.parser.c"
    break;

  case 348:
#line 1306 "glsl.y"
                                { (yyval.function_call_generic) = new_glsl_node(context, FUNCTION_CALL,
					(yyvsp[-3].function_identifier),
					new_glsl_node(context, FUNCTION_CALL_PARAMETER_LIST, NULL),
					NULL); }
#line 5515 "glsl.parser.c"
    break;

  case 349:
#line 1313 "glsl.y"
                                { (yyval.function_call_parameter_list) = new_glsl_node(context, FUNCTION_CALL_PARAMETER_LIST, (yyvsp[0].assignment_expression), NULL); }
#line 5521 "glsl.parser.c"
    break;

  case 350:
#line 1316 "glsl.y"
                                { (yyval.function_call_parameter_list) = new_glsl_node(context, FUNCTION_CALL_PARAMETER_LIST, (yyvsp[-2].function_call_parameter_list), (yyvsp[0].assignment_expression), NULL); }
#line 5527 "glsl.parser.c"
    break;

  case 351:
#line 1319 "glsl.y"
                                         { (yyval.function_identifier) = (yyvsp[0].type_specifier); }
#line 5533 "glsl.parser.c"
    break;

  case 352:
#line 1322 "glsl.y"
                                { (yyval.function_identifier) = new_glsl_node(context, POSTFIX_EXPRESSION, (yyvsp[0].postfix_expression), NULL); }
#line 5539 "glsl.parser.c"
    break;

  case 353:
#line 1325 "glsl.y"
                                              { (yyval.primary_expression) = (yyvsp[0].variable_identifier); }
#line 5545 "glsl.parser.c"
    break;

  case 354:
#line 1328 "glsl.y"
                                { (yyval.primary_expression) = new_glsl_node(context, INTCONSTANT, NULL); (yyval.primary_expression)->data.i = (yyvsp[0].INTCONSTANT); }
#line 5551 "glsl.parser.c"
    break;

  case 355:
#line 1331 "glsl.y"
                                { (yyval.primary_expression) = new_glsl_node(context, UINTCONSTANT, NULL); (yyval.primary_expression)->data.ui = (yyvsp[0].UINTCONSTANT); }
#line 5557 "glsl.parser.c"
    break;

  case 356:
#line 1334 "glsl.y"
                                { (yyval.primary_expression) = new_glsl_node(context, FLOATCONSTANT, NULL); (yyval.primary_expression)->data.f = (yyvsp[0].FLOATCONSTANT); }
#line 5563 "glsl.parser.c"
    break;

  case 357:
#line 1337 "glsl.y"
                                { (yyval.primary_expression) = new_glsl_node(context, TRUE_VALUE, NULL); }
#line 5569 "glsl.parser.c"
    break;

  case 358:
#line 1340 "glsl.y"
                                { (yyval.primary_expression) = new_glsl_node(context, FALSE_VALUE, NULL); }
#line 5575 "glsl.parser.c"
    break;

  case 359:
#line 1343 "glsl.y"
                                { (yyval.primary_expression) = new_glsl_node(context, DOUBLECONSTANT, NULL); (yyval.primary_expression)->data.d = (yyvsp[0].DOUBLECONSTANT); }
#line 5581 "glsl.parser.c"
    break;

  case 360:
#line 1346 "glsl.y"
                                { (yyval.primary_expression) = new_glsl_node(context, PAREN_EXPRESSION, (yyvsp[-1].expression), NULL); }
#line 5587 "glsl.parser.c"
    break;


#line 5591 "glsl.parser.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, context, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (&yylloc, context, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

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
                      yytoken, &yylval, &yylloc, context);
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, context);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

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


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, context, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, context);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[+*yyssp], yyvsp, yylsp, context);
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
  return yyresult;
}
#line 1349 "glsl.y"


#include "glsl_ast.h"

//The scanner macro, needed for integration with flex, causes problems below
#undef scanner

static void glsl_error(GLSL_LTYPE *loc, struct glsl_parse_context *c, const char *s)
{
	c->error = true;
	if (c->error_cb)
		c->error_cb(s, loc->first_line, loc->first_column, loc->last_column);
}

int list_length(struct glsl_node *n, int list_token)
{
	if (n->code != list_token) {
		return 1;
	} else {
		int i;
		int count = 0;
		for (i = 0; i < n->child_count; i++) {
			count += list_length(n->children[i], list_token);
		}
		return count;
	}
}

static void list_gather(struct glsl_node *n, struct glsl_node *new_list, int list_token)
{
	int i;
	for (i = 0; i < n->child_count; i++) {
		struct glsl_node *child = n->children[i];
		if (child->code != list_token)
			new_list->children[new_list->child_count++] = child;
		else
			list_gather(child, new_list, list_token);
	}
}

static void list_collapse(struct glsl_parse_context *context, struct glsl_node *n)
{
	int i;
	for (i = 0; i < n->child_count; i++) {
		struct glsl_node *child = n->children[i];
		if (glsl_ast_is_list_node(child)) {
			int list_token = child->code;
			int length = list_length(child, list_token);
			struct glsl_node *g = (struct glsl_node *)glsl_parse_alloc(context, offsetof(struct glsl_node, children[length]), 8);
			g->code = list_token;
			g->child_count = 0;
			list_gather(child, g, list_token);
			n->children[i] = g;
			child = g;
		}
		list_collapse(context, child);
	}
}

static bool parse_internal(struct glsl_parse_context *context)
{
	context->error = false;
	glsl_parse(context);
	if (context->root) {
		if (glsl_ast_is_list_node(context->root)) {
			//
			// list_collapse() can't combine all the TRANSLATION_UNIT nodes
			// since it would need to replace g_glsl_node_root so we combine
			// the TRANSLATION_UNIT nodes here.
			//
			int list_code = context->root->code;
			int length = list_length(context->root, list_code);
			struct glsl_node *new_root = (struct glsl_node *)glsl_parse_alloc(context, offsetof(struct glsl_node, children[length]), 8);
			new_root->code = TRANSLATION_UNIT;
			new_root->child_count = 0;
			list_gather(context->root, new_root, list_code);
			assert(new_root->child_count == length);
			context->root = new_root;
		}
		//
		// Collapse other list nodes
		//
		list_collapse(context, context->root);
	}
	return context->error;
}

bool glsl_parse_file(struct glsl_parse_context *context, FILE *file)
{
	glsl_lex_init(&(context->scanner));

	glsl_set_in(file, context->scanner);

	bool error;

	error = parse_internal(context);

	glsl_lex_destroy(context->scanner);
	context->scanner = NULL;
	return error;
}

bool glsl_parse_string(struct glsl_parse_context *context, const char *str)
{
	char *text;
	size_t sz;
	bool error;

	glsl_lex_init(&(context->scanner));

	sz = strlen(str);
	text = malloc(sz + 2);
	strcpy(text, str);
	text[sz + 1] = 0;
	glsl__scan_buffer(text, sz + 2, context->scanner);

	error = parse_internal(context);

	free(text);
	glsl_lex_destroy(context->scanner);
	context->scanner = NULL;
	return error;
}

void glsl_parse_context_init(struct glsl_parse_context *context)
{
	context->root = NULL;
	context->scanner = NULL;
	context->first_buffer = NULL;
	context->cur_buffer_start = NULL;
	context->cur_buffer = NULL;
	context->cur_buffer_end = NULL;
	context->error_cb = NULL;
	context->error = false;
}

void glsl_parse_set_error_cb(struct glsl_parse_context *context, glsl_parse_error_cb_t error_cb)
{
	context->error_cb = error_cb;
}


void glsl_parse_context_destroy(struct glsl_parse_context *context)
{
	glsl_parse_dealloc(context);
}
