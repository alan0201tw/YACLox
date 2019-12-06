#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"

#define OBJ_TYPE(value)         (AS_OBJ(value)->type)
#define IS_STRING(value)        isObjType(value, OBJ_STRING)
#define AS_STRING(value)        ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)       (((ObjString*)AS_OBJ(value))->chars)

typedef enum
{
    OBJ_STRING,
} ObjType;

struct sObj
{
    ObjType type;
};

// Reference : 
// http://www.craftinginterpreters.com/strings.html#struct-inheritance
// You can take a pointer to a struct and 
// safely convert it to a pointer to its first field and back.
struct sObjString
{
    // if we cast a sObjString pointer to a sObj pointer, it can 
    // access sObj's member field safely.
    Obj obj;
    int length;
    char* chars;
};

ObjString* copyString(const char* chars, int length);

// Why use a function rather than macro?
//
// If a macro uses a parameter more than once, 
// that expression gets evaluated multiple times.
//
// IS_STRING(POP()) -> POP() gets evaluated twice, 
// which means it pops twice
static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif