/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

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
#line 531 "glsl.parser.h"

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
