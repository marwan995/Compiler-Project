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
    closeFile(&warningFileHandler);
    closeFile(&syntaxErrorsFileHandler);
}

#endif // UTILS_H