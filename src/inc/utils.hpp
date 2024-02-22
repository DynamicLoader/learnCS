#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <cstdint>
#include <string>
#include <map>
#include <stdexcept>
#include <memory>
#include <iostream>

#include <bzlib.h>
#include <vector>

#include "common.h"
#include "symbols.hpp"

#define RESDEF(name) extern const char _binary_##name##_start[], _binary_##name##_end[]
#define RES(name) _binary_##name##_start, _binary_##name##_end

// Resource defination
RESDEF(target_tar_bz2);

RESDEF(help_txt);
RESDEF(ace_warning_txt);
RESDEF(intpr_help_txt);
RESDEF(version_mono_txt);
RESDEF(version_color_txt);

#if defined(ENABLE_INTPR) && ENABLE_INTPR
RESDEF(stdintpr_tar_bz2);
#endif

namespace Compiler
{
namespace Utils
{
class Resource
{
  public:
    Resource(const char *begin, const char *end, bool bzipped = false) : _begin(begin), _end(end), _bzipped(bzipped)
    {
        if (bzipped)
        {
            this->_unzipped = std::make_shared<char *>(new char[COMPILER_BZLIB_BUFSIZE]);
            this->_unzipped_size = COMPILER_BZLIB_BUFSIZE;
            auto ret = BZ2_bzBuffToBuffDecompress(*(this->_unzipped), &this->_unzipped_size, (char *)this->_begin,
                                                  this->_end - this->_begin, 0, 0);
            if (ret != BZ_OK)
            {
                throw std::runtime_error("Unable to decompress target lib!");
            }
            if (!this->_parseTar())
                throw std::runtime_error("Unable to parse target lib! The executable may be corrupted.");
        }
    }
    ~Resource() = default;

    const char *begin()
    {
        return this->_bzipped ? *(this->_unzipped) : this->_begin;
    }
    const char *end()
    {
        return this->_bzipped ? (*(this->_unzipped) + this->_unzipped_size) : this->_end;
    }
    size_t length()
    {
        return this->_bzipped ? this->_unzipped_size : (this->_end - this->_begin);
    }
    const uint8_t *data()
    {
        return this->_bzipped ? (const uint8_t *)*(this->_unzipped) : (const uint8_t *)this->_begin;
    }

    auto getFileList()
    {
        if (!this->_bzipped)
            throw std::invalid_argument("No file list in non-tar resource");
        return this->_tarfiles;
    }

    std::string readFileStr(const std::string &name)
    {
        if (!this->_bzipped)
            throw std::invalid_argument("Can not read file from non-tar resource");
        auto f = this->_tarfiles.find(name);
        if (f == this->_tarfiles.end())
        {
            return "";
        }
        return std::string(*(this->_unzipped) + f->second.first, f->second.second);
    }

    std::vector<uint8_t> readFileRaw(const std::string &name)
    {
        if (!this->_bzipped)
            throw std::invalid_argument("Can not read file from non-tar resource");
        auto f = this->_tarfiles.find(name);
        if (f == this->_tarfiles.end())
        {
            return {};
        }
        std::vector<uint8_t> ret;
        for (auto i = 0; i < f->second.second; ++i)
        {
            ret.push_back(*(*(this->_unzipped) + f->second.first + i));
        }
        return ret;
    }

    static const auto getGlobalResource(const std::string &name)
    {
        const static std::map<std::string, Utils::Resource> resmap = {
            {"libtarget", Utils::Resource(RES(target_tar_bz2), true)},
#if defined(ENABLE_INTPR) && ENABLE_INTPR
            {"pestdintpr", Utils::Resource(RES(stdintpr_tar_bz2), true)},
            {"ace", Utils::Resource(RES(ace_warning_txt))},
            {"intprhelp", Utils::Resource(RES(intpr_help_txt))},
#endif
            {"help", Utils::Resource(RES(help_txt))},
            {"version_mono", Utils::Resource(RES(version_mono_txt))},
            {"version_color", Utils::Resource(RES(version_color_txt))},
        };
        auto ret = resmap.find(name);
        return (ret == resmap.end() ? Resource(nullptr, nullptr) : ret->second);
    }

  private:
    bool _parseTar()
    {
        if (this->length() < 1)
            return false;

        const int block_size = 512;
        size_t pos = 0;
        auto *header = (tar_posix_hdr_t *)this->data();

        if ((this->length() & 0x1FF) != 0 || this->length() < block_size)
            return false;

        while (1)
        {
            header = (tar_posix_hdr_t *)(this->data() + pos);
            if (std::string(header->magic, 5) != "ustar")
                break;
            pos += block_size;
            size_t file_size = std::stoull(std::string(header->size), 0, 8); // Octal!
            size_t file_block_count = (file_size + block_size - 1) / block_size;

            switch (header->typeflag)
            {
            case '0': // intentionally dropping through
            case '\0':
                // normal file
                _tarfiles.insert(std::make_pair(std::string(header->name), std::make_pair(pos, file_size)));
                break;
            case '1':
                // hard link
                break;
            case '2':
                // symbolic link
                break;
            case '3':
                // device file/special file
                break;
            case '4':
                // block device
                break;
            case '5':
                // directory
                break;
            case '6':
                // named pipe
                break;
            default:
                break;
            }

            pos += file_block_count * block_size;
            if (pos >= this->length())
                break;
        }
        return true;
    }

    struct tar_posix_hdr_t
    {                       /* byte offset */
        char name[100];     /*   0 */
        char mode[8];       /* 100 */
        char uid[8];        /* 108 */
        char gid[8];        /* 116 */
        char size[12];      /* 124 */
        char mtime[12];     /* 136 */
        char chksum[8];     /* 148 */
        char typeflag;      /* 156 */
        char linkname[100]; /* 157 */
        char magic[6];      /* 257 */
        char version[2];    /* 263 */
        char uname[32];     /* 265 */
        char gname[32];     /* 297 */
        char devmajor[8];   /* 329 */
        char devminor[8];   /* 337 */
        char prefix[155];   /* 345 */
                            /* 500 */
    };

    const char *_begin = nullptr;
    const char *_end = nullptr;
    bool _bzipped = false;
    std::shared_ptr<char *> _unzipped;
    unsigned int _unzipped_size = 0;

    std::map<std::string, std::pair<size_t, size_t>> _tarfiles; // name,start,len
};

class LoggerImpl
{

  private:
    int _level;
    std::string _levelName;
    std::ostream *_os;
    int _current = 0;
    bool _colorful = true;

  public:
    LoggerImpl(std::ostream &os = std::cerr, int level = 0, const std::string &levelName = "")
        : _os(&os), _level(level), _levelName(levelName)
    {
    }

    // compatible with std::ostream
    LoggerImpl &operator<<(std::ostream &(*pf)(std::ostream &))
    {
        if (_current >= _level)
            *_os << pf;
        return *this;
    }

    template <typename T> LoggerImpl &operator<<(T v)
    {
        if (_current >= _level)
        {
            if (_colorful)
                *_os << _levelName;
            *_os << v;
            if (_colorful)
                *_os << COMPILER_LOG_END_COLOR;
        }
        return *this;
    }

  protected:
    void setOutputLevel(int level)
    {
        _current = level;
    }

    void setColorful(bool v)
    {
        _colorful = v;
    }

    void setOutput(std::ostream *os)
    {
        if (!os)
            return;
        _os = os;
    }

    friend class Logger;
};

class Logger : public LoggerImpl
{
  private:
    std::ostream &_os;

  public:
    LoggerImpl error;
    LoggerImpl warning;
    LoggerImpl info;
    LoggerImpl note;

    Logger(std::ostream &os = std::cerr)
        : LoggerImpl(os, 5, COMPILER_LOG_DEBUG_COLOR), _os(os), error(os, 1, COMPILER_LOG_ERROR_COLOR),
          warning(os, 2, COMPILER_LOG_WARNING_COLOR), note(os, 3, COMPILER_LOG_NOTE_COLOR),
          info(os, 4, COMPILER_LOG_INFO_COLOR)
    {
        setOutputLevel(COMPILER_DEFAULT_LOG_LEVEL);
        // error << "error";
        // warning << "warning";
        // info << "info";
        // note << "note";
        // *this << "debug" << std::endl;
    }

    void setOutputLevel(int level)
    {
        LoggerImpl::setOutputLevel(level);
        error.setOutputLevel(level);
        warning.setOutputLevel(level);
        note.setOutputLevel(level);
        info.setOutputLevel(level);
    }

    void setColorful(bool v)
    {
        LoggerImpl::setColorful(v);
        error.setColorful(v);
        warning.setColorful(v);
        note.setColorful(v);
        info.setColorful(v);
    }

    void setOutput(std::ostream &os)
    {
        auto v = &os;
        LoggerImpl::setOutput(v);
        error.setOutput(v);
        warning.setOutput(v);
        note.setOutput(v);
        info.setOutput(v);
    }
};

/**
 * assert that tid is one of types
 */

template <typename T> bool assert_types(T tid, T t)
{
    return (tid == t);
}

template <typename T, typename... Ts> bool assert_types(T tid, T t, Ts... types)
{
    return (tid == t ? true : assert_types(tid, types...));
}

template <typename T> bool assert_pair(T i, T v)
{
    return (i == v);
}

template <typename T, typename... Ts> bool assert_pair(T i, T v, Ts... args)
{
    return (i == v ? assert_pair(args...) : false);
}

template <typename... Args> static std::string str_format(const std::string &format, Args... args)
{
    auto size_buf = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
    std::unique_ptr<char[]> buf(new char[size_buf]);

    if (!buf)
        return std::string("");

    std::snprintf(buf.get(), size_buf, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size_buf - 1);
}

} // namespace Utils

} // namespace Compiler

#endif