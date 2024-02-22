#ifndef PREPROCDRIVER_HPP
#define PREPROCDRIVER_HPP

#include "prescanner.hpp"
#include "preproc.hpp"
#include "location.hh"
#include "defineTable.hpp"
#include "utils.hpp"

#include <cstdint>
#include <sstream>
#include <iostream>
#include <fstream>

#include <stack>

namespace Compiler
{

class CodeBlock
{
};

class DriverImpl;

class PreprocDriver
{
  public:
    DriverImpl &drv;

    Preprocessor pproc{Preprocessor(*this)};
    Prescanner pscan{Prescanner(*this)};

    DefineTable &table;
    location loc;

    std::stringstream *result;

    bool ignore_trigger = false;

    PreprocDriver(DriverImpl &d, DefineTable &t) : drv(d), table(t)
    {
    }

    void process(std::istream &in, std::stringstream *out)
    {
        if (&in == &std::cin)
        {
            loc.initialize(new std::string("STDIN"));
        }

        result = out;

        pscan.switch_streams(&in);
        pproc.parse();
    }

    void process(std::string infile, std::stringstream *out)
    {
        std::ifstream in(infile, std::ios::in);

        if (!in)
        {
            auto res = Utils::Resource::getGlobalResource("libtarget");
            auto fl = res.getFileList();
            auto ff = fl.find(infile);
            if (ff == fl.end())
                throw std::string("Cannot open file: ") + infile;
            std::stringstream ini(res.readFileStr(infile));
            loc.initialize(new std::string(infile));
            process(ini, out);
            return;
        }

        loc.initialize(new std::string(infile));
        process(in, out);
    }
};

} // namespace Compiler

#endif