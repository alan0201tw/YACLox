#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"

// add parentheses around the numerical value to avoid any tragic shenanigans
/**
 *  For example... 
 * 
 *  #define H_SIZE 32                    // header size
 *  #define B_SIZE 1024                  // body size
 *  #define P_SIZE H_SIZE + B_SIZE       // packet size
 *  
 *  and you expect 3 * P_SIZE to be 3 times of the packet size, you get
 *  3 * H_SIZE + B_SIZE, which leads to a tragedy, based on real events.
 */
#define STACK_MAX (256)

typedef struct
{
    Chunk* chunk;
    // instruction pointer
    uint8_t* ip;
    // declare the stack
    Value stack[STACK_MAX];
    // Since the stack grows and shrinks as values are pushed and popped, 
    // we need to track where the top of the stack is in the array
    Value* stackTop;
} VM;

typedef enum
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
void freeVM();

InterpretResult interpret(const char* source);
// stack operations
void push(Value value);
Value pop();

// added
int getLine(Chunk* chunk, int offset);

#endif