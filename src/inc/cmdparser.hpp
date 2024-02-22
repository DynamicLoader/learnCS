#ifndef __CMDPARSER_HPP__
#define __CMDPARSER_HPP__

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

#include <getopt.h>

#include "common.h"

#define COMPILER_CMDARGS_HELPER_GSFUNC(name, var)                                                                      \
    auto name()                                                                                                        \
    {                                                                                                                  \
        return var;                                                                                                    \
    }

namespace Compiler
{

class CmdArgsParser
{
  private:
    int _argc = 0;
    const char **_argv = nullptr;
    cmd_args_t _args;

    const char *_optstr = "w:o:ehryp:D:O:";
    struct option _longopts[20] = {{"out", required_argument, NULL, 'o'},
                                   {"log", required_argument, NULL, 256 + 'l'},
                                   {"trans", required_argument, NULL, 256 + 't'},
                                   {"help", no_argument, NULL, 'h'},
                                   {"nocolor", no_argument, NULL, 256 + 'c'},
                                   {"version", no_argument, NULL, 256 + 'v'},
                                   {0, 0, 0, 0}};
    std::istream *_in = &std::cin;
    std::ostream *_out = &std::cout;
    std::ostream *_log = &std::cerr;

  public:
    CmdArgsParser(int argc, const char *argv[])
    {
        if (argc < 1 || !argv)
            throw std::invalid_argument("Unexpected!");

        _argc = argc;
        _argv = argv;
    }

    int getArgs()
    {
        if (_argc == 1)
            return -1;
        int opt;
        while ((opt = getopt_long(_argc, (char *const *)_argv, _optstr, _longopts, nullptr)) != -1)
        {
            switch (opt)
            {
            case 'o':
                _args.outfile = optarg;
                break;
            case 'e':
                _args.langver = COMPILER_EXTVER;
                break;
            case 'h':
                _args.helpwant = true;
                break;
            case 'w':
                _args.loglevel = std::stoi(std::string(optarg));
                if (_args.loglevel > 5 || _args.loglevel < 0)
                    throw std::invalid_argument("Invalid log level");
                break;
            case 'r':
                _args.intpr = true;
                _args.langver = COMPILER_EXTVER;
                _args.trans_id = cmd_args_t::NONE;
                break;
            case 'y':
                _args.supress_ace = true;
                break;
            case 'p':
                _args.pefile = optarg;
                _args.langver = COMPILER_EXTVER;
                _args.trans_id = cmd_args_t::NONE;
                break;
            case 'D':
                _args.defs.push_back(optarg);
                break;
            case 'O':
                _args.optimizeLevel = std::stoi(std::string(optarg));
                break;

            // long options
            case 256 + 'l':
                _args.logfile = optarg;
                _args.log_colorful = false; // disable colorful output when logfile is set
                break;
            case 256 + 't':
                _args.trans_id = (cmd_args_t::trans_t)std::stoi(std::string(optarg));
                if (_args.trans_id >= cmd_args_t::trans_t::_MAX)
                    throw std::invalid_argument("Invalid translator typeid");
                break;
            case 256 + 'c':
                _args.log_colorful = false;
                break;
            case 256 + 'v':
                _args.versionwant = true;
                break;
            default:
                throw std::invalid_argument("Unexpected or invalid argument provided! Stopped.");
                break;
            }
        }

        if (optind < _argc)
        {
            while (optind < _argc)
            {
                if (!_args.infile)
                {
                    _args.infile = _argv[optind++];
                }
                else
                {
                    std::cerr << std::string("Unrecognized param:") + _argv[optind++];
                }
            }
        }
        return 0;
    }

    int procArgs()
    {
        if (_args.outfile)
        {
            auto fout = new std::ofstream(_args.outfile);
            if (fout->good())
            {
                _out = fout;
            }
            else
            {
                delete fout;
                throw std::invalid_argument("Failed to open " + std::string(_args.outfile) + " as output");
            }
        }

        if (_args.logfile)
        {
            auto fout = new std::ofstream(_args.logfile);
            if (fout->good())
            {
                _log = fout;
            }
            else
            {
                delete fout;
                throw std::invalid_argument("Failed to open " + std::string(_args.logfile) + " as log output");
            }
        }

        return 0;
    }

    COMPILER_CMDARGS_HELPER_GSFUNC(inStream, _in)
    COMPILER_CMDARGS_HELPER_GSFUNC(inFile, _args.infile)
    COMPILER_CMDARGS_HELPER_GSFUNC(outStream, _out)
    COMPILER_CMDARGS_HELPER_GSFUNC(logStream, _log)
    COMPILER_CMDARGS_HELPER_GSFUNC(transId, _args.trans_id)
    COMPILER_CMDARGS_HELPER_GSFUNC(helpWant, _args.helpwant)
    COMPILER_CMDARGS_HELPER_GSFUNC(versionWant, _args.versionwant)
    COMPILER_CMDARGS_HELPER_GSFUNC(intpr, _args.intpr)
    COMPILER_CMDARGS_HELPER_GSFUNC(supressACE, _args.supress_ace)
    COMPILER_CMDARGS_HELPER_GSFUNC(logColorful, _args.log_colorful)
    COMPILER_CMDARGS_HELPER_GSFUNC(pefile, _args.pefile)
    COMPILER_CMDARGS_HELPER_GSFUNC(defs, _args.defs)

    COMPILER_CMDARGS_HELPER_GSFUNC(driverOpts, (drv_opts_t)_args)
};

} // namespace Compiler

#endif