#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "mman.h"
#include <stdbool.h>

#define version "1.0"
#define DEFAULT_BUF_SIZE 4096

unsigned char* cell;

typedef struct _AsmOpcode {
    char* code;             //Assembly code
    size_t length;          //Length of code
    int num;                //Num of code.
}AsmOpcode;

typedef enum _BFCodeType {
    OP_INC_IP,              // '>'
    OP_DEC_IP,              // '<'
    OP_INC_DATA,            // '+'
    OP_DEC_DATA,            // '-'
    OP_JMP_IF,              // ']'
    OP_SET_JMP,             // '['
    OP_CALL_GETCHAR,        // ','
    OP_CALL_PUTCHAR,        // '.'
    OP_RET,                 // Return
    OP_PUSH_REGS,           // Save regs' state
    OP_POP_REGS             // Recover regs' state
}BFCodeType;

typedef struct _AsmCodeBlock {
    void* code;             // Assembly code address
    size_t length;          // Length of code
    int num;                // Number of instruction
}AsmCodeBlock;



typedef unsigned char byte;
typedef void (*AsmCodeExec)(void);

/**
 * EBX:ptr to cell
 * ECX:ptr to getchar
 * EDX:ptr to putchar
 */

const AsmOpcode asmOpcode[] = {
    //inc ebx
    {"\x43",1,1},
    //dec ebx
    {"\x4b",1,1},
    //inc byte ptr [ebx]
    {"\xFE\x03",2,1},
    //dec byte ptr [ebx]
    {"\xFE\x0B",2,1},
    /**
     *  0:  8a 03                   mov    al,BYTE PTR [ebx]
     *  2:  84 c0                   test   al,al
     *  4:  58                      pop    eax
     *  5:  74 02                   je     9 <endif>
     *  7:  ff e0                   jmp    eax
     *  <endif>:
     *
     * jmp if [ebx]==0
     */
    {"\x8A\x03\x84\xC0\x58\x74\x02\xFF\xE0",9,5},

    /**
     * push 0x000000
     * Need a 4-byte operand.
     *
     * Push the address for ']' to jmp
     */
    {"\x68\x00\x00\x00\x00",5,1},

    /**
     *  0:  51                      push   ecx
     *  1:  52                      push   edx
     *  2:  ff d1                   call   ecx
     *  4:  88 03                   mov    BYTE PTR [ebx],al
     *  6:  5a                      pop    edx
     *  7:  59                      pop    ecx
     *
     * call getchar and save to [ebx]
     */
    {"\x51\x52\xFF\xD1\x88\x03\x5A\x59",8,6},

    /**
     * @brief
     *  0:  51                      push   ecx
     *  1:  52                      push   edx
     *  2:  ff 33                   push   DWORD PTR [ebx]
     *  4:  ff d2                   call   edx
     *  6:  58                      pop    eax
     *  7:  5a                      pop    edx
     *  8:  59                      pop    ecx
     *
     * call putchar([ebx])
    */
    {"\x51\x52\xFF\x33\xFF\xD2\x58\x5A\x59",9,7},

    /**
        * ret
        *
        */
    {"\xC3",1,1},

    /**
        * push eax
        * push ebx
        * push ecx
        * push edx
        *
        * Save current regs' state
        */
    {"\x50\x53\x51\x52",4,4},

    /**
        * pop edx
        * pop ecx
        * pop ebx
        * pop eax
        *
        * Recover reg's state
        */
    {"\x5A\x59\x5B\x58",4,4},



};

void usage() {
    puts("Brainfuck interpreter "version);
    puts("");
    puts("Usage: bfi [-h] [-s size] [-i input] [-p] [-c] [-e] [-o output]");
    puts("-h        : show this message.");
    puts("-s        : set the size of cells");
    puts("-i        : set input file.If not specificed,bfi will read from stdin");
    puts("-p        : print information after program exists");
    puts("-c        : compile without executing");
    puts("-e        : execute compiled file");
    puts("-o        : output file");
}

/**
 * @brief Read source code from file
 * @param file path to file
 * @param length Length of file.
 * @return 
*/
char* readfile(const char* file, size_t* length) {
    char* res;
    FILE* fp = fopen(file, "rb");

    if (fp == NULL) {
        printf("Open file \"%s\" failed!", file);
        abort();
    }

    fseek(fp, 0, SEEK_END);
    (*length) = ftell(fp);
    rewind(fp);

    res = (char*)malloc((*length) + 1);
    if (res == NULL)
    {
        fclose(fp);
        return NULL;
    }

    fread(res, 1, (*length), fp);
    res[(*length)] = '\0';

    return res;
}

/**
 * @brief read source code from console,until EOF.(Windows:Ctrl+Z and Enter)
 * @param length length Length of file.
 * @return 
*/
char* readconsole(size_t* length) {
    char* buf;
    size_t idx = 0;
    size_t capacity = DEFAULT_BUF_SIZE;

    buf = (char*)malloc(DEFAULT_BUF_SIZE);
    if (buf == NULL) {
        fprintf(stderr, "Malloc for buffer failed!");
        abort();
    }

    char ch = getchar();

    while (ch != EOF) {
        if (idx == capacity - 1) {
            //resize
            capacity = capacity * 1.5;
            buf = realloc(buf, capacity);
            if (!buf) {
                free(buf);
                return NULL;
            }
        }
        if (!isspace(ch))
            buf[idx++] = ch;
        ch = getchar();
    }


    buf[idx + 1] = '\0';
    (*length) = idx;
    return buf;
}

/**
 * @brief Return the type of ch
 * @param ch 
 * @return 
*/
BFCodeType getBFCodeType(char ch) {
    switch (ch) {
    case '>':return OP_INC_IP;
    case '<':return OP_DEC_IP;
    case '+':return OP_INC_DATA;
    case '-':return OP_DEC_DATA;
    case '.':return OP_CALL_PUTCHAR;
    case ',':return OP_CALL_GETCHAR;
    case '[':return OP_SET_JMP;
    case ']':return OP_JMP_IF;
    default:
        fprintf(stderr, "Unsupported instruction:%c\n", ch);
        exit(0);
    }
}

/**
 * @brief Compile the source code
 * @param source 
 * @param length 
 * @return 
*/
AsmCodeBlock compileSourceCode(const char* source, size_t length) {
    size_t opcode_length = 0;
    size_t idx = 0;
    size_t code_cnt = 0;

    AsmCodeBlock res = { NULL,0,0 };

    for (size_t i = 0; i < length; i++) {

        if (source[i] == '#' || source[i] == '%')
        {
            while (source[i] != '\n' && i < length)
                i++;
            continue;
        }

        if (isspace(source[i]))
            continue;
        opcode_length += asmOpcode[getBFCodeType(source[i])].length;
    }

    // opcode_length+9 for OP_PUSH_REGS,OP_POP_REGS and OP_RET
    byte* opcodes = mmap(NULL, opcode_length + 9,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (opcodes == NULL)
        return res;

#define WRITE_CODE(type)                                            \
    do{                                                             \
        memcpy(opcodes+idx,asmOpcode[type].code,asmOpcode[type].length);\
        idx+=asmOpcode[type].length;                                \
        code_cnt+=asmOpcode[type].num;                              \
    }while(0)

    WRITE_CODE(OP_PUSH_REGS);

    for (size_t i = 0; i < length; i++) {

        if (source[i] == '#' || source[i] == '%')
        {
            while (source[i] != '\n' && i < length)
                i++;
            continue;
        }

        if (isspace(source[i]))
            continue;
        
        BFCodeType type = getBFCodeType(source[i]);
        
        WRITE_CODE(type);
    }
    WRITE_CODE(OP_POP_REGS);
    WRITE_CODE(OP_RET);

    res.code = opcodes;
    res.length = opcode_length + 9;
    res.num = code_cnt;

    return res;

#undef WRITE_CODE

}

void saveCompileCode(AsmCodeBlock block, const char* file) {
    FILE* fp = fopen(file, "wb");
    size_t length;

    if (fp == NULL) {
        printf("Open file \"%s\" failed\n", file);
        abort();
    }

    fwrite(&block, sizeof(AsmCodeBlock), 1, fp);

    length = fwrite(block.code, 1, block.length, fp);

    if (length < block.length) {
        printf("Write codes to file \"%s\" failed\n", file);
        abort();
    }

    fclose(fp);
}

void completeBlock(AsmCodeBlock block) {
    for (size_t i = 0; i < block.length; i++) {
        byte* ptr = (byte*)block.code + i;
        if (memcmp(ptr,asmOpcode[OP_SET_JMP].code, asmOpcode[OP_SET_JMP].length)==0) {
            memcpy(ptr + 1, &ptr, sizeof(byte*));
            i += 4;
        }
    }
}

AsmCodeBlock readCompiledFile(const char* file) {
    FILE* fp;
    AsmCodeBlock block;
    size_t length;

    fp = fopen(file, "rb");

    if (fp == NULL) {
        fprintf(stderr, "Open file \"%s\" failed\n", file);
        abort();
    }

    fread(&block, sizeof(AsmCodeBlock), 1, fp);

    block.code = mmap(NULL, block.length + 9,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (block.code == NULL) {
        fprintf(stderr, "Malloc for assembly codes failed\n");
        abort();
    }

    length = fread(block.code, 1, block.length, fp);

    if (length < block.length) {
        fprintf(stderr, "Read compiled codes from file \"%s\" failed\n", file);
        abort();    
    }

    fclose(fp);
    return block;
}

void executeCompiledCode(AsmCodeBlock block) {
    completeBlock(block);
    void* p_putchar = putchar, * p_getchar = getchar, * p_code = block.code;

#ifdef __GNUC__
    __asm__ __volatile__("movl %0,%%ebx"::"m"(cell));
    __asm__ __volatile__("movl %0,%%ecx"::"m"(p_getchar));
    __asm__ __volatile__("movl %0,%%edx"::"m"(p_putchar));
    __asm__ __volatile__("call %0"::"m"(opcodes));
#error 'gcc is not supported'' 
#else
    __asm {
        mov ebx, cell
        mov ecx, p_getchar
        mov edx, p_putchar
        call p_code
    }
#endif
}


int main(int argc, char** argv)
{
    size_t cell_size = 65535;
    char* input_file = NULL, *output_file = NULL;
    char* source_code;
    size_t length;
    bool print_inf = false, compile_only = false, execute_compiled = false;
    clock_t clock_start, clock_end;

#ifdef _DEBUG
    print_inf = true;
#endif

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            usage();
            exit(0);
        }
        else if (strcmp(argv[i], "-s") == 0) {
            if (i == argc - 1) {
                usage();
                exit(0);
            }
            cell_size = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-i") == 0) {
            if (i == argc - 1) {
                usage();
                exit(0);
            }
            input_file = argv[++i];
        }
        else if (strcmp(argv[i], "-o") == 0) {
            if (i == argc - 1) {
                usage();
                exit(0);
            }
            output_file = argv[++i];
        }
        else if (strcmp(argv[i], "-p") == 0) {
            print_inf = true;
        }
        else if (strcmp(argv[i], "-c") == 0) {
            compile_only = true;
        }
        else if (strcmp(argv[i], "-e") == 0) {
            execute_compiled = true;
        }
        else {
            printf("Unknown arg:%s\n", argv[i]);
            usage();
            exit(0);
        }
    }


    cell = (char*)malloc(cell_size);
    if (cell == NULL) {
        fprintf(stderr, "Malloc for cells failed.\n");
        abort();
    }

    memset(cell, 0, cell_size);

    AsmCodeBlock codeBlock;
    
    if (execute_compiled) {
        if (!input_file) {
            fprintf(stderr, "To execute compiled file you should specific a input file\n");
            abort();
        }
        codeBlock = readCompiledFile(input_file);
    }
    else {
        if (!input_file)
            source_code = readconsole(&length);
        else source_code = readfile(input_file, &length);

        codeBlock = compileSourceCode(source_code, length);

        if (compile_only) {
            if (!output_file) {
                printf("To compile codes you should specific a output file\n");
                abort();
            }
            saveCompileCode(codeBlock, output_file);
        }
    }

    if (execute_compiled || !compile_only) {
        clock_start = clock();
        executeCompiledCode(codeBlock);
        clock_end = clock();
        if (print_inf) {
            clock_t t = clock_end - clock_start;
            printf("\n\n");
            printf("Time totally used:%d ms\n", t);
            if(!execute_compiled)
                printf("Source code:%d bytes\n", length);
            printf("Assembly code:%d instructions, %d bytes\n", codeBlock.num, codeBlock.length);
        }
    }
}