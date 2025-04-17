#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOLS 100

typedef struct Symbol {
    int id; // index in the symbol table
    char *name;
    char* type;
    char* dataType;
    int scope; // 0 for global, 1 otherwise
    int isInitialized;
    
    int paramCount; // number of parameters for functions
    struct Symbol *params;
    int isParam;
} Symbol;


typedef struct SymbolTable {
    Symbol symbols[MAX_SYMBOLS];
    struct SymbolTable *parent;
} SymbolTable;

SymbolTable* current;

int blockIdx = 0; 

void initSymbols(SymbolTable *table) {
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        table->symbols[i].id = -1;
    }
}

void initSymbolTable(int lineNumber) {
    printf("Initializing symbol table at line: %i...\n", lineNumber);
    current = (SymbolTable*)malloc(sizeof(SymbolTable));
    current->parent = NULL;
    initSymbols(current);
}

void enterScope(int lineNumber) {
    printf("Entering scope at line: %i...\n", lineNumber);
    blockIdx++;
    SymbolTable *newScope = (SymbolTable*)malloc(sizeof(SymbolTable));
    newScope->parent = current;    
    current = newScope;
    initSymbols(current);
}

void exitScope(int lineNumber) {
    printf("Exiting scope at line: %i...\n", lineNumber);
    
    SymbolTable *oldScope = current;
    current = current->parent;
    free(oldScope);
    
    blockIdx--;
}

int isSymbolDeclared(char *name) {
    // TODO: We need to stop at last function scope
    SymbolTable *temp = current;
    while (temp != NULL) {
        for (int i = 0; i < MAX_SYMBOLS; i++) {
            if (temp->symbols[i].id != -1 && strcmp(temp->symbols[i].name, name) == 0) {
                return 1;
            }
        }
        temp = temp->parent;
    }
    return 0;
}

int insertSymbol(char *name, char* type, char* dataType, int lineNumber) {
    printf("Adding symbol: %s, type: %s, dataType: %s\n, at line: %i", name, type, dataType, lineNumber);
    if(current == NULL) {
        initSymbolTable(lineNumber); // initialize the symbol table if not already done (global scope)
    }

    if (isSymbolDeclared(name)) {
        printf("Error: Symbol %s already declared\n", name);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (current->symbols[i].id == -1) {
            current->symbols[i].id = i;
            current->symbols[i].name = strdup(name);
            current->symbols[i].type = type;
            current->symbols[i].dataType = dataType;
            current->symbols[i].scope = blockIdx;
            current->symbols[i].isInitialized = 0;
            current->symbols[i].paramCount = 0;
            current->symbols[i].params = NULL;
            return i;
        }
    }
    return -1;
}

Symbol* lookUp(char *name, int lineNumber) {
    SymbolTable *temp = current;
    while (temp != NULL) {
        for (int i = 0; i < MAX_SYMBOLS; i++) {
            if (temp->symbols[i].id != -1 && strcmp(temp->symbols[i].name, name) == 0) {
                printf("Symbol found: %s, type: %s, dataType: %s\n", temp->symbols[i].name, temp->symbols[i].type, temp->symbols[i].dataType);
                return &temp->symbols[i];
            }
        }
        temp = temp->parent;
    }
    printf("Symbol %s not found\n", name);
    return NULL;
}