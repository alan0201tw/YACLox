#include "vm.h"

#include "debug.h"

// a damned global variable
VM vm;

void initVM()
{

}

void freeVM()
{

}

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

    for(;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
        case OP_CONSTANT_LONG:
        {
            uint32_t index = READ_BYTE();
            index |= READ_BYTE() << 8;
            index |= READ_BYTE() << 16;
            push(vm.chunk->constants.values[index]);
            break;
        }
        case OP_CONSTANT:
        {
            Value constant = READ_CONSTANT();
            printValue(constant);
            printf("\n");
            break;
        }
        case OP_RETURN:
            return INTERPRET_OK;

        default:
            break;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
}

InterpretResult interpret(Chunk* chunk)
{
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}