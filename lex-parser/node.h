#ifndef NODE_H
#define NODE_H

typedef struct Node {
    int iValue;          /* integer value */
    float fValue;        /* float value */
    int bValue;          /* boolean value */
    char cValue;         /* char value */
    char *sValue;        /* string value */

    char *type;
} Node;

#endif