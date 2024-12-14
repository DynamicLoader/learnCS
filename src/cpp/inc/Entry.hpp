#ifndef __ENTRY_HPP__
#define __ENTRY_HPP__

#include <stdexcept>
#include <string>
#include <vector>
#include <map>

typedef std::vector<std::string> property_t;
typedef std::string attribute_t;

/**
 * @brief 条目的类型。
 *
 */
enum class EntryType
{
    article = 1,
    inproceedings,
    proceedings,
    book,
    incollection,
    phdthesis,
    mastersthesis,
    www,
    person,
    data,
    __entry_type_max
};

/**
 * @brief 条目对象。作为大数据库的基本对象。
 *
 * @attention 使用 set_someentry 和 get_someentry 来设置和获取属性。这里不符合常规的命名是为了避免大小写转换引入的开销。
 */
class Entry
{
  public:
    // Create a global empty property
    const static property_t emptyProp;
    const static attribute_t emptyAttr;
    std::map<std::string, attribute_t> attributes;
    std::map<std::string, property_t> properties;

    template <class Archive> void serialize(Archive &archive)
    {
        archive(type, attributes, properties);
    }

    void pushProps(const std::string &name, const std::string &p)
    {
        if (properties.find(name) == properties.end())
            properties[name] = property_t(1, p);
        else
            properties[name].push_back(p);
    }

    const property_t &getProps(const std::string &name) const
    {
        if (properties.find(name) != properties.end())
            return properties.at(name);
        else
            return emptyProp;
    }

    void setAttr(const std::string &name, const attribute_t &p)
    {
        attributes[name] = p;
    }

    const attribute_t &getAttr(const std::string &name) const
    {
        if (attributes.find(name) != attributes.end())
            return attributes.at(name);
        else
            return emptyAttr;
    }

  public:
    EntryType type;

    void setEntryType(int t)
    {
        if (t > 0 and static_cast<EntryType>(t) < EntryType::__entry_type_max)
            type = EntryType(t);
        else
            throw std::runtime_error("Invalid entry type");
    }
};

#endif