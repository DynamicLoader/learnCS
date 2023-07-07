/**
 * @file ErrorNo.h
 * @author Lu Yi
 * @brief To define global error code
 * @version 0.1
 * @date 2023-05-24
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __ERRORNO_H__
#define __ERRORNO_H__

constexpr int ERR_NONE = 0;

/**
 * @brief All error number should be negative;
 *
 */

constexpr int ModuleCommon = 0;
constexpr int ModuleTNAM = -100;
constexpr int ModuleRNAM = -200;
constexpr int ModuleQIM = -300;
constexpr int ModuleZIF = -400;

// Common errors
constexpr int ERR_OPEN_FILE = -10;
constexpr int ERR_READ_FILE = -11;
constexpr int ERR_WRITE_FILE = -12;
constexpr int ERR_SEEK_FILE = -13;
constexpr int ERR_EARLY_EOF = -14;

// RNAM
constexpr int ERR_INVALID_IMAGE_SIZE = -20;
constexpr int ERR_BUSY = -21;

constexpr int ERR_NO_SOURCE = -30;
constexpr int ERR_FORGET_ENCODE = -31;
constexpr int ERR_FORGET_DECODE = -32;

constexpr int ERR_WRONG_CHAN_INDEX = -40;

// TNAM
constexpr int ERR_NO_PARENT = -10;
constexpr int ERR_EMPTY_INPUT = -11;

// ZIF
constexpr int ERR_UNSUPPORTED_VERSION = -10;
constexpr int ERR_INVALID_FORMAT = -11;

#define ERRORNO_EX(module, name) (Module##module + name)
#define ERRORNO(name) (ERRORNO_EX(Common, name))

#endif