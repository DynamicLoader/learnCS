#ifndef __INTKEY_HPP__
#define __INTKEY_HPP__

#include <algorithm>
#include <string>
#include <vector>

/**
 * @brief A class that wraps md5 result. Stores with a 128-bit integer.
 *
 */

#ifdef __SIZEOF_INT128__
class intKey_t
{
  public:
    __int128 __key;

    intKey_t()
    {
        __key = 0;
    }

    intKey_t(const std::string &str)
    {
        __key = 0;
        auto len = std::min(str.size(), (size_t)16);

        auto ptr = str.c_str();
        for (auto i = 0; i < len; i++)
            ((uint8_t *)&__key)[len - i - 1] = (uint8_t)ptr[i];
    }

    static intKey_t fromHexString(const std::string &str)
    {
        intKey_t key;
        key.__key = 0;
        auto len = str.size();
        for (auto i = 0; i < len; i++)
        {
            char c = str[i];
            if (c >= '0' && c <= '9')
            {
                key.__key = (key.__key << 4) + (c - '0');
            }
            else if (c >= 'a' && c <= 'f')
            {
                key.__key = (key.__key << 4) + (c - 'a' + 10);
            }
            else if (c >= 'A' && c <= 'F')
            {
                key.__key = (key.__key << 4) + (c - 'A' + 10);
            }
        }
        return key;
    }

    const bool operator<(const intKey_t &t) const
    {
        return __key < t.__key;
    }

    const bool operator==(const intKey_t &t) const
    {
        return __key == t.__key;
    }

    const bool operator!=(const intKey_t &t) const
    {
        return __key != t.__key;
    }

    template <class Archive> void serialize(Archive &ar)
    {
        ar(__key);
    }

    explicit operator std::string() const
    {
        if (__key == 0)
            return "0";

        std::string s;

        constexpr char hexLut[] = "0123456789abcdef";

        auto num = __key;
        while (num > 0)
        {
            s.push_back(hexLut[num & 0xf]);
            num >>= 4;
        }
        std::reverse(s.begin(), s.end());

        return std::move(s);
    };

    std::vector<unsigned char> to_bytes()
    {
        std::vector<unsigned char> s;
        for (int i = 0; i < 16; i++)
        {
            s.push_back(((__key >> (16 - i - 1) * 8) & 0xff));
        }
        return s;
    }

    static const std::vector<unsigned char> KeySerializer(const intKey_t &t)
    {
        return std::vector<unsigned char>(
            {static_cast<unsigned char>(t.__key & 0xff), static_cast<unsigned char>((t.__key >> 8) & 0xff)});
    }
};

namespace std
{
template <> struct hash<intKey_t>
{
    std::size_t operator()(const intKey_t &k) const
    {
        return ((k.__key >> 64) & 0xFFFFFFFFFFFFFFFFull) ^ (k.__key & 0xFFFFFFFFFFFFFFFFull);
    }
};
} // namespace std

#else

class intKey_t
{
  public:
    int64_t __key1;
    int64_t __key2;

    intKey_t()
    {
        __key1 = 0;
        __key2 = 0;
    }

    intKey_t(const std::string &str)
    {
        __key1 = 0;
        __key2 = 0;
        auto len = std::min(str.size(), (size_t)16);
        for (auto i = 0; i < len; i++)
        {
            if (i < 8)
                __key1 = (__key1 << 8) + (unsigned char)(str[i]);
            else
                __key2 = (__key2 << 8) + (unsigned char)(str[i]);
        }
    }

    const bool operator<(const intKey_t &t) const
    {
        if (__key1 < t.__key1)
            return true;
        if (__key1 > t.__key1)
            return false;
        return __key2 < t.__key2;
    }

    const bool operator==(const intKey_t &t) const
    {
        return __key1 == t.__key1 && __key2 == t.__key2;
    }

    const bool operator!=(const intKey_t &t) const
    {
        return __key1 != t.__key1 || __key2 != t.__key2;
    }

    template <class Archive> void serialize(Archive &ar)
    {
        ar(__key1, __key2);
    }

    explicit operator std::string() const
    {
        std::string s;
        constexpr char hexLut[] = "0123456789abcdef";
        for (int i = 0; i < 16; i++)
        {
            int64_t key = (i < 8) ? __key1 : __key2;
            int shift = (i < 8) ? (8 - i - 1) * 8 : (16 - i - 1) * 8;
            s.push_back(hexLut[(key >> shift) & 0xf]);
        }
        return std::move(s);
    }

    static const std::vector<unsigned char> KeySerializer(const intKey_t &t)
    {
        return std::vector<unsigned char>((unsigned char *)&t.__key1, (unsigned char *)&t.__key2 + sizeof(int64_t));
    }
};
#endif

#endif