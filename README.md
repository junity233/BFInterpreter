# BFInterpreter
һ����JIT��BF������
[README-english.md](English Edition)

```
ʹ�÷���: bfi [-h] [-s size] [-i input] [-p] [-c] [-e] [-o output]
-h        : չʾ�û��ֲ�
-s        : ָ��cell������Ĭ��Ϊ65535��
-i        : ָ�������ļ���Ĭ�ϴ�stdin���룩
-p        : ��ִ�н�������ʾ�����Ϣ
-c        : ���뵫��ִ��
-e        : ִ���ѱ�����ļ�
-o        : ָ������ļ�
```

�������Ƚ�BF�������Ϊ�������ԣ�������Ҫ��ʵ��ַ�ĵط���Ϊ0.(����Ϊ�˱������õĴ���)
���Ž���������ʵ�ĵ�ַ�������Щ��0����Ȼ��ִ�����´��루����C������Ƕ��ࣩ:
```asm
        mov ebx, cell
        mov ecx, p_getchar
        mov edx, p_putchar
        call p_code
```
cell��ָ��cells��ָ�룬p_getcharָ����getchar��p_putcharָ����putchar��p_codeָ�����õĴ���

# ����ϸ��

## �Ĵ�������;:

|�Ĵ���  |             ��;                     |
|--------|--------------------------------------|
|  EAX   |������ʱ�����ͷ���ֵ                  |
|  EBX   |����ָ��cells��ָ��                   |
|  ECX   |����ָ����getchar��ָ��             |
|  EDX   |����ָ����putchar��ָ��             |

## BFָ��Ͷ�Ӧ�Ļ��ָ��

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
push 0x00       ;����ʵ��ַ���
```
8. '\]
```asm
mov    al,BYTE PTR [ebx]
test   al,al
pop    eax
je     9 <endif>       ;je�����ƫ��
jmp    eax
endif:
```

## �������ָ��:

1. ����Ĵ���
�ڿ�ͷ��������������´��������浱ǰ�Ĵ�����ֵ��
```asm
push eax
push ebx
push ecx
push edx
```
2. �ָ��Ĵ���
�ڽ�β��������������´������ָ��Ĵ�����ֵ��
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
��

