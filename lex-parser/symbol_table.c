#ifndef __SYMBOL_TABLE_C__
#define __SYMBOL_TABLE_C__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "parser.h"
#include "node.h"
#include "utils.h"
#define MAX_SYMBOLS 10000
#define MAX_PARAMS 200

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
    Node* nodeValue;

    bool isForLoop;
    bool isUsed; // used in the code
    int symbolLine; // line number of the symbol declaration
    int paramCount; // number of parameters for functions
    int* paramsIds;
    bool isParam;
    bool hasReturn;
} Symbol;

Symbol symbolTable[MAX_SYMBOLS];
Symbol symbolTableSnapshot[MAX_SYMBOLS];
int snapshotCount = 0; // Tracks number of snapshots

int blockIdx = -1; 
int lastFunctionIdx = -1; 
int insideFunctionIdx = -1;
char error_msg[256];
static int g_snapshot_id_counter = 0;  // Only for snapshots

typedef struct ArgList {
    char** types;
    int count;
} ArgList;

void initSymbolTable() {
    blockIdx = 0;
    snapshotCount = 0;
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        symbolTable[i].id = -1;
        symbolTableSnapshot[i].id = -1; 
    }
}
void updateSnapshot(Symbol* symbol) {
    for (int i = 0; i < snapshotCount; i++) {
        if (symbolTableSnapshot[i].id != -1 &&
            strcmp(symbolTableSnapshot[i].name, symbol->name) == 0 &&
            symbolTableSnapshot[i].symbolLine == symbol->symbolLine &&
            strcmp(symbolTableSnapshot[i].dataType, symbol->dataType) == 0 &&
            symbolTableSnapshot[i].scope == symbol->scope) {
                printf("name is %s",symbolTableSnapshot[i].name);
            // Exact match found - don't add duplicate
            return;
        }
    }

    // Only add new snapshot if table isn't full
    if (snapshotCount >= MAX_SYMBOLS) {
        fprintf(stderr, "Error: Snapshot table is full\n");
        exit(1);
    }

    // Add new snapshot entry
    symbolTableSnapshot[snapshotCount].id = g_snapshot_id_counter++;
    symbolTableSnapshot[snapshotCount].name = strdup(symbol->name);
    symbolTableSnapshot[snapshotCount].type = strdup(symbol->type);
    symbolTableSnapshot[snapshotCount].dataType = strdup(symbol->dataType);
    symbolTableSnapshot[snapshotCount].scope = symbol->scope;
    symbolTableSnapshot[snapshotCount].isInitialized = symbol->isInitialized;
    symbolTableSnapshot[snapshotCount].nodeValue = symbol->nodeValue;
    symbolTableSnapshot[snapshotCount].isForLoop = symbol->isForLoop;
    symbolTableSnapshot[snapshotCount].isUsed = symbol->isUsed;
    symbolTableSnapshot[snapshotCount].symbolLine = symbol->symbolLine;
    symbolTableSnapshot[snapshotCount].paramCount = symbol->paramCount;
    symbolTableSnapshot[snapshotCount].isParam = symbol->isParam;
    symbolTableSnapshot[snapshotCount].hasReturn = symbol->hasReturn;

    if (symbol->paramCount > 0) {
        symbolTableSnapshot[snapshotCount].paramsIds = malloc(MAX_PARAMS * sizeof(int));
        memcpy(symbolTableSnapshot[snapshotCount].paramsIds, symbol->paramsIds, MAX_PARAMS * sizeof(int));
    } else {
        symbolTableSnapshot[snapshotCount].paramsIds = NULL;
    }

    snapshotCount++;
}

void checkMain() {
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (symbolTable[i].id != -1 && 
            strcmp(symbolTable[i].name, "main") == 0 && 
            strcmp(symbolTable[i].type, "func") == 0) {
            return; // Found main function
        }
    }
    customError("No main function defined in the program");
}
void printSymbolTable() {
    FILE* file = fopen("symbol_table.txt", "w");
    if (!file) {
        perror("Could not open symbol table output file");
        return;
    }

    fprintf(file, "ID\tName\tType\tDataType\tScope\tInitialized\tLine\tUsed\tParam\n");

    // Print all active symbols first
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (symbolTableSnapshot[i].id != -1) {
            fprintf(file, "%d\t%s\t%s\t%s\t%d\t%d\t%d\t%d\t%d\n",
                    symbolTableSnapshot[i].id,
                    symbolTableSnapshot[i].name ? symbolTableSnapshot[i].name : "(null)",
                    symbolTableSnapshot[i].type ? symbolTableSnapshot[i].type : "(null)",
                    symbolTableSnapshot[i].dataType ? symbolTableSnapshot[i].dataType : "(null)",
                    symbolTableSnapshot[i].scope,
                    symbolTableSnapshot[i].isInitialized,
                    symbolTableSnapshot[i].symbolLine,
                    symbolTableSnapshot[i].isUsed,
                    symbolTableSnapshot[i].isParam);
        }
    }

    fclose(file);
}

void enterScope(int lineNumber) {
    printf("Entering scope at line: %i...\n", lineNumber);
    blockIdx++;
}

void exitScope(int lineNumber) {
    printf("Exiting scope at line: %i...\n", lineNumber);
    
    
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (symbolTable[i].id != -1 && symbolTable[i].scope == blockIdx) {
            updateSnapshot(&symbolTable[i]);
        }
    }
    
    // Second pass: free current scope variables
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

bool checkPramsForFunction(char*name){
    if(insideFunctionIdx < 0) {
        return false;
    }

    if (symbolTable[insideFunctionIdx].id == -1) {
        return false; // No function in scope
    }

    int size =symbolTable[insideFunctionIdx].paramCount; 
    if (size == 0) {
        return false; // No parameters in the function
    }

    for (int i = 0; i < size; i++) {
        int paramId = symbolTable[insideFunctionIdx].paramsIds[i];
        if (strcmp(symbolTable[paramId].name, name) == 0 && symbolTable[i].scope == blockIdx) {
            return true; // Parameter found
        }
    }
    return false; 
}
int isSymbolInSameScope(char *name) {
    if (checkPramsForFunction(name)) {
        return 1; // Parameter found in the function
    }
    
    // TODO: We need to stop at last function scope
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (symbolTable[i].id != -1 && strcmp(symbolTable[i].type, "func") == 0 
             && strcmp(symbolTable[i].name, name) == 0 ) {
            return 1; 
        }
    
        if (symbolTable[i].id != -1 && symbolTable[i].scope == blockIdx
             && strcmp(symbolTable[i].name, name) == 0 && symbolTable[i].isParam == false) {
            return 1; 
        }

    }
 
    return 0;
}


void checkForUnusedVars() {
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (symbolTable[i].id != -1 && symbolTable[i].scope == blockIdx && !symbolTable[i].isUsed && strcmp(symbolTable[i].type, "func") != 0) {
            snprintf(error_msg, sizeof(error_msg), "Variable %s is declared but not used\n", symbolTable[i].name);
            yywarn(error_msg, symbolTable[i].symbolLine);
        }
    }
}

bool insertParamToFunction(int functionIdx, int paramIdx) {
    if (symbolTable[functionIdx].paramCount >= MAX_PARAMS) {
        customError("Maximum number of parameters exceeded for function %s\n", symbolTable[functionIdx].name);
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

int lookup(char *name) {
    int currentScope = blockIdx;
    while( currentScope >= 0) {
        for (int i = 0; i < MAX_SYMBOLS; i++) {
            int equalScope = currentScope;
            if(symbolTable[i].id != -1 && (symbolTable[i].isParam || symbolTable[i].isForLoop)) {
                equalScope += 1;
            }

            if (symbolTable[i].id != -1 && symbolTable[i].scope == equalScope && strcmp(symbolTable[i].name, name) == 0) {
                printf("Found symbol: %s, id: %i\n", name, symbolTable[i].id);
                return symbolTable[i].id;
            }
        }
        currentScope--;
    }
    customError("Variable %s is not defined", name);
    exit(1);
}

void setVarUsed(char *name) {
    int idx = lookup(name);
    symbolTable[idx].isUsed = true;
    updateSnapshot(&symbolTable[idx]);
}

char* getSymbolDataType(char *name) {
    int id = lookup(name);
    if (id < 0 || id >= MAX_SYMBOLS || symbolTable[id].id == -1) {
        printf("Error: Invalid symbol ID %d\n", id);
        exit(1);
    }
    return symbolTable[id].dataType;
}

int insertSymbol(char *name, char* type, char* dataType, bool isInitialized, bool isParam, bool isForLoop, int lineNumber) {
    printf("Adding symbol: %s, type: %s, dataType: %s, at line: %i\n", name, type, dataType, lineNumber);

    if (isSymbolInSameScope(name)) {
        customError("Symbol %s already declared", name);
        return -1;
    }

    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (symbolTable[i].id == -1) {

            symbolTable[i].id = i;
            symbolTable[i].name = strdup(name);
            symbolTable[i].type = strdup(type);
            symbolTable[i].dataType = strdup(dataType);
            symbolTable[i].symbolLine = lineNumber;

            if(isParam || isForLoop) {
                symbolTable[i].scope = blockIdx + 1;
            } else {
                symbolTable[i].scope = blockIdx;
            }

            if(isParam) {
                bool isInserted = insertParamToFunction(lastFunctionIdx, i);
                if (!isInserted) {
                    printf("Error: Could not insert parameter %s to function at line %i\n", name, lineNumber);
                    exit(1);
                }
            } 

            symbolTable[i].isForLoop = isForLoop;
            symbolTable[i].isParam = isParam;
            symbolTable[i].isInitialized = isInitialized;
            symbolTable[i].paramCount = 0;
            
            if (strcmp(type, "func") == 0) {
                lastFunctionIdx = i;
                insideFunctionIdx = i;
                initParams(lastFunctionIdx);
                if (strcmp(dataType, "void") == 0) {
                    symbolTable[i].hasReturn = true;
                } else {
                    symbolTable[i].hasReturn = false;
                }
            }
            updateSnapshot(&symbolTable[i]);            
            return i;
        }
    }

    printf("Error: Symbol table is full, cannot insert symbol %s at line %i\n", name, lineNumber);
    exit(1);
}

void insertParam(char *name, char* type, char* dataType, bool isInitialized, Node* node, int lineNumber) {
    int paramIdx = insertSymbol(name, type, dataType, isInitialized, true, false, lineNumber);
    symbolTable[paramIdx].nodeValue = node;
}

void insertVarConst (char *name, char* type, char* dataType, bool isInitialized, int lineNumber) {
     insertSymbol(name, type, dataType, isInitialized, false, false, lineNumber);
   
}

void insertFunc(char *name, char* type, char* dataType, int lineNumber) {
    for(int i=0; i < MAX_SYMBOLS; i++) {
        if (symbolTable[i].id != -1 && strcmp(symbolTable[i].name, name) == 0 && strcmp(symbolTable[i].type, "func") == 0) {
            customError("Function %s already declared", name);
            return;
        }
    }
    insertSymbol(name, type, dataType, false, false, false, lineNumber);
    
}

void insertForLoopVar(char *name, char* type, char* dataType, int lineNumber) {
    if(dataType == NULL){
        dataType = getSymbolDataType(name);
        if(strcmp(dataType, "int") != 0){
            customError("For loop variable %s must be of type int", name);
           return ;
        }
    }
    
    insertSymbol(name, type, dataType, true, false, true, lineNumber);
}

void validateNotConst(char *name) {
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (symbolTable[i].id != -1 && strcmp(symbolTable[i].name, name) == 0) {
            if (strcmp(symbolTable[i].type, "const") == 0) {
                customError("Cannot modify constant %s", name);
                exit(1);
                return;
            }
        }
    }
}

void validateReturnType(char *returnType, int lineNumber) {
    if (lastFunctionIdx == -1) {
        printf("Error: No function in scope to validate return type at line %i\n", lineNumber);
        exit(1);
    }
    if (strcmp(symbolTable[lastFunctionIdx].dataType, "void") == 0) {
        return;
    }

    if (strcmp(symbolTable[lastFunctionIdx].dataType, returnType) != 0) {
        customError("Return type mismatch for function %s: expected %s, got %s at line %i\n",
               symbolTable[lastFunctionIdx].name, symbolTable[lastFunctionIdx].dataType, returnType, lineNumber);
    }
}

void markFunctionReturnType(int lineNumber) {
    if (lastFunctionIdx == -1) {
        printf("Error: No function in scope to mark return type at line %i\n", lineNumber);
        exit(1);
    }
    if(blockIdx-1 > symbolTable[lastFunctionIdx].scope) {
        return;
    }
    if(blockIdx-1 == symbolTable[lastFunctionIdx].scope) {
        symbolTable[lastFunctionIdx].hasReturn = true;
        updateSnapshot(&symbolTable[lastFunctionIdx]);
    } else {
        printf("blockIdx: %d, function scope: %d\n", blockIdx, symbolTable[lastFunctionIdx].scope);
        customError("Function %s is not in the current scope at line %i\n", symbolTable[lastFunctionIdx].name, lineNumber);
    }
}

void checkLastFunctionReturnType(int lineNumber) {
    if (lastFunctionIdx == -1) {
        printf("Error: No function in scope to check return type at line %i\n", lineNumber);
        exit(1);
    }
    if (!symbolTable[lastFunctionIdx].hasReturn) {
        snprintf(error_msg, sizeof(error_msg), 
                 "Function %s does not have a return statement at line %d",
               symbolTable[lastFunctionIdx].name, lineNumber);
        yyerror(error_msg);
    }
}

void checkInitialized(char *name, int lineNumber) {
    int i = lookup(name);
    if (symbolTable[i].isParam) {
        return; // Skip checking for parameters
    }
    if (!symbolTable[i].isInitialized) {
        customError("Variable %s is not initialized at line %i\n", name, lineNumber);
    }

}

int getInitializedParamCount(int funcIdx) {
    int count = 0;
    for (int i = 0; i < symbolTable[funcIdx].paramCount; i++) {
        int paramId = symbolTable[funcIdx].paramsIds[i];
        if (symbolTable[paramId].isInitialized) {
            count++;
        }
    }
    return count;
}

void validateFunctionCall(char* functionName, char** argumentTypes, int argumentCount) {
    int funcIdx = lookup(functionName);

    if (strcmp(symbolTable[funcIdx].type, "func") != 0) {
        customError("%s is not a function", functionName);
        return;
    }

    int initializedParamCount = getInitializedParamCount(funcIdx);
    int totalParamCount = symbolTable[funcIdx].paramCount;
    if (argumentCount <  totalParamCount - initializedParamCount || argumentCount > totalParamCount) {
        customError("Function '%s' expects at least %d arguments, but %d were provided ",
               functionName, symbolTable[funcIdx].paramCount - initializedParamCount, argumentCount);
        return;
    }

    for (int i = 0; i < argumentCount; ++i) {
        int paramId = symbolTable[funcIdx].paramsIds[i];
        printf("paramId: %d, funcId: %d\n", paramId, funcIdx);
        char* expectedType = symbolTable[paramId].dataType;

        if (strcmp(expectedType, argumentTypes[i]) != 0) {
            customError("Type mismatch for parameter %d calling function '%s': expected '%s', got '%s'",
                   i + 1, functionName, expectedType, argumentTypes[i]);
            return;
        }
    }
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
void cleanupSymbolTableSnapshot() {
    for (int i = 0; i < snapshotCount; i++) {
        if (symbolTableSnapshot[i].id != -1) {
            free(symbolTableSnapshot[i].name);
            free(symbolTableSnapshot[i].type);
            free(symbolTableSnapshot[i].dataType);
            if (symbolTableSnapshot[i].paramCount > 0) {
                free(symbolTableSnapshot[i].paramsIds);
            }
            symbolTableSnapshot[i].id = -1;
        }
    }
    snapshotCount = 0;
}
#endif