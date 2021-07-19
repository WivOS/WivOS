#include <stdio.h>
#include "glsl_parser.h"
#include "glsl_ast.h"

extern const char *code_to_str[4096];

uint32_t inCount = 0;
uint32_t outCount = 0;
uint32_t uniformCount = 0;

int findType(struct glsl_node *node, uint32_t type) {
    for(int i = 0; i < node->child_count; i++) {
        if(node->children[i]->code == type) return i;
    }

    return -1;
}

void parseDeclaration(struct glsl_node *node) {
    struct glsl_node *initDeclaratorList = node->children[0];
    switch(initDeclaratorList->children[0]->code) {
        case SINGLE_DECLARATION:
            {
                struct glsl_node *singleDeclaration = initDeclaratorList->children[0];
                struct glsl_node *arraySpecifierList = singleDeclaration->children[findType(singleDeclaration, ARRAY_SPECIFIER_LIST)];

                printf("DECL ");
                
                struct glsl_node *type = singleDeclaration->children[findType(singleDeclaration, FULLY_SPECIFIED_TYPE)];
                switch(type->children[findType(type, TYPE_QUALIFIER_LIST)]->children[0]->code) {
                    case IN:
                        printf("IN[%d", inCount);
                        if(arraySpecifierList->child_count == 0) {
                            printf("]");
                            inCount++;
                        }
                        else {
                            inCount += arraySpecifierList->children[findType(arraySpecifierList, ARRAY_SPECIFIER)]->children[0]->data.ui;
                            printf("..%d]", inCount - 1);
                        }
                        break;
                    case OUT:
                        printf("OUT[%d", outCount);
                        if(arraySpecifierList->child_count == 0) {
                            printf("]");
                            outCount++;
                        }
                        else {
                            outCount += arraySpecifierList->children[findType(arraySpecifierList, ARRAY_SPECIFIER)]->children[0]->data.ui;
                            printf("..%d]", outCount - 1);
                        }
                        break;
                    case UNIFORM:
                        printf("CONST[%d", uniformCount);
                        if(arraySpecifierList->child_count == 0) {
                            printf("]");
                            uniformCount++;
                        }
                        else {
                            uniformCount += arraySpecifierList->children[findType(arraySpecifierList, ARRAY_SPECIFIER)]->children[0]->data.ui;
                            printf("..%d]", uniformCount - 1);
                        }
                        break;
                    default:
                        printf("Unknown type qualifier %s\n", code_to_str[type->children[findType(type, TYPE_QUALIFIER_LIST)]->children[0]->code]);
                }
                printf(";\n");

                printf("Single declaration found with name %s\n", singleDeclaration->children[findType(singleDeclaration, IDENTIFIER)]->data.str);

            }
            break;
        default:
            printf("Unknown declaration type %s\n", code_to_str[initDeclaratorList->children[0]->code]);
    }
}

void main() {
    printf("Hola\n");

    /*char ch[2];
    ch[1] = '\0';

    FILE *fd = fopen("/dev/tty0", "rw");*/

    /*while(fread(ch, 1, 1, stdout) > 0) {
        printf("a: %s\n", ch);
    }*/

    struct glsl_parse_context context;
    glsl_parse_context_init(&context);

    char *glslFragment =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec4 vertexColor;\n"
    "uniform float myValues[12];\n"
    "void main() {\n"
    "   FragColor = vertexColor;\n"
    "}";

    glsl_parse_string(&context, glslFragment);
    printf("\nAST tree:\n\n");
	glsl_ast_print((&context)->root, 0);

    for(int i = 0; i < context.root->child_count; i++) {
        if(context.root->children[i]->code == DECLARATION) {
            parseDeclaration(context.root->children[i]);
        }
    }

    printf("Parsed\n");
}