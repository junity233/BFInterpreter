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

The interpreter first compile the bf source code to assembly code.In the code real addresses is filled with 0x00.(for saving compiled code)
Then the interpreter fill these 0x00 with real addresses,and execute assembly code below:
```asm
        mov ebx, cell
        mov ecx, p_getchar
        mov edx, p_putchar
        call p_code
```
In the code 'cell' is pointer to the cells,'p_getchar' is pointer to function 'int getchar(void)','p_putchar' is pointer to function 'int putchar(int)',
and 'p_code' is pointer to the compiled codes.

# compilation details

## Registers:

|Register|                   Purpose            |
|--------|--------------------------------------|
|  EAX   |Store temporary value and return value|
|  EBX   |Store pointer to cells                |
|  ECX   |Store pointer to 'getchar'            |
|  EDX   |Store pointer to 'putchar'            |

## Assembly code corresponding to BF instruction

1. '>'
```asm
inc ebx
```
2. '<'
```asm
dec ebx
```
3. '+'
```asm
inc [ebx]
```
4. '-'
```asm
dec [ebx]
```
5. ','
```asm
push   ecx
push   edx
call   ecx
mov    BYTE PTR [ebx],al
pop    edx
pop    ecx

```
6. '.'
```asm
push   ecx
push   edx
push   DWORD PTR [ebx]
call   edx
pop    eax
pop    edx
pop    ecx

```
7. '\['
```asm
push 0x00       ;Will be filled with real address
```
8. '\]
```asm
mov    al,BYTE PTR [ebx]
test   al,al
pop    eax
je     9 <endif>       ;Relative displacement
jmp    eax
endif:
```

## Other assembly code:

1. Store registers
At the beginning of assembly compiler will insert these code to store registers:
```asm
push eax
push ebx
push ecx
push edx
```
2. Recover registers
At the end the compiler will insert these code to recover registers:
```asm
pop edx
pop ecx
pop ebx
pop eax
```
3.Ret
```
ret
```

