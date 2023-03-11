#pragma once

#include <dlfcn.h>
#include <cstdio>
#include <vector>

class LoadDLL {
private:
    using InitFunc = void* (*)(void);
    using DeinitFunc = void (*)(void*);
    using DLL_POINTER_t = struct {
        InitFunc initFunc = nullptr;
        DeinitFunc deinitFunc = nullptr;
        void* anyFunc = nullptr;
    };
    using DLL_API_t = DLL_POINTER_t (*)(const char*);
    using onErrorCB_t = int (*)(const char* format, ...);
    using objMap_t = struct {
        void* objPtr = nullptr;
        DeinitFunc deinitFunc = nullptr;
    };

    void* _dll = nullptr;
    onErrorCB_t _err = nullptr;
    DLL_API_t _getClass = nullptr;
    DLL_API_t _getFunc = nullptr;
    std::vector<objMap_t> _objMap;

public:
    LoadDLL(const char* dllPath, onErrorCB_t onErrorCallback = printf)
    {
        this->_err = onErrorCallback;
        this->_dll = dlopen(dllPath, RTLD_LAZY);
        if (!_dll) {
            if (onErrorCallback)
                this->_err("Failed to load DLL(%s): %s", dllPath, dlerror());
        } else {
            this->_getClass = (DLL_API_t)dlsym(this->_dll, "getClass");
            this->_getFunc = (DLL_API_t)dlsym(this->_dll, "getFunc");
            if (!this->_getClass || !this->_getFunc) {
                this->_err("Failed to load DLL(%s): Invalid DLL format!", dllPath);
                dlclose(this->_dll);
                this->_dll = nullptr;
            }
        }
    }
    ~LoadDLL()
    {
        if (this->_dll)
            dlclose(this->_dll);
        this->_err("Unloaded DLL");
    }
    operator bool() { return (this->_dll != nullptr) && (this->_getClass != nullptr) && (this->_getFunc != nullptr); }

    void* newClassObj(const char* className)
    {
        if (!(*this))
            return nullptr;
        DLL_POINTER_t ptr = this->_getClass(className);
        if (!ptr.initFunc || !ptr.deinitFunc){
            this->_err("Failed to load class[%s]: Invalid constructor function got!", className);
            return nullptr;
        }
        void* ret = ptr.initFunc();
        if (!ret) {
            this->_err("Failed to load class[%s]: Cannot get valid pointer to new class!", className);
            return nullptr;
        }
        this->_objMap.push_back({ .objPtr = ret, .deinitFunc = ptr.deinitFunc });
        return ret;
    }

    void freeClassObj(void* classObj)
    {
        for (auto x = this->_objMap.begin(); x < this->_objMap.end(); x++) {
            if ((*x).objPtr == classObj) {
                (*x).deinitFunc(classObj);
                this->_objMap.erase(x);
            }
        }
    }

    void* getFunc(const char* funcName)
    {
        if (!(*this))
            return nullptr;
        DLL_POINTER_t ptr = this->_getFunc(funcName);
        if (!ptr.anyFunc)
            return nullptr;
        return ptr.anyFunc;
    }

    template <typename T>
    T getFunc(const char* funcName) { return (T)this->getFunc(funcName); }

    template <class T>
    T& newClassObj(const char* className) { return *((T*)this->newClassObj(className)); }

    template <class T>
    void freeClassObj(T& ref){ this->freeClassObj((void*)ref);}
};
