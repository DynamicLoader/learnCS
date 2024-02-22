
#include <iostream>
#include <fstream>
#include <string>
#include "cmdparser.hpp"
#include "driver.hpp"
#include "interpreter.hpp"
#include "utils.hpp"

#ifdef __WIN32
#include <windows.h>

__attribute__((constructor())) void __before_main()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
}
#endif

int main(int argc, char const *argv[], char *env[])
{
    using namespace Compiler;
    using namespace std;

    CmdArgsParser cap(argc, argv);
    try
    {
        if (cap.getArgs() != -1)
        {
            cap.procArgs();
        }
    }
    catch (std::invalid_argument e)
    {
        cerr << "Error while parsing command line arguments: " << e.what() << endl;
        return -1;
    }

    if (cap.versionWant())
    {
        auto ver = Utils::Resource::getGlobalResource("version_mono");
        if (ver.length() < 1)
        {
            std::cout << "No version info was builtin!" << std::endl;
            return 0;
        }
        std::cout << std::string(ver.begin(), ver.length()) << std::endl;
        return 0;
    }

    if (cap.helpWant())
    {
        auto help = Utils::Resource::getGlobalResource("help");
        if (help.length() < 1)
        {
            std::cout << "No help was builtin!" << std::endl;
            return 0;
        }
#if defined(ENABLE_INTPR) && ENABLE_INTPR
        auto intprhelp = Utils::Resource::getGlobalResource("intprhelp");
        std::cout << Utils::str_format(std::string(help.begin(), help.length()), argv[0],
                                       std::string(intprhelp.begin(), intprhelp.length()).c_str())
                  << std::endl;
#else
        std::cout << Utils::str_format(std::string(help.begin(), help.length()), argv[0], "") << std::endl;
#endif
        return 0;
    }
    
#if defined(ENABLE_INTPR) & ENABLE_INTPR
    Utils::Logger logger(std::cerr);
    logger.setColorful(cap.logColorful());
    bool acrun = cap.supressACE();

    if (cap.intpr() && !acrun)
    {
        auto wtxt = Utils::Resource::getGlobalResource("ace");
        auto wstr = std::string((char *)wtxt.data(), wtxt.length());
        logger.warning << wstr;
        std::string opt;
        getline(cin, opt); // Must read from console
        if (opt == "YES")
            acrun = true;
    }

#endif

    Driver driver;
    try
    {
        driver.options(cap.driverOpts());
        if (cap.inFile() != nullptr)
        {
            driver.parse(std::string(cap.inFile()), *cap.logStream());
        }
        else
        {
            cout << "Reading input from STDIN..." << endl;
            driver.parse(std::cin, *cap.logStream());
        }

        if (cap.transId() == cmd_args_t::trans_t::DEBUG)
        {
            driver.output(*cap.outStream());
        }
        else if (cap.transId() == cmd_args_t::trans_t::SIMPLE)
        {
            if (cap.driverOpts().langver > 0)
                throw std::string("Simple Translator does NOT support extension!");
            driver.output<SimpleTranslator>(*cap.outStream());
        }
    }
    catch (std::string e)
    {
        *cap.logStream() << "Exception: " << e << endl;
        return -2;
    }
    catch (std::runtime_error e)
    {
        *cap.logStream() << "Error: " << e.what() << endl;
        return -3;
    }

#if defined(ENABLE_INTPR) & ENABLE_INTPR
    if (cap.intpr())
    {
        Interpreter intpr;

        intpr.proc(driver.getStmts(), driver.getCompiledInfo());
        logger.note << "=== The JIT code have been generated. ===" << std::endl;

        if (acrun)
        {
            intpr.run(driver.entry());
            logger.note << "=== Run completed ===" << std::endl;
        }
        else
        {
            logger.note << "=== Run canceled ===" << std::endl;
        }
    }

    if (cap.pefile())
    { // output as executable
        std::ofstream ope(cap.pefile(), std::ios::binary);
        if (!ope)
        {
            logger.error << "Cannot open " << cap.pefile() << " as output!" << endl;
            return -2;
        }
        auto intprres = Utils::Resource::getGlobalResource("pestdintpr");
        auto fl = intprres.getFileList();
        if (!intprres.length() || fl.size() != 1)
        {
            logger.error << "The binary was corrupted." << endl;
            return -3;
        }
        auto rawPe = intprres.readFileRaw(fl.begin()->first);
        ope.write((const char *)rawPe.data(), rawPe.size());
        ope << "__STDINTPR__"; // magic
        Interpreter::embed(ope, driver.getStmts(), driver.getCompiledInfo());
    }
#endif

    *cap.logStream() << "Finished." << endl;

    return 0;
}
