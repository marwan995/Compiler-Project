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

typedef struct ArgList {
    char** types;
    int count;
} ArgList;

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
        if (symbolTable[i].id != -1 && symbolTable[i].scope == blockIdx && !symbolTable[i].isParam) {
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
        if (symbolTable[i].id != -1 && strcmp(symbolTable[i].type, "func") == 0 
             && strcmp(symbolTable[i].name, name) == 0 ) {
            return 1; //its a fucntion name
        }
        if (symbolTable[i].id != -1 && symbolTable[i].scope == blockIdx
             && strcmp(symbolTable[i].name, name) == 0 ) {
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

void validateNotConst(char *name, int lineNumber) {
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (symbolTable[i].id != -1 && strcmp(symbolTable[i].name, name) == 0) {
            if (strcmp(symbolTable[i].type, "const") == 0) {
                printf("Error: Cannot modify constant %s at line %i\n", name, lineNumber);
                exit(1);
            }
        }
    }
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
    exit(1);
}

char* getSymbolDataType(char *name, int lineNumber) {
    int id = lookup(name, lineNumber);
    if (id < 0 || id >= MAX_SYMBOLS || symbolTable[id].id == -1) {
        printf("Error: Invalid symbol ID %d\n", id);
        exit(1);
    }
    return symbolTable[id].dataType;
}

void checkInitialized(char *name, int lineNumber) {
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (symbolTable[i].id != -1 && strcmp(symbolTable[i].name, name) == 0) { 
            if (symbolTable[i].isParam) {
              continue; // Skip checking for parameters
            }
            if (!symbolTable[i].isInitialized) {
                printf("Error: Variable %s is not initialized at line %i\n", name, lineNumber);
                exit(1);
            }
        }
    }
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

void validateFunctionCall(char* functionName, char** argumentTypes, int argumentCount, int lineNumber) {
    int funcIdx = lookup(functionName, lineNumber);

    if (strcmp(symbolTable[funcIdx].type, "func") != 0) {
        printf("Error: %s is not a function (line %d)\n", functionName, lineNumber);
        exit(1);
    }

    if (argumentCount != symbolTable[funcIdx].paramCount) {
        printf("Error: Function '%s' expects %d arguments, but %d were provided (line %d)\n",
               functionName, symbolTable[funcIdx].paramCount, argumentCount, lineNumber);
        exit(1);
    }

    printSymbolTable(); // Print the symbol table for debugging

    for (int i = 0; i < argumentCount; ++i) {
        int paramId = symbolTable[funcIdx].paramsIds[i];
        printf("paramId: %d, funcId: %d\n", paramId, funcIdx);
        char* expectedType = symbolTable[paramId].dataType;

        if (strcmp(expectedType, argumentTypes[i]) != 0) {
            printf("Error: Type mismatch for parameter %d calling function '%s': expected '%s', got '%s' (line %d)\n",
                   i + 1, functionName, expectedType, argumentTypes[i], lineNumber);
            exit(1);
        }
    }

    printf("Function call to '%s' is valid (line %d)\n", functionName, lineNumber);
}

ArgList* createArgList() {
    ArgList* list = malloc(sizeof(ArgList));
    list->types = malloc(sizeof(char*) * MAX_PARAMS);
    list->count = 0;
    return list;
}

void addArgType(ArgList* list, char* type) {
    list->types[list->count++] = strdup(type);
}

void printParms(){
    int funcIdx = lastFunctionIdx;
    printf("Function %s has %d parameters:\n", symbolTable[funcIdx].name, symbolTable[funcIdx].paramCount);
    for (int i = 0; i < symbolTable[funcIdx].paramCount; i++) {
        int paramId = symbolTable[funcIdx].paramsIds[i];
        printf("Param %d: %s, Type: %s\n", i + 1, symbolTable[paramId].name, symbolTable[paramId].dataType);
    }
    printf("End of parameters\n");
}