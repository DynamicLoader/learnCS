#pragma once

class Test1 {
public:
    virtual void test(const char* str) = 0;
    virtual void set(int i) = 0;
    virtual int get() = 0;
};