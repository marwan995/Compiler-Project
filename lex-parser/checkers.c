#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "node.h"

bool validateAssignmentType(char* dataType, Node* expr) {
    printf("Validating assignment type: %s\n", dataType);
    if (strcmp(expr->dataType, dataType) == 0) {
        return true;
    } 
    return false;
}

Node* getNode(char* dataType) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->dataType = strdup(dataType);
    return newNode;
}

Node* checkArithmitcExpressionTypes (Node* expr1, Node* expr2) {

    if (strcmp(expr1->dataType, expr2->dataType) == 0 
        && strcmp(expr1->dataType, "void") != 0 && strcmp(expr1->dataType, "string") != 0) {
        return getNode(expr1->dataType);
    } else if ((strcmp(expr1->dataType, "int") == 0 && strcmp(expr2->dataType, "float") == 0) ||
               (strcmp(expr1->dataType, "float") == 0 && strcmp(expr2->dataType, "int") == 0)) {
        return getNode("float");
    } else if ((strcmp(expr1->dataType, "int") == 0 && strcmp(expr2->dataType, "char") == 0) ||
               (strcmp(expr1->dataType, "char") == 0 && strcmp(expr2->dataType, "int") == 0)) {
        return getNode("int");
    } else if ((strcmp(expr1->dataType, "bool") == 0 && strcmp(expr2->dataType, "bool") == 0)) {
        return getNode("int");
    } else if ((strcmp(expr1->dataType, "bool") == 0 && strcmp(expr2->dataType, "int") == 0) ||
               (strcmp(expr1->dataType, "int") == 0 && strcmp(expr2->dataType, "bool") == 0)) {
        return getNode("int");
    } else if ((strcmp(expr1->dataType, "bool") == 0 && strcmp(expr2->dataType, "float") == 0) ||
               (strcmp(expr1->dataType, "float") == 0 && strcmp(expr2->dataType, "bool") == 0)) {
        return getNode("float");
    }
    customError("Type mismatch between %s and %s\n", expr1->dataType, expr2->dataType);
}
bool checkSwitchValues(Node* expr) {
    if (!expr) {
        customError("Null expression in switch type check");
        return false;
    }

    if (strcmp(expr->dataType, "int") != 0 &&
        strcmp(expr->dataType, "char") != 0 &&
        strcmp(expr->dataType, "bool") != 0) {
        customError("Switch expression must be int, char, or bool, got %s", expr->dataType);
        return false;
    }

    return true;
}
Node* checkComparisonExpressionTypes (Node* expr1, Node* expr2) {
    const char* invalidTypes[] = {"void", "string", "char"};

    for(int i = 0; i < 3; i++) {
        if ((strcmp(expr1->dataType, invalidTypes[i]) == 0 || strcmp(expr2->dataType, invalidTypes[i]) == 0) &&
            (strcmp(expr1->dataType, expr2->dataType) != 0)) {
            customError("Invalid expr1 dataType for comparison: %s %s,\n Invalid dataType for comparison: %s\n", expr1->dataType, expr2->dataType, expr1->dataType, expr2->dataType);
            return NULL;
        }
    }

    return getNode("bool");
}

Node* checkUnaryOperationTypes (Node* expr) {
    printf("Checking unary operation types: %s\n", expr->dataType);
    printf("Checking unary operation types: %s\n", expr->type);

    if (strcmp(expr->dataType, "int") == 0 || strcmp(expr->dataType, "float") == 0) {
        return getNode(expr->dataType);
    } else if (strcmp(expr->dataType, "char") == 0) {
        return getNode("int");
    } else {
        printf("Error: Invalid dataType for unary operation: %s\n", expr->dataType);
        exit(1);
    }
    return NULL;
}