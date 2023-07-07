/**
 * @file Exceptions.hpp
 * @author Lu Yi
 * @brief Custom Exceptions for the projects
 * @version 0.1
 * @date 2023-05-19
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __EXCEPTIONS_HPP__
#define __EXCEPTIONS_HPP__

#include <exception>

#define _EXCEPTION_IMPL_HELPER_(name, desc, more)                                                                      \
    class name : public std::exception                                                                                 \
    {                                                                                                                  \
      public:                                                                                                          \
        const char *what() const noexcept override                                                                     \
        {                                                                                                              \
            return desc;                                                                                               \
        }                                                                                                              \
        more                                                                                                           \
    }

namespace Exception
{

_EXCEPTION_IMPL_HELPER_(BadArgument,"Bad argument for function or class", );

} // namespace Exception

#endif