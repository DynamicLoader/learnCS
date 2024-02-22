#ifndef DEFINE_TABLE_HPP
#define DEFINE_TABLE_HPP

#include <map>
#include <stdexcept>
#include <utility>
#include <string>
#include <regex>
#include <iostream>

namespace Compiler
{

enum DefineType_t
{
    simple_def,
    str,
    integer,
    decimal,
};

class Definition
{
    DefineType_t type_check(std::string str)
    {
        if (str.empty())
            return DefineType_t::simple_def;

        std::regex integer_check("^-?[\\d]+");
        std::regex float_check("^-?[\\d]+\\.[\\d]+");

        if (std::regex_match(str, integer_check))
            return DefineType_t::integer;
        else if (std::regex_match(str, float_check))
            return DefineType_t::decimal;
        else
            return DefineType_t::str;
    }

  public:
    DefineType_t type;
    std::string name;
    std::string value;

    int val_int;
    double val_float;

    Definition()
    {
        name = value = "";
        type = DefineType_t::simple_def;
    }

    Definition(std::string n, std::string v) : name(n), value(v)
    {
        type = type_check(v);
        if (type == DefineType_t::integer)
            val_int = std::stoi(v);
        else if (type == DefineType_t::decimal)
            val_float = std::stod(v);
    }

    explicit operator bool() const
    {
        if (type == DefineType_t::simple_def or type == DefineType_t::str)
            return true;
        else if (type == DefineType_t::integer)
            return val_int != 0;
        else if (type == DefineType_t::decimal)
            return val_float != 0.0;
        else
            return false;
    }
};

class DefineTable
{
    std::map<std::string, Definition> table;

  public:
    void add(std::string name, std::string value)
    {
        if (table.find(name) == table.end())
            table.insert(std::make_pair(name, Definition(name, value)));
        else
            throw std::runtime_error("Redefinition of macro \"" + name + "\"");
    }

    void remove(std::string name)
    {
        table.erase(name);
    }

    Definition &operator[](std::string name)
    {
        if (!contains(name))
            throw std::runtime_error("Undefined macro.");

        while (table[name].type == str and this->contains(table[name].value))
        {
            table[name] = table[table[name].value];
            table[name].name = name;
        }

        return table[name];
    }

    bool contains(std::string name)
    {
        return table.find(name) != table.end();
    }
};

} // namespace Compiler

#endif