typedef enum { typeCon, typeId, typeOpr } nodeEnum;

/* constants */
typedef struct {
    char *type; /* "int", "float", "bool", "char", "string" */
    union {
        int iValue;    /* integer */
        float fValue;  /* float */
        int bValue;    /* boolean */
        char cValue;   /* char */
        char *sValue;  /* string */
    };
} conNodeType;

/* identifiers */
typedef struct {
    char *name;
} idNodeType;

/* operators */
typedef struct {
    int oper;                   /* operator */
    int nops;                   /* number of operands */
    struct nodeTypeTag *op[1];  /* operands, extended at runtime */
} oprNodeType;

typedef struct nodeTypeTag {
    nodeEnum type;              /* type of node */
    union {
        conNodeType con;        /* constants */
        idNodeType id;          /* identifiers */
        oprNodeType opr;        /* operators */
    };
    char *typeName;             /* for type nodes */
} nodeType;