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

static ObjString* allocateString(char* chars, int length)
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
    ObjString* string = ALLOCATE_OBJ_SIZE( ObjString, 
        sizeof(ObjString) + length * sizeof(char) , OBJ_STRING);
    string->length = length;
    memcpy(string->chars, chars, length);

    return string;
}

ObjString* takeString(char* chars, int length)
{
    return allocateString(chars, length);
}

ObjString* copyString(const char* chars, int length)
{
    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length);
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