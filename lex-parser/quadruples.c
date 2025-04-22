#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "node.h"
#include "utils.h"
#include "symbol_table.c"

#define MAX_Labels 100

int labelCounter = 1;

int ifIndex = -1;
int loopIndex = -1;

int ifLabels[MAX_Labels];
int loopLabels[MAX_Labels];
int switchLabels[MAX_Labels];

void handleOperation(const char* operation) {
    fprintf(quadFileHandler.filePointer, "\t%s\n", operation);
}

void quadPushConst(Node* node) {
    if (node == NULL) {
        fprintf(stderr, "Node is NULL\n");
        return;
    }
 
    char buffer[256];
    if (strcmp(node->dataType, "int") == 0) {
        sprintf(buffer, "%d", node->iValue);
    } else if (strcmp(node->dataType, "float") == 0) {
        sprintf(buffer, "%f", node->fValue);
    } else if (strcmp(node->dataType, "bool") == 0) {
        sprintf(buffer, "%s", node->bValue ? "true" : "false");
    } else if (strcmp(node->dataType, "char") == 0) {
        sprintf(buffer, "'%c'", node->cValue);
    } else if (strcmp(node->dataType, "string") == 0) {
        sprintf(buffer, "\"%s\"", node->sValue);
    } else {
        fprintf(stderr, "Unknown data type: %s\n", node->dataType);
        return;
    }
    fprintf(quadFileHandler.filePointer, "\tpush %s\n", buffer);

}

void quadPushVar(char* name) {
    if (name == NULL) {
        fprintf(stderr, "Variable name is NULL\n");
        return;
    }
    fprintf(quadFileHandler.filePointer, "\tpush %s\n", name);
}

void quadOperation(char* operation) {
    fprintf(quadFileHandler.filePointer, "\t%s\n", operation);
}

void quadPopVar(char* name) {
    fprintf(quadFileHandler.filePointer, "\tpop %s\n", name);
    return;
}

void quadPrefix(char* name, char* operation) {
    quadPushVar(name);
    quadPushVar("1");
    quadOperation(operation);
    quadPopVar(name);
}

void quadPostfix(char* name, char* operation) {
    quadPushVar(name);
    quadPopVar("_temp_");
    quadPrefix(name, operation);
    quadPushVar("_temp_");
}


void quadPrint() {
    fprintf(quadFileHandler.filePointer, "\tprint\n");
}

void quadAddFunctionParams(char* name) {
    int funcIdx = lookup(name);
    printf("Function xx%s has %d parameters\n", name, symbolTable[funcIdx].paramCount);
    for(int i = symbolTable[funcIdx].paramCount - 1; i >= 0; i--) {
        fprintf(quadFileHandler.filePointer, "\tpop %s\n", symbolTable[symbolTable[funcIdx].paramsIds[i]].name);
    }
}

void quadFunctionLabel(char * name) {
    fprintf(quadFileHandler.filePointer, "func_%s:\n", name);
    fprintf(quadFileHandler.filePointer, "\tpop %s\n", "_call_");
}

void quadJumpCall() {
    fprintf(quadFileHandler.filePointer, "\tjmp _call_\n");
}

void quadFunctionCall(char * name, int argCount) {
    int funcIdx = lookup(name);

    for(int i = symbolTable[funcIdx].paramCount - 1; i >= argCount; i--) {
        quadPushConst(symbolTable[symbolTable[funcIdx].paramsIds[i]].nodeValue);
    }

    fprintf(quadFileHandler.filePointer, "\tpush %s\n", "pc");
    fprintf(quadFileHandler.filePointer, "\tpush %s\n", "2");
    fprintf(quadFileHandler.filePointer, "\tadd\n");

    fprintf(quadFileHandler.filePointer, "\tjmp func_%s\n", name);
}

void quadJumpFalse(int labelNum) {
    fprintf(quadFileHandler.filePointer, "\tjf FALSE_LABEL%i\n", labelNum);
}

void quadJump(int labelNum) {
    fprintf(quadFileHandler.filePointer, "\tjmp LABEL%i\n", labelNum);
}

void quadFalseLabel(int labelNum) {
    fprintf(quadFileHandler.filePointer, "FALSE_LABEL%i:\n", labelNum);
}

void quadLabel(int labelNum) {
    fprintf(quadFileHandler.filePointer, "LABEL%i:\n", labelNum);
}

void quadJumpFalseLabel(int labelNum) {
    fprintf(quadFileHandler.filePointer, "\tjmp FALSE_LABEL%i\n", labelNum);
}

bool quadIsInLoop() {
    return loopIndex != -1;
}

void quadLoopInit() {
    loopLabels[++loopIndex] = labelCounter++;
    quadLabel(loopLabels[loopIndex]);
}

void quadLoopBegin() {
    loopLabels[++loopIndex] = labelCounter++;
    quadJumpFalse(loopLabels[loopIndex]);
}

void quadLoopExit() {
    quadJump(loopLabels[loopIndex - 1]);
    quadFalseLabel(loopLabels[loopIndex]); loopIndex -= 2;
}

Node* createNode(char* dataType, char* type) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    node->dataType = strdup(dataType);
    node->type = strdup(type);
    return node;
}

Node* createVarNode(char* dataType, char* type, char* name) {
    Node* node = createNode(dataType, type);
    node->name = strdup(name);
    return node;
}

Node * createIntNode(int iValue) {
    Node* node = createNode("int", "const");
    node->iValue = iValue;
    return node;
}

Node* createFloatNode(float fValue) {
    Node* node = createNode("float", "const");
    node->fValue = fValue;
    return node;
}

Node* createBoolNode(bool bValue) {
    Node* node = createNode("bool", "const");
    node->bValue = bValue;
    return node;
}

Node* createCharNode(char cValue) {
    Node* node = createNode("char", "const");
    node->cValue = cValue;
    return node;
}

Node* createStringNode(char* sValue) {
    Node* node = createNode("string", "const");
    node->sValue = strdup(sValue);
    return node;
}

void quadTest(Node* node) {
    if (node == NULL) {
        fprintf(stderr, "Node is NULL\n");
        return;
    }
    printf("Node type: %s\n", node->type);
    printf("Node dataType: %s\n", node->dataType);
}