#pragma once

#define MAX_CLASS_COUNT 64
#define MAX_FUNC_COUNT 256

#if __cplusplus < 201400L
#error Current C++ standard is lower than C++14...
#endif

#ifdef MSVC
#define DLLEXPORT extern "C" __declspec(dllexport)
#else
#define DLLEXPORT extern "C"
#endif

#include <cstdio>
#include <string.h>

typedef void* (*InitFunc)(void);
typedef void (*DeinitFunc)(void*);

typedef struct {
    struct
    {
        const char* className = "";
        InitFunc initFunc = nullptr;
        DeinitFunc deinitFunc = nullptr;
    } ClassInfo[MAX_CLASS_COUNT];
    struct
    {
        const char* funcName = "";
        void* func = nullptr;
    } FuncInfo[MAX_FUNC_COUNT];

} DLL_EXPORT_TABLE_t;

typedef struct {
    InitFunc initFunc = nullptr;
    DeinitFunc deinitFunc = nullptr;
    void* anyFunc = NULL;
} DLL_POINTER_t;

extern DLL_EXPORT_TABLE_t DLL_EXPORT_TABLE;

DLLEXPORT DLL_POINTER_t getClass(const char* className)
{
    DLL_POINTER_t ret;
    for (unsigned int i = 0; i < MAX_CLASS_COUNT; i++) {
        if (strncmp(DLL_EXPORT_TABLE.ClassInfo[i].className, className, 32) == 0) {
            ret.deinitFunc = DLL_EXPORT_TABLE.ClassInfo[i].deinitFunc;
            ret.initFunc = DLL_EXPORT_TABLE.ClassInfo[i].initFunc;
            return ret;
        }
    }
    printf("DLL: No class found!");
    return ret;
}

DLLEXPORT DLL_POINTER_t getFunc(const char* funcName)
{
    DLL_POINTER_t ret;
    for (unsigned int i = 0; i < MAX_FUNC_COUNT; i++) {
        if (strncmp(DLL_EXPORT_TABLE.FuncInfo[i].funcName, funcName, 32) == 0) {
            ret.anyFunc = DLL_EXPORT_TABLE.FuncInfo[i].func;
            return ret;
        }
    }
    return ret;
}
