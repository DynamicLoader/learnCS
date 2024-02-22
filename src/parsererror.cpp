
#include "driver.hpp"

namespace Compiler
{

void Parser::error(const location &loc, const std::string &msg)
{
    drv.dbg.error << loc << ": " << msg << std::endl;
    throw msg;
}

} // namespace Compiler
