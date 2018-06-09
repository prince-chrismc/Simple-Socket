
#include "CmdLineParser.h"

CommandLineParser::CommandLineParser(int argc, char** argv)
{
    _parse(argc, argv);
}

void CommandLineParser::_parse(int argc, char** argv)
{
    m_sCommand = argv[0];          // Save filename
    for (int i = 1; i < argc; i++) // Then save each arg
    {
        m_vecArgs.push_back(argv[i]);
    }
}

bool CommandLineParser::DoesArgExists(const std::string& name)
{
    if (name.empty()) return false;

    for (auto& arg : m_vecArgs)
    {
        if (arg.find(name) != std::string::npos) // case insensitive search
        {
            return true;
        }
    }

    return false;
}

std::string CommandLineParser::GetPairValue(std::string name)
{
    if (name.empty()) return "";

    name += "=";
    std::string retval;
    for (auto& pair : m_vecArgs)
    {
        const size_t switch_index = pair.find(name);
        if (switch_index != std::string::npos)
        {
            retval = pair.substr(switch_index + name.size() + 1);   // Return string after =.
            break;
        }
    }

    return retval;
}
