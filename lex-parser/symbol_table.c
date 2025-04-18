#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SYMBOLS 1000
#define MAX_PARAMS 20

/*
    Symbol is func, var, const or param
*/
typedef struct Symbol {
    int id; // index in the symbol table
    char* name; // name of the symbol
    char* type; // func, var, const, param
    char* dataType; // int, float, char, etc.
    int scope; // 0 for global
    bool isInitialized;
    
    int paramCount; // number of parameters for functions
    int* paramsIds;
    bool isParam;
} Symbol;

Symbol symbolTable[MAX_SYMBOLS];

int blockIdx = -1; 
int lastFunctionIdx = -1; 

void initSymbolTable(int lineNumber) {
    printf("Initializing symbol table at line: %i...\n", lineNumber);
    blockIdx = 0;
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        symbolTable[i].id = -1;
    }
}

void enterScope(int lineNumber) {
    printf("Entering scope at line: %i...\n", lineNumber);
    blockIdx++;
}

void exitScope(int lineNumber) {
    printf("Exiting scope at line: %i...\n", lineNumber);
    // Free symbols in the current scope
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (symbolTable[i].id != -1 && symbolTable[i].scope == blockIdx) {
            free(symbolTable[i].name);
            free(symbolTable[i].type);
            free(symbolTable[i].dataType);

            symbolTable[i].id = -1;
        }
    }    
    blockIdx--;
}

int isSymbolInSameScope(char *name) {
    // TODO: We need to stop at last function scope
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (symbolTable[i].id != -1 && symbolTable[i].scope == blockIdx
             && strcmp(symbolTable[i].name, name) == 0) {
            return 1; // Symbol is declared
        }
    }
    return 0;
}

bool insertParamToFunction(int functionIdx, int paramIdx) {
    if (symbolTable[functionIdx].paramCount >= MAX_PARAMS) {
        printf("Error: Maximum number of parameters exceeded for function %s\n", symbolTable[functionIdx].name);
        return false;
    }
    symbolTable[functionIdx].paramsIds[symbolTable[functionIdx].paramCount] = paramIdx;
    symbolTable[functionIdx].paramCount++;
    return true;
}

void initParams(int functionIdx) {
    symbolTable[functionIdx].paramsIds = malloc(MAX_PARAMS * sizeof(int));
    for (int j = 0; j < MAX_PARAMS; j++) {
        symbolTable[functionIdx].paramsIds[j] = -1;
    }
}

int insertSymbol(char *name, char* type, char* dataType, bool isInitialized, bool isParam, int lineNumber) {
    if (blockIdx == -1) {
        initSymbolTable(lineNumber);
    }
    printf("Adding symbol: %s, type: %s, dataType: %s, at line: %i\n", name, type, dataType, lineNumber);

    if (isSymbolInSameScope(name)) {
        printf("Error: Symbol %s already declared\n", name);
        exit(1);
    }

    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (symbolTable[i].id == -1) {

            symbolTable[i].id = i;
            symbolTable[i].name = strdup(name);
            symbolTable[i].type = strdup(type);
            symbolTable[i].dataType = strdup(dataType);

            if(isParam) {
                symbolTable[i].scope = blockIdx + 1; // params are in the next scope
                bool isInserted = insertParamToFunction(lastFunctionIdx, i);
                if (!isInserted) {
                    printf("Error: Could not insert parameter %s to function at line %i\n", name, lineNumber);
                    exit(1);
                }
            } else {
                symbolTable[i].scope = blockIdx;
            }
            symbolTable[i].isParam = isParam;
            symbolTable[i].isInitialized = isInitialized;
            symbolTable[i].paramCount = 0;
            
            if (strcmp(type, "func") == 0) {
                lastFunctionIdx = i;
                initParams(lastFunctionIdx);
            }
            return i;
        }
    }

    printf("Error: Symbol table is full, cannot insert symbol %s at line %i\n", name, lineNumber);
    exit(1);
}

int insertParam(char *name, char* type, char* dataType, bool isInitialized, int lineNumber) {
    int result = insertSymbol(name, type, dataType, isInitialized, true, lineNumber);
    return result;
}

int insertVarConst (char *name, char* type, char* dataType, bool isInitialized, int lineNumber) {
    int result = insertSymbol(name, type, dataType, isInitialized, false, lineNumber);
    return result;
}

int insertFunc(char *name, char* type, char* dataType, int lineNumber) {
    int result = insertSymbol(name, type, dataType, false, false, lineNumber);
    return result;
}

int lookup(char *name, int lineNumber) {
    printf("Looking up symbol: %s at line: %i...\n", name, lineNumber);
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (symbolTable[i].id != -1 && strcmp(symbolTable[i].name, name) == 0) {
            printf("Found symbol: %s, id: %i\n", name, symbolTable[i].id);
            return symbolTable[i].id;
        }
    }

    printf("Symbol %s not found\n", name);
    return -1;
}

void printSymbolTable() {
    printf("Symbol Table:\n");
    printf("ID\tName\tType\tDataType\tScope\tInitialized\n");
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (symbolTable[i].id != -1) {
            printf("%d\t%s\t%s\t%s\t%d\t%d\n", symbolTable[i].id, symbolTable[i].name, symbolTable[i].type, symbolTable[i].dataType, symbolTable[i].scope, symbolTable[i].isInitialized);
        }
    }
}