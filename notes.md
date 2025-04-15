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
```