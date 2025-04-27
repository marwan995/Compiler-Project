## commands 
-  `flex .\calc1.l`
-  `bison --yacc .\calc1.y -d`
-  `gcc .\y.tab.c .\lex.yy.c`
## make 


```bash
# Build the project with specific Lex, Yacc, and target name
make LEX_SRC=<PATH FOR .l> YACC_SRC=<PATH FOR .y> TARGET=<EXE filename>

# Run the compiled executable
make run

# Clean generated files
make clean

# run tests 
make test
```
- we may add a condition to check that the function have return for all its block structure children
- we need to add the minus operation (x = -x)
- full symbol table
- switch case
- Not expression correct way to wright not (!y)
- overlapping between if in quads and assembly
- retrun value is neglacted which is wrong