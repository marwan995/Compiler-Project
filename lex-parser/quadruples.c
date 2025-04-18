#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#include "node.h"


#define MAX_LABELS 100
/* Quadruple Structure */
/* Quadruple File Handling */
char* quadFile = NULL;
FILE* quadFileP = NULL;

FILE* create_file(char* path) {
    FILE* file = fopen(path, "w");
    if (!file) {
        printf("Error: File %s not found\n", path);
        exit(1);
    }
    return file;
}

void set_file_path(char* filePath) {
    quadFile = strdup(filePath);
    quadFileP = create_file(filePath);
}

void close_file() {
    if (quadFileP) {
        fclose(quadFileP);
        quadFileP = NULL;
    }
    if (quadFile) {
        free(quadFile);
        quadFile = NULL;
    }
}

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
    fprintf(quadFileP, "\t%s\n", operation);
}

void handleVariable(const char* variable,bool isPush) {
    if (isPush) {
        fprintf(quadFileP, "\tPUSH %s\n", variable);
        return;
    }else{
        fprintf(quadFileP, "\tPOP %s\n", variable);
        return;
    }
}

void push_int(int val) {
    fprintf(quadFileP, "\tPUSH %d\n", val);
}

void push_float(float val) {
    fprintf(quadFileP, "\tPUSH %f\n", val);
}

void push_string(char* str) {
    fprintf(quadFileP, "\tPUSH %s\n", str);
}

void handle_identifier(char* symbol, char* type) {
    fprintf(quadFileP, "\t%s %s\n", type, symbol);
}

void handle_quad_function(char* function, char* type) {
    if (strcmp(type, "start") == 0)
        fprintf(quadFileP, "%s:\n", function);
    else if (strcmp(type, "end") == 0)
        fprintf(quadFileP, "\tEND %s\n", function);
    else if (strcmp(type, "call") == 0)
        fprintf(quadFileP, "\tCALL %s\n", function);
}

void ret() {
    fprintf(quadFileP, "\tRET\n");
}

void push_end_loop(int endLoopNum) {
    endLoop[++endLoopIndex] = endLoopNum;
}

void jump_end_loop() {
    if (endLoopIndex < 0) {
        fprintf(quadFileP, "Error: No end label to jump\n");
        return;
    }
    fprintf(quadFileP, "\tJMP EndLoop_%d\n", endLoop[endLoopIndex]);
}

void pop_end_loop() {
    if (endLoopIndex < 0) {
        fprintf(quadFileP, "Error: No end label to add\n");
        return;
    }
    int endLoopNum = endLoop[endLoopIndex--];
    fprintf(quadFileP, "EndLoop_%d:\n", endLoopNum);
}

void push_start_loop(int startLoopNum, char* label) {
    startLoop[++startLoopIndex] = startLoopNum;
    fprintf(quadFileP, "Start%s_%d:\n", label, startLoopNum);
}

void jump_start_loop(char* label) {
    if (startLoopIndex < 0) {
        fprintf(quadFileP, "Error: No start label to jump\n");
        return;
    }
    fprintf(quadFileP, "\tJMP Start%s_%d\n", label, startLoop[startLoopIndex]);
}

void pop_start_loop() {
    if (startLoopIndex < 0) {
        fprintf(quadFileP, "Error: No start label to pop\n");
        return;
    }
    startLoopIndex--;
}

void jump_false_condition(int falseLabelNum) {
    fprintf(quadFileP, "\tJF FalseLabel_%d\n", falseLabelNum);
    falseLabel[++falseLabelIndex] = falseLabelNum;
}

void pop_false_label() {
    if (falseLabelIndex < 0) {
        fprintf(quadFileP, "Error: No false label to pop\n");
        return;
    }
    fprintf(quadFileP, "FalseLabel_%d:\n", falseLabel[falseLabelIndex--]);
}

void push_switch_ident(char* identifier) {
    switchIdent[++switchIdentIndex] = strdup(identifier);
}

void peek_switch_ident() {
    if (switchIdentIndex < 0) {
        fprintf(quadFileP, "Error: No switch identifier to peek\n");
        return;
    }
    fprintf(quadFileP, "\tPUSH %s\n", switchIdent[switchIdentIndex]);
}

void pop_switch_ident() {
    if (switchIdentIndex < 0) {
        fprintf(quadFileP, "Error: No switch identifier to pop\n");
        return;
    }
    free(switchIdent[switchIdentIndex]);
    switchIdentIndex--;
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

void test(Node* node) {
    if (node == NULL) {
        fprintf(stderr, "Node is NULL\n");
        return;
    }
    printf("Node type: %s\n", node->type);
}