/*

MIT License

Copyright (c) 2018 Chris McArthur, prince.chrismc(at)gmail(dot)com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

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
