#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

#define ALLOCATE_OBJ_SIZE(type, size, objectType) \
    (type*)allocateObject(size, objectType)

static Obj* allocateObject(size_t size, ObjType type)
{    
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;
    // add the allocated object to the obejct list for GC tracking
    object->next = vm.objects;
    vm.objects = object;
    //
    return object;
}

static ObjString* allocateString(char* chars, int length, uint32_t hash)
{
    // ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    // string->length = length;
    // string->chars = chars;
    // 
    // above : invalid use of flexible array member
    //
    // Reference : https://stackoverflow.com/questions/35423293/flexible-array-member-not-getting-copied-when-i-make-a-shallow-copy-of-a-struct
    // to fix this, manually assign the element or copy the memory explicitly
    //
    ObjString* string = ALLOCATE_OBJ_SIZE(ObjString, 
        sizeof(ObjString) + length * sizeof(char) , OBJ_STRING);
    string->length = length;
    memcpy(string->chars, chars, length);

    string->hash = hash;

    return string;
}

static uint32_t hashString(const char* key, int length)
{
    // Reference :
    // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1a_hash
    //
    // FNV-1a hashing
    uint32_t hash = 2166136261u;

    for (size_t i = 0; i < length; ++i)
    {
        hash ^= key[i];
        // 32 bit FNV_prime = 16777619
        // if 64 bit, use 1099511628211
        hash *= 16777619;
    }

    return hash;
}

ObjString* takeString(char* chars, int length)
{
    uint32_t hash = hashString(chars, length);

    return allocateString(chars, length, hash);
}

ObjString* copyString(const char* chars, int length)
{
    uint32_t hash = hashString(chars, length);

    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length, hash);
}

void printObject(Value value)
{
    switch (OBJ_TYPE(value))
    {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}