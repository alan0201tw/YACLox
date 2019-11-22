#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct
{
    Token current;
    Token previous;

    bool hadError;
    bool isInPanicMode;
} Parser;

// this enum specifies the precedence between different expressions,
// thus, specifies the precedence of operators at the same time.
typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . () []
    PREC_PRIMARY
} Precedence;

// typedef a ParseFn as function type
typedef void (*ParseFn)();

typedef struct
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

Parser parser;

Chunk* compilingChunk;

static Chunk* currentChunk()
{
    return compilingChunk;
}

static void errorAt(Token* token, const char* message)
{
    // suppress other errors while in panic mode
    if(parser.isInPanicMode) return;
    parser.isInPanicMode = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF)
    {
        fprintf(stderr, " at end");
    }
    else if (token->type == TOKEN_ERROR)
    {
        // Nothing.
    }
    else
    {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void error(const char* message)
{
    errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char* message)
{
    errorAt(&parser.current, message);
}

static void advance()
{
    parser.previous = parser.current;

    for(;;)
    {
        parser.current = scanToken();
        if(parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char* message)
{
    if (parser.current.type == type)
    {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static void emitByte(uint8_t byte)
{
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn()
{
    emitByte(OP_RETURN);
}

// TODO : following functions should be supporting OP_CONSTANT_LONG

static uint8_t makeConstant(Value value)
{
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX)
    {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

static void emitConstant(Value value)
{
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void endCompiler()
{
    emitReturn();

#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError)
    {
        disassembleChunk(currentChunk(), "code");
    }
#endif
}

static void expression();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static void binary()
{
    TokenType operatorType = parser.previous.type;

    // compile the right operand
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    // Emit the operator instruction
    // : Transfer operator token to an OpCode
    switch(operatorType)
    {
        case TOKEN_PLUS: emitByte(OP_ADD); break;
        case TOKEN_MINUS: emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR: emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH: emitByte(OP_DIVIDE); break;
        default:
            return; // unreachable
    }
}

/**
 *  As far as the back end is concerned, there’s literally nothing to 
 *  a grouping expression. Its sole function is syntactic—it lets you 
 *  insert a lower precedence expression where a higher precedence is expected.
 *  Thus, it has no runtime semantics on its own and therefore doesn’t emit 
 *  any bytecode. The inner call to expression() takes care of generating 
 *  bytecode for the expression inside the parentheses.
 */
static void grouping()
{
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number()
{
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

static void unary()
{
    TokenType operatorType = parser.previous.type;

    // compile the operand
    // by calling this instead of expression(), we disallow expressions
    // with precedence lower than unary.
    //
    // for example, if we call expression()
    // - 5 + 3 becomes - (5 + 3)
    // instead we take precedence into account, and obtain the expression "5"
    // at level PREC_PRIMARY.
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction.
    switch (operatorType)
    {
    case TOKEN_MINUS: emitByte(OP_NEGATE); break;
    
    default:
        return; // should never reach here
    }

    // the emitted OP_NEGATE will negate the previous instructions/values 
    // emitted by the expression() call.
    /**
     *  It might seem a little weird to write the negate instruction after 
     *  its operand’s bytecode since the - appears on the left, 
     *  but think about it in terms of order of execution:
     *  We evaluate the operand first which leaves its value on the stack.
     *  Then we pop that value, negate it, and push the result.
     * 
     *  So the OP_NEGATE instruction should be emitted last.
     */
}

ParseRule rules[] = 
{
    { grouping, NULL,    PREC_NONE },       // TOKEN_LEFT_PAREN
    { NULL,     NULL,    PREC_NONE },       // TOKEN_RIGHT_PAREN
    { NULL,     NULL,    PREC_NONE },       // TOKEN_LEFT_BRACE
    { NULL,     NULL,    PREC_NONE },       // TOKEN_RIGHT_BRACE
    { NULL,     NULL,    PREC_NONE },       // TOKEN_COMMA
    { NULL,     NULL,    PREC_NONE },       // TOKEN_DOT
    { unary,    binary,  PREC_TERM },       // TOKEN_MINUS
    { NULL,     binary,  PREC_TERM },       // TOKEN_PLUS
    { NULL,     NULL,    PREC_NONE },       // TOKEN_SEMICOLON
    { NULL,     binary,  PREC_FACTOR },     // TOKEN_SLASH
    { NULL,     binary,  PREC_FACTOR },     // TOKEN_STAR
    { NULL,     NULL,    PREC_NONE },       // TOKEN_BANG
    { NULL,     NULL,    PREC_NONE },       // TOKEN_BANG_EQUAL
    { NULL,     NULL,    PREC_NONE },       // TOKEN_EQUAL
    { NULL,     NULL,    PREC_NONE },       // TOKEN_EQUAL_EQUAL
    { NULL,     NULL,    PREC_NONE },       // TOKEN_GREATER
    { NULL,     NULL,    PREC_NONE },       // TOKEN_GREATER_EQUAL
    { NULL,     NULL,    PREC_NONE },       // TOKEN_LESS
    { NULL,     NULL,    PREC_NONE },       // TOKEN_LESS_EQUAL
    { NULL,     NULL,    PREC_NONE },       // TOKEN_IDENTIFIER
    { NULL,     NULL,    PREC_NONE },       // TOKEN_STRING
    { number,   NULL,    PREC_NONE },       // TOKEN_NUMBER
    { NULL,     NULL,    PREC_NONE },       // TOKEN_AND
    { NULL,     NULL,    PREC_NONE },       // TOKEN_CLASS
    { NULL,     NULL,    PREC_NONE },       // TOKEN_ELSE
    { NULL,     NULL,    PREC_NONE },       // TOKEN_FALSE
    { NULL,     NULL,    PREC_NONE },       // TOKEN_FOR
    { NULL,     NULL,    PREC_NONE },       // TOKEN_FUN
    { NULL,     NULL,    PREC_NONE },       // TOKEN_IF
    { NULL,     NULL,    PREC_NONE },       // TOKEN_NIL
    { NULL,     NULL,    PREC_NONE },       // TOKEN_OR
    { NULL,     NULL,    PREC_NONE },       // TOKEN_PRINT
    { NULL,     NULL,    PREC_NONE },       // TOKEN_RETURN
    { NULL,     NULL,    PREC_NONE },       // TOKEN_SUPER
    { NULL,     NULL,    PREC_NONE },       // TOKEN_THIS
    { NULL,     NULL,    PREC_NONE },       // TOKEN_TRUE
    { NULL,     NULL,    PREC_NONE },       // TOKEN_VAR
    { NULL,     NULL,    PREC_NONE },       // TOKEN_WHILE
    { NULL,     NULL,    PREC_NONE },       // TOKEN_ERROR
    { NULL,     NULL,    PREC_NONE },       // TOKEN_EOF
};

static void parsePrecedence(Precedence precedence)
{
    // ook up a prefix parser for the current token
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if(prefixRule == NULL)
    {
        error("Expect expression");
        return;
    }
    prefixRule();

    // infix expression
    while(precedence <= getRule(parser.current.type)->precedence)
    {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule();
    }
}

static ParseRule* getRule(TokenType type)
{
    return &rules[type];
}

void expression()
{
    parsePrecedence(PREC_ASSIGNMENT);
}

/**
 *  We pass in the chunk where the compiler will write the code, 
 *  and then compile() returns whether or not compilation succeeded.
 */
bool compile(const char* source, Chunk* chunk)
{
    initScanner(source);
    compilingChunk = chunk;

    parser.hadError = false;
    parser.isInPanicMode = false;

    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression");

    endCompiler();

    return !parser.hadError;
}








/**
 *  Reference : http://www.craftinginterpreters.com/scanning-on-demand.html#a-token-at-a-time
 * 
 *  That %.*s in the format string is a neat feature. 
 *  Usually, you set the output precision — the number of characters to show — by placing a number 
 *  inside the format string. Using * instead lets you pass the precision as an argument. 
 *  So that printf() call prints the first token.length characters of the string at token.start. 
 *  We need to limit the length like that because the lexeme points into the original source 
 *  string and doesn’t have a terminator at the end.
 */
//printf("%2d '%.*s'\n", token.type, token.length, token.start);