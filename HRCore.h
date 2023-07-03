/**
 * @file HRCore.h
 * @author Lu Yi
 * @brief Main include file
 * @version 1.1.0
 * @date 2022-12-06
 * @warning NO safety garantee!
 *
 * @copyright Copyright (c) 2022 - 2023 Lu Yi
 *
 */
#pragma once

// ============================ Begin User Configure =============================

/**
 * @brief This section shows the configure of HRCore
 * @defgroup gp_config Global Configure
 * @{
 */

/**
 * @brief Debug level setting. Default: 2
 * @details 5 level debug is support:
 *
 * - 0 No debug info output
 * - 1 Error is output
 * - 2 Warning + 1
 * - 3 Info + 2
 * - 4 Verbose + 3
 */
#define HRCORE_DEBUG_LEVEL 2

#ifndef HRCORE_HIGHPERF
/**
 * @brief HRCore high performance macro. Default: 0
 * @details Define this to 1, multiply and division will use FFT.
 * @attention Currently not support!
 */
#define HRCORE_HIGHPERF 0
#endif

#ifndef HRCORE_UNIT_SIZE
/**
 * @brief Decide how many bits should be used by single storage unit. Default: 32
 * @details It depends on the max bit width of the platform, selecting from 16,32,64.
 * @note 64 bit usually not support since a 128-bit integer type is needed to be supported on the platform originally.
 */
#define HRCORE_UNIT_SIZE 32
#endif

/**
 * @brief Decide whether use delemeter ',' per thousand in the output of integer part. Default: 1
 */
#ifndef HRCORE_ENABLE_IO_DELIMETER
#define HRCORE_ENABLE_IO_DELIMETER 1
#endif

#ifndef HRCORE_ENABLE_MD
/**
 * @brief Decide whether multiplication and division is enabled. Default: 1
 * @details Define this to 1 to enable multiplication and division.
 * @note It's useful when desire to cut off some memory usage
 */
#define HRCORE_ENABLE_MD 1
#endif

#ifndef HRCORE_ENABLE_FP
/**
 * @brief Determind whether Value::FixedPoint is enabled or not. Default: 1
 * @details Set to 1 to enable Value::FixedPoint support.
 * @note It's useful when desire to cut down some memory usage
 */
#define HRCORE_ENABLE_FP 1
#endif

/**
 * @brief Decide how many bits as a base of Float. Default: 24
 * @details The value of it should >= 16, otherwise precision may lost.
 * @note Value that is too small will trigger a compile warning.
 *
 */
#ifndef HRCORE_FP_BITS_BASE
#define HRCORE_FP_BITS_BASE 24
#endif

/**
 * @brief Decide how many bits for a digit in Float. Default: 8
 * @details The value of it should >= 6, otherwise precision may lost.
 * @note Value that is too small will trigger a compile warning.
 *
 */
#ifndef HRCORE_FP_BITS_PER_DIGIT
#define HRCORE_FP_BITS_PER_DIGIT 8
#endif

/**
 * @brief Set default precision of Value::Float. Default: 10
 */
#ifndef HRCORE_FP_DEFAULT_PRECISION
#define HRCORE_FP_DEFAULT_PRECISION 10
#endif

/**
 * @brief Decide whether Storage::VirtualMMU should be enabled or not. Default: 1
 * @details Storage::VirtualMMU is an implement of Storage::Interface using std::vector. Set to 0 if don't need.
 */
#ifndef HRCORE_ENABLE_STORAGE_VMMU
#define HRCORE_ENABLE_STORAGE_VMMU 1
#endif

/**
 * @brief Define the max length of table of VMMU. Default: 1024
 * @details Space consume is sizeof(void*) * HRCORE_STORAGE_VMMU_MAX_LEN for each object of VMMU.
 * @note This can affect the max count of element it can contain.
 */
#ifndef HRCORE_STORAGE_VMMU_MAX_LEN
#define HRCORE_STORAGE_VMMU_MAX_LEN 1024
#endif

/**
 * @brief Decide whether Utils should be enabled or not. Default: 1
 */
#ifndef HRCORE_ENABLE_UTILS
#define HRCORE_ENABLE_UTILS 1

/**
 * @brief Decide whether IO utils should be enabled or not. Default: 1
 * @details IO utils can be useful on expression calculation. Set to 0 if don't need.
 */
#ifndef HRCORE_ENABLE_UTILS_IO
#define HRCORE_ENABLE_UTILS_IO 1
#endif

#endif

/// @}

// ================================ End User Configure =============================

#ifndef __MERGE__ // Define this to avoid preprocessor join STL headers into merged file
#include <algorithm>
#include <iostream>
#include <exception>
#include <cstdint>

#include <deque> // For expressions
#include <stack>
#include <complex> // For FFT support in the future

#endif

// Pre-compile checks
#if HRCORE_DEBUG_LEVEL >= 4
#define HRCORE_DBG(x) x
#else
#define HRCORE_DBG(x)
#endif

#if HRCORE_DEBUG_LEVEL >= 3
#define HRCORE_DBGI(x) x
#else
#define HRCORE_DBGI(x)
#endif

#if HRCORE_DEBUG_LEVEL >= 2
#define HRCORE_DBGW(x) x
#else
#define HRCORE_DBGW(x)
#endif

#if HRCORE_DEBUG_LEVEL >= 1
#define HRCORE_DBGE(x) x
#else
#define HRCORE_DBGE(x)
#endif

#if HRCORE_ENABLE_FP

#if HRCORE_FP_BITS_BASE < 16
#warning HRCORE_FP_BITS_BASE too small! Float may lose precision.
#endif

#if HRCORE_FP_BITS_PER_DIGIT < 6
#warning HRCORE_FP_BITS_PER_DIGIT too small! Float may lose precision.
#endif

#endif

/**
 * @brief HRCore main namespace, contains all classes.
 * @namespace HRCore
 * @details This namespace also export user classes.
 */
namespace HRCore {

#if (HRCORE_UNIT_SIZE == 16)
using ival_t = uint16_t;
using idval_t = uint32_t;
#define HRCORE_UNIT_DIV 4
#elif (HRCORE_UNIT_SIZE == 32)
using ival_t = uint32_t;
using idval_t = uint64_t;
#define HRCORE_UNIT_DIV 5
#elif (HRCORE_UNIT_SIZE == 64)
using ival_t = uint64_t;
using idval_t = __uint128_t; // Only support with GCC
#define HRCORE_UNIT_DIV 6
#else
#error Invalid Unit Size (i.e. macro HRCORE_UNIT_SIZE) provided!
#endif

/// @brief Complex number type using std::complex<float>
using complex_t = std::complex<float>;

/// @brief A namespace contains HRCore standard exceptions
namespace Exception {

/**
 * @defgroup exception_group Exceptions
 * @brief The section shows the exception types of HRCore.
 * @{
 */

/**
 * @brief HRCore general exception implenent macro defination
 * @note Not using template to avoid a switch cost
 */
#ifndef __HRCORE_EXCEPTION_HELPER__
#define __HRCORE_EXCEPTION_HELPER__(w)            \
    : public std::exception                       \
    {                                             \
    public:                                       \
        const char* what() const throw() override \
        {                                         \
            return w;                             \
        }                                         \
    }
#endif

    class InternalError __HRCORE_EXCEPTION_HELPER__("HRCore internal error"); ///< @brief Exception of internal error. Please dig into it and solve
    class LLR __HRCORE_EXCEPTION_HELPER__("Left operator argument is not bigger than right"); ///< @brief Exception of left < right
    class InvalidArgument __HRCORE_EXCEPTION_HELPER__("Invalid argument provided"); ///< @brief Exception of Invalid Argument
    class DividedByZero __HRCORE_EXCEPTION_HELPER__("Devided by 0 is not allowed"); ///< @brief Exception of divided by 0

#if HRCORE_ENABLE_FP
                                                                                    // class FPAdjustFailed __HRCORE_EXCEPTION_HELPER__("FixedPoint adjust point failed"); ///< @brief Exception of FixedPoint adjust point failed
    class FPConvertFailed __HRCORE_EXCEPTION_HELPER__("FixedPoint failed to convert"); ///< @brief Exception of FixedPoint failed to input
#endif

#if HRCORE_ENABLE_UTILS_IO
    /// @brief Exception os Expression failed to convert
    class ExpressionConvertFailed __HRCORE_EXCEPTION_HELPER__("Expression Failed to convert! Maybe the expression is invalid?");
    /// @brief Exception of Expression evaluation: Invalid operation on seleted data type
    class ExpressionInvalidOperation __HRCORE_EXCEPTION_HELPER__("Invalid operation on seleted data type");
#endif

    ///@}
} // namespace Exception
}

#include "inc/StorageIF.hpp"
#include "inc/VirtualMMU.hpp"
#include "inc/LinkedList.hpp"
#include "inc/Value.hpp"
#include "inc/Utils.hpp"

namespace HRCore {
using BINT = Value::Integer;

#if HRCORE_ENABLE_FP
using BFLOAT = Value::Float;
#endif

#if HRCORE_ENABLE_UTILS_IO
template <typename T, typename U>
using BEXP = Utils::Expression<T, U>;
#endif

} // namespace HRCore
