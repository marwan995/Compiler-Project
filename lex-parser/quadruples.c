#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#include "node.h"
#include "utils.h"


#define MAX_LABELS 100
/* Quadruple Structure */
/* Quadruple File Handling */


/* Label Stacks */
int falseLabelNum = 0;
int falseLabelIndex = -1;
int falseLabel[MAX_LABELS];

int startLoopNum = 0;
int startLoopIndex = -1;
int startLoop[MAX_LABELS];

int endLoopNum = 0;
int endLoopIndex = -1;
int endLoop[MAX_LABELS];

int switchIdentIndex = -1;
char* switchIdent[MAX_LABELS];

/* Helper Functions */
char* newTemp() {
    static int tempCount = 0;
    char* temp = malloc(10);
    sprintf(temp, "t%d", tempCount++);
    insertVarConst(temp, "var", "temp", true, 0);
    return temp;
}

char* newLabel() {
    static int labelCount = 0;
    char* label = malloc(10);
    sprintf(label, "L%d", labelCount++);
    return label;
}

/* Quadruple Generation Functions */

void handleOperation(const char* operation) {
    fprintf(quadFileHandler.filePointer, "\t%s\n", operation);
}

void handleVariable(const char* variable,bool isPush) {
    if (isPush) {
        fprintf(quadFileHandler.filePointer, "\tPUSH %s\n", variable);
        return;
    }else{
        fprintf(quadFileHandler.filePointer, "\tPOP %s\n", variable);
        return;
    }
}

void handle_identifier(char* symbol, char* type) {
    fprintf(quadFileHandler.filePointer, "\t%s %s\n", type, symbol);
}

void handle_quad_function(char* function, char* type) {
    if (strcmp(type, "start") == 0)
        fprintf(quadFileHandler.filePointer, "%s:\n", function);
    else if (strcmp(type, "end") == 0)
        fprintf(quadFileHandler.filePointer, "\tEND %s\n", function);
    else if (strcmp(type, "call") == 0)
        fprintf(quadFileHandler.filePointer, "\tCALL %s\n", function);
}

/* Node Creation */
 
Node* createNode(char* dataType){
    Node* node = (Node*)malloc(sizeof(Node));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    node->type = strdup(dataType);
    return node;
}

Node * createIntNode(int iValue) {
    Node* node = createNode("int");
    node->iValue = iValue;
    return node;
}

Node* createFloatNode(float fValue) {
    Node* node = createNode("float");
    node->fValue = fValue;
    return node;
}

Node* createBoolNode(bool bValue) {
    Node* node = createNode("bool");
    node->bValue = bValue;
    return node;
}

Node* createCharNode(char cValue) {
    Node* node = createNode("char");
    node->cValue = cValue;
    return node;
}

Node* createStringNode(char* sValue) {
    Node* node = createNode("string");
    node->sValue = strdup(sValue);
    return node;
}

void quadDefineVar(char* varName) {
    fprintf(quadFileHandler.filePointer, "\tDEFINE %s\n", varName);
}

void quadPop(char* varName) {
    fprintf(quadFileHandler.filePointer, "\tPOP %s\n", varName);
}

void quadPush(char* varName) {
    fprintf(quadFileHandler.filePointer, "\tPUSH %s\n", varName);
}

void test(Node* node) {
    if (node == NULL) {
        fprintf(stderr, "Node is NULL\n");
        return;
    }
    printf("Node type: %s\n", node->type);
}