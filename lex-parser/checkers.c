#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "node.h"

bool validateAssignmentType(char* type, Node* expr) {
    if (strcmp(expr->type,type) == 0) {
        return true;
    } 
    return false;
}

Node* getNode(char* type) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->type = strdup(type);

    return newNode;
}

Node* checkArithmitcExpressionTypes (Node* expr1, Node* expr2) {
    if (strcmp(expr1->type, expr2->type) == 0 
        && strcmp(expr1->type, "void") != 0 && strcmp(expr1->type, "string") != 0) {
        return getNode(expr1->type);
    } else if ((strcmp(expr1->type, "int") == 0 && strcmp(expr2->type, "float") == 0) ||
               (strcmp(expr1->type, "float") == 0 && strcmp(expr2->type, "int") == 0)) {
        return getNode("float");
    } else if ((strcmp(expr1->type, "int") == 0 && strcmp(expr2->type, "char") == 0) ||
               (strcmp(expr1->type, "char") == 0 && strcmp(expr2->type, "int") == 0)) {
        return getNode("int");
    } else if ((strcmp(expr1->type, "bool") == 0 && strcmp(expr2->type, "bool") == 0)) {
        return getNode("int");
    } else if ((strcmp(expr1->type, "bool") == 0 && strcmp(expr2->type, "int") == 0) ||
               (strcmp(expr1->type, "int") == 0 && strcmp(expr2->type, "bool") == 0)) {
        return getNode("int");
    } else if ((strcmp(expr1->type, "bool") == 0 && strcmp(expr2->type, "float") == 0) ||
               (strcmp(expr1->type, "float") == 0 && strcmp(expr2->type, "bool") == 0)) {
        return getNode("float");
    }
    printf("Error: Type mismatch between %s and %s\n", expr1->type, expr2->type);
    exit(1);
}

Node* checkComparisonExpressionTypes (Node* expr1, Node* expr2) {
    const char* invalidTypes[] = {"void", "string", "char"};

    for(int i = 0; i < 3; i++) {
        if ((strcmp(expr1->type, invalidTypes[i]) == 0 || strcmp(expr2->type, invalidTypes[i]) == 0) &&
            (strcmp(expr1->type, expr2->type) != 0)) {
                printf("Error: Invalid expr1 type for comparison: %s %s\n", expr1->type, expr2->type);
            printf("Error: Invalid type for comparison: %s\n", invalidTypes[i]);
            exit(1);
        }
    }

    return getNode("bool");
}

Node* checkUnaryOperationTypes (Node* expr) {
    if (strcmp(expr->type, "int") == 0 || strcmp(expr->type, "float") == 0 || strcmp(expr->type, "bool") == 0) {
        return getNode(expr->type);
    } else if (strcmp(expr->type, "char") == 0) {
        return getNode("int");
    } else {
        printf("Error: Invalid type for unary operation: %s\n", expr->type);
        exit(1);
    }
    return NULL;
}