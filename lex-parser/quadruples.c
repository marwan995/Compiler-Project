#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "node.h"
#include "utils.h"
#include "symbol_table.c"


static int tempCounter = 0;

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
    } else {
        sprintf(str, "%s", node->name ? node->name : "_");
    }
    return str;
}

void printQuad(char* op, char* arg1, char* arg2, char* result) {
    fprintf(quadFileHandler.filePointer, "%s, %s, %s, %s\n", op, arg1 ? arg1 : "_", arg2 ? arg2 : "_", result ? result : "_");
}

Node* quadOperation(char* operation, Node* left, Node* right) {
    if (left == NULL || right == NULL) {
        fprintf(stderr, "Error: Null operand in quadOperation\n");
        return NULL;
    }
    char* arg1 = nodeTypeToString(left);
    char* arg2 = nodeTypeToString(right);
    char* temp = newTemp();
    
    printQuad(operation, arg1, arg2, temp);
    
    free(arg1);
    free(arg2);
    
    Node* result = createNode(left->dataType, temp);
    result->name = temp;
    return result;
}

void quadAssign(char* var, Node* expr) {    
    char* arg1 = nodeTypeToString(expr);
    printQuad("assign", arg1, NULL, var);
    free(arg1);
}

static Node* quadUnaryOperation(char* varName, char* op, bool isPrefix) {
    char* temp = newTemp();

    if (isPrefix) {
        printQuad(op, varName, "1", varName);
        
        Node* node = createNode(getSymbolDataType(varName), varName);
        node->name = strdup(varName);
        return node;
    } else {
        printQuad("assign", varName, NULL, temp);
        
        // Then var = var + 1 (or var - 1)
        printQuad(op, varName, "1", varName);

        // Use the temp (original value)
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
    char labelName[64];
    sprintf(labelName, "FALSE_LABEL%d", labelNum);
    char* condStr = nodeTypeToString(cond);
    printQuad("if_false", condStr, NULL, labelName);
    free(condStr);
}

void quadJump(int labelNum) {
    char labelName[64];
    sprintf(labelName, "LABEL%d", labelNum);
    printQuad("jmp", NULL, NULL, labelName);
}

void quadFalseLabel(int labelNum) {
    char labelName[64];
    sprintf(labelName, "FALSE_LABEL%d", labelNum);
    printQuad("label", NULL, NULL, labelName);
}

void quadLabel(int labelNum) {
    char labelName[64];
    sprintf(labelName, "LABEL%d", labelNum);
    printQuad("label", NULL, NULL, labelName);
}
