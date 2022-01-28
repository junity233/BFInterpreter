# BFInterpreter
Interpreter for BrainFuck with JIT

```
Usage: bfi [-h] [-s size] [-i input] [-p] [-c] [-e] [-o output]
-h        : show this message.
-s        : specific the size of cells
-i        : specific input file.If not specificed,bfi will read from stdin
-p        : print information after program exists
-c        : compile without executing
-e        : execute compiled file
-o        : specific output file
```

The interpreter first compile the bf source code to assembly code.In the code the real address is filled with 0x00.(for saving compiled code)
Then the interpreter fill these 0x00 with real address,and execute assembly code below:
```asm
        mov ebx, cell
        mov ecx, p_getchar
        mov edx, p_putchar
        call p_code
```
In the code 'cell' is pointer to the cells,'p_getchar' is pointer to function 'int getchar(void)','p_putchar' is pointer to function 'int putchar(int)',
and 'p_code' is pointer to the compiled codes.
