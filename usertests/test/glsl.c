#include "glsl.h"
#include "opengl.h"

int64_t token;
char *src, *oldSrc;
int64_t line;

enum {
    Num = 128, Fun, Sys, Glo, Loc, Id,
    Bool, Else, If, Int, UInt, Float, Double, Layout, Uniform, Out, In, Return, While,
    Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

typedef struct {
    int64_t token;
    int64_t hash;
    char *name;
    int64_t class;
    int64_t type;
    int64_t value;
    int64_t location;
    int64_t vecSize;
    int64_t variableType;
    int64_t BClass;
    int64_t BType;
    int64_t BValue;
} identifier_t;

enum { BOOL, INT, UINT, FLOAT, DOUBLE, LAYOUT, UNIFORM, OUT, IN, VEC, MAT };
int *idMain;

double tokenVal;
identifier_t *currentId;
identifier_t *symbolsTable;

int64_t variableBaseType;
int64_t baseType;
int64_t exprType;
char *declarationString;
char *declarationStringPos;
char *codeString;
char *codeStringPos;
uint64_t codeLine = 0;

void next() {
    char *lastPos;
    int64_t hash;
    while(token = *src) {
        ++src;

        if(token == '\n') line++;
        else if(token == '#') {
            while(*src != 0 && *src != '\n') { // TODO, Check macros
                src++;
            }
        }
        else if((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {
            lastPos = src - 1;
            hash = token;
            while((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) {
                hash = hash * 147 + *src;
                src++;
            }

            currentId = symbolsTable;
            while(currentId->token) {
                if(currentId->hash == hash && !memcmp((char *)currentId->name, lastPos, src - lastPos)) {
                    token = currentId->token;
                    return;
                }
                currentId = currentId + sizeof(identifier_t);
            }
            char *name = (char *)lastPos;
            while((*name >= 'a' && *name <= 'z') || (*name >= 'A' && *name <= 'Z') || (*name >= '0' && *name <= '9') || (*name == '_')) {
                name++;
            }
            currentId->name = (char *)malloc(name - lastPos + 1);
            memcpy(currentId->name, (void *)lastPos, (name - lastPos));
            currentId->name[(name - lastPos)] = 0;
            currentId->hash = hash;
            token = currentId->token = Id;
            return;
        }
        else if(token >= '0' && token <= '9') {
            tokenVal = token - '0';
            while(*src >= '0' && *src <= '9') {
                tokenVal = tokenVal * 10 + *src++ - '0';
            }
            if(*src == '.') {
                src++;
                uint64_t position = 1;
                while(*src >= '0' && *src <= '9') {
                    tokenVal += ((float)(*src++ - '0') / (10 * position));
                    position *= 10;
                }
            }

            token = Num; // Num us always float or double
            return;
        }
        else if(token == '/') {
            if (*src == '/') {
                while (*src != 0 && *src != '\n') {
                    ++src;
                }
            } else {
                token = Div;
                return;
            }
        }
        else if(token == '=') {
            if (*src == '=') {
                src++;
                token = Eq;
            } else {
                token = Assign;
            }
            return;
        }
        else if(token == '+') {
            if (*src == '+') {
                src++;
                token = Inc;
            } else {
                token = Add;
            }
            return;
        }
        else if(token == '-') {
            if (*src == '-') {
                src++;
                token = Dec;
            } else {
                token = Sub;
            }
            return;
        }
        else if(token == '!') {
            if (*src == '=') {
                src++;
                token = Ne;
            }
            return;
        }
        else if(token == '<') {
            if (*src == '=') {
                src++;
                token = Le;
            } else if (*src == '<') { // TODO: Check if this exists in glsl
                src++;
                token = Shl;
            } else {
                token = Lt;
            }
            return;
        }
        else if(token == '>') {
            if (*src == '=') {
                src++;
                token = Ge;
            } else if (*src == '>') { // TODO: Check if this exists in glsl
                src++;
                token = Shr;
            } else {
                token = Gt;
            }
            return;
        }
        else if(token == '|') {
            if (*src == '|') {
                src++;
                token = Lor;
            } else {
                token = Or;
            }
            return;
        }
        else if(token == '&') {
            if (*src == '&') {
                src++;
                token = Lan;
            } else {
                token = And;
            }
            return;
        }
        else if(token == '^') {
            token = Xor;
            return;
        }
        else if(token == '%') {
            token = Mod;
            return;
        }
        else if(token == '*') {
            token = Mul;
            return;
        }
        else if(token == '[') {
            token = Brak;
            return;
        }
        else if(token == '?') {
            token = Cond;
            return;
        }
        else if(token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':' || token == '.') {
            return;
        }
    }
}

void match(int tk) {
    if(token == tk) {
        next();
    } else {
        printf("%d: expected token: %d\n", line, tk);
        while(1);
    }
}

uint64_t *text; // function location, but i think is not needed because we don't have jumps in tgsi
uint64_t layoutData = 0;
uint64_t outData = 0;
uint64_t uniformData = 0;
uint64_t tempVariable = 0;
uint64_t assignRight = 0;
uint64_t immPosition = 0;

bool justCopy = 0;
uint64_t returnVariable = 0;

const char *cVArray = "xyzw";

void expression(int64_t level) {
    identifier_t *id;
    int64_t tmp;
    int64_t *addr;
    {
        if(!token) {
            printf("%d: unexpected token EOF of expression\n", line);
            while(1);
        }
        if(token == Num) {
            volatile uint32_t tokenValUInt = 0;
            *((float *)&tokenValUInt) = tokenVal;
            declarationStringPos += sprintf(declarationStringPos, "IMM[%d] FLT32 {0x%08x, 0x%08x, 0x%08x, 0x%08x}\n", immPosition, tokenValUInt, 0, 0, 0); // TODO implement a proper way to optimize this

            if(assignRight && justCopy) {
                if(assignRight == 1)
                    tempVariable++;
                switch(assignRight) {
                    case 1:
                        codeStringPos += sprintf(codeStringPos, "  %d: MOV TEMP[%d].x, IMM[%d].xxxx\n", codeLine++, (tempVariable - 1), immPosition);
                        break;
                    case 2:
                        codeStringPos += sprintf(codeStringPos, "  %d: MOV TEMP[%d].y, IMM[%d].xxxx\n", codeLine++, (tempVariable - 1), immPosition);
                        break;
                    case 3:
                        codeStringPos += sprintf(codeStringPos, "  %d: MOV TEMP[%d].z, IMM[%d].xxxx\n", codeLine++, (tempVariable - 1), immPosition);
                        break;
                    case 4:
                        codeStringPos += sprintf(codeStringPos, "  %d: MOV TEMP[%d].w, IMM[%d].xxxx\n", codeLine++, (tempVariable - 1), immPosition);
                        break;
                }
            }

            immPosition++;

            match(Num);
        } else if(token == Id) {
            match(Id);

            id = currentId;

            if(token == '(') {
                match('(');

                if(id->class == Sys) {
                    if(!strncmp(id->name, "vec", 3) && id->name[3] >= '1' && id->name[3] <= '4') {
                        //vec1 vec2 vec3 function, just copy the temp register
                        justCopy = 1;
                    }
                    //TODO
                } else if(id->class == Fun) {
                    //TODO
                } else {
                    printf("%d: bad function call\n", line);
                    while(1);
                }

                tmp = 0;
                while(token != ')') {
                    assignRight++;
                    expression(Assign);

                    tmp++;

                    if(token == ',') {
                        match(',');
                    }
                }

                match(')');

                if(id->class == Sys) {
                    if(justCopy) { //Vec type functions
                        returnVariable = (tempVariable - 1);
                    }
                }

                if(tmp > 0) {
                    //TODO
                }
                exprType = id->type;
                assignRight = 0;
                justCopy = 0;
            } else if (id->class == Num) {
                exprType = INT;
                assignRight = 0;
            } else { // TODO: handle variable.(xyzw) = case
                if(id->class == Loc) {
                    //TODO
                } else if(id->class == Glo) { // IN and UNIFORM
                    if(assignRight && justCopy) {
                        if(assignRight == 1)
                            tempVariable++;
                        if(id->vecSize > 0) {
                            switch(id->vecSize) { // TODO: uniforms
                                case 1:
                                    codeStringPos += sprintf(codeStringPos, "  %d: MOV TEMP[%d].x, IN[%d].xxxx\n", codeLine++, (tempVariable - 1), id->value);
                                    break;
                                case 2:
                                    codeStringPos += sprintf(codeStringPos, "  %d: MOV TEMP[%d].xy, IN[%d].xyxx\n", codeLine++, (tempVariable - 1), id->value);
                                    assignRight++;
                                    break;
                                case 3:
                                    codeStringPos += sprintf(codeStringPos, "  %d: MOV TEMP[%d].xyz, IN[%d].xyzx\n", codeLine++, (tempVariable - 1), id->value);
                                    assignRight += 2;
                                    break;
                                case 4:
                                    codeStringPos += sprintf(codeStringPos, "  %d: MOV TEMP[%d], IN[%d]\n", codeLine++, (tempVariable - 1), id->value);
                                    assignRight++;
                                    break;
                            }
                        }
                    } else if(id->variableType == IN || id->variableType == LAYOUT) {
                        tempVariable++;
                        codeStringPos += sprintf(codeStringPos, "  %d: MOV TEMP[%d], IN[%d]\n", codeLine++, (tempVariable - 1), id->value);
                        returnVariable = (tempVariable - 1);
                    }
                } else {
                    printf("%d: undefined variable\n", line);
                    while(1);
                }

                exprType = id->type;
            }
        } else if(token == '!') {
            match('!');
            expression(Inc);

            exprType = INT; //TODO
        } else if(token == Add) {
            match(Add);
            expression(Inc);
            exprType = INT;
        } else if(token == Sub) {
            match(Sub);

            if(token == Num) {
                match(Num);
            } else {
                expression(Inc);
            }

            exprType = INT;
        } else if(token == Inc || token == Dec) {
            tmp = token;
            match(token);
            expression(Inc);

            //TODO
        } else {
            printf("%d: bad expression, %d\n", line, token);
            while(1);
        }
    }

    {
        while (token >= level) {
            tmp = exprType;

            //TODO: ternary if supported
            if(token == Assign) {
                match(Assign);

                expression(Assign);

                codeStringPos += sprintf(codeStringPos, "  %d: MOV OUT[%d], TEMP[%d]\n", codeLine++, id->value, returnVariable);

                exprType = tmp;
            } else if (token == Add) {
                match(Add);

                expression(Mul);
                exprType = tmp;
            } else if (token == Mul) {
                match(Mul);

                uint64_t tempVaribleThis = tempVariable++;

                expression(Mul);

                /*
                    0: MUL TEMP[0], CONST[1][12], IN[0].xxxx
                    1: MAD TEMP[1], CONST[1][13], IN[0].yyyy, TEMP[0]
                    2: MAD TEMP[2], CONST[1][14], IN[0].zzzz, TEMP[1]
                    3: MAD TEMP[3], CONST[1][15], IN[0].wwww, TEMP[2]
                */

                if(id->type == MAT) { // Parse other types
                    if(id->variableType == UNIFORM) {
                        uint64_t tempVariableTemp = tempVariable++;
                        codeStringPos += sprintf(codeStringPos, "  %d: MUL TEMP[%d], CONST[1][%d], TEMP[%d].xxxx\n", codeLine++, tempVariableTemp, id->value + 0, returnVariable);
                        for(size_t i = 1; i < id->vecSize; i++) {
                            tempVariableTemp = tempVariable++;
                            codeStringPos += sprintf(codeStringPos, "  %d: MAD TEMP[%d], CONST[1][%d], TEMP[%d].%c%c%c%c, TEMP[%d]\n", codeLine++, tempVariableTemp, id->value + i, returnVariable, cVArray[i], cVArray[i], cVArray[i], cVArray[i],  (tempVariableTemp - 1));
                        }
                        returnVariable = tempVariableTemp;
                    }
                }

                codeStringPos += sprintf(codeStringPos, "  %d: MOV TEMP[%d], TEMP[%d]\n", codeLine++, tempVaribleThis, returnVariable);

                returnVariable = tempVaribleThis;

                exprType = tmp;
            } else if (token == Brak) {
                match(Brak);

                expression(Assign);
                match(']');
            }
        }
    }
}

int indexOfBp;

void function_parameter() {
    int64_t variableType;
    int64_t type;
    int64_t params;
    params = 0;
    while(token != ')') {
        variableType = IN;
        type = INT;

        if(token == In) {
            match(In);
        } else if(token == Out) {
            type = OUT;
            match(Out);
        }

        if(token == Int) {
            match(Int);
        } else if(token == UInt) {
            type = UINT;
            match(UInt);
        } else if(token == Float) {
            type = FLOAT;
            match(Float);
        } else if(token == Double) {
            type = DOUBLE;
            match(Double);
        } // TODO: handle vectors

        if(token != Id) {
            printf("%d: bad parameter declaration\n", line);
            while(1);
        }
        if(currentId->class == Loc) {
            printf("%d: duplicate parameter declaration\n", line);
            while(1);
        }

        match(Id);

        currentId->BClass = currentId->class; currentId->class = Loc;
        currentId->BType = currentId->type; currentId->type = type;
        currentId->BValue = currentId->value; currentId->value = params++;
    
        if(token == ',') {
            match(',');
        }
    }

    indexOfBp = params + 1;
}

void statement() {
    int64_t *a, *b;

    if(token == If) {
        match(If);
        match('(');
        expression(Assign);
        match(')');

        //TODO write assembly tgsi

        statement();
        if(token == Else) {
            match(Else);

            statement();
        }
    } else if(token == While) {
        match(While);

        match('(');
        expression(Assign);
        match(')');

        //TODO write assembly tgsi

        statement();
    } else if(token == Return) {
        match(Return);

        if(token != ';') expression(Assign);

        match(';');

        //TODO write assembly tgsi

        statement();
    } else if(token == '{') {
        match('{');

        while(token != '}') {
            statement();
        }

        match('}');
    } else if(token == ';') {
        match(';');
    } else {
        expression(Assign);
        match(';');
    }
}

void function_body() {
    int64_t posLocal;
    int64_t type;
    posLocal = indexOfBp;

    while(token == Int || token == Bool || token == UInt || token == Float || token == Double) { // TODO: vectors
        // Use temp variables
        switch(token) {
            case Int: baseType = INT; break;
            case UInt: baseType = UINT; break;
            case Bool: baseType = BOOL; break;
            case Float: baseType = FLOAT; break;
            case Double: baseType = DOUBLE; break;
            default: break;
        }
        match(token);
        
        while(token != ';') {
            type = baseType;
        
            if(token != Id) {
                printf("%d: bad local declaration\n", line);
                while(1);
            }
            if(currentId->class == Loc) {
                printf("%d: duplicate local declaration\n", line);
                while(1);
            }
            match(Id);

            currentId->BClass = currentId->class; currentId->class = Loc;
            currentId->BType = currentId->type; currentId->type = type;
            currentId->BValue = currentId->value; currentId->value = ++posLocal;
        
            if(token == ',') {
                match(',');
            }
        }
        match(';');
    }

    while(token != '}') {
        statement();
    }
}

void function_declaration() {
    match('(');
    function_parameter();
    match(')');
    match('{');
    function_body();

    currentId = symbolsTable;
    while (currentId->token) {
        if (currentId->class == Loc) {
            currentId->class = currentId->BClass;
            currentId->type  = currentId->BType;
            currentId->value = currentId->BValue;
        }
        currentId = currentId + sizeof(identifier_t);
    }

    match('}');
}

void global_declaration() {
    int64_t type;
    int64_t i;
    int64_t location = -1;
    int64_t vecType = 0;
    int dataSize = 0;

    baseType = INT;
    variableBaseType = OUT;

    if(token == Layout) {
        match(Layout);
        variableBaseType = LAYOUT;
        match('(');
        //TODO: Add more attributes
        if(!strcmp(currentId->name, "location")) {
            match(Id);
            match(Assign);
            location = tokenVal;
            match(Num);
        }
        match(')');
        match(In);
        //TODO layout locations
    } else if(token == Out) {
        match(Out);
        variableBaseType = OUT; 
    } else if(token == In) {
        match(In);
        variableBaseType = IN;
    } else if(token == Uniform) {
        match(Uniform);
        variableBaseType = UNIFORM;
    }

    if(token == Int) {
        match(Int);
        baseType = INT;
        dataSize = 1;
    } else if(token == UInt) {
        match(UInt);
        baseType = UINT;
        dataSize = 1;
    } else if(token == Bool) {
        match(Bool);
        baseType = BOOL;
        dataSize = 1;
    } else if(token == Float) {
        match(Float);
        baseType = FLOAT;
        dataSize = 1;
    } else if(token == Double) {
        match(Double);
        baseType = DOUBLE;
        dataSize = 1;
    } else if(token == Id) {
        char *name = currentId->name;
        if(*name == 'm') { // dvecn
            vecType = name[3] - '0';

            baseType = MAT;
            match(Id);
            dataSize = vecType;
        } else {
            if(*name == 'b') { // bvecn

            } else if(*name == 'i') { // ivecn
                
            } else if(*name == 'u') { // uvecn
                
            } else if(*name == 'd') { // dvecn
                
            } else { //vecn: this is the only supported for now
                while(*name <= '0' || *name > '4') {
                    name++;
                }
                vecType = *name - '0'; // data is always a 4 component vector
            }
            baseType = VEC;
            match(Id);
            dataSize = 1;
        }
    }

    while(token != ';') {
        type = baseType;
        
        if(token != Id) {
            printf("%d: bad global declaration\n", line);
            while(1);
        }
        if(currentId->class) {
            printf("%d: duplicate global declaration\n", line);
            while(1);
        }
        match(Id);
        currentId->type = type;

        if(token == '(') {
            currentId->class = Fun;
            currentId->value = (int)(text + 1);
            function_declaration();
        } else {
            currentId->variableType = variableBaseType;
            currentId->class = Glo;
            currentId->vecSize = vecType; // Always 4 component
            if(variableBaseType == LAYOUT) {
                currentId->value = (int64_t)layoutData;
                layoutData = layoutData + dataSize;
            } else if(variableBaseType == OUT) {
                currentId->value = (int64_t)outData;
                outData = outData + dataSize;
            } else if(variableBaseType == UNIFORM) {
                currentId->value = (int64_t)uniformData;
                uniformData = uniformData + dataSize;
            } // TODO: Uniforms
        }

        if (token == ',') {
            match(',');
        }

        if(token == 0) break;
    }
    next();
}

void program() {
    next();
    while(token > 0) {
        global_declaration();
    }
}

size_t poolSize;

char *preDeclarationString = NULL;
char *preDeclarationStringPos = NULL;

char *compile_glsl(const char *string) {
    line = 1;

    poolSize = 256 * 1024;

    symbolsTable = (identifier_t *)malloc(poolSize);
    declarationStringPos = declarationString = (char *)malloc(poolSize);
    preDeclarationStringPos = preDeclarationString = (char *)malloc(poolSize);
    codeStringPos = codeString = (char *)malloc(poolSize);
    memset(symbolsTable, 0, poolSize);
    memset(declarationString, 0, poolSize);
    memset(codeString, 0, poolSize);

    preDeclarationStringPos += sprintf(preDeclarationStringPos, "VERT\n"); // Adapt this

    outData++; // gl_Position

    src = "bool else if int uint float double layout uniform out in return while "
          "void main gl_Position vec4";

    int64_t i = Bool;
    while(i <= While) {
        next();
        currentId->token = i++;
    }

    next(); currentId->token = Bool;
    next(); idMain = currentId;
    next(); currentId->class = Glo;
    currentId->type = FLOAT; // TODO Vectors
    next(); currentId->class = Sys;

    src = oldSrc = strdup(string);

    src[strlen(string)] = 0;

    program();

    codeStringPos += sprintf(codeStringPos, "  %d: END\n", codeLine++);

    for(size_t i = 0; i < layoutData; i++) {
        preDeclarationStringPos += sprintf(preDeclarationStringPos, "DCL IN[%d]\n", i);
    }

    //TODO: parse types for now generic
    preDeclarationStringPos += sprintf(preDeclarationStringPos, "DCL OUT[%d], POSITION\n", 0);
    for(size_t i = 1; i < outData; i++) {
        preDeclarationStringPos += sprintf(preDeclarationStringPos, "DCL OUT[%d], COLOR\n", i);
    }

    preDeclarationStringPos += sprintf(preDeclarationStringPos, "DCL TEMP[0..%d], LOCAL\n", (tempVariable - 1));
    preDeclarationStringPos += sprintf(preDeclarationStringPos, "DCL CONST[1][0..%d]\n", (uniformData - 1));

    printf("\n%s%s%s", preDeclarationString, declarationString, codeString);

    char *outString = malloc(poolSize * 2);
    memset(outString, 0, poolSize * 2);

    sprintf(outString, "%s%s%s", preDeclarationString, declarationString, codeString);

    printf("Layout variables: %x\n", layoutData);
    printf("Out variables: %x\n", outData);

    free(preDeclarationString);
    free(declarationString);
    free(codeString);

    return outString;
}