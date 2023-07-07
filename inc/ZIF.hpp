/**
 * @file ZIF.hpp
 * @author Lu Yi, Lin HongJie
 * @brief ZhengYP Image Format
 * @version 0.1
 * @date 2023-05-16
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __ZIF_HPP__
#define __ZIF_HPP__

#include <cstdint>
#include <fstream>
#include <ctime>
#include <functional>

#include "ErrorNo.h"
#include "Exceptions.hpp"
#include "QUtils.hpp"

#define RESERVED_ZIF_TYPE 0
#define RNAM_ENCODE_ZIF 1
#define TNAM_ENCODE_ZIF 2

#define ZIF_VERSION 0x5A

/**
 * @brief A namespace that contains the implement of ZIF RW
 *
 */
namespace ZIF
{

constexpr uint32_t ZIF_MAGIC = (('Z' - 'A') << 24) + (('I' - 'A') << 16) + (('F' - 'A') << 8) + ZIF_VERSION;

/**
 * @brief Header struct defination of ZIF
 *
 */
struct Header_t
{
    uint8_t DESC[3] = {'Z', 'I', 'F'};
    uint32_t Magic = ZIF_MAGIC;
    uint8_t Type = 0;

    uint16_t _align32_0 = 0;

    /* More attributes can be added here*/
    uint32_t _reserved[8] = {0};

    uint32_t DataStructLen = 0;
};

/**
 * @brief Tail struct defination of ZIF
 * 
 */
struct Tail_t
{
    char CreatedBy[32] = "SuperNAM-Z V1.0, By L.L.Z.L";
    time_t CTime = 0;
    time_t MTime = 0;
    // uint32_t CRC32 = 0;
};

/**
 * @brief ZIF Coder
 * 
 */
class Coder
{
  public:
    using InputImplCB_t =
        std::function<int(std::ifstream &is, uint32_t len)>; // int (*)(std::ifstream &is, uint32_t len);
    using OutputImplCB_t =
        std::function<int(std::ofstream &os, uint32_t &len)>; // int (*)(std::ofstream &os, uint32_t &len);

    /**
     * @brief Construct a new Coder object
     * 
     * @param icb Input callback
     * @param ocb Output callback
     * @param type Internal type
     */
    Coder(InputImplCB_t icb, OutputImplCB_t ocb, uint8_t type) : _icb(icb), _ocb(ocb), _type(type)
    {
        if (!icb || !ocb)
        {
            throw(Exception::BadArgument());
        }
    }

    /**
     * @brief Read file from given path
     * 
     * @param path Path of file to read
     * @return int 0 if success
     * @see ERRORNO
     */
    int readFile(const std::string &path)
    {
        std::ifstream fin(path, std::ios_base::binary | std::ios_base::in); // Don't strip URL here!
        if (!fin.is_open())
        {
            return ERRORNO(ERR_OPEN_FILE); // Cannot open file
        }
        auto ret = _readFile(fin);
        fin.close();
        return ret;
    }

    /**
     * @brief Write file with given path
     * 
     * @param path Path of file to write
     * @return int 0 if success
     */
    int writeFile(const std::string &path)
    {
        std::ofstream fout(path, std::ios_base::out | std::ios_base::binary); // Don't strip URL here!
        if (!fout.is_open())
        {
            return ERRORNO(ERR_OPEN_FILE); // Cannot open file in output mode
        }
        auto ret = _writeFile(fout);
        fout.close();
        return ret;
    }

  private:
    uint8_t _type = RESERVED_ZIF_TYPE;
    InputImplCB_t _icb = nullptr;
    OutputImplCB_t _ocb = nullptr;
    Tail_t _info;

    int _readFile(std::ifstream &fin)
    {

        Header_t hdr;
        Tail_t tal;

        fin.read((char *)&hdr, sizeof(Header_t));
        if (!fin.good())
        {
            return ERRORNO(ERR_EARLY_EOF); // Unexpected end of file
        }
        if (hdr.DESC[0] != 'Z' || hdr.DESC[1] != 'I' || hdr.DESC[2] != 'F' || hdr.Magic != ZIF_MAGIC)
        {
            return ERRORNO_EX(ZIF, ERR_UNSUPPORTED_VERSION); // Bad header or unsupported version
        }

        if (hdr.Type == this->_type)
        {
            int ret = this->_icb(fin, hdr.DataStructLen);
            if (ret != 0)
            {
                return ret;
            }
        }
        else
        {
            return ERRORNO_EX(ZIF, ERR_INVALID_FORMAT); // Unsupported format
        }

        fin.seekg(sizeof(Header_t) + hdr.DataStructLen);
        if (!fin.good())
        {
            return ERRORNO(ERR_SEEK_FILE);
        }

        fin.read((char *)&tal, sizeof(Tail_t));
        if (!fin.good())
        {
            return ERRORNO(ERR_READ_FILE);
        }

        _info = tal;
        return 0;
    }

    int _writeFile(std::ofstream &fout)
    {

        Header_t hdr;
        Tail_t tal;

        fout.seekp(sizeof(Header_t), std::ios_base::beg);
        if (!fout.good())
        {
            return ERRORNO(ERR_SEEK_FILE);
        }

        int ret = this->_ocb(fout, hdr.DataStructLen);
        if (ret)
        {
            return ret;
        }

        hdr.Type = this->_type;
        fout.seekp(0, std::ios_base::beg);
        if (!fout.good())
        {
            return ERRORNO(ERR_SEEK_FILE);
        }
        fout.write((const char *)&hdr, sizeof(Header_t));

        tal.CTime = time(nullptr);
        tal.MTime = tal.CTime;
        fout.seekp(sizeof(Header_t) + hdr.DataStructLen, std::ios_base::beg);
        if (!fout.good())
        {
            return ERRORNO(ERR_SEEK_FILE);
        }
        fout.write((const char *)&tal, sizeof(Tail_t));
        fout.flush();
        return 0;
    }
};

} // namespace ZIF

/**
 * @brief Pre-defined lambda function to be used when output is not supported
 * 
 */
#define ZIF_CODER_NO_OUTPUT                                                                                            \
    [](std::ofstream &os, uint32_t &len) {                                                                             \
        (void)os;                                                                                                      \
        (void)len;                                                                                                     \
        return -1;                                                                                                     \
    }

/**
 * @brief Pre-defined lambda function to be used when input is not supported
 * 
 */
#define ZIF_CODER_NO_INPUT                                                                                             \
    [](std::ifstream &is, uint32_t len) {                                                                              \
        (void)is;                                                                                                      \
        (void)len;                                                                                                     \
        return -1;                                                                                                     \
    }

#endif