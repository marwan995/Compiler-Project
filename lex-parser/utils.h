#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* filePath;
    FILE* filePointer;
} FileHandler;

FileHandler quadFileHandler = {NULL, NULL};
FileHandler assemblyFileHandler = {NULL, NULL};
FileHandler warningFileHandler = {NULL, NULL};
FileHandler syntaxErrorsFileHandler = {NULL, NULL};

FILE* createFile(char* path) {
    FILE* file = fopen(path, "w");
    if (!file) {
        printf("Error: File %s could not be created\n", path);
        exit(1);
    }
    return file;
}

void setFilePath(FileHandler* handler, char* filePath) {
    handler->filePath = strdup(filePath);
    handler->filePointer = createFile(filePath);
}
void setFiles() {
    setFilePath(&quadFileHandler, "quadruples.txt");
    setFilePath(&assemblyFileHandler, "assembly.txt");
    setFilePath(&warningFileHandler, "warnings.txt");
    setFilePath(&syntaxErrorsFileHandler, "syntax_errors.txt");
}

void closeFile(FileHandler* handler) {
    if (handler->filePointer) {
        fclose(handler->filePointer);
        handler->filePointer = NULL;
    }
    if (handler->filePath) {
        free(handler->filePath);
        handler->filePath = NULL;
    }
}

void cleanUpFiles() {
    closeFile(&quadFileHandler);
    closeFile(&assemblyFileHandler);
    closeFile(&warningFileHandler);
    closeFile(&syntaxErrorsFileHandler);
}

void customError(char* format, ...) {
    char error_msg[256];
    va_list args;

    va_start(args, format);
    vsnprintf(error_msg, sizeof(error_msg), format, args);
    va_end(args);

    yyerror(error_msg);
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
#endif // UTILS_H