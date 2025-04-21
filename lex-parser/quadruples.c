#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "node.h"
#include "utils.h"

#define MAX_LABELS 100

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

void quadOperation(const char* operation) {
    fprintf(quadFileHandler.filePointer, "\t%s\n", operation);
}

void quadAssign(const char* name, Node* value) {
    printf("we are in quadAssign\n");
    fprintf(quadFileHandler.filePointer, "\tpop %s\n", name);
    return;
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