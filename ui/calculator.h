#ifndef __CALCULATOR_H__
#define __CALCULATOR_H__

#include <string>
#include <atomic>

using flag_t = std::atomic_bool;
extern flag_t stopCalc;

extern std::string easyCalculate(const char* A, const char* B, const char* C, uint8_t mode);
extern std::string advCalculate(const char* input);

#endif