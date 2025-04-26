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

bool isFunctReturned = false;

void assemblyPushConst(Node* node) {
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
    fprintf(assemblyFileHandler.filePointer, "\tpush %s\n", buffer);

}

void assemblyPushVar(char* name) {
    if (name == NULL) {
        fprintf(stderr, "Variable name is NULL\n");
        return;
    }
    fprintf(assemblyFileHandler.filePointer, "\tpush %s\n", name);
}

void assemblyOperation(char* operation) {
    fprintf(assemblyFileHandler.filePointer, "\t%s\n", operation);
}

void assemblyPopVar(char* name) {
    fprintf(assemblyFileHandler.filePointer, "\tpop %s\n", name);
    return;
}

void assemblyPrefix(char* name, char* operation) {
    assemblyPushVar(name);
    assemblyPushVar("1");
    assemblyOperation(operation);
    assemblyPopVar(name);
}

void assemblyPostfix(char* name, char* operation) {
    assemblyPushVar(name);
    assemblyPopVar("_temp_");
    assemblyPrefix(name, operation);
    assemblyPushVar("_temp_");
}


void assemblyPrint() {
    fprintf(assemblyFileHandler.filePointer, "\tprint\n");
}

void assemblyAddFunctionParams(char* name) {
    int funcIdx = lookup(name);
    printf("Function xx%s has %d parameters\n", name, symbolTable[funcIdx].paramCount);
    for(int i = symbolTable[funcIdx].paramCount - 1; i >= 0; i--) {
        fprintf(assemblyFileHandler.filePointer, "\tpop %s\n", symbolTable[symbolTable[funcIdx].paramsIds[i]].name);
    }
}

void assemblyFunctionLabel(char * name) {
    fprintf(assemblyFileHandler.filePointer, "func_%s:\n", name);
    fprintf(assemblyFileHandler.filePointer, "\tpop %s\n", "_call_");
}

void assemblyJumpCall() {
    fprintf(assemblyFileHandler.filePointer, "\tjmp _call_\n");
}

void assemblyFunctionCall(char * name, int argCount) {
    int funcIdx = lookup(name);

    for(int i = symbolTable[funcIdx].paramCount - 1; i >= argCount; i--) {
        assemblyPushConst(symbolTable[symbolTable[funcIdx].paramsIds[i]].nodeValue);
    }

    fprintf(assemblyFileHandler.filePointer, "\tpush %s\n", "pc");
    fprintf(assemblyFileHandler.filePointer, "\tpush %s\n", "2");
    fprintf(assemblyFileHandler.filePointer, "\tadd\n");

    fprintf(assemblyFileHandler.filePointer, "\tjmp func_%s\n", name);
}

void assemblyJumpFalse(int labelNum) {
    fprintf(assemblyFileHandler.filePointer, "\tjf FALSE_LABEL%i\n", labelNum);
}

void assemblyJump(int labelNum) {
    fprintf(assemblyFileHandler.filePointer, "\tjmp LABEL%i\n", labelNum);
}

void assemblyFalseLabel(int labelNum) {
    fprintf(assemblyFileHandler.filePointer, "FALSE_LABEL%i:\n", labelNum);
}

void assemblyLabel(int labelNum) {
    fprintf(assemblyFileHandler.filePointer, "LABEL%i:\n", labelNum);
}

void assemblyJumpFalseLabel(int labelNum) {
    fprintf(assemblyFileHandler.filePointer, "\tjmp FALSE_LABEL%i\n", labelNum);
}

bool assemblyIsInLoop() {
    return loopIndex != -1;
}

void assemblyLoopInit() {
    loopLabels[++loopIndex] = labelCounter++;
    assemblyLabel(loopLabels[loopIndex]);
}

void assemblyLoopBegin() {
    loopLabels[++loopIndex] = labelCounter++;
    assemblyJumpFalse(loopLabels[loopIndex]);
}

void assemblyLoopExit() {
    assemblyJump(loopLabels[loopIndex - 1]);
    assemblyFalseLabel(loopLabels[loopIndex]);
    loopIndex -= 2;
}


void assemblyTest(Node* node) {
    if (node == NULL) {
        fprintf(stderr, "Node is NULL\n");
        return;
    }
    printf("Node type: %s\n", node->type);
    printf("Node dataType: %s\n", node->dataType);
}