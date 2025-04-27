#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "node.h"
#include "utils.h"
#include "symbol_table.c"

#define MAX_LABELS 100

static int tempCounter = 0;
static int quadLabelCounter = 1;

static int quadLoopIndex = -1;

int quadSwitchIndex = -1;
int quadSwitchSkipIndex = -1;
int quadSwitchOutIndex = -1;

int quadLoopLabels[MAX_LABELS];

int quadSwitchLabels[MAX_LABELS];
int quadSwitchSkipLabels[MAX_LABELS];
int quadSwitchOutIndicies[MAX_LABELS];
Node* quadSwitchExpression[MAX_LABELS];

char* newTemp() {
    char* temp = (char*)malloc(10 * sizeof(char));
    sprintf(temp, "t%d", tempCounter++);
    return temp;
}

char* nodeTypeToString(Node* node) {
    if(node == NULL) {
        printf("nodeTypeToString: NULL node\n");
        return strdup("_");
    }

    char* str = (char*)malloc(256 * sizeof(char));
    if(strcmp(node->type, "const") == 0) {
        if(strcmp(node->dataType, "int") == 0) {
            sprintf(str, "%d", node->iValue);
        } else if(strcmp(node->dataType, "float") == 0) {
            sprintf(str, "%f", node->fValue);
        } else if(strcmp(node->dataType, "bool") == 0) {
            sprintf(str, "%s", node->bValue ? "true" : "false");
        } else if(strcmp(node->dataType, "char") == 0) {
            sprintf(str, "'%c'", node->cValue);
        } else if(strcmp(node->dataType, "string") == 0) {
            sprintf(str, "\"%s\"", node->sValue);
        } else {
            customError("Unknown data type: %s", node->dataType);
        }
    } else if(strcmp(node->type, "func") == 0) {
        sprintf(str, "@ret");
    } else {
        sprintf(str, "%s", node->name ? node->name : "_");
    }
    return str;
}

void printQuad(char* op, char* arg1, char* arg2, char* result) {
    fprintf(quadFileHandler.filePointer, "%-12s\t%-12s\t%-12s\t%-12s\n", 
            op ? op : "_", 
            arg1 ? arg1 : "_", 
            arg2 ? arg2 : "_", 
            result ? result : "_");
}

bool quadIsInLoop() {
    return quadLoopIndex != -1;
}

void quadJumpFalseLabel(int labelNum) {
    char labelName[20];
    sprintf(labelName, "FALSE_LABEL%d", labelNum);
    printQuad("jmp", NULL, NULL, labelName);
}
void quadPrint(Node* node) {
    char* arg1 = nodeTypeToString(node);
    printQuad("print", arg1, NULL, NULL);
    free(arg1);
}

Node* quadOperation(char* operation, Node* left, Node* right) {
    printf("quadOperation: %s\n", operation);
    if (left == NULL || (right == NULL && strcmp(operation, "not") != 0)) {
        fprintf(stderr, "Error: Null operand in quadOperation\n");
        return NULL;
    }
    char* arg1 = nodeTypeToString(left);
    char* arg2 = strcmp(operation, "not") == 0 ? NULL : nodeTypeToString(right);
    char* temp = newTemp();
    
    printQuad(operation, arg1, arg2, temp);
    
    free(arg1);
    if (arg2 != NULL) {
        free(arg2);
    }
    
    Node* result = createNode(left->dataType, temp);
    result->name = temp;
    return result;
}

void quadAssign(char* var, Node* expr) {
    char* arg1 = nodeTypeToString(expr);
    printQuad("assign", arg1, NULL, var);
    free(arg1);
}

Node* quadUnaryOperationNotMinus(Node* node,char*oper) {
    char* temp = newTemp();
    bool isReturnFromFunction = strcmp(node->type, "@ret") == 0;

    char* varName = isReturnFromFunction ? "@ret" : node->name;
    char* dataType = isReturnFromFunction ? node->dataType : getSymbolDataType(varName);
    char* type = isReturnFromFunction ? "var" : node->type;

    printQuad(oper, varName, "_", temp);
    Node* retNode = createNode(dataType, type);
    retNode->name = temp;
    return retNode;
}

Node* quadUnaryOperation(char* varName, char* op, bool isPrefix) {
    char* temp = newTemp();

    if (isPrefix) {
        printQuad(op, varName, "1", varName);
        
        Node* node = createNode(getSymbolDataType(varName), varName);
        node->name = strdup(varName);
        return node;
    } else {
        printQuad("assign", varName, NULL, temp);
        printQuad(op, varName, "1", varName);

        Node* node = createNode(getSymbolDataType(varName), temp);
        node->name = temp;
        return node;
    }
}

Node* quadPostfixIncrement(char* varName) {
    return quadUnaryOperation(varName, "add", false);
}

Node* quadPostfixDecrement(char* varName) {
    return quadUnaryOperation(varName, "sub", false);
}

Node* quadPrefixIncrement(char* varName) {
    return quadUnaryOperation(varName, "add", true);
}

Node* quadPrefixDecrement(char* varName) {
    return quadUnaryOperation(varName, "sub", true);
}


void quadJumpIfFalse(Node* cond, int labelNum) {
    char labelName[20];
    sprintf(labelName, "FALSE_LABEL%d", labelNum);
    char* condStr = nodeTypeToString(cond);
    printQuad("if_false", condStr, NULL, labelName);
    free(condStr);
}

void quadJump(int labelNum) {
    char labelName[20];
    sprintf(labelName, "LABEL%d", labelNum);
    printQuad("jmp", NULL, NULL, labelName);
}

void quadFalseLabel(int labelNum) {
    char labelName[20];
    sprintf(labelName, "FALSE_LABEL%d", labelNum);
    printQuad("label", NULL, NULL, labelName);
}

void quadLabel(int labelNum) {
    char labelName[20];
    sprintf(labelName, "LABEL%d", labelNum);
    printQuad("label", NULL, NULL, labelName);
}

void quadAddFunctionParams(char* name) {
    int funcIdx = lookup(name);
    printf("Quad: Function %s has %d parameters\n", name, symbolTable[funcIdx].paramCount);
    for (int i = symbolTable[funcIdx].paramCount - 1; i >= 0; i--) {
        char* paramName = symbolTable[symbolTable[funcIdx].paramsIds[i]].name;
        printQuad("pop_param", paramName, "_", "_");
    }
}

void quadFunctionLabel(char* name) {
    printQuad("func_label", name, "_", "_");
    // printQuad("pop", "_call_", "_", "_");
}

void quadJumpCall() {
    printQuad("jmp", "_call_", "_", "_");
}

Node* quadFunctionCall(char *name, int argCount) {
    int funcIdx = lookup(name);

    printf("Arg count: %d\n", argCount);
    for(int i = 0; i < argCount; i++) {
        char* argName = symbolTable[symbolTable[funcIdx].paramsIds[i]].name;
        printQuad("push", argName, "_", "_");
    }

    for (int i = symbolTable[funcIdx].paramCount - 1; i >= argCount; i--) {
        char tempStr[32];
        char* value = nodeTypeToString(symbolTable[symbolTable[funcIdx].paramsIds[i]].nodeValue);
        snprintf(tempStr, sizeof(tempStr), "%s", value);
        printQuad("push_const", tempStr, "_", "_");
    }

    // printQuad("push", "pc", "_", "_");
    // printQuad("add", "pc", "2", "pc");

    char funcLabel[128];
    snprintf(funcLabel, sizeof(funcLabel), "func_%s", name);
    printQuad("jmp", funcLabel, "_", "_");

    Node* retNode = createNode(symbolTable[funcIdx].dataType, "@ret"); // the @ret can be changed
    retNode->name = strdup("@ret");
    return retNode;
}

void quadLoopInit() {
    quadLoopLabels[++quadLoopIndex] = quadLabelCounter++;
    quadLabel(quadLoopLabels[quadLoopIndex]);
}

void quadLoopBegin(Node* condition) {
    quadLoopLabels[++quadLoopIndex] = quadLabelCounter++;
    quadJumpIfFalse(condition, quadLoopLabels[quadLoopIndex]);
}

void quadLoopExit() {
    quadJump(quadLoopLabels[quadLoopIndex - 1]);
    quadFalseLabel(quadLoopLabels[quadLoopIndex]);
    quadLoopIndex -= 2;
}

void quadSwitchBegin(Node* expression) {
    if(strcmp(expression->type, "const") == 0) {
        customError("Switch expression must be a variable");
        exit(1);
    }

    quadSwitchExpression[++quadSwitchOutIndex] = expression;
    quadSwitchOutIndicies[quadSwitchOutIndex] = ++quadSwitchIndex;
    quadSwitchLabels[quadSwitchIndex] = quadLabelCounter++;
    quadSwitchSkipLabels[++quadSwitchSkipIndex] = quadLabelCounter++;;
}

void quadSwitchCaseBegin(Node* expression) {
    char constValue[32];

    char* value = nodeTypeToString(expression);
    snprintf(constValue, sizeof(constValue), "%s", value);

    char* temp = newTemp();
    printQuad("eq", quadSwitchExpression[quadSwitchOutIndex]->name, constValue, temp);

    quadSwitchLabels[++quadSwitchIndex] = quadLabelCounter++;
    char falseLabel[32];
    snprintf(falseLabel, sizeof(falseLabel), "L%d", quadSwitchLabels[quadSwitchIndex]);
    printQuad("jf", temp, "_", falseLabel);

    char skipLabel[32];
    snprintf(skipLabel, sizeof(skipLabel), "L%d", quadSwitchSkipLabels[quadSwitchSkipIndex]);
    printQuad("label", skipLabel, "_", "_");
}

void quadSwitchCaseEnd() {
    quadSwitchSkipLabels[++quadSwitchSkipIndex] = quadLabelCounter++;

    char skipLabel[32];
    snprintf(skipLabel, sizeof(skipLabel), "L%d", quadSwitchSkipLabels[quadSwitchSkipIndex]);
    printQuad("jmp", skipLabel, "_", "_");

    char falseLabel[32];
    snprintf(falseLabel, sizeof(falseLabel), "L%d", quadSwitchLabels[quadSwitchIndex]);
    printQuad("label", falseLabel, "_", "_");
}

void quadSwitchEnd() {
    char skipLabel[32];
    snprintf(skipLabel, sizeof(skipLabel), "L%d", quadSwitchSkipLabels[quadSwitchSkipIndex]);
    printQuad("label", skipLabel, "_", "_");

    int outIndex = quadSwitchOutIndicies[quadSwitchOutIndex];

    char outLabel[32];
    snprintf(outLabel, sizeof(outLabel), "L%d", quadSwitchLabels[outIndex]);
    printQuad("label", outLabel, "_", "_");

    quadSwitchIndex = outIndex - 1;
    quadSwitchSkipIndex = outIndex - 1;
    quadSwitchOutIndex--;
}

Node* quadReturn(Node* node) {
    if (node == NULL) {
        printQuad("return", "_", "_", "_");
        return createNode("void", "_");
    } else {
        char* arg1 = nodeTypeToString(node);
        printQuad("return", "_", "_", arg1);
        free(arg1);

        Node* retNode = createNode(node->dataType, node->type);
        node->name = strdup("@ret");
        return node;
    }
}