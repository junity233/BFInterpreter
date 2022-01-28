# BFInterpreter
一个有JIT的BF解释器
[README-english.md](English Edition)

```
使用方法: bfi [-h] [-s size] [-i input] [-p] [-c] [-e] [-o output]
-h        : 展示用户手册
-s        : 指定cell数量（默认为65535）
-i        : 指定输入文件（默认从stdin输入）
-p        : 在执行结束后显示相关信息
-c        : 编译但不执行
-e        : 执行已编译的文件
-o        : 指定输出文件
```

解释器先将BF代码编译为机器语言，并将需要真实地址的地方置为0.(这是为了保存编译好的代码)
接着解释器用真实的地址来填充这些“0”，然后执行以下代码（这是C语言内嵌汇编）:
```asm
        mov ebx, cell
        mov ecx, p_getchar
        mov edx, p_putchar
        call p_code
```
cell是指向cells的指针，p_getchar指向函数getchar，p_putchar指向函数putchar，p_code指向编译好的代码

# 编译细节

## 寄存器及用途:

|寄存器  |             用途                     |
|--------|--------------------------------------|
|  EAX   |保存临时变量和返回值                  |
|  EBX   |保存指向cells的指针                   |
|  ECX   |保存指向函数getchar的指针             |
|  EDX   |保存指向函数putchar的指针             |

## BF指令和对应的汇编指令

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
push 0x00       ;用真实地址填充
```
8. '\]
```asm
mov    al,BYTE PTR [ebx]
test   al,al
pop    eax
je     9 <endif>       ;je是相对偏移
jmp    eax
endif:
```

## 其它汇编指令:

1. 保存寄存器
在开头编译器会插入以下代码来保存当前寄存器的值：
```asm
push eax
push ebx
push ecx
push edx
```
2. 恢复寄存器
在结尾编译器会插入以下代码来恢复寄存器的值：
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
略

