#ifndef __COMMON_H__
#define __COMMON_H__

#ifndef COMPILER_CONFIG
#define COMPILER_CONFIG

// Config here

constexpr unsigned int COMPILER_BZLIB_BUFSIZE = 1048576 * 16; // 16M

#define COMPILER_DEFAULT_LOG_LEVEL 3

#define COMPILER_LOG_ERROR_COLOR "\033[1;31m" // Bright Red

#define COMPILER_LOG_WARNING_COLOR "\033[1;33m" // Bright Yellow

#define COMPILER_LOG_NOTE_COLOR "\033[1;36m"

#define COMPILER_LOG_INFO_COLOR "\033[1;37m"

#define COMPILER_LOG_DEBUG_COLOR "\033[1;32m"

#define COMPILER_LOG_END_COLOR "\033[0m"

#endif

// End of Config

#ifndef COMPILER_EXTVER
#define COMPILER_EXTVER 202301
#endif

#ifndef COMPILER_WEAK_DEF
#define COMPILER_WEAK_DEF __attribute__((weak))
#endif

#ifndef COMPILER_DTORS_DEF
#define COMPILER_DTORS_DEF __attribute__((destructor))
#endif

#include <vector>

namespace Compiler
{

struct optimizer_opts_t
{
    int optimizeLevel = 0;
};

struct parser_opts_t
{
    int optmode = 0;
    int langver = 0;
};

struct preproc_opts_t
{
    std::vector<const char *> defs;
};

struct drv_opts_t : parser_opts_t, optimizer_opts_t, preproc_opts_t
{
    int loglevel = COMPILER_DEFAULT_LOG_LEVEL;
    bool log_colorful = true;
};

struct cmd_args_t : drv_opts_t
{
    bool helpwant = false;
    bool versionwant = false;
    enum trans_t
    {
        NONE = 0,
        SIMPLE,
        DEBUG,
        _MAX
    } trans_id = SIMPLE;
    const char *infile = nullptr;
    const char *outfile = nullptr;
    const char *logfile = nullptr;
    const char *pefile = nullptr;
    bool intpr = false;
    bool supress_ace = false;

    using drv_opts_t::log_colorful;
};

class Prescanner;
class Preprocessor;
class Scanner;
class Parser;
class Emitter;
class Optimizer;
class BoolChain;
class DriverImpl;
class Driver;
class PreprocDriver;

} // namespace Compiler

#endif